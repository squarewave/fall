#define TYPEINFO_FILE "packager_typeinfo.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <Windows.h>
#include <sys/types.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

#include "assets.h"
#include "serialize.h"
#include "serialize.cpp"

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

#define ZERO(x) memset(&(x), 0, sizeof(x));

const int MAX_CURRENT_BITMAPS = 1024;
const int MAX_ASSET_SPECS = 4096;

struct Bitmap {
  i32 width, height;
  i32 anchor_x, anchor_y;
  AssetType asset_type;
  AssetAttributes asset_attributes;
  u32* pixels;
};

struct EntireFile {
  char* contents;
  i32 content_size;
};

struct AssetPackager {
  char* current_archive;
  i32 bitmap_count;
  Bitmap bitmaps[MAX_CURRENT_BITMAPS];
};

AssetPackager g_packager = {0};
char* archive_mem_head = NULL;
char* archive_mem = NULL;
u32* temp_atlas_data = NULL;

EntireFile read_entire_file(char* filepath) {
  EntireFile result = {};
  char *buffer;

  HANDLE file = CreateFile(filepath, GENERIC_READ, 0, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    printf("Failed to open file %s %d\n", filepath, GetLastError());
    exit(1);
  }

  LARGE_INTEGER file_size;
  if (!GetFileSizeEx(file, &file_size)) {
    printf("File has no contents: %s\n", filepath);
    exit(1);
  }

  buffer = (char*)malloc((size_t)file_size.QuadPart);
  DWORD bytes_read;
  if (!ReadFile(file, buffer, file_size.LowPart, &bytes_read, NULL)) {
    printf("Failed to read file %s\n", filepath);
    exit(1);
  }

  result.contents = buffer;
  result.content_size = file_size.LowPart;
  CloseHandle(file);

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
  fclose(f);

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
    rects[i].w = g_packager.bitmaps[i].width + 2;
    rects[i].h = g_packager.bitmaps[i].height + 2;
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
        packed_tex->left = rects[i].x + 1;
        packed_tex->right = packed_tex->left + b.width;
        packed_tex->top = rects[i].y + 1;
        packed_tex->bottom = packed_tex->top + b.height;
        packed_tex->asset_type = b.asset_type;
        packed_tex->attributes = b.asset_attributes;
        packed_tex->anchor_x = b.anchor_x;
        packed_tex->anchor_y = b.anchor_y;

        for (int y = 0; y < b.height; ++y) {
          for (int x = 0; x < b.width; ++x) {
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

#if 1
    static int counter;
    char buffer[64];
    sprintf(buffer, "texture_debug_%d.png", counter++);
    printf("Writing to %s\n", buffer);
    write_entire_file(buffer, archive_mem_head - written, written);
#endif

    atlas->png_size = written;

  } while (!all_packed);

  write_archive_mem(g_packager.current_archive);
  g_packager.bitmap_count = 0;
  reset_archive_mem();
  g_packager.current_archive = NULL;
}

void add_png(AssetType asset_type, AssetAttributes attrs, char* filename, i32 anchor_x = -256, i32 anchor_y = -256) {
  auto file = read_entire_file(filename);
  Bitmap bitmap = {0};
  int channels;
  bitmap.pixels = (u32*)stbi_load_from_memory((unsigned char*)file.contents, file.content_size,
                        &bitmap.width, &bitmap.height,
                        &channels, 4);
  bitmap.asset_type = asset_type;
  bitmap.asset_attributes = attrs;
  if (anchor_x == -256) {
    anchor_x = bitmap.width / 2 + 1;
  }
  if (anchor_y == -256) {
    anchor_y = bitmap.height / 2 + 1;
  }
  bitmap.anchor_x = anchor_x;
  bitmap.anchor_y = anchor_y;
  free(file.contents);

  g_packager.bitmaps[g_packager.bitmap_count++] = bitmap;
}

int main(int argc, char const *argv[]) {
  const size_t allocator_capacity = 1024 * 1024;
  Allocator* a = (Allocator*)malloc(allocator_capacity);
  a->capacity = allocator_capacity;
  archive_mem = (char*)malloc(1024 * 1024 * 1024);
  assert(archive_mem);
  archive_mem_head = archive_mem;
  temp_atlas_data = (u32*)malloc(TEXTURE_ATLAS_SIZE_BYTES);
  assert(temp_atlas_data);
  AssetAttributes attrs = {};

  auto file_result = read_entire_file("asset_specs");

  int spec_count;
  AssetSpec* specs = (AssetSpec*)
    deserialize_struct_array(a, TypeInfo_ID_AssetSpec,
                             file_result.contents,
                             file_result.content_size,
                             &spec_count,
                             MAX_ASSET_SPECS);

  begin_archive((char*)"build/assets/test_images.pak");

  for (i32 i = 0; i < spec_count; i++) {
    auto spec = specs[i];
    if (spec.use_anchor) {
      add_png(spec.asset_type, spec.asset_attributes, spec.filepath, spec.anchor_x, spec.anchor_y);
    } else {
      add_png(spec.asset_type, spec.asset_attributes, spec.filepath);
    }
  }

  end_archive();
  return 0;
}
