#include <assert.h>
#include <math.h>
#include <minwindef.h>

#include "asset_manager.h"
#include "debug.h"
#include "game_math.h"
#include "imgui_extensions.h"
#include "meat_space.h"
#include "memory.h"
#include "render_commands.h"
#include "xxhash.h"
#include "rnd.h"

MeatSpaceEntity* pick_entity(MeatSpace* meat_space, vec2 p) {
  // TODO(doug): this would be much faster with an AABB tree or something
  for (i32 i = 0; i < meat_space->entities_count; ++i) {
    auto entity = meat_space->entities + i;

    if (is_in_rect(p, entity->selection_bounds + entity->p)) {
      return entity;
    }
  }

  return NULL;
}

MeatSpaceEntity* get_entity_by_id(MeatSpace* meat_space, i64 id) {
  MeatSpaceEntity* result = NULL;
  // TODO(doug): this would be much faster with a hash table or something
  for (i32 i = 0; i < meat_space->entities_count; ++i) {
    auto entity = meat_space->entities + i;
    if (entity->id == id) {
      result = entity;
      break;
    }
  }

  return result;
}

vec2 viewport_to_world_position(MeatSpace* meat_space, vec2 viewport_position) {
  f32 viewport_width = meat_space->camera.viewport_right - meat_space->camera.viewport_left;
  f32 viewport_height = meat_space->camera.viewport_top - meat_space->camera.viewport_bottom;
  f32 x_norm = _clamp(viewport_position.x / viewport_width, 0.0f, 1.0f) - 0.5f;
  f32 y_norm = _clamp(viewport_position.y / viewport_height, 0.0f, 1.0f) - 0.5f;
  f32 world_x = x_norm * meat_space->camera.scale.x + meat_space->camera.position.x;
  f32 world_y = y_norm * meat_space->camera.scale.y + meat_space->camera.position.y;
  return vec2{ world_x, world_y };
}

f32 max_world_z(MeatSpace* meat_space) {
  return 0.5f * meat_space->camera.scale.y + meat_space->camera.position.y;
}

f32 min_world_z(MeatSpace* meat_space) {
  return -0.5f * meat_space->camera.scale.y + meat_space->camera.position.y;
}

vec2 mouse_world_position(MeatSpace* meat_space) {
  f32 viewport_x = g_input->mouse.x - meat_space->camera.viewport_left;
  f32 viewport_y = g_input->mouse.y - meat_space->camera.viewport_bottom;
  return viewport_to_world_position(meat_space, vec2{ viewport_x, viewport_y });
}

void set_cell_occupied(MeatSpace* meat_space, i32 entity_index, GridCell cell) {
  i32* position_grid = meat_space->position_grid;
  PositionGridBucketNode* position_grid_buckets = meat_space->position_grid_buckets;
  i32 x = cell.x & POSITION_GRID_WIDTH - 1;
  i32 y = cell.y & POSITION_GRID_WIDTH - 1;

  i32 our_bucket_plus_one = ++meat_space->position_grid_bucket_index;
  assert(our_bucket_plus_one < POSITION_GRID_BUCKETS_SIZE);
  position_grid_buckets[our_bucket_plus_one - 1].entity_index_plus_one = entity_index + 1;
  i32* target_bucket_plus_one = &position_grid[y * POSITION_GRID_WIDTH + x];
  while (*target_bucket_plus_one) {
    target_bucket_plus_one = &position_grid_buckets[*target_bucket_plus_one - 1].next_bucket_plus_one;
  }
  *target_bucket_plus_one = our_bucket_plus_one;
}

inline void refresh_collision_box(MeatSpace* meat_space, i32 entity_index, CollisionVolume_box box) {
  auto entity = meat_space->entities + entity_index;


  f32 left = box.offset.x + entity->p.x - box.dimensions.x / 2.0f;
  f32 right = box.offset.x + entity->p.x + box.dimensions.x / 2.0f;
  f32 bottom = box.offset.y + entity->p.y - box.dimensions.y / 2.0f;
  f32 top = box.offset.y + entity->p.y + box.dimensions.y / 2.0f;

  i32 min_x = _floor((left - meat_space->grid.origin.x) / meat_space->grid.cell_width);
  i32 max_x = _floor((right - meat_space->grid.origin.x) / meat_space->grid.cell_width) + 1;
  i32 min_y = _floor((bottom - meat_space->grid.origin.y) / meat_space->grid.cell_width);
  i32 max_y = _floor((top - meat_space->grid.origin.y) / meat_space->grid.cell_width) + 1;

  for (i32 x = min_x; x < max_x; x++) {
    for (i32 y = min_y; y < max_y; y++) {
      set_cell_occupied(meat_space, entity_index, GridCell{ x, y });
    }
  }
}

void refresh_collision_circle(MeatSpace* meat_space, i32 entity_index, CollisionVolume_circle circle) {
  auto entity = meat_space->entities + entity_index;

  f32 left = circle.offset.x + entity->p.x - circle.radius;
  f32 right = circle.offset.x + entity->p.x + circle.radius;
  f32 bottom = circle.offset.y + entity->p.y - circle.radius;
  f32 top = circle.offset.y + entity->p.y + circle.radius;

  i32 min_x = _floor((left - meat_space->grid.origin.x) / meat_space->grid.cell_width);
  i32 max_x = _floor((right - meat_space->grid.origin.x) / meat_space->grid.cell_width) + 1;
  i32 min_y = _floor((bottom - meat_space->grid.origin.y) / meat_space->grid.cell_width);
  i32 max_y = _floor((top - meat_space->grid.origin.y) / meat_space->grid.cell_width) + 1;

  for (i32 x = min_x; x < max_x; x++) {
    for (i32 y = min_y; y < max_y; y++) {
      set_cell_occupied(meat_space, entity_index, GridCell{ x, y });
    }
  }
}

void refresh_collision_volume(MeatSpace* meat_space, i32 i, CollisionVolume volume) {
  switch (volume.type) {
  case CollisionVolume_Type_none: break;
  case CollisionVolume_Type_box: {
    refresh_collision_box(meat_space, i, volume.box);
  } break;
  case CollisionVolume_Type_circle: {
    refresh_collision_circle(meat_space, i, volume.circle);
  } break;
  case CollisionVolume_Type_common: break;
  case CollisionVolume_Type_elipse: break;
  default: ;
  }
}

