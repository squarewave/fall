#ifndef ASSET_PACKAGER_H__
#define ASSET_PACKAGER_H__

#include "platform.h"

const int TEXTURE_CHANNELS = 4;
const int TEXTURE_ATLAS_DIAMETER = 4096;
const int TEXTURE_ATLAS_MAP_SIZE = TEXTURE_ATLAS_DIAMETER * TEXTURE_ATLAS_DIAMETER;
const int TEXTURE_ATLAS_SIZE_BYTES = TEXTURE_ATLAS_DIAMETER * TEXTURE_ATLAS_DIAMETER * TEXTURE_CHANNELS;

reflectable enum AssetType {
  AssetType_unspecified = 0,
  AssetType_square,
  AssetType_crew,
  AssetType_selection_line,
  AssetType_selection_circle,
  AssetType_tile,
  AssetType_boulder_large,
  AssetType_tree_medium,
  AssetType_tree_large,

  AssetType_count,
};

i32 AssetType_variation_counts[] = {
  0, // AssetType_unspecified
  1, // AssetType_square
  1, // AssetType_crew
  1, // AssetType_square
  1, // AssetType_selection_line
  1, // AssetType_tile
  1, // AssetType_boulder_large
  2, // AssetType_tree_medium
  1, // AssetType_tree_large

  0, // AssetType_count
};

reflectable enum AssetDirection {
  AssetDirection_unspecified = 0,
  AssetDirection_left,
  AssetDirection_right,
  AssetDirection_forward,
  AssetDirection_backward,

  AssetDirection_horizontal,
  AssetDirection_vertical,

  AssetDirection_count,
};

reflectable enum AssetMoveState {
  AssetMoveState_unspecified = 0,
  AssetMoveState_standing,
  AssetMoveState_running,
  AssetMoveState_walking,
  AssetMoveState_jumping,
  AssetMoveState_falling,

  AssetMoveState_count,
};

reflectable enum AssetLivingState {
  AssetLivingState_unspecified = 0,
  AssetLivingState_dead,
  AssetLivingState_destroyed,

  AssetLivingState_count,
};

reflectable enum AssetClass {
  AssetClass_unspecified = 0,
  AssetClass_science,

  AssetClass_count,
};

reflectable enum AssetColor {
  AssetColor_unspecified = 0,
  AssetColor_dark,
  AssetColor_light,
  AssetColor_caramel,
  AssetColor_blue,

  AssetColor_green,
  AssetColor_red,

  AssetColor_count,
};

const u32 GRASS_TOP_LEFT      = 0x01;
const u32 GRASS_TOP_RIGHT     = 0x02;
const u32 GRASS_BOTTOM_RIGHT  = 0x04;
const u32 GRASS_BOTTOM_LEFT   = 0x08;

reflectable struct AssetAttributes {
  i64 tracking_id;
  i32 variation_number;
  AssetDirection direction;
  AssetMoveState move_state;
  AssetClass asset_class;
  AssetColor color;
  u32 bitflags;
  // TODO(doug): just get rid of this. Superfluous with entity ID.
  AssetLivingState living_state;
};

reflectable struct TextureAsset {
  void* handle;
  f32 left;
  f32 right;
  f32 top;
  f32 bottom;
  i32 px_width, px_height;
  i32 anchor_x, anchor_y;
};

reflectable enum ArchiveEntryType {
  ArchiveEntryType_none,

  ArchiveEntryType_texture_atlas,
};

#pragma pack (1)
reflectable struct PackedTexture {
  i32 top, left, bottom, right;
  i32 anchor_x, anchor_y;
  AssetType asset_type;
  AssetAttributes attributes;
};

#pragma pack (1)
reflectable struct ArchiveEntryHeader_texture_atlas {
  ArchiveEntryType type;
  void* texture_handle; // Placeholder to be filled in by loading program
  i32 png_size;
  i32 packed_texture_count;
};

#pragma pack (1)
reflectable struct GameArchiveHeader {
  i32 archive_entry_count;
};

#endif /* end of include guard: ASSET_PACKAGER_H__ */
