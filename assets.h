#ifndef ASSET_PACKAGER_H__
#define ASSET_PACKAGER_H__

#include "platform.h"

const int TEXTURE_CHANNELS = 4;
const int TEXTURE_ATLAS_DIAMETER = 4096;
const int TEXTURE_ATLAS_MAP_SIZE = TEXTURE_ATLAS_DIAMETER * TEXTURE_ATLAS_DIAMETER;
const int TEXTURE_ATLAS_SIZE_BYTES = TEXTURE_ATLAS_DIAMETER * TEXTURE_ATLAS_DIAMETER * TEXTURE_CHANNELS;

enum AssetDirection {
  AssetDirection_unspecified = 0,
  AssetDirection_left,
  AssetDirection_right,
  AssetDirection_forward,
  AssetDirection_backward,

  AssetDirection_count,
};

enum AssetMoveState {
  AssetMoveState_unspecified = 0,
  AssetMoveState_standing,
  AssetMoveState_running,
  AssetMoveState_walking,
  AssetMoveState_jumping,
  AssetMoveState_falling,

  AssetMoveState_count,
};

enum AssetClass {
  AssetClass_unspecified = 0,
  AssetClass_science,

  AssetClass_count,
};

enum AssetColor {
  AssetColor_unspecified = 0,
  AssetColor_dark,
  AssetColor_light,
  AssetColor_caramel,
  AssetColor_blue,

  AssetColor_count,
};

enum AssetType {
  AssetType_unspecified = 0,
  AssetType_crew,

  AssetType_count,
};

struct AssetAttributes {
  AssetDirection direction;
  AssetMoveState move_state;
  AssetClass asset_class;
  AssetColor color;
  i64 tracking_id;
};

struct TextureAsset {
  void* handle;
  f32 left, right, top, bottom;
  i32 px_width, px_height;
};

enum ArchiveEntryType {
  ArchiveEntryType_none,

  ArchiveEntryType_texture_atlas,
};

#pragma pack (1)
struct PackedTexture {
  int top, left, bottom, right;
  AssetType asset_type;
  AssetAttributes attributes;
};

#pragma pack (1)
struct ArchiveEntryHeader_texture_atlas {
  ArchiveEntryType type;
  void* texture_handle; // Placeholder to be filled in by loading program
  int png_size;
  int packed_texture_count;
  PackedTexture packed_textures[0];
};

#pragma pack (1)
struct GameArchiveHeader {
  int archive_entry_count;
};

#endif /* end of include guard: ASSET_PACKAGER_H__ */
