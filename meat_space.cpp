#include "asset_manager.h"
#include "game.h"
#include "game_math.h"
#include "meat_space.h"
#include "render_commands.h"

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
  MeatSpaceBrain_player* selecting_player = NULL;
  rect2 selection_rect;

  for (int i = 0; i < meat_space->brains_count; ++i) {
    auto brain = meat_space->brains + i;

    switch (brain->type) {
      case MeatSpaceBrainType_player: {
        auto player = &brain->player;
        if (was_pressed(g_input->mouse.button_l)) {
          player->selecting = true;
          player->selection_start = mouse_world_position(meat_space);
        }
        if (was_released(g_input->mouse.button_l)) {
          if (player->selecting) {
            player->selecting = false;
            player->selected_entities_count = 0;
            selecting_player = player;
          }
        }

        if (player->selecting || selecting_player) {
          vec2 selection_end = mouse_world_position(meat_space);
          selection_rect.bottom_left.x = min(player->selection_start.x, selection_end.x);
          selection_rect.bottom_left.y = min(player->selection_start.y, selection_end.y);
          selection_rect.top_right.x = max(player->selection_start.x, selection_end.x);
          selection_rect.top_right.y = max(player->selection_start.y, selection_end.y);

          AssetAttributes attrs = {};
          attrs.direction = AssetDirection_horizontal;
          TextureAsset horizontal_line = assets_get_texture(AssetType_selection_line, attrs);
          attrs.direction = AssetDirection_vertical;
          TextureAsset vertical_line = assets_get_texture(AssetType_selection_line, attrs);

          f32 left;
          f32 right;
          f32 top;
          f32 bottom;

          // top line
          left = selection_rect.bottom_left.x;
          right = selection_rect.top_right.x - 0.5f;
          top = selection_rect.top_right.y;
          bottom = top - 0.5f;
          push_textured_quad(g_render_commands,
                             horizontal_line.handle,
                             horizontal_line.left, horizontal_line.bottom, left, bottom, 0xffffffff,
                             horizontal_line.right, horizontal_line.bottom, right, bottom, 0xffffffff,
                             horizontal_line.left, horizontal_line.top, left, top, 0xffffffff,
                             horizontal_line.right, horizontal_line.top, right, top, 0xffffffff);

          // bottom line
          left = selection_rect.bottom_left.x + 0.5f;
          right = selection_rect.top_right.x;
          bottom = selection_rect.bottom_left.y;
          top = bottom + 0.5f;
          push_textured_quad(g_render_commands,
                             horizontal_line.handle,
                             horizontal_line.left, horizontal_line.bottom, right, bottom, 0xffffffff,
                             horizontal_line.right, horizontal_line.bottom, left, bottom, 0xffffffff,
                             horizontal_line.left, horizontal_line.top, right, top, 0xffffffff,
                             horizontal_line.right, horizontal_line.top, left, top, 0xffffffff);

          // left line
          left = selection_rect.bottom_left.x;
          right = left + 0.5f;
          top = selection_rect.top_right.y - 0.5f;
          bottom = selection_rect.bottom_left.y;
          push_textured_quad(g_render_commands,
                             vertical_line.handle,
                             vertical_line.left, vertical_line.bottom, left, bottom, 0xffffffff,
                             vertical_line.right, vertical_line.bottom, right, bottom, 0xffffffff,
                             vertical_line.left, vertical_line.top, left, top, 0xffffffff,
                             vertical_line.right, vertical_line.top, right, top, 0xffffffff);

          // right line
          right = selection_rect.top_right.x;
          left = right - 0.5f;
          top = selection_rect.top_right.y;
          bottom = selection_rect.bottom_left.y + 0.5f;
          push_textured_quad(g_render_commands,
                             vertical_line.handle,
                             vertical_line.left, vertical_line.bottom, left, top, 0xffffffff,
                             vertical_line.right, vertical_line.bottom, right, top, 0xffffffff,
                             vertical_line.left, vertical_line.top, left, bottom, 0xffffffff,
                             vertical_line.right, vertical_line.top, right, bottom, 0xffffffff);
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
    }

    // entity->p.x -= g_input->dt * 1.0f;
    // entity->p.y += g_input->dt * 1.0f;
    if (entity->selected) {
      AssetAttributes attrs = {};
      TextureAsset selection = assets_get_texture(AssetType_selection_circle, attrs);
      f32 left = round(entity->p.x - (((f32)selection.px_width - 0.5f) / 2.0f));
      f32 right = round(entity->p.x + (((f32)selection.px_width - 0.5f) / 2.0f));
      f32 bottom = round(entity->p.y - (((f32)selection.px_height - 0.5f) / 2.0f));
      f32 top = round(entity->p.y + (((f32)selection.px_height - 0.5f) / 2.0f));
      push_textured_quad(g_render_commands,
                         selection.handle,
                         selection.left, selection.bottom, left + selection.offset_x, bottom + selection.offset_y, 0xffffffff,
                         selection.right, selection.bottom, right + selection.offset_x, bottom + selection.offset_y, 0xffffffff,
                         selection.left, selection.top, left + selection.offset_x, top + selection.offset_y, 0xffffffff,
                         selection.right, selection.top, right + selection.offset_x, top + selection.offset_y, 0xffffffff);
    }

    {
      TextureAsset tex = assets_get_texture(entity->asset_type, entity->asset_attributes);
      f32 left = round(entity->p.x - (tex.px_width / 2.0f));
      f32 right = round(entity->p.x + (tex.px_width / 2.0f));
      f32 bottom = round(entity->p.y - (tex.px_height / 2.0f));
      f32 top = round(entity->p.y + (tex.px_height / 2.0f));
      push_textured_quad(g_render_commands,
                         tex.handle,
                         tex.left, tex.bottom, left + tex.offset_x, bottom + tex.offset_y, 0xffffffff,
                         tex.right, tex.bottom, right + tex.offset_x, bottom + tex.offset_y, 0xffffffff,
                         tex.left, tex.top, left + tex.offset_x, top + tex.offset_y, 0xffffffff,
                         tex.right, tex.top, right + tex.offset_x, top + tex.offset_y, 0xffffffff);
    }
  }

  set_camera_position(g_render_commands, meat_space->camera.position.x, meat_space->camera.position.y);
  set_camera_scale(g_render_commands, meat_space->camera.scale.x, meat_space->camera.scale.y);
}
