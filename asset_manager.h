#ifndef ASSET_MANAGER_H__
#define ASSET_MANAGER_H__

#include "assets.h"

struct AssetManager {
  void* main_archive_async_handle;
  GameArchiveHeader* main_archive;
  PlatformFileLastWriteTime main_archive_last_write_time;
  void* asset_types_to_atlases;

  ArchiveEntryHeader_texture_atlas* main_atlas;
};

extern AssetManager g_asset_manager;

void assets_init();
void assets_refresh();
TextureAsset assets_get_atlas(AssetType type);
TextureAsset assets_get_texture(AssetType type, AssetAttributes attrs);

#endif /* end of include guard: ASSET_MANAGER_H__ */
