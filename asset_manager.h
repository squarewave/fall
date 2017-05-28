#ifndef ASSET_MANAGER_H__
#define ASSET_MANAGER_H__

#include "assets.h"

struct AssetManager {
  void* main_archive_async_handle;
  GameArchiveHeader* main_archive;
  void* asset_types_to_atlases;
};

extern AssetManager g_asset_manager;

void assets_init();
TextureAsset assets_get_texture(AssetType type, AssetAttributes attrs);

#endif /* end of include guard: ASSET_MANAGER_H__ */