CollisionVolume* get_collision_volumes(MeatSpace* meat_space, i32 entity_index, i32* count) {
  auto entity = meat_space->entities + entity_index;

  i32 index = entity->collision_volumes;
  CollisionVolume * result = meat_space->collision_volumes + index;
  while (index < meat_space->collision_volumes_count &&
          meat_space->collision_volumes[index].common.entity_id == entity->id) {
    index++;
  }
  *count = index - entity->collision_volumes;
  return result;
}

void refresh_collision_volumes(MeatSpace* meat_space, i32 entity_index) {
  i32 count;
  auto volumes = get_collision_volumes(meat_space, entity_index, &count);
  for (i32 j = 0; j < count; j++) {
    auto volume = volumes[j];
    refresh_collision_volume(meat_space, entity_index, volume);
  }
}

void refresh_position_grid(MeatSpace* meat_space) {
  ZERO_STRUCT(meat_space->position_grid_buckets);
  ZERO_STRUCT(meat_space->position_grid);
  meat_space->position_grid_bucket_index = 0;

  for (i32 i = 0; i < meat_space->entities_count; ++i) {
    auto entity = meat_space->entities + i;
    if (has_flag(entity->flags, ENTITY_FLAG_TRAVERSABLE)) {
      continue;
    }

    refresh_collision_volumes(meat_space, i);
  }
}

i32 get_nontraversable_entities_near_cell(MeatSpace* meat_space, GridCell cell, i32* buffer, i32 buffer_size) {
  i32* position_grid = meat_space->position_grid;
  PositionGridBucketNode* position_grid_buckets = meat_space->position_grid_buckets;
  i32 count = 0;
  i32 x_mod = cell.x & POSITION_GRID_WIDTH - 1;
  i32 y_mod = cell.y & POSITION_GRID_WIDTH - 1;

  i32 bucket_plus_one = position_grid[y_mod * POSITION_GRID_WIDTH + x_mod];
  while (bucket_plus_one) {
    auto bucket = position_grid_buckets[bucket_plus_one - 1];
    if (count < buffer_size) {
      buffer[count] = bucket.entity_index_plus_one - 1;
    }
    bucket_plus_one = bucket.next_bucket_plus_one;
    count++;
  }

  return count;
}

i32 get_nearest_enemy(MeatSpace* meat_space, i32 entity) {
  assert(meat_space->entities[entity].selection_group == SelectionGroup_player ||
         meat_space->entities[entity].selection_group == SelectionGroup_enemy);
  auto enemy_group = (meat_space->entities[entity].selection_group == SelectionGroup_player) ?
    SelectionGroup_enemy : SelectionGroup_player;
  f32 smallest_distance_sq = FLT_MAX;
  i32 result = -1;
  vec2 p = meat_space->entities[entity].p;
  for (i32 i = 0; i < meat_space->entities_count; i++) {
    if (i == entity) {
      continue;
    }

    auto enemy = meat_space->entities[i];
    if (has_flag(enemy.flags, ENTITY_FLAG_CHARACTER) && enemy.selection_group == enemy_group) {
      f32 dist_sq = magnitude_sq(p - enemy.p);
      if (dist_sq < smallest_distance_sq) {
        smallest_distance_sq = dist_sq;
        result = i;
      }
    }
  }

  return result;
}

b32 is_cell_occupied(MeatSpace* meat_space, GridCell cell, i32 ignore_me, b32 exclude_moving) {
  i32 buffer[16];
  i32 count = get_nontraversable_entities_near_cell(meat_space, cell, buffer, ARRAY_LENGTH(buffer));
  assert(count <= ARRAY_LENGTH(buffer));
  for (i32 i = 0; i < count; i++) {
    if (buffer[i] == ignore_me) {
      continue;
    }
    auto entity = meat_space->entities + buffer[i];

    i32 volumes_count;
    auto volumes = get_collision_volumes(meat_space, buffer[i], &volumes_count);
    auto top_right = cell_top_right(meat_space->grid, cell);
    auto bottom_left = cell_bottom_left(meat_space->grid, cell);

    for (i32 j = 0; j < volumes_count; j++) {
      auto volume = volumes[j];
      vec2 volume_center = vec2{
        volume.common.offset.x + entity->p.x,
        volume.common.offset.y + entity->p.y,
      };

      if (is_in_rect(volume_center, rect2{ bottom_left, top_right })) {
        return true;
      }

      switch (volume.type) {
      case CollisionVolume_Type_box: {
        f32 left = volume.box.offset.x + entity->p.x - volume.box.dimensions.x / 2.0f;
        f32 right = volume.box.offset.x + entity->p.x + volume.box.dimensions.x / 2.0f;
        f32 bottom = volume.box.offset.y + entity->p.y - volume.box.dimensions.y / 2.0f;
        f32 top = volume.box.offset.y + entity->p.y + volume.box.dimensions.y / 2.0f;
        if (top_right.x > left && bottom_left.x < right && top_right.y > bottom && bottom_left.y < top) {
          return true;
        }
      } break;
      case CollisionVolume_Type_circle: {
        f32 radius_sq = volume.circle.radius * volume.circle.radius;
        if (magnitude_sq(volume_center - top_right) < radius_sq ||
            magnitude_sq(volume_center - bottom_left) < radius_sq ||
            magnitude_sq(volume_center - vec2{ top_right.x, bottom_left.y }) < radius_sq ||
            magnitude_sq(volume_center - vec2{ bottom_left.x, top_right.y }) < radius_sq) {
          return true;
        }
      } break;
      case CollisionVolume_Type_none: break;
      case CollisionVolume_Type_common: break;
      case CollisionVolume_Type_elipse: break;
      default:;
      }
    }
  }

  return false;
}

