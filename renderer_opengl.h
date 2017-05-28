#ifndef RENDERER_OPENGL_H__
#define RENDERER_OPENGL_H__

#include "assert.h"
#include "gl/gl.h"

#include "platform.h"

enum OpenGLProgramType {
  OpenGLProgramType_textured_quads,
  OpenGLProgramType_Count,
};

struct OpenGLProgramBase {
  GLuint program_id;

  GLuint world_coords;
  GLuint uv_coords;
  GLuint color;

  GLuint view_transform;
};

struct OpenGLProgram_textured_quads {
  OpenGLProgramBase base;

  GLuint texture_sampler;
};

struct OpenGLState {
  i32 program_memory_used;
  char program_memory[1024];
  OpenGLProgramBase* programs[OpenGLProgramType_Count];
  GLuint vertex_buffer;
};

struct RenderCommands;

void opengl_init(i32 window_width, i32 window_height);
void opengl_render_commands(RenderCommands* render_commands, i32 window_width, i32 window_height);

#endif /* end of include guard: RENDERER_OPENGL_H__ */
