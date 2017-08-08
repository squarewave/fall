#include <assert.h>

#include "imgui/imgui.h"

#include "assets.h"
#include "asset_manager.h"
#include "game.h"
#include "imgui_extensions.h"
#include "meat_space.h"
#include "memory.h"
#include "noise.h"
#include "platform.h"
#include "render_commands.h"
#include "rnd.h"
#include "serialize.h"
#include "tools.h"

#ifdef FALL_INTERNAL
#include "debug.h"
#include "asset_editor.h"
#include "entity_editor.h"
#endif

GameState* g_game_state;
TransientState* g_transient_state;
PlatformInput* g_input;
PlatformServices g_platform;
RenderCommands* g_render_commands;
char* g_debug_print_ring_buffer;
i32* g_debug_print_ring_buffer_write_head;
GAME_UPDATE_AND_RENDER(game_update_and_render) {
  g_game_state = memory->game_state;
  g_transient_state = memory->transient_state;
  g_input = memory->input;
  g_platform = memory->platform;
  g_render_commands = memory->render_commands;
  g_debug_print_ring_buffer = memory->debug_print_ring_buffer;
  g_debug_print_ring_buffer_write_head = memory->debug_print_ring_buffer_write_head;

  reset_transient_memory();

  if (!g_game_state->initialized) {
    g_game_state->allocator.capacity = GAME_MEMORY_SIZE;
    g_transient_state->allocator.capacity = TRANSIENT_MEMORY_SIZE;

    assets_init();
    g_game_state->perlin_scale = 45.0f;

    i32 dimensions[] = {64, 64};
    f32* buffer = transient_alloc_array(f32, 64 * 64 * 2);
    char* result_buffer = transient_alloc_array(char, 256 * 256 * 4);
    f32 scale = 1.0f / g_game_state->perlin_scale;
    for (i32 x = 0; x < 256; x++) {
      for (i32 y = 0; y < 256; y++) {
        f32 perlin = noise2((f32)x * scale, (f32)y * scale);
        char noise = perlin > -0.1f ? 0xff : 0x00;
        result_buffer[(y * 256 + x) * 4 + 0] = noise;
        result_buffer[(y * 256 + x) * 4 + 1] = noise;
        result_buffer[(y * 256 + x) * 4 + 2] = noise;
        result_buffer[(y * 256 + x) * 4 + 3] = 0xff;
      }
    }

#ifdef FALL_INTERNAL
    g_game_state->asset_editor = game_alloc(AssetEditor);
    g_game_state->entity_editor = game_alloc(EntityEditor);
#endif

    g_game_state->dummy_texture = g_platform.register_texture(result_buffer, 256, 256, 4);

    rnd_pcg_t pcg;
    rnd_pcg_seed(&pcg, 0u);

    i64 id = 0;
    auto meat_space = g_game_state->TMP_meat_space = game_alloc(MeatSpace);
    Grid grid = Grid{ vec2{0}, 6.0f };
    meat_space->grid = grid;
    meat_space->projectiles_free_stack[0] = -1;
    rnd_pcg_seed(&meat_space->pcg, 1u);
    f32 cell_width = meat_space->grid.cell_width;

    auto f = g_platform.DEBUG_read_entire_file("../templates");
    size_t templates_size = f.content_size;
    void* templates_serialized = f.contents;

    g_game_state->meat_space_entity_templates.templates = (MeatSpaceEntityTemplate*)
      deserialize_struct_array(&g_game_state->allocator, TypeInfo_ID_MeatSpaceEntityTemplate,
                               templates_serialized, templates_size,
                               &g_game_state->meat_space_entity_templates.templates_count,
                               MAX_MEAT_SPACE_TEMPLATES);
    g_platform.DEBUG_free_file_memory(f.contents);

    f = g_platform.DEBUG_read_entire_file("../collision_volumes");
    size_t collision_volumes_size = f.content_size;
    void* collision_volumes_serialized = f.contents;

    g_game_state->meat_space_entity_templates.collision_volumes = (CollisionVolume*)
      deserialize_struct_array(&g_game_state->allocator, TypeInfo_ID_CollisionVolume,
                               collision_volumes_serialized, collision_volumes_size,
                               &g_game_state->meat_space_entity_templates.collision_volumes_count,
                               MAX_COLLISION_VOLUMES);
    g_platform.DEBUG_free_file_memory(f.contents);

    for (i32 i = 0; i < g_game_state->meat_space_entity_templates.collision_volumes_count; i++) {
      if (g_game_state->meat_space_entity_templates.collision_volumes[i].type == CollisionVolume_Type_none) {
        g_game_state->meat_space_entity_templates.max_collision_volume_index = i - 1;
        break;
      }
    }

    meat_space->camera.position = vec2{ 0.0f, 0.0f };
    meat_space->camera.scale = vec2{ 1920.0f / PX_PER_PIXEL, 1080.0f / PX_PER_PIXEL };
    meat_space->camera.viewport_left = 0.0f;
    meat_space->camera.viewport_right = g_render_commands->screen_width;
    meat_space->camera.viewport_bottom = 0.0f;
    meat_space->camera.viewport_top = g_render_commands->screen_height;

    vec2 bottom_left = meat_space->camera.position - meat_space->camera.scale * 0.5;
    vec2 top_right = bottom_left + meat_space->camera.scale;
    i32 size_x = (i32)((top_right.x - bottom_left.x) / cell_width) + 2;
    i32 size_y = (i32)((top_right.y - bottom_left.y) / cell_width) + 2;

    LOG("%d, %d", size_x, size_y);

    {
      auto boulder = create_entity_from_template(meat_space, MeatSpaceEntityTemplateId_boulder_large);
      boulder->p = snap_to_grid(meat_space->grid, vec2{ -5.0f, 0.0f });
    }

    for (i32 x = 0; x < 1; x++) {
      for (i32 y = 0; y < 1; y++) {
        auto crew = create_entity_from_template(meat_space, MeatSpaceEntityTemplateId_crew_gun);
        GridCell cell = GridCell{ x + 5, y + 5 };
        crew->p = snap_to_grid(meat_space->grid, cell_to_position(meat_space->grid, cell));
        crew->selection_group = SelectionGroup_player;
        crew->asset_attributes.asset_class = AssetClass_science;
        crew->asset_attributes.color = AssetColor_dark;
        crew->asset_attributes.direction = AssetDirection_forward;
        crew->asset_attributes.move_state = AssetMoveState_standing;
      }
    }

    for (i32 x = 0; x < size_x; ++x) {
      for (i32 y = 0; y < size_y; ++y) {
        f32 px = (f32)x * cell_width + bottom_left.x;
        f32 py = (f32)y * cell_width + bottom_left.y;

        f32 noise_scale = 1.0f / 93.0f;
        f32 noise_threshold = 0.2f;
        b32 noise_00 = noise2(px * noise_scale, py * noise_scale) > noise_threshold;
        b32 noise_01 = noise2(px * noise_scale, (py + cell_width) * noise_scale) > noise_threshold;
        b32 noise_10 = noise2((px + cell_width) * noise_scale, py * noise_scale) > noise_threshold;
        b32 noise_11 = noise2((px + cell_width) * noise_scale, (py + cell_width)  * noise_scale) > noise_threshold;

        u32 bitflags = 0;
        if (noise_00) {
          bitflags |= GRASS_BOTTOM_LEFT;
        }
        if (noise_01) {
          bitflags |= GRASS_TOP_LEFT;
        }
        if (noise_10) {
          bitflags |= GRASS_BOTTOM_RIGHT;
        }
        if (noise_11) {
          bitflags |= GRASS_TOP_RIGHT;
        }

        auto tile = create_entity_from_template(meat_space, MeatSpaceEntityTemplateId_tile);
        tile->p = snap_to_grid(meat_space->grid, vec2{ px,py });
        tile->asset_attributes.bitflags = bitflags;
      }
    }

//    for (i32 x = 0; x < 5; x++) {
//      for (i32 y = 6; y < 8; y++) {
//        auto crew = create_entity_from_template(meat_space, MeatSpaceEntityTemplateId_crew);
//        GridCell cell = GridCell{ x, y };
//        crew->p = snap_to_grid(meat_space->grid, cell_to_position(meat_space->grid, cell));
//        crew->selection_group = SelectionGroup_enemy;
//        crew->asset_attributes.asset_class = AssetClass_science;
//        crew->asset_attributes.color = AssetColor_dark;
//        crew->asset_attributes.direction = AssetDirection_forward;
//        crew->asset_attributes.move_state = AssetMoveState_standing;
//      }
//    }

    {
      MeatSpaceBrain player = {};
      player.player.type = MeatSpaceBrain_Type_player;
      player.player.selected_entities = game_alloc_array(i64, MAX_SELECTED_ENTITIES);
      meat_space->brains[meat_space->brains_count++] = player;
    }

    g_game_state->initialized = true;
  } else {
    assets_refresh();
  }

  if (g_game_state->pending_entity_placement_id != MeatSpaceEntityTemplateId_none) {
    if (was_pressed(g_input->mouse.button_r)) {
      eat_button_input(&g_input->mouse.button_r);
      g_game_state->pending_entity_placement_id = MeatSpaceEntityTemplateId_none;
    } else {
      if (was_pressed(g_input->mouse.button_l)) {
        auto meat_space = g_game_state->TMP_meat_space;
        auto crew = create_entity_from_template(meat_space,
                                                g_game_state->pending_entity_placement_id,
                                                g_game_state->pending_entity_variation_number);
        crew->p = snap_to_grid(meat_space->grid, mouse_world_position(meat_space));
        crew->selection_group = g_game_state->pending_entity_placement_selection_group;
        g_game_state->pending_entity_variation_number =
          rnd_pcg_next(&g_game_state->TMP_meat_space->pcg);
      } else {
        auto t = get_entity_template(g_game_state->TMP_meat_space,
                                     g_game_state->pending_entity_placement_id);
        auto position = mouse_world_position(g_game_state->TMP_meat_space);
        auto color = g_game_state->pending_entity_placement_selection_group == SelectionGroup_enemy ?
          0xffaaaaff :
          0xffffffff;
        AssetAttributes asset_attributes = {};
        asset_attributes.variation_number = g_game_state->pending_entity_variation_number;
        draw_entity_texture(g_game_state->TMP_meat_space,
                            snap_to_grid(g_game_state->TMP_meat_space->grid, position) + t->texture_anchor,
                            color,
                            t->asset_type, asset_attributes,
                            t->z_bias);
      }

      if (was_down(g_input->mouse.button_l)) {
        eat_button_input(&g_input->mouse.button_l);
      }
    }
  }

#if FALL_INTERNAL
  update_and_render_tools();
#endif

#ifdef FALL_INTERNAL
  if (g_game_state->step_through_frames) {
    if (!was_pressed(g_input->keyboard.n)) {
      g_input->dt = 0.0f;
    }
  }
#endif

  if (g_game_state->editor_mode == EditorMode_assets) {
    asset_editor_update_and_render();
  } else if (g_game_state->editor_mode == EditorMode_entities) {
    entity_editor_update_and_render();
  } else {
    meat_space_update_and_render(g_game_state->TMP_meat_space);
  }
}

