#ifndef RENDERER_OPENGL_H__
#define RENDERER_OPENGL_H__

#include "assert.h"
#include "gl/gl.h"

#include "platform.h"

enum OpenGLProgramType {
  OpenGLProgramType_lines,
  OpenGLProgramType_textured_quads,
  OpenGLProgramType_lines_depth_peeling,
  OpenGLProgramType_textured_quads_depth_peeling,
  OpenGLProgramType_peel_composite,
  OpenGLProgramType_count,
};

struct OpenGLProgramBase {
  GLuint program_id;

  GLuint world_coords;
  GLuint uv_coords;
  GLuint color;

  GLuint view_transform;

  GLuint depth_sampler;
};

struct OpenGLProgram_textured_quads {
  OpenGLProgramBase base;

  GLuint texture_sampler;
};

struct OpenGLProgram_textured_quads_depth_peeling {
  OpenGLProgramBase base;

  GLuint texture_sampler;
};

struct OpenGLProgram_lines {
  OpenGLProgramBase base;
};

struct OpenGLProgram_lines_depth_peeling {
  OpenGLProgramBase base;
};

struct OpenGLProgram_peel_composite {
  OpenGLProgramBase base;

  GLuint peel_samplers[4];
};

struct OpenGLFrameBuffer {
  GLuint handle;
  GLuint color_buffer;
  GLuint depth_buffer;
};

struct OpenGLState {
  i32 program_memory_used;
  char program_memory[1024];
  OpenGLProgramBase* programs[OpenGLProgramType_count];
  GLuint vertex_buffer;
  OpenGLFrameBuffer peel_buffers[4];
};

struct RenderCommands;

void opengl_init(i32 window_width, i32 window_height);
void opengl_render_commands(RenderCommands* render_commands, i32 window_width, i32 window_height);

#endif /* end of include guard: RENDERER_OPENGL_H__ */