b32 can_volume_fit_at_cell(MeatSpace* meat_space,
                           CollisionVolume volume,
                           GridCell cell,
                           i32 ignore_me,
                           b32 exclude_moving) {
  vec2 cell_position = cell_to_position(meat_space->grid, cell);
  switch (volume.type) {
  case CollisionVolume_Type_box: {
    auto box = volume.box;
    f32 left = box.offset.x + cell_position.x - box.dimensions.x / 2.0f;
    f32 right = box.offset.x + cell_position.x + box.dimensions.x / 2.0f;
    f32 bottom = box.offset.y + cell_position.y - box.dimensions.y / 2.0f;
    f32 top = box.offset.y + cell_position.y + box.dimensions.y / 2.0f;

    i32 min_x = _floor((left - meat_space->grid.origin.x) / meat_space->grid.cell_width) + 1;
    i32 max_x = _floor((right - meat_space->grid.origin.x) / meat_space->grid.cell_width) + 1;
    i32 min_y = _floor((bottom - meat_space->grid.origin.y) / meat_space->grid.cell_width) + 1;
    i32 max_y = _floor((top - meat_space->grid.origin.y) / meat_space->grid.cell_width) + 1;

    for (i32 x = min_x; x < max_x; x++) {
      for (i32 y = min_y; y < max_y; y++) {
        if (is_cell_occupied(meat_space, GridCell{x,y}, ignore_me, exclude_moving)) {
          return false;
        }
      }
    }

    return true;
  } break;
  case CollisionVolume_Type_circle: {
    auto circle = volume.circle;
    f32 left = circle.offset.x + cell_position.x - circle.radius;
    f32 right = circle.offset.x + cell_position.x + circle.radius;
    f32 top = circle.offset.y + cell_position.y + circle.radius;
    f32 bottom = circle.offset.y + cell_position.y - circle.radius;

    i32 min_x = _floor((left - meat_space->grid.origin.x) / meat_space->grid.cell_width);
    i32 max_x = _floor((right - meat_space->grid.origin.x) / meat_space->grid.cell_width) + 1;
    i32 min_y = _floor((bottom - meat_space->grid.origin.y) / meat_space->grid.cell_width);
    i32 max_y = _floor((top - meat_space->grid.origin.y) / meat_space->grid.cell_width) + 1;

    for (i32 x = min_x; x < max_x; x++) {
      for (i32 y = min_y; y < max_y; y++) {
        if (is_cell_occupied(meat_space, GridCell{ x,y }, ignore_me, exclude_moving)) {
          return false;
        }
      }
    }
    return true;
  } break;
  case CollisionVolume_Type_none: break;
  case CollisionVolume_Type_common: break;
  case CollisionVolume_Type_elipse: break;
  default:;
  }

  assert(false);
  return false;
}

b32 can_entity_fit_at_cell(MeatSpace* meat_space,
                           i32 entity_index,
                           GridCell cell,
                           b32 exclude_moving) {
  i32 collision_volumes_count;
  auto collision_volumes = get_collision_volumes(meat_space, entity_index, &collision_volumes_count);
  for (i32 i = 0; i < collision_volumes_count; i++) {
    if (!can_volume_fit_at_cell(meat_space, collision_volumes[i], cell, entity_index, exclude_moving)) {
      return false;
    }
  }
  return true;
}

struct AStarNode {
  GridCell cell;
  b32 exists;
  i32 came_from;
  f32 cost_to_get_here;
  f32 est_cost_to_pass_through_here;
  i32 next_plus_one;
  b32 closed;
  i32 open_heap_index;
};

struct AStarWorkingMemory {
  AStarNode nodes[16384];
  i32 nodes_count;

  i32 open_size;
  i32 open[16384];
};

inline i32 open_heap_parent(i32 child) {
  return (child - 1) / 2;
}

inline i32 open_heap_left_child(i32 child) {
  return child * 2 + 1;
}

inline f32 open_heap_score(AStarWorkingMemory* m, i32 index) {
  return m->nodes[m->open[index]].est_cost_to_pass_through_here;
}

inline void open_heap_swap(AStarWorkingMemory* m, i32 a, i32 b) {
  i32 tmp = m->open[a];
  m->open[a] = m->open[b];
  m->open[b] = tmp;
  m->nodes[m->open[a]].open_heap_index = b;
  m->nodes[m->open[b]].open_heap_index = a;
}

inline i32 open_heap_sink(AStarWorkingMemory* m, i32 cursor) {
  while (true) {
    i32 left_child = open_heap_left_child(cursor);
    i32 right_child = left_child + 1;

    if (left_child >= m->open_size) {
      break;
    } else if (right_child >= m->open_size) { // we only have a left child
      if (open_heap_score(m, cursor) >
          open_heap_score(m, left_child)) {
        open_heap_swap(m, cursor, left_child);
      }
      break;
    }

    i32 lesser_child =
      open_heap_score(m, left_child) <
      open_heap_score(m, right_child) ?
      left_child : right_child;
    if (open_heap_score(m, cursor) >
        open_heap_score(m, lesser_child)) {
      open_heap_swap(m, cursor, lesser_child);
      cursor = lesser_child;
    } else {
      break;
    }
  }
  return cursor;
}

i32 open_heap_sift(AStarWorkingMemory* m, i32 index) {
  f32 score = open_heap_score(m, index);
  while (index) {
    i32 parent = open_heap_parent(index);
    if (open_heap_score(m, parent) > score) {
      open_heap_swap(m, index, parent);
      index = parent;
    } else {
      break;
    }
  }
  return index;
}

i32 open_heap_insert(AStarWorkingMemory* m, i32 node) {
  i32 index = m->open_size;
  m->open[index] = node;
  m->open_size++;
  return open_heap_sift(m, index);
}

i32 open_heap_recalc(AStarWorkingMemory* m, i32 index) {
  index = open_heap_sift(m, index);
  return open_heap_sink(m, index);
}

i32 open_heap_delete_min(AStarWorkingMemory* m) {
  if (m->open_size == 1) {
    i32 result = m->open[0];
    m->open_size--;
    return result;
  }
  i32 result = m->open[0];
  i32 last = m->open[--m->open_size];
  m->open[0] = last;

  open_heap_sink(m, 0);
  return result;
}

