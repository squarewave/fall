#pragma once

#include "platform.h"
// PlatformTexture
// PlatformFileLastWriteTime
#include "assets.h"
// AssetAttributes
// AssetSpec
// TextureAsset
// PackedTexture
// ArchiveEntryHeader_texture_atlas
// GameArchiveHeader
enum TypeInfo_ID {
  TypeInfo_ID_none,

  TypeInfo_ID_void,

  TypeInfo_ID_u64,
  TypeInfo_ID_u32,
  TypeInfo_ID_u16,
  TypeInfo_ID_u8,
  TypeInfo_ID_i64,
  TypeInfo_ID_i32,
  TypeInfo_ID_i16,
  TypeInfo_ID_i8,
  TypeInfo_ID_b32,
  TypeInfo_ID_f32,
  TypeInfo_ID_f64,

  TypeInfo_ID_size_t,
  TypeInfo_ID_char,
  TypeInfo_ID_int,

  TypeInfo_ID_end_primitives,
  TypeInfo_ID_PlatformTexture,
  TypeInfo_ID_PlatformFileLastWriteTime,
  TypeInfo_ID_AssetType,
  TypeInfo_ID_AssetDirection,
  TypeInfo_ID_AssetMoveState,
  TypeInfo_ID_AssetLivingState,
  TypeInfo_ID_AssetClass,
  TypeInfo_ID_AssetColor,
  TypeInfo_ID_AssetAttributes,
  TypeInfo_ID_AssetSpec,
  TypeInfo_ID_DummyUnion_Type,
  TypeInfo_ID_DummyUnion,
  TypeInfo_ID_TextureAsset,
  TypeInfo_ID_ArchiveEntryType,
  TypeInfo_ID_PackedTexture,
  TypeInfo_ID_ArchiveEntryHeader_texture_atlas,
  TypeInfo_ID_GameArchiveHeader,

  TypeInfo_ID_count
};

enum TypeKind {
  TypeKind_struct,
  TypeKind_enum,
  TypeKind_union,
  TypeKind_primitive,
};

enum MemberKind {
  MemberKind_enum,
  MemberKind_array,
  MemberKind_pointer,
  MemberKind_array_of_pointers,
  MemberKind_value,
};

struct MemberInfo {
  char* member_name;
  TypeInfo_ID parent_type;
  TypeInfo_ID member_type;
  MemberKind member_kind;
  int enum_value;
  size_t offset;
  size_t array_size;
  char flags;
  int table_index;
};

