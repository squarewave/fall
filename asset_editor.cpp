#include "asset_editor.h"

#include "asset_manager.h"
#include "debug.h"
#include "render_commands.h"
#include "serialize.h"

#undef g_editor
#define g_editor (g_game_state->asset_editor)

using namespace ImGui;

inline vec2 get_mouse_world_p() {
  f32 screen_width = g_render_commands->screen_width;
  f32 screen_height = g_render_commands->screen_height;
  vec2 screen_center = vec2{ screen_width / 2.0f, screen_height / 2.0f };
  return (vec2{ g_input->mouse.x, g_input->mouse.y } -screen_center) * (1.0f / g_editor->zoom_level);
}

void save_asset_specs() {
  size_t out_size;
  void* serialized = serialize_struct_array(&g_transient_state->allocator,
                                            TypeInfo_ID_AssetSpec,
                                            g_editor->asset_specs,
                                            g_editor->asset_specs_count,
                                            &out_size);
  g_platform.DEBUG_write_entire_file("../asset_specs", out_size, serialized);
}

void asset_editor_update_and_render() {
  if (!g_editor->initialized) {
    auto f = g_platform.DEBUG_read_entire_file("../asset_specs");
    AssetSpec* specs_ptr = (AssetSpec*)
      deserialize_struct_array(&g_game_state->allocator, TypeInfo_ID_AssetSpec,
                               f.contents,
                               f.content_size,
                               &g_editor->asset_specs_count,
                               EDITABLE_STRING_BUFFER_LENGTH,
                               MAX_ASSET_SPECS);
    g_platform.DEBUG_free_file_memory(f.contents);
    g_editor->asset_specs = specs_ptr;
    g_editor->asset_being_edited = -1;
    g_editor->zoom_level = 4.0f;

    g_editor->initialized = true;
  }

  SetNextWindowPos(ImVec2(10, 120));
  Begin("Asset Editor", NULL, ImVec2(0, 0), 0.3f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
  for (i32 i = 0; i < g_editor->asset_specs_count; i++) {
    PushID(i);
    if (g_editor->asset_being_edited == i) {
      if (Button("Save", ImVec2(80, 20))) {
        save_asset_specs();
        g_editor->asset_being_edited = -1;
      }
    } else {
      if (Button("Edit", ImVec2(80, 20))) {
        save_asset_specs();
        g_editor->asset_being_edited = i;
      }
    }
    SameLine();
    if (Button("Delete", ImVec2(80, 20))) {
      OpenPopup("Are you sure?");
    }
    SetNextWindowSize(ImVec2(240, 80));
    if (BeginPopupModal("Are you sure?", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
      Text("Delete %s?", g_editor->asset_specs[i].filepath);
      if (Button("Yes")) {
        i32 remaining = g_editor->asset_specs_count - i - 1;
        memmove(g_editor->asset_specs + i, g_editor->asset_specs + i + 1, remaining * sizeof(AssetSpec));
        if (g_editor->asset_being_edited > i) {
          g_editor->asset_being_edited--;
        } else if (g_editor->asset_being_edited == i) {
          g_editor->asset_being_edited = -1;
        }
        g_editor->asset_specs_count--;
        save_asset_specs();
        CloseCurrentPopup();
      }
      SameLine();
      if (Button("No")) {
        CloseCurrentPopup();
      }
      EndPopup();
    }
    SameLine();
    Text("%s", g_editor->asset_specs[i].filepath);
    PopID();
  }

  if (Button("Add", ImVec2(80, 20))) {
    g_editor->asset_specs_count++;
  }

  End();

  if (g_input->mouse.dwheel != 0) {
    g_editor->zoom_level += (f32)g_input->mouse.dwheel;
    if (g_editor->zoom_level < 1.0f) {
      g_editor->zoom_level = 1.0f;
    }
  }

  f32 screen_width = g_render_commands->screen_width;
  f32 screen_height = g_render_commands->screen_height;
  vec2 screen_center = vec2{ screen_width / 2.0f, screen_height / 2.0f };

  if (g_editor->asset_being_edited != -1) {
    assert(g_editor->asset_being_edited < g_editor->asset_specs_count);
    auto asset_spec = &g_editor->asset_specs[g_editor->asset_being_edited];
    auto tex = assets_get_texture(asset_spec->asset_type, asset_spec->asset_attributes);
    ivec2 anchor_offset = ivec2{ 0,0 };
    if (g_editor->dragging) {
      vec2 mouse_world_p = get_mouse_world_p();
      vec2 dp = mouse_world_p - g_editor->dragging_start;
      anchor_offset.x = -round((i32)dp.x);
      anchor_offset.y = -round((i32)dp.y);
      if (!was_down(g_input->mouse.button_l)) {
        auto tex_ref = assets_get_packed_texture(asset_spec->asset_type, asset_spec->asset_attributes);
        tex_ref->anchor_x += anchor_offset.x;
        tex_ref->anchor_y += anchor_offset.y;
        asset_spec->anchor_x = tex_ref->anchor_x;
        asset_spec->anchor_y = tex_ref->anchor_y;
        asset_spec->use_anchor = true;
        save_asset_specs();
        assets_save_edits();
        g_editor->dragging = false;
      }
    }
    tex.anchor_x += anchor_offset.x;
    tex.anchor_y += anchor_offset.y;
    f32 left = (-tex.anchor_x + 1.0f);
    f32 right = (tex.px_width - tex.anchor_x + 1.0f);
    f32 bottom = (-tex.anchor_y + 1.0f);
    f32 top = (tex.px_height - tex.anchor_y + 1.0f);
    rect2 asset_rect = rect2{ {left, bottom}, {right, top} };
    f32 z = -0.1f;
    u32 color = 0xffffffff;
    push_textured_quad(g_render_commands,
                       tex.handle,
                       tex.left, tex.bottom, left, bottom, z, color,
                       tex.right, tex.bottom, right, bottom, z, color,
                       tex.left, tex.top, left, top, z, color,
                       tex.right, tex.top, right, top, z, color);

    color = 0xffff0000;
    z -= 0.1f;
    push_quad_frame(g_render_commands,
                    left, bottom, z, color,
                    left, top, z, color,
                    right, top, z, color,
                    right, bottom, z, color);

    color = 0xff00ff00;
    z -= 0.1f;
    push_quad_frame(g_render_commands,
                    0.0f, 0.0f, z, color,
                    0.0f, 1.0f, z, color,
                    1.0f, 1.0f, z, color,
                    1.0f, 0.0f, z, color);
    Begin("Properties");
    inspect_struct(AssetSpec, asset_spec);
    End();

    if (was_pressed(g_input->mouse.button_l)) {
      vec2 mouse_world_p = get_mouse_world_p();
      if (is_in_rect(mouse_world_p, asset_rect)) {
        g_editor->dragging_start = mouse_world_p;
        g_editor->dragging = true;
      }
    }
  }

  set_clear_color(g_render_commands, 0.5f, 0.5f, 0.5f);
  set_camera_position(g_render_commands,
                      0.0f,
                      0.0f);
  set_camera_scale(g_render_commands,
                   screen_width / g_editor->zoom_level,
                   screen_height / g_editor->zoom_level);
}