i32 node_map_add(AStarWorkingMemory* m, GridCell cell) {
  assert(++m->nodes_count < ARRAY_LENGTH(m->nodes) / 2);
  i32 hash = XXH32(&cell, sizeof(cell), 0) & 4095;
  i32 index = hash;
  while (m->nodes[index].next_plus_one) {
    index = m->nodes[index].next_plus_one - 1;
  }
  auto prev_closed = index;
  while (m->nodes[index].closed) {
    index++;
    assert((index & 4095) != prev_closed);
  }
  if (prev_closed != index) {
    m->nodes[prev_closed].next_plus_one = (index & 4095) + 1;
  }
  m->nodes[index & 4095].cell = cell;
  m->nodes[index & 4095].exists = true;
  return index & 4095;
}

i32 node_map_get(AStarWorkingMemory* m, GridCell cell) {
  i32 index = XXH32(&cell, sizeof(cell), 0) & 4095;
  if (m->nodes[index].exists && cell == m->nodes[index].cell) {
    return index;
  }
  while (m->nodes[index].next_plus_one) {
    index = m->nodes[index].next_plus_one - 1;
    if (m->nodes[index].exists && cell == m->nodes[index].cell) {
      return index;
    }
  }

  return -1;
}

b32 find_path(MeatSpace* meat_space,
              GridCell start, GridCell end,
              i32 entity_index, GridCell* next) {
  AStarWorkingMemory m = {};

  vec2 end_world_space = cell_to_position(meat_space->grid, end);
  vec2 start_world_space = cell_to_position(meat_space->grid, start);
  // TODO(doug): cell_width isn't correct if this is at an angle. Do some trig.
  vec2 travel_by = normalize(start_world_space - end_world_space) * meat_space->grid.cell_width;

  while (!can_entity_fit_at_cell(meat_space, entity_index, end, true)) {
    end_world_space = end_world_space + travel_by;
    end = position_to_cell(meat_space->grid, end_world_space);
  }

  if (start == end) {
    return false;
  }

  i32 start_node = node_map_add(&m, start);
  open_heap_insert(&m, start_node);
  m.nodes[start_node].est_cost_to_pass_through_here = cell_distance(start, end);
  m.nodes[start_node].came_from = -1;

  f32 sqrt2 = sqrtf(2.0f);
  i32 break_after = 1024;
  while (m.open_size && --break_after) {
    for (i32 i = 0; i < m.open_size; i++) {
      assert(m.open[i] < ARRAY_LENGTH(m.nodes));
    }
    i32 current = open_heap_delete_min(&m);
    for (i32 i = 0; i < m.open_size; i++) {
      assert(m.open[i] < ARRAY_LENGTH(m.nodes));
    }
    auto current_cell = m.nodes[current].cell;
    i32 added = node_map_get(&m, current_cell);
    m.nodes[added].closed = true;

#ifdef FALL_INTERNAL
    if (g_game_state->draw_path_finding) {
      vec2 p = cell_to_position(meat_space->grid, m.nodes[current].cell);
      f32 left = p.x - 1.0f;
      f32 right = p.x + 1.0f;
      f32 bottom = p.y - 1.0f;
      f32 top = p.y + 1.0f;
      f32 z = min_world_z(meat_space);
      f32 color = 0xffffff00;
      push_quad_frame(g_render_commands,
                      left, bottom, z, color,
                      left, top, z, color,
                      right, top, z, color,
                      right, bottom, z, color);
    }
#endif

    if (m.nodes[current].cell == end) {
      i32 previous = m.nodes[current].came_from;
      while (previous != start_node) {
        if (g_game_state->draw_path_finding) {
          vec2 p = cell_to_position(meat_space->grid, m.nodes[previous].cell);
          f32 left = p.x - 1.0f;
          f32 right = p.x + 1.0f;
          f32 bottom = p.y - 1.0f;
          f32 top = p.y + 1.0f;
          f32 z = min_world_z(meat_space) - 0.2f;
          f32 color = 0xff00ff00;
          push_quad_frame(g_render_commands,
                          left, bottom, z, color,
                          left, top, z, color,
                          right, top, z, color,
                          right, bottom, z, color);
        }

        current = previous;
        assert(m.nodes[current].came_from != current);
        previous = m.nodes[current].came_from;
      }
      *next = m.nodes[current].cell;
      return true;
    }

    for (i32 x = -1; x < 2; x++) {
      for (i32 y = -1; y < 2; y++) {
        if (!x && !y) {
          continue;
        }

        auto cell = GridCell{ current_cell.x + x, current_cell.y + y };
        if (cell != end) {
          if (!can_entity_fit_at_cell(meat_space, entity_index, cell, true)) {
            continue;
          }
        }
        auto diagonal = x && y;
        if (diagonal) {
          auto cell_1 = GridCell{ current_cell.x + x, current_cell.y };
          if (!can_entity_fit_at_cell(meat_space, entity_index, cell_1, true)) {
            continue;
          }
          auto cell_2 = GridCell{ current_cell.x, current_cell.y + y };
          if (!can_entity_fit_at_cell(meat_space, entity_index, cell_2, true)) {
            continue;
          }
        }

        f32 dist = x && y ? sqrt2 : 1.0f;
        f32 cost = m.nodes[current].cost_to_get_here + dist;
        i32 existing = node_map_get(&m, cell);
        if (existing == -1) {
          existing = node_map_add(&m, cell);
          assert(existing >= 0 && existing < ARRAY_LENGTH(m.nodes));
#ifdef FALL_INTERNAL
          if (g_game_state->draw_path_finding) {
            vec2 p = cell_to_position(meat_space->grid, cell);
            f32 left = p.x - 1.0f;
            f32 right = p.x + 1.0f;
            f32 bottom = p.y - 1.0f;
            f32 top = p.y + 1.0f;
            f32 z = min_world_z(meat_space) + 0.1f;
            f32 color = 0xffff0000;
            push_quad_frame(g_render_commands,
                            left, bottom, z, color,
                            left, top, z, color,
                            right, top, z, color,
                            right, bottom, z, color);
          }
#endif
          m.nodes[existing].came_from = current;
          m.nodes[existing].cost_to_get_here = cost;
          m.nodes[existing].est_cost_to_pass_through_here = cost * 0.9999f + (f32)cell_taxicab_distance(cell, end);
          m.nodes[existing].open_heap_index = open_heap_insert(&m, existing);
          for (i32 i = 0; i < m.open_size; i++) {
            assert(m.open[i] < ARRAY_LENGTH(m.nodes));
          }
        } else {
          if (m.nodes[existing].closed) {
            continue;
          }

          if (cost >= m.nodes[existing].cost_to_get_here) {
            continue;
          }

          m.nodes[existing].came_from = current;
          m.nodes[existing].cost_to_get_here = cost;
          m.nodes[existing].est_cost_to_pass_through_here = cost * 0.9999f + (f32)cell_taxicab_distance(cell, end);
          m.nodes[existing].open_heap_index = open_heap_recalc(&m, m.nodes[existing].open_heap_index);
          for (i32 i = 0; i < m.open_size; i++) {
            assert(m.open[i] < ARRAY_LENGTH(m.nodes));
          }
        }
      }
    }
  }

  return false;
}

