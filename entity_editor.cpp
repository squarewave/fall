#include "entity_editor.h"

#include "asset_manager.h"
#include "debug.h"
#include "game.h"
#include "meat_space.h"
#include "render_commands.h"
#include "serialize.h"
#include "typeinfo.h"

using namespace ImGui;
#undef g_editor
#define g_editor (g_game_state->entity_editor)

bool next_collision_volume(MeatSpaceEntityTemplateCollection* template_collection,
                           MeatSpaceEntityTemplate* entity_template,
                           CollisionVolume** volume);


void save_entity_templates() {
  {
    size_t out_size;
    void* serialized = serialize_struct_array(&g_transient_state->allocator,
                                              TypeInfo_ID_MeatSpaceEntityTemplate,
                                              g_game_state->meat_space_entity_templates.templates,
                                              g_game_state->meat_space_entity_templates.templates_count,
                                              &out_size);
    g_platform.DEBUG_write_entire_file("../templates", out_size, serialized);
  }
  {
    size_t out_size;
    void* serialized = serialize_struct_array(&g_transient_state->allocator,
                                              TypeInfo_ID_CollisionVolume,
                                              g_game_state->meat_space_entity_templates.collision_volumes,
                                              g_game_state->meat_space_entity_templates.collision_volumes_count,
                                              &out_size);
    g_platform.DEBUG_write_entire_file("../collision_volumes", out_size, serialized);
  }
}

void draw_template_collision_volumes(MeatSpaceEntityTemplateCollection* meat_space, MeatSpaceEntityTemplate* entity) {
  CollisionVolume* volume = NULL;
  while (next_collision_volume(meat_space, entity, &volume)) {
    f32 z = entity->z_bias - 0.1f;
    u32 color = 0xffffffff;
    switch (volume->type) {
    case CollisionVolume_Type_box: {
      auto box = volume->box;

      f32 left = box.offset.x - volume->box.dimensions.x / 2.0f;
      f32 right = box.offset.x + volume->box.dimensions.x / 2.0f;
      f32 bottom = box.offset.y - volume->box.dimensions.y / 2.0f;
      f32 top = box.offset.y + volume->box.dimensions.y / 2.0f;

      push_quad_frame(g_render_commands,
                      left, bottom, z, color,
                      left, top, z, color,
                      right, top, z, color,
                      right, bottom, z, color);
    } break;
    case CollisionVolume_Type_circle: {
      auto circle = volume->circle;
      push_circle_frame(g_render_commands,
                        circle.offset.x,
                        circle.offset.y,
                        z,
                        circle.radius,
                        color);
    } break;
    default:;
    }
  }
}

void entity_editor_update_and_render() {
  if (!g_editor->initialized) {
    g_editor->zoom_level = 4.0f;
    g_editor->template_being_edited = -1;
    g_editor->initialized = true;
  }

  f32 screen_width = g_render_commands->screen_width;
  f32 screen_height = g_render_commands->screen_height;

  SetNextWindowPos(ImVec2(10, 120));
  Begin("Entity Editor", NULL, ImVec2(0, 0), 0.3f,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
  auto ts = &g_game_state->meat_space_entity_templates;
  for (i32 i = 0; i < ts->templates_count; i++) {
    PushID(i);
    if (g_editor->template_being_edited == i) {
      if (Button("Save", ImVec2(80, 20))) {
        save_entity_templates();
        g_editor->template_being_edited = -1;
      }
    } else {
      if (Button("Edit", ImVec2(80, 20))) {
        if (g_editor->template_being_edited != -1) {
          save_entity_templates();
        }
        g_editor->template_being_edited = i;
      }
    }
    SameLine();
    Text("%s", enum_member_name(MeatSpaceEntityTemplateId, ts->templates[i].id));
    PopID();
  }
  End();

  if (g_input->mouse.dwheel != 0) {
    g_editor->zoom_level += (f32)g_input->mouse.dwheel;
    if (g_editor->zoom_level < 1.0f) {
      g_editor->zoom_level = 1.0f;
    }
  }

  if (g_editor->template_being_edited != -1) {

    assert(g_editor->template_being_edited < g_game_state->meat_space_entity_templates.templates_count);
    auto t = g_game_state->meat_space_entity_templates.templates + g_editor->template_being_edited;

    auto tex = assets_get_texture(t->asset_type, {});
    f32 left = (-tex.anchor_x + 1.0f);
    f32 right = (tex.px_width - tex.anchor_x + 1.0f);
    f32 bottom = (-tex.anchor_y + 1.0f);
    f32 top = (tex.px_height - tex.anchor_y + 1.0f);
    f32 z = -0.1f;
    u32 color = 0xffffffff;
    push_textured_quad(g_render_commands,
                       tex.handle,
                       tex.left, tex.bottom, left, bottom, z, color,
                       tex.right, tex.bottom, right, bottom, z, color,
                       tex.left, tex.top, left, top, z, color,
                       tex.right, tex.top, right, top, z, color);

    // draw_template_collision_volumes(ts, t);

    SetNextWindowPos(ImVec2(screen_width - 410.0f, 120.0f));
    Begin("Properties", NULL, ImVec2(400.0f, 800.0f), 0.3f,
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    inspect_struct_no_collapse(MeatSpaceEntityTemplate, t);

    End();
  }

  set_clear_color(g_render_commands, 0.5f, 0.5f, 0.5f);
  set_camera_position(g_render_commands,
                      0.0f,
                      0.0f);
  set_camera_scale(g_render_commands,
                   screen_width / g_editor->zoom_level,
                   screen_height / g_editor->zoom_level);
}
 