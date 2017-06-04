#include <assert.h>
#include <minwindef.h>

#include "asset_manager.h"
#include "debug.h"
#include "game_math.h"
#include "imgui_extensions.h"
#include "meat_space.h"
#include "render_commands.h"


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

vec2 mouse_world_position(MeatSpace* meat_space) {
  f32 viewport_width = meat_space->camera.viewport_right - meat_space->camera.viewport_left;
  f32 viewport_height = meat_space->camera.viewport_top - meat_space->camera.viewport_bottom;
  f32 x_norm = clamp((g_input->mouse.x - meat_space->camera.viewport_left) / viewport_width, 0.0f, 1.0f) - 0.5f;
  f32 y_norm = clamp((g_input->mouse.y - meat_space->camera.viewport_bottom) / viewport_height, 0.0f, 1.0f) - 0.5f;
  f32 world_x = x_norm * meat_space->camera.scale.x + meat_space->camera.position.x;
  f32 world_y = y_norm * meat_space->camera.scale.y + meat_space->camera.position.y;
  return vec2 {world_x, world_y};
}

void meat_space_update_and_render(MeatSpace* meat_space) {
  b32 player_selecting = false;
  MeatSpaceBrain_player* selecting_player = NULL;
  MeatSpaceEntity* picked_entity = NULL;
  MeatSpaceEntity* selected_entity = NULL;
  rect2 selection_rect = {};

  for (i32 i = 0; i < meat_space->brains_count; ++i) {
    auto brain = meat_space->brains + i;

    switch (brain->type) {
      case MeatSpaceBrainType_player: {
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
          }

          player->mouse_down_entity = NULL;
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

    if (entity->selected || is_in_rect(entity->p, selection_rect) || picked_entity == entity) {
      AssetAttributes attrs = {};
      auto tex = assets_get_texture(AssetType_selection_circle, attrs);
      f32 left = round(entity->p.x - tex.anchor_x / 2);
      f32 right = round(entity->p.x + tex.px_width / 2 - tex.anchor_x / 2);
      f32 bottom = round(entity->p.y - tex.anchor_y / 2);
      f32 top = round(entity->p.y + tex.px_height / 2 - tex.anchor_y / 2);
      u32 color = entity->selected ? 0xffffffff : 0x77ffffff;

      push_textured_quad(g_render_commands,
                         tex.handle,
                         tex.left, tex.bottom, left, bottom, color,
                         tex.right, tex.bottom, right, bottom, color,
                         tex.left, tex.top, left, top, color,
                         tex.right, tex.top, right, top, color);
    }

    {
      auto tex = assets_get_texture(entity->asset_type, entity->asset_attributes);
      f32 left = round(entity->p.x - tex.anchor_x);
      f32 right = round(entity->p.x + tex.px_width - tex.anchor_x);
      f32 bottom = round(entity->p.y - tex.anchor_y);
      f32 top = round(entity->p.y + tex.px_height - tex.anchor_y);
      push_textured_quad(g_render_commands,
                         tex.handle,
                         tex.left, tex.bottom, left, bottom, 0xffffffff,
                         tex.right, tex.bottom, right, bottom, 0xffffffff,
                         tex.left, tex.top, left, top, 0xffffffff,
                         tex.right, tex.top, right, top, 0xffffffff);

      if (entity->selected) {
        inspect_struct(MeatSpaceEntity, entity);
        ImGui::Image(tex);
      }
    }
  }

  if (player_selecting) {
    AssetAttributes attrs = {};
    attrs.direction = AssetDirection_horizontal;
    auto horizontal_line = assets_get_texture(AssetType_selection_line, attrs);
    attrs.direction = AssetDirection_vertical;
    auto vertical_line = assets_get_texture(AssetType_selection_line, attrs);

    {
      // top line
      f32 left = selection_rect.bottom_left.x;
      f32 right = selection_rect.top_right.x - 0.5f;
      f32 top = selection_rect.top_right.y;
      f32 bottom = top - 0.5f;
      push_textured_quad(g_render_commands,
                         horizontal_line.handle,
                         horizontal_line.left, horizontal_line.bottom, left, bottom, 0xffffffff,
                         horizontal_line.right, horizontal_line.bottom, right, bottom, 0xffffffff,
                         horizontal_line.left, horizontal_line.top, left, top, 0xffffffff,
                         horizontal_line.right, horizontal_line.top, right, top, 0xffffffff);
    }

    {
      // bottom line
      f32 left = selection_rect.bottom_left.x + 0.5f;
      f32 right = selection_rect.top_right.x;
      f32 bottom = selection_rect.bottom_left.y;
      f32 top = bottom + 0.5f;
      push_textured_quad(g_render_commands,
                         horizontal_line.handle,
                         horizontal_line.left, horizontal_line.bottom, right, bottom, 0xffffffff,
                         horizontal_line.right, horizontal_line.bottom, left, bottom, 0xffffffff,
                         horizontal_line.left, horizontal_line.top, right, top, 0xffffffff,
                         horizontal_line.right, horizontal_line.top, left, top, 0xffffffff);
    }

    {
      // left line
      f32 left = selection_rect.bottom_left.x;
      f32 right = left + 0.5f;
      f32 top = selection_rect.top_right.y - 0.5f;
      f32 bottom = selection_rect.bottom_left.y;
      push_textured_quad(g_render_commands,
                         vertical_line.handle,
                         vertical_line.left, vertical_line.bottom, left, bottom, 0xffffffff,
                         vertical_line.right, vertical_line.bottom, right, bottom, 0xffffffff,
                         vertical_line.left, vertical_line.top, left, top, 0xffffffff,
                         vertical_line.right, vertical_line.top, right, top, 0xffffffff);
    }

    {
      // right line
      f32 right = selection_rect.top_right.x;
      f32 left = right - 0.5f;
      f32 top = selection_rect.top_right.y;
      f32 bottom = selection_rect.bottom_left.y + 0.5f;
      push_textured_quad(g_render_commands,
                         vertical_line.handle,
                         vertical_line.left, vertical_line.bottom, left, top, 0xffffffff,
                         vertical_line.right, vertical_line.bottom, right, top, 0xffffffff,
                         vertical_line.left, vertical_line.top, left, bottom, 0xffffffff,
                         vertical_line.right, vertical_line.top, right, bottom, 0xffffffff);
    }
  }

  set_camera_position(g_render_commands, meat_space->camera.position.x, meat_space->camera.position.y);
  set_camera_scale(g_render_commands, meat_space->camera.scale.x, meat_space->camera.scale.y);
}
