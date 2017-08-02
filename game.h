#ifndef GAME_H__
#define GAME_H__

#include "platform.h"
#include "assets.h"
#include "meat_space.h"
#include "memory.h"

const float PX_PER_PIXEL = 4.0f;

struct MeatSpace;
struct AssetManager;
struct AssetEditor;
struct EntityEditor;

#ifdef FALL_INTERNAL
reflectable enum EditorMode {
  EditorMode_none,
  EditorMode_assets,
  EditorMode_entities,
};
#endif

reflectable struct MeatSpaceEntityTemplateCollection {
  MeatSpaceEntityTemplate* templates;
  i32 templates_count;

  CollisionVolume* collision_volumes;
  i32 collision_volumes_count;
};

reflectable struct GameState {
  b32 initialized;

  i64 highest_entity_id;

  MeatSpace* TMP_meat_space;
  AssetManager* asset_manager;
  f32 perlin_scale;

  MeatSpaceEntityTemplateCollection meat_space_entity_templates;

#ifdef FALL_INTERNAL
  AssetEditor* asset_editor;
  EntityEditor* entity_editor;
  PlatformTexture dummy_texture;
  b32 show_grid;
  b32 show_occupied_spaces;
  EditorMode editor_mode;
  MeatSpaceEntityTemplateId pending_entity_placement_id;
  b32 draw_path_finding;
  b32 draw_collision_volumes;
  b32 step_through_frames;
  SelectionGroup pending_entity_placement_selection_group;
  u32 pending_entity_variation_number;
#endif

  Allocator allocator;
};

reflectable struct TransientState {
  Allocator allocator;
};

inline size_t game_memory_used() {
  return g_game_state->allocator.used;
}

#define game_alloc(type) (type*)game_alloc_(sizeof(type))
#define game_alloc_array(type, count) (type*)game_alloc_(sizeof(type) * (count))
inline void* game_alloc_(size_t size) {
  return allocator_alloc_(&g_game_state->allocator, size);
}

inline size_t transient_memory_used() {
  return g_transient_state->allocator.used;
}

#define transient_alloc(type) (type*)transient_alloc_(sizeof(type))
#define transient_alloc_array(type, count) (type*)transient_alloc_(sizeof(type) * (count))
inline void* transient_alloc_(size_t size) {
  return allocator_alloc_(&g_transient_state->allocator, size);
}

inline void reset_transient_memory() {
  reset_allocator(&g_transient_state->allocator);
}

char* tprintf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int size = vsnprintf(NULL, 0, "%s", args);
  char * a = transient_alloc_array(char, size + 1);
  vsnprintf(a, size + 1, "%s", args);

  va_end(args);

  return a;
}

#endif /* end of include guard: GAME_H__ */