inline MeatSpaceProjectile* create_projectile(MeatSpace* meat_space,
                                              MeatSpaceProjectile projectile) {
  MeatSpaceProjectile* result;
  if (meat_space->projectiles_free_stack_count > 0) {
    i32 i = meat_space->projectiles_free_stack[--meat_space->projectiles_free_stack_count];
#ifdef FALL_INTERNAL
    meat_space->projectiles_free_stack[meat_space->projectiles_free_stack_count] = -1;
#endif
    assert(i >= 0);
    result = &meat_space->projectiles[i];
  } else {
    assert(meat_space->projectiles_count + 1 < ARRAY_LENGTH(meat_space->projectiles));
    result = &meat_space->projectiles[meat_space->projectiles_count++];
  }
  *result = projectile;
  return result;
}

inline void destroy_projectile(MeatSpace* meat_space, i32 projectile) {
  meat_space->projectiles[projectile].type = MeatSpaceProjectile_Type_none;
  if (projectile + 1 == meat_space->projectiles_count) {
    meat_space->projectiles_count--;
  } else {
    assert(meat_space->projectiles_free_stack_count < ARRAY_LENGTH(meat_space->projectiles_free_stack) - 1);
    meat_space->projectiles_free_stack[meat_space->projectiles_free_stack_count++] = projectile;
  }
}

MeatSpaceEntityTemplate* get_entity_template(MeatSpace* meat_space, MeatSpaceEntityTemplateId template_id) {
  MeatSpaceEntityTemplate* result = NULL;
  for (i32 i = 0; i < meat_space->template_collection.templates_count; i++) {
    auto t_candidate = meat_space->template_collection.templates + i;
    if (t_candidate->id == template_id) {
      result = t_candidate;
      break;
    }
  }
  return result;
}

MeatSpaceEntity* create_entity_from_template(MeatSpace* meat_space,
                                             MeatSpaceEntityTemplateId template_id,
                                             i32 variation_number) {
  assert(meat_space->entities_count + 1 < ARRAY_LENGTH(meat_space->entities));
  auto result = &meat_space->entities[meat_space->entities_count++];
  memset(result, 0, sizeof(*result));
  auto t = get_entity_template(meat_space, template_id);

  assert(t);

  result->id = ++g_game_state->highest_entity_id;
  result->flags = t->flags;
  result->selection_bounds = t->selection_bounds;
  result->asset_type = t->asset_type;
  result->asset_attributes.tracking_id = result->id;
  if (variation_number == -1) {
    variation_number = rnd_pcg_next(&meat_space->pcg);
  }
  result->asset_attributes.variation_number = variation_number;
  i32 initial_count = meat_space->collision_volumes_count;
  if (t->collision_boxes != -1) {
    i32 box = t->collision_boxes;
    while (box < meat_space->template_collection.collision_volumes_count &&
           meat_space->template_collection.collision_volumes[box].common.entity_id == t->id) {
      auto volume = &meat_space->collision_volumes[meat_space->collision_volumes_count++];
      *volume = meat_space->template_collection.collision_volumes[box];
      volume->common.entity_id = result->id;
      box++;
    }
    result->collision_volumes = initial_count;
  } else {
    auto volume = &meat_space->collision_volumes[meat_space->collision_volumes_count++];
    volume->type = CollisionVolume_Type_box;
    volume->box.entity_id = result->id;
    volume->box.dimensions = vec2{ meat_space->grid.cell_width, meat_space->grid.cell_width };
    volume->box.offset = vec2{};
    result->collision_volumes = initial_count;
  }
  result->target_center = t->target_center;
  result->firing_center = t->firing_center;
  result->z_bias = t->z_bias;
  result->z_bias += rnd_pcg_nextf(&meat_space->pcg) * 0.05f;

  result->max_health = result->health = t->max_health;
  result->weapon = t->weapon;
  return result;
}

void draw_grid(MeatSpace* meat_space, Grid grid) {
  AssetAttributes attrs = {};
  attrs.direction = AssetDirection_horizontal;
  auto horizontal_line = assets_get_texture(AssetType_selection_line, attrs);
  attrs.direction = AssetDirection_vertical;
  auto vertical_line = assets_get_texture(AssetType_selection_line, attrs);

  vec2 bottom_left = meat_space->camera.position - meat_space->camera.scale * 0.5;
  vec2 top_right = bottom_left + meat_space->camera.scale;
  f32 grid_z = min_world_z(meat_space);
  f32 line_width = 0.25f;
  for (f32 x = bottom_left.x; x < top_right.x; x += grid.cell_width) {
    f32 right = cell_right(grid, position_to_cell(grid, vec2{ x, 0.0f }));
    f32 left = right - line_width;
    f32 top = top_right.y;
    f32 bottom = bottom_left.y;
    u32 color = 0x44000000;
    push_textured_quad(g_render_commands,
                       vertical_line.handle,
                       vertical_line.left, vertical_line.bottom, left, bottom, grid_z, color,
                       vertical_line.right, vertical_line.bottom, right, bottom, grid_z, color,
                       vertical_line.left, vertical_line.top, left, top, grid_z, color,
                       vertical_line.right, vertical_line.top, right, top, grid_z, color);
  }
  for (f32 y = bottom_left.y; y < top_right.y; y += grid.cell_width) {
    f32 right = top_right.x;
    f32 left = bottom_left.x;
    auto cell = position_to_cell(grid, vec2{ 0.0f, y });
    f32 top = cell_top(grid, cell) + line_width / 2.0f;
    f32 bottom = top - line_width;
    u32 color = 0x44000000;
    push_textured_quad(g_render_commands,
                       horizontal_line.handle,
                       horizontal_line.left, horizontal_line.bottom, left, bottom, grid_z, color,
                       horizontal_line.right, horizontal_line.bottom, right, bottom, grid_z, color,
                       horizontal_line.left, horizontal_line.top, left, top, grid_z, color,
                       horizontal_line.right, horizontal_line.top, right, top, grid_z, color);
  }
}

