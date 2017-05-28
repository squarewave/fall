#include <assert.h>

#include "stb/stb_image.h"

#include "platform.h"
#include "assets.h"
#include "asset_manager.h"
#include "Judy.h"

const char* MAIN_ARCHIVE_PATH = "assets/test_images.pak";

AssetManager g_asset_manager = {};

void assets_init() {
  stbi_set_flip_vertically_on_load(true);
  g_asset_manager.main_archive_async_handle = g_platform.begin_read_entire_file(MAIN_ARCHIVE_PATH);
}

inline ArchiveEntryHeader_texture_atlas* get_atlas_for_type(AssetType type) {
  void* pget;
  JLG(pget, g_asset_manager.asset_types_to_atlases, type);
  if (!pget) {
    return NULL;
  }
  return *((ArchiveEntryHeader_texture_atlas**)pget);
}

inline void set_atlas_for_type(AssetType type, ArchiveEntryHeader_texture_atlas* atlas) {
  void** pinsert = (void**)JudyLIns(&g_asset_manager.asset_types_to_atlases, type, PJE0);
  assert(pinsert); // TODO(doug): better OOM handling
  *pinsert = (void*)atlas;
}

inline void assets_complete_init() {
  PlatformEntireFile archive = g_platform.entire_file_result(g_asset_manager.main_archive_async_handle);
  GameArchiveHeader* header = (GameArchiveHeader*)archive.contents;
  ArchiveEntryType* next_type = (ArchiveEntryType*)(header + 1);
  for (int i = 0; i < header->archive_entry_count; ++i) {
    switch (*next_type) {
      case ArchiveEntryType_texture_atlas: {
        ArchiveEntryHeader_texture_atlas* atlas = (ArchiveEntryHeader_texture_atlas*)next_type;
        for (int j = 0; j < atlas->packed_texture_count; ++j) {
          PackedTexture tex = atlas->packed_textures[j];
          set_atlas_for_type(tex.asset_type, atlas);
        }

        int x, y, channels;
        char* png = (char*)(atlas->packed_textures + atlas->packed_texture_count);
        char* bitmap = (char*)stbi_load_from_memory((unsigned char*)png, atlas->png_size, &x, &y, &channels, 4);
        atlas->texture_handle = g_platform.register_texture(bitmap, x, y, channels).handle;
        stbi_image_free(bitmap);
      } break;
      case ArchiveEntryType_none: break;
    }
  }

  g_asset_manager.main_archive = header;
}

inline void assets_maybe_complete_init() {
  if (!g_asset_manager.main_archive && g_platform.file_io_complete(g_asset_manager.main_archive_async_handle)) {
    assets_complete_init();
  }
}

TextureAsset assets_get_texture(AssetType type, AssetAttributes attrs) {
  TextureAsset result;

  assets_maybe_complete_init();
  auto atlas = get_atlas_for_type(type);
  if (!atlas) {
    return {};
  }

  for (int j = 0; j < atlas->packed_texture_count; ++j) {
    PackedTexture tex = atlas->packed_textures[j];
    if (tex.asset_type == type) {
      result.handle = atlas->texture_handle;
      result.top = 1.0f - (f32)tex.top / (f32)TEXTURE_ATLAS_DIAMETER;
      result.bottom = 1.0f - (f32)tex.bottom / (f32)TEXTURE_ATLAS_DIAMETER;
      result.left = (f32)tex.left / (f32)TEXTURE_ATLAS_DIAMETER;
      result.right = (f32)tex.right / (f32)TEXTURE_ATLAS_DIAMETER;
      break;
    }
  }

  return result;
}
