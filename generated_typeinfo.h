#pragma once

#include "platform.h"
#include "meat_space.h"
#include "assets.h"
#include "geometry.h"
#include "game.h"
#include "asset_manager.h"
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

  TypeInfo_ID_char,
  TypeInfo_ID_int,

  TypeInfo_ID_end_primitives,
  TypeInfo_ID_PlatformFileLastWriteTime,
  TypeInfo_ID_MeatSpaceEntity,
  TypeInfo_ID_MeatSpaceBrainType,
  TypeInfo_ID_MeatSpaceBrain,
  TypeInfo_ID_MeatSpaceCamera,
  TypeInfo_ID_MeatSpace,
  TypeInfo_ID_AssetDirection,
  TypeInfo_ID_AssetMoveState,
  TypeInfo_ID_AssetClass,
  TypeInfo_ID_AssetColor,
  TypeInfo_ID_AssetType,
  TypeInfo_ID_AssetAttributes,
  TypeInfo_ID_TextureAsset,
  TypeInfo_ID_ArchiveEntryType,
  TypeInfo_ID_PackedTexture,
  TypeInfo_ID_ArchiveEntryHeader_texture_atlas,
  TypeInfo_ID_GameArchiveHeader,
  TypeInfo_ID_vec2,
  TypeInfo_ID_ivec2,
  TypeInfo_ID_rect2,
  TypeInfo_ID_GameState,
  TypeInfo_ID_TransientState,
  TypeInfo_ID_AssetManager,

  TypeInfo_ID_count
};

enum TypeKind {
  TypeKind_struct,
  TypeKind_enum,
  TypeKind_union,
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
  int table_index;
};

