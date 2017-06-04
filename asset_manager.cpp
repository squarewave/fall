#include <assert.h>

#include "stb/stb_image.h"

#include "assets.h"
#include "asset_manager.h"
#include "game.h"
#include "memory.h"
#include "platform.h"

char* MAIN_ARCHIVE_PATH = (char*)"assets/test_images.pak";

#define g_asset_manager (g_game_state->asset_manager)

void assets_init() {
  g_asset_manager = transient_alloc(AssetManager);

  stbi_set_flip_vertically_on_load(true);
  g_asset_manager->main_archive_async_handle = g_platform.begin_read_entire_file(MAIN_ARCHIVE_PATH);
  g_asset_manager->main_archive_last_write_time = g_platform.get_last_write_time(MAIN_ARCHIVE_PATH);
}

inline void set_atlas_for_type(AssetType type, ArchiveEntryHeader_texture_atlas* atlas) {
  g_asset_manager->main_atlas = atlas;
  // void** pinsert = (void**)JudyLIns(&g_asset_manager->asset_types_to_atlases, type, PJE0);
  // assert(pinsert); // TODO(doug): better OOM handling
  // *pinsert = (void*)atlas;
}

inline ArchiveEntryHeader_texture_atlas* get_atlas_for_type(AssetType type) {
  return g_asset_manager->main_atlas;
  // void* pget;
  // JLG(pget, g_asset_manager->asset_types_to_atlases, type);
  // if (!pget) {
  //   return NULL;
  // }
  // return *((ArchiveEntryHeader_texture_atlas**)pget);
}

void unload_archive(GameArchiveHeader* header) {
  ArchiveEntryType* next_type = (ArchiveEntryType*)(header + 1);
  for (int i = 0; i < header->archive_entry_count; ++i) {
    switch (*next_type) {
      case ArchiveEntryType_texture_atlas: {
        auto atlas = (ArchiveEntryHeader_texture_atlas*)next_type;
        for (int j = 0; j < atlas->packed_texture_count; ++j) {
          auto tex = atlas->packed_textures[j];
          set_atlas_for_type(tex.asset_type, NULL);
        }

        g_platform.unregister_texture(atlas->texture_handle);
      } break;
      case ArchiveEntryType_none: break;
    }
  }

  g_asset_manager->main_archive = header;
}

void assets_refresh() {
  if (g_platform.file_has_been_touched(MAIN_ARCHIVE_PATH, &g_asset_manager->main_archive_last_write_time)) {
    stbi_set_flip_vertically_on_load(true);
    LOG("Refreshing %s\n", MAIN_ARCHIVE_PATH);
    unload_archive((GameArchiveHeader*)g_platform.entire_file_result(g_asset_manager->main_archive_async_handle).contents);
    g_platform.free_file_memory(g_asset_manager->main_archive_async_handle);
    g_asset_manager->main_archive_async_handle = g_platform.begin_read_entire_file(MAIN_ARCHIVE_PATH);
    g_asset_manager->main_archive = NULL;
  }
}

inline void assets_complete_init() {
  auto archive = g_platform.entire_file_result(g_asset_manager->main_archive_async_handle);

  auto header = (GameArchiveHeader*)archive.contents;
  auto next_type = (ArchiveEntryType*)(header + 1);
  for (int i = 0; i < header->archive_entry_count; ++i) {
    switch (*next_type) {
      case ArchiveEntryType_texture_atlas: {
        auto atlas = (ArchiveEntryHeader_texture_atlas*)next_type;
        for (int j = 0; j < atlas->packed_texture_count; ++j) {
          auto tex = atlas->packed_textures[j];
          set_atlas_for_type(tex.asset_type, atlas);
          assert(get_atlas_for_type(tex.asset_type));
        }

        int x, y, channels;
        auto png = (char*)(atlas->packed_textures + atlas->packed_texture_count);
        auto bitmap = (char*)stbi_load_from_memory((unsigned char*)png, atlas->png_size, &x, &y, &channels, 4);
        atlas->texture_handle = g_platform.register_texture(bitmap, x, y, channels).handle;
        stbi_image_free(bitmap);
      } break;
      case ArchiveEntryType_none: break;
    }
  }

  g_asset_manager->main_archive = header;
}

inline void assets_maybe_complete_init() {
  if (!g_asset_manager->main_archive && g_platform.file_io_complete(g_asset_manager->main_archive_async_handle)) {
    assets_complete_init();
  }
}

TextureAsset assets_get_atlas(AssetType type) {
  TextureAsset result = {};

  assets_maybe_complete_init();
  auto atlas = get_atlas_for_type(type);

  if (!atlas) {
    return result;
  }

  result.handle = atlas->texture_handle;
  result.top = 1.0f;
  result.bottom = 0.0f;
  result.left = 0.0f;
  result.right = 1.0f;
  result.px_width = TEXTURE_ATLAS_DIAMETER;
  result.px_height = TEXTURE_ATLAS_DIAMETER;
  return result;
}

TextureAsset assets_get_texture(AssetType type, AssetAttributes attrs) {
  TextureAsset result = {};

  assets_maybe_complete_init();
  auto atlas = get_atlas_for_type(type);
  if (!atlas) {
    LOG("No atlas: %d", type);
    return result;
  }

  for (int j = 0; j < atlas->packed_texture_count; ++j) {
    auto tex = atlas->packed_textures[j];
    if (tex.asset_type == type &&
        tex.attributes.move_state == attrs.move_state &&
        tex.attributes.asset_class == attrs.asset_class &&
        tex.attributes.color == attrs.color &&
        tex.attributes.direction == attrs.direction) {
      result.handle = atlas->texture_handle;
      result.top = 1.0f - (f32)tex.top / (f32)TEXTURE_ATLAS_DIAMETER;
      result.bottom = 1.0f - (f32)tex.bottom / (f32)TEXTURE_ATLAS_DIAMETER;
      result.left = (f32)tex.left / (f32)TEXTURE_ATLAS_DIAMETER;
      result.right = (f32)tex.right / (f32)TEXTURE_ATLAS_DIAMETER;
      result.px_width = tex.right - tex.left;
      result.px_height = tex.bottom - tex.top;
      result.anchor_x = tex.anchor_x;
      result.anchor_y = tex.anchor_y;
      break;
    }
  }

  return result;
}