void draw_selection_rect(MeatSpace* meat_space, rect2 selection_rect) {
  AssetAttributes attrs = {};
  attrs.direction = AssetDirection_horizontal;
  auto horizontal_line = assets_get_texture(AssetType_selection_line, attrs);
  attrs.direction = AssetDirection_vertical;
  auto vertical_line = assets_get_texture(AssetType_selection_line, attrs);

  f32 selection_z = min_world_z(meat_space);
  {
    // top line
    f32 left = selection_rect.bottom_left.x;
    f32 right = selection_rect.top_right.x - 0.5f;
    f32 top = selection_rect.top_right.y;
    f32 bottom = top - 0.5f;
    push_textured_quad(g_render_commands,
                       horizontal_line.handle,
                       horizontal_line.left, horizontal_line.bottom, left, bottom, selection_z, 0xffffffff,
                       horizontal_line.right, horizontal_line.bottom, right, bottom, selection_z, 0xffffffff,
                       horizontal_line.left, horizontal_line.top, left, top, selection_z, 0xffffffff,
                       horizontal_line.right, horizontal_line.top, right, top, selection_z, 0xffffffff);
  }

  {
    // bottom line
    f32 left = selection_rect.bottom_left.x + 0.5f;
    f32 right = selection_rect.top_right.x;
    f32 bottom = selection_rect.bottom_left.y;
    f32 top = bottom + 0.5f;
    push_textured_quad(g_render_commands,
                       horizontal_line.handle,
                       horizontal_line.left, horizontal_line.bottom, right, bottom, selection_z, 0xffffffff,
                       horizontal_line.right, horizontal_line.bottom, left, bottom, selection_z, 0xffffffff,
                       horizontal_line.left, horizontal_line.top, right, top, selection_z, 0xffffffff,
                       horizontal_line.right, horizontal_line.top, left, top, selection_z, 0xffffffff);
  }

  {
    // left line
    f32 left = selection_rect.bottom_left.x;
    f32 right = left + 0.5f;
    f32 top = selection_rect.top_right.y - 0.5f;
    f32 bottom = selection_rect.bottom_left.y;
    push_textured_quad(g_render_commands,
                       vertical_line.handle,
                       vertical_line.left, vertical_line.bottom, left, bottom, selection_z, 0xffffffff,
                       vertical_line.right, vertical_line.bottom, right, bottom, selection_z, 0xffffffff,
                       vertical_line.left, vertical_line.top, left, top, selection_z, 0xffffffff,
                       vertical_line.right, vertical_line.top, right, top, selection_z, 0xffffffff);
  }

  {
    // right line
    f32 right = selection_rect.top_right.x;
    f32 left = right - 0.5f;
    f32 top = selection_rect.top_right.y;
    f32 bottom = selection_rect.bottom_left.y + 0.5f;
    push_textured_quad(g_render_commands,
                       vertical_line.handle,
                       vertical_line.left, vertical_line.bottom, left, top, selection_z, 0xffffffff,
                       vertical_line.right, vertical_line.bottom, right, top, selection_z, 0xffffffff,
                       vertical_line.left, vertical_line.top, left, bottom, selection_z, 0xffffffff,
                       vertical_line.right, vertical_line.top, right, bottom, selection_z, 0xffffffff);
  }
}

void draw_health(vec2 p, f32 health, f32 max_health) {
  auto tex = assets_get_texture(AssetType_square, {});
  f32 base_left = p.x - max_health / 2.0f;
  for (i32 i = 0; i < (i32)max_health; i++) {
    f32 left = base_left + (f32)i;
    f32 right = left + 1.0f;
    f32 bottom = p.y - 4.0f;
    f32 top = bottom + 1.0f;
    f32 z = p.y - 0.1f;
    auto color = health > (f32)i ? 0xff0000ff : 0xaa000000;
    push_textured_quad(g_render_commands,
                       tex.handle,
                       tex.left, tex.bottom, left, bottom, z, color,
                       tex.right, tex.bottom, right, bottom, z, color,
                       tex.left, tex.top, left, top, z, color,
                       tex.right, tex.top, right, top, z, color);
  }
}

void draw_collision_volumes(MeatSpace* meat_space, MeatSpaceEntity* entity) {
  if (entity->collision_volumes != -1) {
    i32 index = entity->collision_volumes;
    while (index < meat_space->collision_volumes_count &&
      meat_space->collision_volumes[index].common.entity_id == entity->id) {
      f32 z = entity->p.y + entity->z_bias - 0.1f;
      u32 color = 0xffffffff;
      auto volume = meat_space->collision_volumes[index];
      switch (volume.type) {
      case CollisionVolume_Type_box: {
        auto box = volume.box;

        f32 left = box.offset.x + entity->p.x - volume.box.dimensions.x / 2.0f;
        f32 right = box.offset.x + entity->p.x + volume.box.dimensions.x / 2.0f;
        f32 bottom = box.offset.y + entity->p.y - volume.box.dimensions.y / 2.0f;
        f32 top = box.offset.y + entity->p.y + volume.box.dimensions.y / 2.0f;

        push_quad_frame(g_render_commands,
                        left, bottom, z, color,
                        left, top, z, color,
                        right, top, z, color,
                        right, bottom, z, color);
      } break;
      case CollisionVolume_Type_circle: {
        auto circle = volume.circle;
        push_circle_frame(g_render_commands,
                          circle.offset.x + entity->p.x,
                          circle.offset.y + entity->p.y,
                          z,
                          circle.radius,
                          color);
      } break;
      default: ;
      }
      index++;
    }
  }
}

