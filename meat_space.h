#ifndef MEAT_SPACE_H__
#define MEAT_SPACE_H__

#include "platform.h"
#include "assets.h"
#include "geometry.h"
#include "grid.h"
#include "rnd.h"

reflectable enum MeatSpaceCommand_Type {
  MeatSpaceCommand_Type_none = 0,
  MeatSpaceCommand_Type_move_to,

  MeatSpaceCommand_Type_count,
};

reflectable struct MeatSpaceCommand_move_to {
  MeatSpaceCommand_Type type;
  GridCell target;
};

reflectable union MeatSpaceCommand {
  MeatSpaceCommand_Type type;
  MeatSpaceCommand_move_to move_to;
};

reflectable enum MeatSpaceWeapon_Type {
  MeatSpaceWeapon_Type_none = 0,
  MeatSpaceWeapon_Type_gun,
  MeatSpaceWeapon_Type_grenade_launcher,
  MeatSpaceWeapon_Type_healing_ray,
};

reflectable struct MeatSpaceWeapon_gun {
  MeatSpaceWeapon_Type type;
  f32 damage;
  f32 range;
  f32 cooldown;
  f32 time_since_last_fired;
};

reflectable struct MeatSpaceWeapon_grenade_launcher {
  MeatSpaceWeapon_Type type;
  f32 damage;
  f32 range;
  f32 cooldown;
  f32 time_since_last_fired;
  f32 radius;
};

reflectable struct MeatSpaceWeapon_healing_ray {
  MeatSpaceWeapon_Type type;
  f32 damage;
  f32 range;
  f32 cooldown;
  f32 time_since_last_fired;
};

reflectable union MeatSpaceWeapon {
  MeatSpaceWeapon_Type type;
  MeatSpaceWeapon_gun gun;
  MeatSpaceWeapon_grenade_launcher grenade_launcher;
  MeatSpaceWeapon_healing_ray healing_ray;
};

reflectable enum SelectionGroup {
  SelectionGroup_none,
  SelectionGroup_player,
  SelectionGroup_nonplayer,
  SelectionGroup_enemy,
};

const u32 ENTITY_FLAG_TRAVERSABLE = 0x01;
const u32 ENTITY_FLAG_CHARACTER   = 0x02;
const u32 ENTITY_FLAG_DEAD        = 0x04;

reflectable struct MeatSpaceEntity {
  // NOTE(doug): these have been thought about
  i64 id;
  vec2 p;
  u32 flags;

  // TODO(doug): should these be stored directly in here, or
  // derived from other state?
  AssetType asset_type;
  AssetAttributes asset_attributes;

  // TODO(doug): these haven't been thought much about.
  f32 z_bias;
  SelectionGroup selection_group;
  b32 selected;
  rect2 selection_bounds;
  i32 collision_volumes;
  vec2 target_center;
  vec2 firing_center;
  f32 max_health;
  f32 health;

  MeatSpaceWeapon weapon;

  b32 unfinished_move;
  GridCell unfinished_move_target;

  MeatSpaceCommand command;
};

reflectable enum MeatSpaceBrain_Type {
  MeatSpaceBrain_Type_none = 0,
  MeatSpaceBrain_Type_player,

  MeatSpaceBrain_Type_count
};

reflectable struct MeatSpaceBrain_player {
  MeatSpaceBrain_Type type;
  i32 selected_entities_count;
  i64* selected_entities;
  MeatSpaceEntity* mouse_down_entity;

  b32 selecting;
  vec2 selection_start;
};

reflectable union MeatSpaceBrain {
  MeatSpaceBrain_Type type;
  MeatSpaceBrain_player player;
};

const i32 MAX_SELECTED_ENTITIES = 256;

reflectable enum MeatSpaceProjectile_Type {
  MeatSpaceProjectile_Type_none,
  MeatSpaceProjectile_Type_common,
  MeatSpaceProjectile_Type_entity_targeted,
  MeatSpaceProjectile_Type_position_targeted,
};

reflectable struct MeatSpaceProjectile_common {
  MeatSpaceProjectile_Type type;
  vec2 p;
};

reflectable struct MeatSpaceProjectile_entity_targeted {
  MeatSpaceProjectile_Type type;
  vec2 p;
  i32 target_entity;
  f32 speed;
  f32 damage;
};

reflectable struct MeatSpaceProjectile_position_targeted {
  MeatSpaceProjectile_Type type;
  vec2 p;
  vec2 target_p;
  f32 speed;
};

reflectable union MeatSpaceProjectile {
  MeatSpaceProjectile_Type type;
  MeatSpaceProjectile_common common;
  MeatSpaceProjectile_entity_targeted entity_targeted;
  MeatSpaceProjectile_position_targeted position_targeted;
};

