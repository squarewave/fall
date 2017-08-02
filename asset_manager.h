#ifndef ASSET_MANAGER_H__
#define ASSET_MANAGER_H__

#include "assets.h"

reflectable struct AssetManager {
  void* main_archive_async_handle;
  GameArchiveHeader* main_archive;
  size_t main_archive_size;
  PlatformFileLastWriteTime main_archive_last_write_time;
  void* asset_types_to_atlases;

  ArchiveEntryHeader_texture_atlas* main_atlas;
};

extern AssetManager g_asset_manager;

void assets_init();
void assets_refresh();
TextureAsset assets_get_atlas(AssetType type);
PackedTexture* assets_get_packed_texture(AssetType type, AssetAttributes attrs);
TextureAsset assets_get_texture(AssetType type, AssetAttributes attrs);

#ifdef FALL_INTERNAL
void assets_save_edits();
#endif

#endif /* end of include guard: ASSET_MANAGER_H__ */