#ifdef FALL_INTERNAL
GAME_DEBUG_END_FRAME(game_debug_end_frame) {
  const i32 buffer_length = 256;
  static f32 framerates[buffer_length] = { 0 };
  static f32 transient_memory[buffer_length] = { 0 };
  static f32 max_transient_memory[buffer_length] = { 0 };
  static f32 game_memory[buffer_length] = { 0 };
  static i32 buffer_index = 0;

  framerates[buffer_index & buffer_length - 1] = ImGui::GetIO().DeltaTime * 1000.0f;
  transient_memory[buffer_index & buffer_length - 1] = (f32)transient_memory_used() / 1000.0f;
  max_transient_memory[buffer_index & buffer_length - 1] = max((f32)transient_memory_used() / 1000.0f,
                                                               max_transient_memory[(buffer_index - 1) & buffer_length - 1]);
  game_memory[buffer_index & buffer_length - 1] = (f32)game_memory_used() / 1000.0f;

  buffer_index++;
  // NOTE(doug): the following line is just a simple way to track where we are in the graph.
  framerates[buffer_index & buffer_length - 1] = 0.0f;
  framerates[buffer_index & buffer_length - 1] = 0.0f;

  ImGui::SetNextWindowPos(ImVec2(10, 10));
  ImGui::Begin("Fixed Overlay", NULL, ImVec2(0, 0), 0.3f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
  if (ImGui::Button("Asset Editor")) {
    g_game_state->editor_mode = EditorMode_assets;
  } else if (ImGui::Button("Entity Editor")) {
    g_game_state->editor_mode = EditorMode_entities;
  } else if (ImGui::Button("In-game Editor")) {
    g_game_state->editor_mode = EditorMode_game;
  } else if (ImGui::Button("Game Mode")) {
    g_game_state->editor_mode = EditorMode_none;
  }
  ImGui::End();

  if (g_game_state->editor_mode == EditorMode_game) {
    ImGui::SetNextWindowPos(ImVec2(10, 120));
    ImGui::Begin("Game HUD", NULL, ImVec2(0, 0), 0.3f,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    for (i32 i = 0; i < g_game_state->meat_space_entity_templates.templates_count; i++) {
      ImGui::PushID(i);
      auto t = g_game_state->meat_space_entity_templates.templates + i;
      ImGui::Text(enum_member_name(MeatSpaceEntityTemplateId, t->id));
      ImGui::PopID();
    }

    ImGui::PlotHistogram("", framerates, buffer_length, 0, "ms / frame", 0.0f, 64.0f);
    ImGui::PlotHistogram("", transient_memory, buffer_length,
                         0, "transient KB used", 0.0f, (f32)TRANSIENT_MEMORY_SIZE);
    ImGui::PlotHistogram("", max_transient_memory, buffer_length,
                         0, "max transient KB used", 0.0f, (f32)TRANSIENT_MEMORY_SIZE);
    ImGui::PlotHistogram("", game_memory, buffer_length,
                         0, "game KB used", 0.0f, (f32)GAME_MEMORY_SIZE);

    inspect_struct(GameState, g_game_state);
    for (i32 i = 0; i < g_game_state->meat_space_entity_templates.templates_count; i++) {
      auto t = &g_game_state->meat_space_entity_templates.templates[i];
      inspect_struct_named(MeatSpaceEntityTemplate,
                           t,
                           enum_member_name(MeatSpaceEntityTemplateId, t->id));
      ImGui::PushID(i);
      if (has_flag(t->flags, ENTITY_FLAG_CHARACTER)) {
        if (ImGui::Button("Player")) {
          g_game_state->pending_entity_placement_id = t->id;
          g_game_state->pending_entity_placement_selection_group = SelectionGroup_player;
          g_game_state->pending_entity_variation_number =
            rnd_pcg_next(&g_game_state->TMP_meat_space->pcg);
        }
        ImGui::SameLine();
        if (ImGui::Button("Enemy")) {
          g_game_state->pending_entity_placement_id = t->id;
          g_game_state->pending_entity_placement_selection_group = SelectionGroup_enemy;
          g_game_state->pending_entity_variation_number =
            rnd_pcg_next(&g_game_state->TMP_meat_space->pcg);
        }
      } else {
        if (ImGui::Button("Create")) {
          g_game_state->pending_entity_placement_id = t->id;
          g_game_state->pending_entity_placement_selection_group = SelectionGroup_none;
          g_game_state->pending_entity_variation_number =
            rnd_pcg_next(&g_game_state->TMP_meat_space->pcg);
        }
      }
      ImGui::PopID();
    }

    ImGui::End();

    show_debug_log();
  }
}

GAME_IMGUI_GET_IO(game_imgui_get_io) {
  return ImGui::GetIO();
}

GAME_IMGUI_NEW_FRAME(game_imgui_new_frame) {
  ImGui::NewFrame();
}

GAME_IMGUI_SHUTDOWN(game_imgui_shutdown) {
  ImGui::Shutdown();
}

GAME_IMGUI_RENDER(game_imgui_render) {
  ImGui::Render();
}

GAME_IMGUI_GET_TEX_DATA_AS_RGBA32(game_imgui_get_tex_data_as_rgba32) {
  ImGui::GetIO().Fonts->GetTexDataAsRGBA32(pixels, width, height);
}
#endif

