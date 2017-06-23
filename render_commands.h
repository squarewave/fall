#ifndef RENDER_COMMANDS_H__
#define RENDER_COMMANDS_H__

#include "platform.h"

#pragma pack (1)
struct TexturedQuadVertex {
  f32 u, v;
  f32 world_x, world_y, world_z;
  u32 color;
};

struct LineVertex {
  f32 world_x, world_y, world_z;
  u32 color;
};

struct RenderCommands {
  // View coordinates and dimensions are in world space.
  f32 view_center_x, view_center_y;
  f32 view_width, view_height;
  f32 screen_width, screen_height;
  TexturedQuadVertex* textured_quad_vertices;
  i32 textured_quad_vertices_count;
  i32 textured_quad_vertices_max_count;
  void** quad_textures;
  LineVertex* line_vertices;
  i32* line_loop_lengths;
  i32 line_loop_lengths_count;
  i32 line_vertices_count;
  i32 line_vertices_max_count;
};

void push_textured_quad(RenderCommands* render_commands,
                        void* texture_handle,
                        f32 u0, f32 v0, f32 world_x0, f32 world_y0, f32 world_z0, u32 c0,
                        f32 u1, f32 v1, f32 world_x1, f32 world_y1, f32 world_z1, u32 c1,
                        f32 u2, f32 v2, f32 world_x2, f32 world_y2, f32 world_z2, u32 c2,
                        f32 u3, f32 v3, f32 world_x3, f32 world_y3, f32 world_z3, u32 c3);

void push_circle_frame(RenderCommands* render_commands,
                       f32 world_x, f32 world_y, f32 world_z, f32 radius, u32 c);

void push_quad_frame(RenderCommands* render_commands,
                     f32 world_x0, f32 world_y0, f32 world_z0, u32 c0,
                     f32 world_x1, f32 world_y1, f32 world_z1, u32 c1,
                     f32 world_x2, f32 world_y2, f32 world_z2, u32 c2,
                     f32 world_x3, f32 world_y3, f32 world_z3, u32 c3);

void set_camera_position(RenderCommands* render_commands, f32 world_x, f32 world_y);

void set_camera_scale(RenderCommands* render_commands, f32 world_width, f32 world_height);

#endif /* end of include guard: RENDER_COMMANDS_H__ */
