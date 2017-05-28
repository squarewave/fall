#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <Windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

#include "assets.h"

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef uint32_t b32;
typedef float f32;
typedef double f64;

const int MAX_CURRENT_BITMAPS = 1024;

struct Bitmap {
  int width, height;
  AssetType asset_type;
  u32* pixels;
};

struct EntireFile {
  char* contents;
  i32 content_size;
};

struct AssetPackager {
  char* current_archive;
  int bitmap_count;
  Bitmap bitmaps[MAX_CURRENT_BITMAPS];
};

AssetPackager g_packager = {0};
char* archive_mem_head = NULL;
char* archive_mem = NULL;
u32* temp_atlas_data = NULL;

EntireFile read_entire_file(char* filepath) {
  EntireFile result = {};

  FILE *f = fopen(filepath, "rb");

  if (f == 0) {
    printf("Failed to open file %s\n", filepath);
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char *)malloc(fsize + 1);
  if (!fread(string, fsize, 1, f)) {
    printf("No results from fread\n");
    exit(1);
  }
  fclose(f);

  string[fsize] = 0;

  result.contents = (char *)string;
  result.content_size = (i32)fsize;

  return result;
}

b32 write_entire_file(char* filepath, char* contents, i32 content_size) {
  FILE *f = fopen(filepath, "wb");

  if (f == 0) {
    printf("Failed to open file %s\n", filepath);
    exit(1);
  }

  i32 written = fwrite(contents, sizeof(char), content_size, f);

  if (written != content_size) {
    printf("Failed to write entire file %s\n", filepath);
    exit(1);
  }

  return true;
}

void begin_archive(char* filename) {
  g_packager.current_archive = filename;
}

int compare_bitmaps(const void* lhs, const void* rhs) {
  Bitmap* lb = (Bitmap*)lhs;
  Bitmap* rb = (Bitmap*)rhs;

  return max(rb->width, rb->height) - max(lb->width, lb->height);
}

#define push_to_archive_mem(type, count) ((type*)_push_to_archive_mem(sizeof(type) * count))
void* _push_to_archive_mem(i32 size) {
  void* result = archive_mem_head;
  memset(archive_mem_head, 0, size);
  archive_mem_head += size;
  return result;
}

void png_write_func(void *context, void *data, int size) {
  int* bytes_written = (int*)context;
  *bytes_written += size;

  void* start = _push_to_archive_mem(size);
  memcpy(start, data, size);
}

void reset_archive_mem() {
  archive_mem_head = archive_mem;
}

void write_archive_mem(char* filepath) {
  write_entire_file(g_packager.current_archive, archive_mem, (i64)archive_mem_head - (i64)archive_mem);
}

void end_archive() {
  GameArchiveHeader* header = push_to_archive_mem(GameArchiveHeader, 1);

  stbrp_context rect_packer = {0};
  stbrp_node* nodes = (stbrp_node*)malloc(sizeof(stbrp_node) * TEXTURE_ATLAS_DIAMETER);
  stbrp_rect* rects = (stbrp_rect*)malloc(sizeof(stbrp_rect) * g_packager.bitmap_count);
  for (int i = 0; i < g_packager.bitmap_count; ++i) {
    rects[i].w = g_packager.bitmaps[i].width;
    rects[i].h = g_packager.bitmaps[i].height;
    rects[i].id = i;
  }

  i32 num_rects = g_packager.bitmap_count;
  b32 all_packed = 0;
  do {
    header->archive_entry_count++;

    ArchiveEntryHeader_texture_atlas* atlas = push_to_archive_mem(ArchiveEntryHeader_texture_atlas, 1);
    memset(temp_atlas_data, 0, TEXTURE_ATLAS_SIZE_BYTES);
    atlas->type = ArchiveEntryType_texture_atlas;
    stbrp_init_target(&rect_packer,
              TEXTURE_ATLAS_DIAMETER, TEXTURE_ATLAS_DIAMETER,
              nodes, TEXTURE_ATLAS_DIAMETER);
    all_packed = stbrp_pack_rects(&rect_packer, rects, num_rects);
    i32 next_num_rects = 0;
    for (int i = 0; i < num_rects; ++i) {
      if (rects[i].was_packed) {
        Bitmap b = g_packager.bitmaps[rects[i].id];
        atlas->packed_texture_count++;
        auto packed_tex = push_to_archive_mem(PackedTexture, 1);
        packed_tex->left = rects[i].x;
        packed_tex->right = rects[i].x + rects[i].w;
        packed_tex->top = rects[i].y;
        packed_tex->bottom = rects[i].y + rects[i].h;
        packed_tex->asset_type = b.asset_type;

        for (int y = 0; y < rects[i].h; ++y) {
          for (int x = 0; x < rects[i].w; ++x) {
            temp_atlas_data[(y + packed_tex->top) * TEXTURE_ATLAS_DIAMETER + x + packed_tex->left] = b.pixels[y * b.width + x];
          }
        }

        stbi_image_free(b.pixels);
      } else {
        rects[next_num_rects++] = rects[i];
      }
    }
    num_rects = next_num_rects;

    int written = 0;
    stbi_write_png_to_func(png_write_func, &written,
                 TEXTURE_ATLAS_DIAMETER, TEXTURE_ATLAS_DIAMETER, TEXTURE_CHANNELS,
                 temp_atlas_data, 0);
    atlas->png_size = written;

  } while (!all_packed);

  write_archive_mem(g_packager.current_archive);
  g_packager.bitmap_count = 0;
  reset_archive_mem();
  g_packager.current_archive = NULL;
}

void add_png(AssetType asset_type, char* filename) {
  auto file = read_entire_file(filename);
  Bitmap bitmap = {0};
  int channels;
  bitmap.pixels = (u32*)stbi_load_from_memory((unsigned char*)file.contents, file.content_size,
                        &bitmap.width, &bitmap.height,
                        &channels, 4);
  bitmap.asset_type = asset_type;
  free(file.contents);
  if (channels != 4) {
    printf("Wrong number of channels in %s: %d\n", filename, channels);
    exit(1);
  }

  g_packager.bitmaps[g_packager.bitmap_count++] = bitmap;
}

int main(int argc, char const *argv[]) {
  archive_mem = (char*)malloc(1024 * 1024 * 1024);
  assert(archive_mem);
  archive_mem_head = archive_mem;
  temp_atlas_data = (u32*)malloc(TEXTURE_ATLAS_SIZE_BYTES);
  assert(temp_atlas_data);

  begin_archive((char*)"assets/test_images.pak");
  add_png(AssetType_dog, (char*)"../assets/img_test.png");
  add_png(AssetType_player, (char*)"../assets/img_test1.png");
  add_png(AssetType_player, (char*)"../assets/img_test2.png");
  add_png(AssetType_player, (char*)"../assets/img_test3.png");
  add_png(AssetType_player, (char*)"../assets/img_test4.png");
  end_archive();
  return 0;
}