MemberInfo TypeInfo_member_table[] = {
  {"platform_data", TypeInfo_ID_PlatformFileLastWriteTime, TypeInfo_ID_char, MemberKind_array, 0, (size_t)&((PlatformFileLastWriteTime*)0)->platform_data, 0},
  {"id", TypeInfo_ID_MeatSpaceEntity, TypeInfo_ID_i64, MemberKind_value, 0, (size_t)&((MeatSpaceEntity*)0)->id, 1},
  {"p", TypeInfo_ID_MeatSpaceEntity, TypeInfo_ID_vec2, MemberKind_value, 0, (size_t)&((MeatSpaceEntity*)0)->p, 2},
  {"asset_type", TypeInfo_ID_MeatSpaceEntity, TypeInfo_ID_AssetType, MemberKind_value, 0, (size_t)&((MeatSpaceEntity*)0)->asset_type, 3},
  {"asset_attributes", TypeInfo_ID_MeatSpaceEntity, TypeInfo_ID_AssetAttributes, MemberKind_value, 0, (size_t)&((MeatSpaceEntity*)0)->asset_attributes, 4},
  {"selected", TypeInfo_ID_MeatSpaceEntity, TypeInfo_ID_b32, MemberKind_value, 0, (size_t)&((MeatSpaceEntity*)0)->selected, 5},
  {"selection_bounds", TypeInfo_ID_MeatSpaceEntity, TypeInfo_ID_rect2, MemberKind_value, 0, (size_t)&((MeatSpaceEntity*)0)->selection_bounds, 6},
  {"MeatSpaceBrainType_none", TypeInfo_ID_MeatSpaceBrainType, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 7},
  {"MeatSpaceBrainType_player", TypeInfo_ID_MeatSpaceBrainType, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 8},
  {"type", TypeInfo_ID_MeatSpaceBrain, TypeInfo_ID_MeatSpaceBrainType, MemberKind_value, 0, (size_t)&((MeatSpaceBrain*)0)->type, 9},
  {"position", TypeInfo_ID_MeatSpaceCamera, TypeInfo_ID_vec2, MemberKind_value, 0, (size_t)&((MeatSpaceCamera*)0)->position, 10},
  {"scale", TypeInfo_ID_MeatSpaceCamera, TypeInfo_ID_vec2, MemberKind_value, 0, (size_t)&((MeatSpaceCamera*)0)->scale, 11},
  {"viewport_top", TypeInfo_ID_MeatSpaceCamera, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((MeatSpaceCamera*)0)->viewport_top, 12},
  {"viewport_bottom", TypeInfo_ID_MeatSpaceCamera, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((MeatSpaceCamera*)0)->viewport_bottom, 13},
  {"viewport_left", TypeInfo_ID_MeatSpaceCamera, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((MeatSpaceCamera*)0)->viewport_left, 14},
  {"viewport_right", TypeInfo_ID_MeatSpaceCamera, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((MeatSpaceCamera*)0)->viewport_right, 15},
  {"entities", TypeInfo_ID_MeatSpace, TypeInfo_ID_MeatSpaceEntity, MemberKind_array, 0, (size_t)&((MeatSpace*)0)->entities, 16},
  {"entities_count", TypeInfo_ID_MeatSpace, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((MeatSpace*)0)->entities_count, 17},
  {"brains", TypeInfo_ID_MeatSpace, TypeInfo_ID_MeatSpaceBrain, MemberKind_array, 0, (size_t)&((MeatSpace*)0)->brains, 18},
  {"brains_count", TypeInfo_ID_MeatSpace, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((MeatSpace*)0)->brains_count, 19},
  {"camera", TypeInfo_ID_MeatSpace, TypeInfo_ID_MeatSpaceCamera, MemberKind_value, 0, (size_t)&((MeatSpace*)0)->camera, 20},
  {"AssetDirection_unspecified", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 21},
  {"AssetDirection_left", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 22},
  {"AssetDirection_right", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 23},
  {"AssetDirection_forward", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 24},
  {"AssetDirection_backward", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 25},
  {"AssetDirection_horizontal", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 5, 0, 26},
  {"AssetDirection_vertical", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 6, 0, 27},
  {"AssetDirection_count", TypeInfo_ID_AssetDirection, TypeInfo_ID_i32, MemberKind_enum, 7, 0, 28},
  {"AssetMoveState_unspecified", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 29},
  {"AssetMoveState_standing", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 30},
  {"AssetMoveState_running", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 31},
  {"AssetMoveState_walking", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 32},
  {"AssetMoveState_jumping", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 33},
  {"AssetMoveState_falling", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 5, 0, 34},
  {"AssetMoveState_count", TypeInfo_ID_AssetMoveState, TypeInfo_ID_i32, MemberKind_enum, 6, 0, 35},
  {"AssetClass_unspecified", TypeInfo_ID_AssetClass, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 36},
  {"AssetClass_science", TypeInfo_ID_AssetClass, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 37},
  {"AssetClass_count", TypeInfo_ID_AssetClass, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 38},
  {"AssetColor_unspecified", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 39},
  {"AssetColor_dark", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 40},
  {"AssetColor_light", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 41},
  {"AssetColor_caramel", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 42},
  {"AssetColor_blue", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 43},
  {"AssetColor_green", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 5, 0, 44},
  {"AssetColor_red", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 6, 0, 45},
  {"AssetColor_count", TypeInfo_ID_AssetColor, TypeInfo_ID_i32, MemberKind_enum, 7, 0, 46},
  {"AssetType_unspecified", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 47},
  {"AssetType_crew", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 48},
  {"AssetType_selection_line", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 2, 0, 49},
  {"AssetType_selection_circle", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 3, 0, 50},
  {"AssetType_count", TypeInfo_ID_AssetType, TypeInfo_ID_i32, MemberKind_enum, 4, 0, 51},
  {"direction", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetDirection, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->direction, 52},
  {"move_state", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetMoveState, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->move_state, 53},
  {"asset_class", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetClass, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->asset_class, 54},
  {"color", TypeInfo_ID_AssetAttributes, TypeInfo_ID_AssetColor, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->color, 55},
  {"tracking_id", TypeInfo_ID_AssetAttributes, TypeInfo_ID_i64, MemberKind_value, 0, (size_t)&((AssetAttributes*)0)->tracking_id, 56},
  {"handle", TypeInfo_ID_TextureAsset, TypeInfo_ID_void, MemberKind_pointer, 0, (size_t)&((TextureAsset*)0)->handle, 57},
  {"left", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->left, 58},
  {"right", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->right, 59},
  {"top", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->top, 60},
  {"bottom", TypeInfo_ID_TextureAsset, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->bottom, 61},
  {"px_width", TypeInfo_ID_TextureAsset, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((TextureAsset*)0)->px_width, 62},
  {"ArchiveEntryType_none", TypeInfo_ID_ArchiveEntryType, TypeInfo_ID_i32, MemberKind_enum, 0, 0, 63},
  {"ArchiveEntryType_texture_atlas", TypeInfo_ID_ArchiveEntryType, TypeInfo_ID_i32, MemberKind_enum, 1, 0, 64},
  {"top", TypeInfo_ID_PackedTexture, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((PackedTexture*)0)->top, 65},
  {"asset_type", TypeInfo_ID_PackedTexture, TypeInfo_ID_AssetType, MemberKind_value, 0, (size_t)&((PackedTexture*)0)->asset_type, 66},
  {"attributes", TypeInfo_ID_PackedTexture, TypeInfo_ID_AssetAttributes, MemberKind_value, 0, (size_t)&((PackedTexture*)0)->attributes, 67},
  {"type", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_ArchiveEntryType, MemberKind_value, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->type, 68},
  {"texture_handle", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_void, MemberKind_pointer, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->texture_handle, 69},
  {"png_size", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->png_size, 70},
  {"packed_texture_count", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->packed_texture_count, 71},
  {"packed_textures", TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeInfo_ID_PackedTexture, MemberKind_array, 0, (size_t)&((ArchiveEntryHeader_texture_atlas*)0)->packed_textures, 72},
  {"archive_entry_count", TypeInfo_ID_GameArchiveHeader, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((GameArchiveHeader*)0)->archive_entry_count, 73},
  {"x", TypeInfo_ID_vec2, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((vec2*)0)->x, 74},
  {"y", TypeInfo_ID_vec2, TypeInfo_ID_f32, MemberKind_value, 0, (size_t)&((vec2*)0)->y, 75},
  {"x", TypeInfo_ID_ivec2, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((ivec2*)0)->x, 76},
  {"y", TypeInfo_ID_ivec2, TypeInfo_ID_i32, MemberKind_value, 0, (size_t)&((ivec2*)0)->y, 77},
  {"bottom_left", TypeInfo_ID_rect2, TypeInfo_ID_vec2, MemberKind_value, 0, (size_t)&((rect2*)0)->bottom_left, 78},
  {"top_right", TypeInfo_ID_rect2, TypeInfo_ID_vec2, MemberKind_value, 0, (size_t)&((rect2*)0)->top_right, 79},
  {"initialized", TypeInfo_ID_GameState, TypeInfo_ID_b32, MemberKind_value, 0, (size_t)&((GameState*)0)->initialized, 80},
  {"TMP_meat_space", TypeInfo_ID_GameState, TypeInfo_ID_MeatSpace, MemberKind_pointer, 0, (size_t)&((GameState*)0)->TMP_meat_space, 81},
  {"asset_manager", TypeInfo_ID_GameState, TypeInfo_ID_AssetManager, MemberKind_pointer, 0, (size_t)&((GameState*)0)->asset_manager, 82},
  {"main_archive_async_handle", TypeInfo_ID_AssetManager, TypeInfo_ID_void, MemberKind_pointer, 0, (size_t)&((AssetManager*)0)->main_archive_async_handle, 83},
  {"main_archive", TypeInfo_ID_AssetManager, TypeInfo_ID_GameArchiveHeader, MemberKind_pointer, 0, (size_t)&((AssetManager*)0)->main_archive, 84},
  {"main_archive_last_write_time", TypeInfo_ID_AssetManager, TypeInfo_ID_PlatformFileLastWriteTime, MemberKind_value, 0, (size_t)&((AssetManager*)0)->main_archive_last_write_time, 85},
  {"asset_types_to_atlases", TypeInfo_ID_AssetManager, TypeInfo_ID_void, MemberKind_pointer, 0, (size_t)&((AssetManager*)0)->asset_types_to_atlases, 86},
  {"main_atlas", TypeInfo_ID_AssetManager, TypeInfo_ID_ArchiveEntryHeader_texture_atlas, MemberKind_pointer, 0, (size_t)&((AssetManager*)0)->main_atlas, 87},

};