void draw_entity_texture(MeatSpace* meat_space,
                         vec2 position, u32 color,
                         AssetType asset_type, AssetAttributes asset_attributes,
                         f32 z_bias) {
  auto tex = assets_get_texture(asset_type, asset_attributes);
  f32 left = (position.x - tex.anchor_x);
  f32 right = (position.x + tex.px_width - tex.anchor_x);
  f32 bottom = (position.y - tex.anchor_y);
  f32 top = (position.y + tex.px_height - tex.anchor_y);
  f32 z = (position.y) + z_bias;
  if (asset_type == AssetType_tile) {
    auto top_right = snap_to_top_right(meat_space->grid, position);
    auto bottom_left = snap_to_bottom_left(meat_space->grid, position);
    left = bottom_left.x;
    right = top_right.x;
    bottom = bottom_left.y;
    top = top_right.y;
  }

  push_textured_quad(g_render_commands,
                     tex.handle,
                     tex.left, tex.bottom, left, bottom, z, color,
                     tex.right, tex.bottom, right, bottom, z, color,
                     tex.left, tex.top, left, top, z, color,
                     tex.right, tex.top, right, top, z, color);
}

void draw_entity_selection_circle(MeatSpaceEntity* entity) {
  AssetAttributes attrs = {};
  auto tex = assets_get_texture(AssetType_selection_circle, attrs);
  f32 left = (entity->p.x - (f32)tex.anchor_x / 2.0f);
  f32 right = (entity->p.x + (f32)(tex.px_width - tex.anchor_x) / 2.0f);
  f32 bottom = (entity->p.y - (f32)tex.anchor_y / 2.0f);
  f32 top = (entity->p.y + (f32)(tex.px_height - tex.anchor_y) / 2.0f);
  u32 color = entity->selected ? 0xffffffff : 0x77ffffff;
  f32 z = (entity->p.y) + 0.127f;

  push_textured_quad(g_render_commands,
                     tex.handle,
                     tex.left, tex.bottom, left, bottom, z, color,
                     tex.right, tex.bottom, right, bottom, z, color,
                     tex.left, tex.top, left, top, z, color,
                     tex.right, tex.top, right, top, z, color);
}

