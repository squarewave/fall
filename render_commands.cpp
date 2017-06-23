#include "assert.h"
#include "geometry.h"
#include "render_commands.h"

void push_textured_quad(RenderCommands* render_commands,
                        void* texture_handle,
                        /* bottom left */ f32 u0, f32 v0, f32 world_x0, f32 world_y0, f32 world_z0, u32 color0,
                        /* bottom right */ f32 u1, f32 v1, f32 world_x1, f32 world_y1, f32 world_z1, u32 color1,
                        /* top left */ f32 u2, f32 v2, f32 world_x2, f32 world_y2, f32 world_z2, u32 color2,
                        /* top right */ f32 u3, f32 v3, f32 world_x3, f32 world_y3, f32 world_z3, u32 color3) {
  assert(render_commands->textured_quad_vertices_count + 4 <= render_commands->textured_quad_vertices_max_count);

  auto vertices = render_commands->textured_quad_vertices + render_commands->textured_quad_vertices_count;
  render_commands->quad_textures[render_commands->textured_quad_vertices_count / 4] = texture_handle;

  vertices[0].u = u0;
  vertices[0].v = v0;
  vertices[0].world_x = world_x0;
  vertices[0].world_y = world_y0;
  vertices[0].world_z = world_z0;
  vertices[0].color = color0;

  vertices[1].u = u1;
  vertices[1].v = v1;
  vertices[1].world_x = world_x1;
  vertices[1].world_y = world_y1;
  vertices[1].world_z = world_z1;
  vertices[1].color = color1;

  vertices[2].u = u2;
  vertices[2].v = v2;
  vertices[2].world_x = world_x2;
  vertices[2].world_y = world_y2;
  vertices[2].world_z = world_z2;
  vertices[2].color = color2;

  vertices[3].u = u3;
  vertices[3].v = v3;
  vertices[3].world_x = world_x3;
  vertices[3].world_y = world_y3;
  vertices[3].world_z = world_z3;
  vertices[3].color = color3;

  render_commands->textured_quad_vertices_count += 4;
}

const i32 CIRCLE_VERTEX_COUNT = 100;

void push_circle_frame(RenderCommands* render_commands, f32 world_x, f32 world_y, f32 world_z, f32 radius, u32 c) {
  assert(render_commands->line_vertices_count + CIRCLE_VERTEX_COUNT <= render_commands->line_vertices_max_count);
  collection_add(render_commands->line_loop_lengths, CIRCLE_VERTEX_COUNT);
  auto vertices = render_commands->line_vertices + render_commands->line_vertices_count;

  for (i32 i = 0; i < CIRCLE_VERTEX_COUNT; i++) {
    vertices[i].world_x = world_x + cosf((f32)i / 100.0f * 2.0f * PI) * radius;
    vertices[i].world_y = world_y + sinf((f32)i / 100.0f * 2.0f * PI) * radius;
    vertices[i].world_z = world_z;
    vertices[i].color = c;
  }

  render_commands->line_vertices_count += CIRCLE_VERTEX_COUNT;
}

void push_quad_frame(RenderCommands* render_commands,
                     f32 world_x0, f32 world_y0, f32 world_z0, u32 c0,
                     f32 world_x1, f32 world_y1, f32 world_z1, u32 c1, 
                     f32 world_x2, f32 world_y2, f32 world_z2, u32 c2, 
                     f32 world_x3, f32 world_y3, f32 world_z3, u32 c3) {
  assert(render_commands->line_vertices_count + 4 <= render_commands->line_vertices_max_count);
  collection_add(render_commands->line_loop_lengths, 4);
  auto vertices = render_commands->line_vertices + render_commands->line_vertices_count;

  vertices[0].world_x = world_x0;
  vertices[0].world_y = world_y0;
  vertices[0].world_z = world_z0;
  vertices[0].color = c0;

  vertices[1].world_x = world_x1;
  vertices[1].world_y = world_y1;
  vertices[1].world_z = world_z1;
  vertices[1].color = c1;

  vertices[2].world_x = world_x2;
  vertices[2].world_y = world_y2;
  vertices[2].world_z = world_z2;
  vertices[2].color = c2;

  vertices[3].world_x = world_x3;
  vertices[3].world_y = world_y3;
  vertices[3].world_z = world_z3;
  vertices[3].color = c3;

  render_commands->line_vertices_count += 4;
}

void set_camera_position(RenderCommands* render_commands, f32 world_x, f32 world_y) {
  render_commands->view_center_x = world_x;
  render_commands->view_center_y = world_y;
}

void set_camera_scale(RenderCommands* render_commands, f32 world_width, f32 world_height) {
  render_commands->view_width = world_width;
  render_commands->view_height = world_height;
}