struct TypeInfo {
  TypeInfo_ID id;
  TypeKind kind;
};

TypeInfo TypeInfo_custom_type_table[] = {
  {TypeInfo_ID_PlatformFileLastWriteTime, TypeKind_struct},
  {TypeInfo_ID_MeatSpaceEntity, TypeKind_struct},
  {TypeInfo_ID_MeatSpaceBrainType, TypeKind_enum},
  {TypeInfo_ID_MeatSpaceBrain, TypeKind_struct},
  {TypeInfo_ID_MeatSpaceCamera, TypeKind_struct},
  {TypeInfo_ID_MeatSpace, TypeKind_struct},
  {TypeInfo_ID_AssetDirection, TypeKind_enum},
  {TypeInfo_ID_AssetMoveState, TypeKind_enum},
  {TypeInfo_ID_AssetClass, TypeKind_enum},
  {TypeInfo_ID_AssetColor, TypeKind_enum},
  {TypeInfo_ID_AssetType, TypeKind_enum},
  {TypeInfo_ID_AssetAttributes, TypeKind_struct},
  {TypeInfo_ID_TextureAsset, TypeKind_struct},
  {TypeInfo_ID_ArchiveEntryType, TypeKind_enum},
  {TypeInfo_ID_PackedTexture, TypeKind_struct},
  {TypeInfo_ID_ArchiveEntryHeader_texture_atlas, TypeKind_struct},
  {TypeInfo_ID_GameArchiveHeader, TypeKind_struct},
  {TypeInfo_ID_vec2, TypeKind_struct},
  {TypeInfo_ID_ivec2, TypeKind_struct},
  {TypeInfo_ID_rect2, TypeKind_struct},
  {TypeInfo_ID_GameState, TypeKind_struct},
  {TypeInfo_ID_TransientState, TypeKind_struct},
  {TypeInfo_ID_AssetManager, TypeKind_struct},
};