MemberInfo TypeInfo_member_table[] = {
  {"width", TypeInfo_ID_PlatformTexture, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((PlatformTexture*)0)->width, 0, 0, 0},
  {"handle", TypeInfo_ID_PlatformTexture, TypeInfo_ID_void, MemberKind_pointer, 0, (size_t)&((PlatformTexture*)0)->handle, 0, 0, 1},
  {"platform_data", TypeInfo_ID_PlatformFileLastWriteTime, TypeInfo_ID_char, MemberKind_array, 0, (size_t)&((PlatformFileLastWriteTime*)0)->platform_data, ARRAY_LENGTH(((PlatformFileLastWriteTime*)0)->platform_data), 0, 2},
  {"AssetType_unspecified", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 3},
  {"AssetType_square", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 0, 0, 4},
  {"AssetType_crew", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 0, 0, 5},
  {"AssetType_selection_line", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 0, 0, 6},
  {"AssetType_selection_circle", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 0, 0, 7},
  {"AssetType_tile", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 5, 0, 0, 0, 8},
  {"AssetType_boulder_small", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 6, 0, 0, 0, 9},
  {"AssetType_boulder_large", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 7, 0, 0, 0, 10},
  {"AssetType_tree_medium", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 8, 0, 0, 0, 11},
  {"AssetType_tree_large", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 9, 0, 0, 0, 12},
  {"AssetType_count", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 10, 0, 0, 0, 13},
  {"AssetDirection_unspecified", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 14},
  {"AssetDirection_left", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 0, 0, 15},
  {"AssetDirection_right", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 0, 0, 16},
  {"AssetDirection_forward", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 0, 0, 17},
  {"AssetDirection_backward", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 0, 0, 18},
  {"AssetDirection_horizontal", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 5, 0, 0, 0, 19},
  {"AssetDirection_vertical", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 6, 0, 0, 0, 20},
  {"AssetDirection_count", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 7, 0, 0, 0, 21},
  {"AssetMoveState_unspecified", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 22},
  {"AssetMoveState_standing", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 0, 0, 23},
  {"AssetMoveState_running", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 0, 0, 24},
  {"AssetMoveState_walking", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 0, 0, 25},
  {"AssetMoveState_jumping", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 0, 0, 26},
  {"AssetMoveState_falling", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 5, 0, 0, 0, 27},
  {"AssetMoveState_count", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 6, 0, 0, 0, 28},
  {"AssetLivingState_unspecified", TypeInfo_ID_AssetLivingState, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 29},
  {"AssetLivingState_dead", TypeInfo_ID_AssetLivingState, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 0, 0, 30},
  {"AssetLivingState_destroyed", TypeInfo_ID_AssetLivingState, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 0, 0, 31},
  {"AssetLivingState_count", TypeInfo_ID_AssetLivingState, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 0, 0, 32},
  {"AssetClass_unspecified", TypeInfo_ID_AssetClass, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 33},
  {"AssetClass_science", TypeInfo_ID_AssetClass, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 0, 0, 34},
  {"AssetClass_count", TypeInfo_ID_AssetClass, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 0, 0, 35},
  {"AssetColor_unspecified", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 36},
  {"AssetColor_dark", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 0, 0, 37},
  {"AssetColor_light", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 0, 0, 38},
  {"AssetColor_caramel", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 0, 0, 39},
  {"AssetColor_blue", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 0, 0, 40},
  {"AssetColor_green", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 5, 0, 0, 0, 41},
  {"AssetColor_red", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 6, 0, 0, 0, 42},
  {"AssetColor_count", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 7, 0, 0, 0, 43},
  {"tracking_id", TypeInfo_ID_AssetAttributes, TypeInfo_ID_i64, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->tracking_id, 0, 0, 44},
  {"variation_number", TypeInfo_ID_AssetAttributes, TypeInfo_ID_u32, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->variation_number, 0, 0, 45},
  {"direction", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetDirection, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->direction, 0, 0, 46},
  {"move_state", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetMoveState, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->move_state, 0, 0, 47},
  {"asset_class", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetClass, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->asset_class, 0, 0, 48},
  {"color", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetColor, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->color, 0, 0, 49},
  {"bitflags", TypeInfo_ID_AssetAttributes, TypeInfo_ID_u32, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->bitflags, 0, 0, 50},
  {"living_state", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetLivingState, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->living_state, 0, 0, 51},
  {"filepath", TypeInfo_ID_AssetSpec, TypeInfo_ID_char, MemberKind_pointer, 0, (size_t)&((AssetSpec*)0)->filepath, 0, 1, 52},
  {"asset_type", TypeInfo_ID_AssetSpec, TypeInfo_ID_AssetType, MemberKind_value, 0, (size_t)&((AssetSpec*)0)->asset_type, 0, 0, 53},
  {"asset_attributes", TypeInfo_ID_AssetSpec, TypeInfo_ID_AssetAttributes, MemberKind_value, 0, (size_t)&((AssetSpec*)0)->asset_attributes, 0, 0, 54},
  {"use_anchor", TypeInfo_ID_AssetSpec, TypeInfo_ID_b32, MemberKind_value, 0, (size_t)&((AssetSpec*)0)->use_anchor, 0, 0, 55},
  {"anchor_x", TypeInfo_ID_AssetSpec, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((AssetSpec*)0)->anchor_x, 0, 0, 56},
  {"anchor_y", TypeInfo_ID_AssetSpec, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((AssetSpec*)0)->anchor_y, 0, 0, 57},
  {"DummyUnion_Type_dummy_val", TypeInfo_ID_DummyUnion_Type, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 58},
  {"dummy_val", TypeInfo_ID_DummyUnion, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((DummyUnion*)0)->dummy_val, 0, 0, 59},
  {"handle", TypeInfo_ID_TextureAsset, TypeInfo_ID_void, MemberKind_pointer, 0, (size_t)&((TextureAsset*)0)->handle, 0, 0, 60},
  {"left", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->left, 0, 0, 61},
  {"right", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->right, 0, 0, 62},
  {"top", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->top, 0, 0, 63},
  {"bottom", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->bottom, 0, 0, 64},
  {"px_width", TypeInfo_ID_TextureAsset, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->px_width, 0, 0, 65},
  {"anchor_x", TypeInfo_ID_TextureAsset, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->anchor_x, 0, 0, 66},
  {"ArchiveEntryType_none", TypeInfo_ID_ArchiveEntryType, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 0, 0, 67},
  {"ArchiveEntryType_texture_atlas", TypeInfo_ID_ArchiveEntryType, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 0, 0, 68},
  {"top", TypeInfo_ID_PackedTexture, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((PackedTexture*)0)->top, 0, 0, 69},
  {"anchor_x", TypeInfo_ID_PackedTexture, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((PackedTexture*)0)->anchor_x, 0, 0, 70},
  {"asset_type", TypeInfo_ID_PackedTexture, TypeInfo_ID_AssetType, MemberKind_value, 0, (size_t)&((PackedTexture*)0)->asset_type, 0, 0, 71},
  {"attributes", TypeInfo_ID_PackedTexture, TypeInfo_ID_AssetAttributes, MemberKind_value, 0, (size_t)&((PackedTexture*)0)->attributes, 0, 0, 72},
  {"type", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_ArchiveEntryType, MemberKind_value, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->type, 0, 0, 73},
  {"texture_handle", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_void, MemberKind_pointer, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->texture_handle, 0, 0, 74},
  {"png_size", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->png_size, 0, 0, 75},
  {"packed_texture_count", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->packed_texture_count, 0, 0, 76},
  {"archive_entry_count", TypeInfo_ID_GameArchiveHeader, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((GameArchiveHeader*)0)->archive_entry_count, 0, 0, 77},

};