void meat_space_update_and_render(MeatSpace* meat_space) {
  refresh_position_grid(meat_space);

  f32 dt = g_input->dt;

  b32 player_selecting = false;
  MeatSpaceBrain_player* selecting_player = NULL;
  MeatSpaceEntity* picked_entity = NULL;
  MeatSpaceEntity* selected_entity = NULL;
  rect2 selection_rect = {};
  auto grid = meat_space->grid;

  if (g_game_state->show_grid) {
    draw_grid(meat_space, grid);
  }

  const f32 camera_move_speed = 240.0f;
  if (was_down(g_input->keyboard.w)) {
    meat_space->camera.position.y += camera_move_speed * dt;
  }
  if (was_down(g_input->keyboard.d)) {
    meat_space->camera.position.x += camera_move_speed * dt;
  }
  if (was_down(g_input->keyboard.s)) {
    meat_space->camera.position.y -= camera_move_speed * dt;
  }
  if (was_down(g_input->keyboard.a)) {
    meat_space->camera.position.x -= camera_move_speed * dt;
  }

  for (i32 i = 0; i < meat_space->brains_count; ++i) {
    auto brain = meat_space->brains + i;

    switch (brain->type) {
      case MeatSpaceBrain_Type_player: {
        auto player = &brain->player;
        auto mouse_world = mouse_world_position(meat_space);

        picked_entity = pick_entity(meat_space, mouse_world);

        if (player->mouse_down_entity && !picked_entity) {
          player->mouse_down_entity = NULL;
          player->selecting = true;
        }

        if (was_pressed(g_input->mouse.button_l)) {
          if (picked_entity) {
            player->mouse_down_entity = picked_entity;
          } else {
            player->selecting = true;
          }
          player->selection_start = mouse_world;
        }

        if (was_released(g_input->mouse.button_l)) {
          if (player->selecting) {
            player->selecting = false;
            player->selected_entities_count = 0;
            selecting_player = player;
          } else if (player->mouse_down_entity) {
            selected_entity = player->mouse_down_entity;
            player->selected_entities_count = 1;
            player->selected_entities[0] = player->mouse_down_entity->id;
          }

          player->mouse_down_entity = NULL;
        }

        if (was_pressed(g_input->mouse.button_r)) {
          for (i32 j = 0; j < player->selected_entities_count; j++) {
            auto grid_space = position_to_cell(grid, mouse_world);
            i64 entity_id = player->selected_entities[j];
            auto entity = get_entity_by_id(meat_space, entity_id);
            if (entity && has_flag(entity->flags, ENTITY_FLAG_CHARACTER) &&
                entity->selection_group == SelectionGroup_player) {
              entity->command.type = MeatSpaceCommand_Type_move_to;
              entity->command.move_to.target = grid_space;
              entity->unfinished_move = false;
            }
          }
        }

        player_selecting = player->selecting;

        if (player->selecting || selecting_player) {
          vec2 selection_end = mouse_world;
          selection_rect.bottom_left.x = min(player->selection_start.x, selection_end.x);
          selection_rect.bottom_left.y = min(player->selection_start.y, selection_end.y);
          selection_rect.top_right.x = max(player->selection_start.x, selection_end.x);
          selection_rect.top_right.y = max(player->selection_start.y, selection_end.y);
        }
      } break;
      default: assert(false);
    }
  }

  for (int i = 0; i < meat_space->entities_count; ++i) {
    auto entity = meat_space->entities + i;

    if (has_flag(entity->flags, ENTITY_FLAG_CHARACTER)) {
      if (entity->selection_group == SelectionGroup_player) {
        if (selecting_player) {
          if (is_in_rect(entity->p, selection_rect)) {
            selecting_player->selected_entities[selecting_player->selected_entities_count++] = entity->id;
            entity->selected = true;
          } else {
            entity->selected = false;
          }
        } else if (selected_entity) {
          entity->selected = entity == selected_entity;
        }
      }

      if (entity->command.type == MeatSpaceCommand_Type_move_to) {
        auto command = &entity->command;
        f32 remaining = 40.0f * dt;
        while (remaining > 0.0f) {
          if (!entity->unfinished_move) {
            if (command->move_to.target == position_to_cell(grid, entity->p)) {
              command->type = MeatSpaceCommand_Type_none;
              break;
            }
            auto current_cell = position_to_cell(grid, entity->p);
            GridCell next_target;
            b32 path_found = find_path(meat_space,
                                       current_cell,
                                       command->move_to.target,
                                       i,
                                       &next_target);

            if (!path_found || !can_entity_fit_at_cell(meat_space, i, next_target, false)) {
              break;
            }
            entity->unfinished_move = true;
            entity->unfinished_move_target = next_target;
          } else {
            if (!can_entity_fit_at_cell(meat_space, i, entity->unfinished_move_target, false)) {
              auto current_cell = position_to_cell(grid, entity->p);
              entity->unfinished_move = true;
              entity->unfinished_move_target = current_cell;
            }
          }


          vec2 target = cell_to_position(grid, entity->unfinished_move_target);
          entity->p = move_toward(entity->p, target, 40.0f * dt, &remaining);
          if (remaining > 0.0f) {
            if (entity->unfinished_move_target == command->move_to.target) {
              command->type = MeatSpaceCommand_Type_none;
              remaining = 0.0f;
            }
            entity->unfinished_move = false;
          }
        }
      }

      if (!has_flag(entity->flags, ENTITY_FLAG_DEAD)) {
        if (entity->health <= 0.0f) {
          set_flag(&entity->flags, ENTITY_FLAG_DEAD);
          set_flag(&entity->flags, ENTITY_FLAG_TRAVERSABLE);
          clear_flag(&entity->flags, ENTITY_FLAG_CHARACTER);
        } else {
          if (entity->weapon.type == MeatSpaceWeapon_Type_gun) {
            entity->weapon.gun.time_since_last_fired += dt;

            if (entity->weapon.gun.time_since_last_fired > entity->weapon.gun.cooldown) {
              i32 enemy = get_nearest_enemy(meat_space, i);
              if (enemy != -1) {
                auto enemy_entity = meat_space->entities + enemy;
                if (magnitude(enemy_entity->p - entity->p) <= entity->weapon.gun.range) {
                  entity->weapon.gun.time_since_last_fired = 0.0f;
                  auto projectile = MeatSpaceProjectile{};
                  projectile.entity_targeted.type = MeatSpaceProjectile_Type_entity_targeted;
                  projectile.entity_targeted.p = entity->p + entity->firing_center;
                  projectile.entity_targeted.target_entity = enemy;
                  projectile.entity_targeted.speed = 240.0f;
                  projectile.entity_targeted.damage = 2.0f;
                  create_projectile(meat_space, projectile);
                }
              }
            }
          }
        }
      }

      if (entity->selected ||
          is_in_rect(entity->p, selection_rect) && entity->selection_group == SelectionGroup_player ||
          picked_entity == entity) {
        draw_entity_selection_circle(entity);
      }


      draw_health(entity->p, entity->health, entity->max_health);
    }

    if (has_flag(entity->flags, ENTITY_FLAG_DEAD)) {
      entity->asset_attributes.living_state = AssetLivingState_dead;
    }

    {
      auto p = entity->p;
      auto color = 0xffffffff;
      if (entity->selection_group == SelectionGroup_enemy) {
        color = 0xffaaaaff;
      }
      if (g_game_state->show_occupied_spaces) {
        if (has_flag(entity->flags, ENTITY_FLAG_TRAVERSABLE)) {
          if (is_cell_occupied(meat_space, position_to_cell(grid, p), i, true)) {
            color = 0xffaaaaff;
          } else if (is_cell_occupied(meat_space, position_to_cell(grid, p), i, false)) {
            color = 0xffaadddd;
          }
        }
      }

      auto asset_type = entity->asset_type;
      auto asset_attributes = entity->asset_attributes;
      auto z_bias = entity->z_bias;

      draw_entity_texture(meat_space, p, color, asset_type, asset_attributes, z_bias);

      if (entity->selected) {
        inspect_struct(MeatSpaceEntity, entity);
      }

#ifdef FALL_INTERNAL
      if (g_game_state->draw_collision_volumes) {
        draw_collision_volumes(meat_space, entity);
      }
#endif
    }
  }

  if (player_selecting) {
    draw_selection_rect(meat_space, selection_rect);
  }

  for (i32 i = 0; i < meat_space->projectiles_count; i++) {
    auto projectile = &meat_space->projectiles[i];
    if (projectile->type == MeatSpaceProjectile_Type_none) {
      continue;
    } else if (projectile->type == MeatSpaceProjectile_Type_entity_targeted) {
      auto entity_targeted = &projectile->entity_targeted;
      auto target = &meat_space->entities[entity_targeted->target_entity];
      auto target_position = target->p + target->target_center;
      auto position = entity_targeted->p;
      f32 remaining = 0.0f;
      projectile->common.p = move_toward(position, target_position, entity_targeted->speed * dt, &remaining);
      if (remaining > 0.0f) {
        target->health -= entity_targeted->damage;
        destroy_projectile(meat_space, i);
      }
    } else if (projectile->type == MeatSpaceProjectile_Type_position_targeted) {
      
    }

    auto tex = assets_get_texture(AssetType_square, {});
    auto position = projectile->common.p;
    f32 radius = 1.0f;
    f32 left = (position.x - radius);
    f32 right = (position.x + radius);
    f32 bottom = (position.y - radius);
    f32 top = (position.y + radius);
    f32 z = (position.y) - 0.1f;

    auto color = 0xff0000ff;
    push_textured_quad(g_render_commands,
                       tex.handle,
                       tex.left, tex.bottom, left, bottom, z, color,
                       tex.right, tex.bottom, right, bottom, z, color,
                       tex.left, tex.top, left, top, z, color,
                       tex.right, tex.top, right, top, z, color);
  }

  set_camera_position(g_render_commands,
                      round(meat_space->camera.position.x),
                      round(meat_space->camera.position.y));
  set_camera_scale(g_render_commands, meat_space->camera.scale.x, meat_space->camera.scale.y);
}