reflectable struct MeatSpaceCamera {
  vec2 position;
  vec2 scale;

  f32 viewport_top;
  f32 viewport_bottom;
  f32 viewport_left;
  f32 viewport_right;
};

#define POSITION_GRID_WIDTH 256
#define POSITION_GRID_BUCKETS_SIZE 4096

// NOTE(doug): all of the plus_one nonsense is basically just to get the
// effect of relative pointers where 0 means it doesn't exist. We could just
// use -1, but then we'd have to initialize everything to -1, which is a bit
// of a pain. But so is the plus one shenanigans. TODO: think about it later.
reflectable struct PositionGridBucketNode {
  i32 entity_index_plus_one;
  i32 next_bucket_plus_one;
};

reflectable enum CollisionVolume_Type {
  CollisionVolume_Type_none,
  CollisionVolume_Type_box,
  CollisionVolume_Type_common,
  CollisionVolume_Type_circle,
  CollisionVolume_Type_elipse,
};

reflectable struct CollisionVolume_common {
  CollisionVolume_Type type;
  i64 entity_id;
  vec2 offset;
};

reflectable struct CollisionVolume_box {
  CollisionVolume_Type type;
  i64 entity_id;
  vec2 offset;
  vec2 dimensions;
};

reflectable struct CollisionVolume_circle {
  CollisionVolume_Type type;
  i64 entity_id;
  vec2 offset;
  f32 radius;
};

reflectable struct CollisionVolume_elipse {
  CollisionVolume_Type type;
  i64 entity_id;
  vec2 offset;
  // TODO
};

reflectable union CollisionVolume {
  CollisionVolume_Type type;
  CollisionVolume_common common;
  CollisionVolume_box box;
  CollisionVolume_circle circle;
  CollisionVolume_elipse elipse;
};

reflectable enum MeatSpaceEntityTemplateId {
  MeatSpaceEntityTemplateId_none,
  MeatSpaceEntityTemplateId_tile,
  MeatSpaceEntityTemplateId_crew_gun,
  MeatSpaceEntityTemplateId_crew_healing_ray,
  MeatSpaceEntityTemplateId_crew_grenade_launcher,
  MeatSpaceEntityTemplateId_boulder_large,
  MeatSpaceEntityTemplateId_tree_medium,
  MeatSpaceEntityTemplateId_tree_large,
};

reflectable struct MeatSpaceEntityTemplate {
  MeatSpaceEntityTemplateId id;
  u32 flags;

  AssetType asset_type;

  f32 z_bias;
  rect2 selection_bounds;
  i32 collision_boxes;
  vec2 target_center;
  vec2 firing_center;
  MeatSpaceWeapon weapon;

  f32 max_health;
};

reflectable struct MeatSpaceEntityTemplateCollection {
  MeatSpaceEntityTemplate* templates;
  i32 templates_count;

  CollisionVolume* collision_volumes;
  i32 collision_volumes_count;
};

reflectable struct MeatSpace {
  MeatSpaceEntity entities[4096];
  i32 entities_count;

  MeatSpaceBrain brains[1024];
  i32 brains_count;

  MeatSpaceProjectile projectiles[4096];
  i32 projectiles_count;
  u32 projectiles_free_stack[64];
  u32 projectiles_free_stack_count;

  CollisionVolume collision_volumes[4096];
  i32 collision_volumes_count;

  Grid grid;

  // NOTE(doug): position grid maps to entities which occupy
  // that grid cell (mod 64), or -1 if none occupy it. The buckets
  // make up a linked list, where the index is the entity's
  // index in entities, and the value is the next entity in
  // the bucket, or -1 if none exists. This should make it
  // quick to find a list of all entities in a particular
  // cell.
  i32 position_grid[POSITION_GRID_WIDTH * POSITION_GRID_WIDTH];
  PositionGridBucketNode position_grid_buckets[POSITION_GRID_BUCKETS_SIZE];
  i32 position_grid_bucket_index;

  MeatSpaceEntityTemplateCollection template_collection;

  MeatSpaceCamera camera;

  ignore rnd_pcg_t pcg;
};

MeatSpaceEntityTemplate* get_entity_template(MeatSpace* meat_space, MeatSpaceEntityTemplateId template_id);
MeatSpaceEntity* create_entity_from_template(MeatSpace* meat_space,
                                             MeatSpaceEntityTemplateId template_id,
                                             i32 variation_number = -1);
void draw_entity_texture(MeatSpace* meat_space,
                         vec2 position, u32 color,
                         AssetType asset_type, AssetAttributes asset_attributes,
                         f32 z_bias);

void meat_space_update_and_render(MeatSpace* meat_space);
vec2 mouse_world_position(MeatSpace* meat_space);

#endif /* end of include guard: MEAT_SPACE_H__ */
