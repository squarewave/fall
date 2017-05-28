#include "assert.h"
#include "render_commands.h"

RenderCommands init_render_commands(i32 max_vertex_count, TexturedQuadVertex* vertex_array, void** textures_array) {
  RenderCommands result;
  result.vertex_array = vertex_array;
  result.quad_textures = textures_array;
  result.max_vertex_count = max_vertex_count;
  return result;
}

void reset_render_commands(RenderCommands* render_commands) {
  render_commands->vertex_count = 0;
}

void push_textured_quad(RenderCommands* render_commands,
                        void* texture_handle,
                        /* bottom left */ f32 u0, f32 v0, f32 world_x0, f32 world_y0, u32 color0,
                        /* bottom right */ f32 u1, f32 v1, f32 world_x1, f32 world_y1, u32 color1,
                        /* top left */ f32 u2, f32 v2, f32 world_x2, f32 world_y2, u32 color2,
                        /* top right */ f32 u3, f32 v3, f32 world_x3, f32 world_y3, u32 color3) {
  assert(render_commands->vertex_count + 4 <= render_commands->max_vertex_count);

  TexturedQuadVertex* vertices = render_commands->vertex_array + render_commands->vertex_count;
  render_commands->quad_textures[render_commands->vertex_count / 4] = texture_handle;

  vertices[0].u = u0;
  vertices[0].v = v0;
  vertices[0].world_x = world_x0;
  vertices[0].world_y = world_y0;
  vertices[0].color = color0;

  vertices[1].u = u1;
  vertices[1].v = v1;
  vertices[1].world_x = world_x1;
  vertices[1].world_y = world_y1;
  vertices[1].color = color1;

  vertices[2].u = u2;
  vertices[2].v = v2;
  vertices[2].world_x = world_x2;
  vertices[2].world_y = world_y2;
  vertices[2].color = color2;

  vertices[3].u = u3;
  vertices[3].v = v3;
  vertices[3].world_x = world_x3;
  vertices[3].world_y = world_y3;
  vertices[3].color = color3;

  render_commands->vertex_count += 4;
}

void set_camera_position(RenderCommands* render_commands, f32 world_x, f32 world_y) {
  render_commands->view_center_x = world_x;
  render_commands->view_center_y = world_y;
}

void set_camera_scale(RenderCommands* render_commands, f32 world_width, f32 world_height) {
  render_commands->view_width = world_width;
  render_commands->view_height = world_height;
}