struct TypeInfo {
  TypeInfo_ID id;
  TypeKind kind;
  size_t size;
  int members_start;
};

TypeInfo TypeInfo_custom_type_table[] = {
  {TypeInfo_ID_PlatformTexture, TypeKind_struct, sizeof(PlatformTexture), 0},
  {TypeInfo_ID_PlatformFileLastWriteTime, TypeKind_struct, sizeof(PlatformFileLastWriteTime), 2},
  {TypeInfo_ID_AssetType, TypeKind_enum, sizeof(AssetType), 3},
  {TypeInfo_ID_AssetDirection, TypeKind_enum, sizeof(AssetDirection), 14},
  {TypeInfo_ID_AssetMoveState, TypeKind_enum, sizeof(AssetMoveState), 22},
  {TypeInfo_ID_AssetLivingState, TypeKind_enum, sizeof(AssetLivingState), 29},
  {TypeInfo_ID_AssetClass, TypeKind_enum, sizeof(AssetClass), 33},
  {TypeInfo_ID_AssetColor, TypeKind_enum, sizeof(AssetColor), 36},
  {TypeInfo_ID_AssetAttributes, TypeKind_struct, sizeof(AssetAttributes), 44},
  {TypeInfo_ID_AssetSpec, TypeKind_struct, sizeof(AssetSpec), 52},
  {TypeInfo_ID_DummyUnion_Type, TypeKind_enum, sizeof(DummyUnion_Type), 58},
  {TypeInfo_ID_DummyUnion, TypeKind_union, sizeof(DummyUnion), 59},
  {TypeInfo_ID_TextureAsset, TypeKind_struct, sizeof(TextureAsset), 60},
  {TypeInfo_ID_ArchiveEntryType, TypeKind_enum, sizeof(ArchiveEntryType), 67},
  {TypeInfo_ID_PackedTexture, TypeKind_struct, sizeof(PackedTexture), 69},
  {TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeKind_struct, sizeof(ArchiveEntryHeader_texture_atlas), 73},
  {TypeInfo_ID_GameArchiveHeader, TypeKind_struct, sizeof(GameArchiveHeader), 77},

};

struct UnionMemberInfo {
  TypeInfo_ID union_type_id;
  i32 type;
  TypeInfo_ID union_member_type_id;
};

UnionMemberInfo TypeInfo_union_member_table[] = {
  {TypeInfo_ID_DummyUnion, DummyUnion_Type_dummy_val, TypeInfo_ID_i32},
};
