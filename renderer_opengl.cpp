#include "gl/glew.h"

#include "platform.h"
#include "geometry.h"
#include "renderer_opengl.h"
#include "render_commands.h"

static OpenGLState g_opengl_state = {0};

#define get_program(type) ((OpenGLProgram_##type*)g_opengl_state.programs[OpenGLProgramType_##type])

#define init_uniform(type, name) get_program(type)->name = glGetUniformLocation(get_program(type)->base.program_id, #name);

void validate_shader(u32 item_id, GLenum status_id) {
  GLint success = GL_FALSE;
  GLint info_log_length = 0;
  glGetShaderiv(item_id, status_id, &success);
  if (success != GL_TRUE) {
    glGetShaderiv(item_id, GL_INFO_LOG_LENGTH, &info_log_length);
    char* buffer = (char*)alloca(info_log_length);
    glGetShaderInfoLog(item_id, info_log_length, NULL, buffer);
    assert(!"Shader validation failed.");
  }
}

void validate_program(u32 item_id, GLenum status_id) {
  GLint success = GL_FALSE;
  GLint info_log_length = 0;
  glGetProgramiv(item_id, status_id, &success);
  if (success != GL_TRUE) {
    glGetProgramiv(item_id, GL_INFO_LOG_LENGTH, &info_log_length);
    char* buffer = (char*)alloca(info_log_length);
    glGetProgramInfoLog(item_id, info_log_length, NULL, buffer);
    assert(!"Program validation failed.");
  }
}

#define compile_program(file_name, type, depth_peeling) (OpenGLProgram_##type*)create_program_(file_name, sizeof(OpenGLProgram_##type), OpenGLProgramType_##type, depth_peeling)
void* create_program_(const char* type_str, size_t size, OpenGLProgramType type, b32 depth_peeling) {
  assert(!g_opengl_state.programs[type]);
  assert(g_opengl_state.program_memory_used + size < ARRAY_LENGTH(g_opengl_state.program_memory));
  g_opengl_state.programs[type] = (OpenGLProgramBase*)(g_opengl_state.program_memory + g_opengl_state.program_memory_used);
  g_opengl_state.program_memory_used += size;
  OpenGLProgramBase* result = g_opengl_state.programs[type];

  char filename_buffer[64];
  sprintf(filename_buffer, "shaders/%s.glsl", type_str);
  auto common_source = g_platform.DEBUG_read_entire_file("shaders/common.glsl");
  auto program_source = g_platform.DEBUG_read_entire_file(filename_buffer);

  char* vertex_defines = R"(
    #version 330
    #define VERTEX_SHADER 1
  )";
  GLchar* vertex_sources[] = {
    (GLchar*)vertex_defines,
    (GLchar*)common_source.contents,
    (GLchar*)program_source.contents
  };
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, ARRAY_LENGTH(vertex_sources), vertex_sources, NULL);
  glCompileShader(vertex_shader_id);
  validate_shader(vertex_shader_id, GL_COMPILE_STATUS);

  char* fragment_defines;
  if (depth_peeling) {
    fragment_defines = R"(
      #version 330
      #define FRAGMENT_SHADER 1
      #define DEPTH_PEELING 1
    )";
  } else {
    fragment_defines = R"(
      #version 330
      #define FRAGMENT_SHADER 1
    )";
  }
  GLchar* fragment_sources[] = {
    (GLchar*)fragment_defines,
    (GLchar*)common_source.contents,
    (GLchar*)program_source.contents
  };
  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, ARRAY_LENGTH(fragment_sources), fragment_sources, NULL);
  glCompileShader(fragment_shader_id);
  validate_shader(fragment_shader_id, GL_COMPILE_STATUS);

  GLuint program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  validate_program(program_id, GL_LINK_STATUS);

  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);
  g_platform.DEBUG_free_file_memory(common_source.contents);
  g_platform.DEBUG_free_file_memory(program_source.contents);

  result->program_id = program_id;
  result->world_coords = glGetAttribLocation(program_id, "world_coords");
  result->uv_coords = glGetAttribLocation(program_id, "uv_coords");
  result->color = glGetAttribLocation(program_id, "color");
  result->view_transform = glGetUniformLocation(program_id, "view_transform");

  if (depth_peeling) {
    result->depth_sampler = glGetUniformLocation(program_id, "depth_sampler");
  }

  assert(!glGetError());

  return result;
}

OpenGLFrameBuffer create_frame_buffer(i32 width, i32 height) {
  assert(glGetError() == GL_NO_ERROR);

  OpenGLFrameBuffer result = {};

  glGenFramebuffers(1, &result.handle);
  glBindFramebuffer(GL_FRAMEBUFFER, result.handle);

  glGenTextures(1, &result.color_buffer);
  glBindTexture(GL_TEXTURE_2D, result.color_buffer);

  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA8,
               width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.color_buffer, 0);

  glGenTextures(1, &result.depth_buffer);
  glBindTexture(GL_TEXTURE_2D, result.depth_buffer);

  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_DEPTH_COMPONENT24,
               width, height, 0,
               GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, result.depth_buffer, 0);

  glBindTexture(GL_TEXTURE_2D, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  return result;
}

void use_program_begin(OpenGLProgram_textured_quads* program) {
  GLenum error;

  glUseProgram(program->base.program_id);

  glEnableVertexAttribArray(program->base.world_coords);
  glVertexAttribPointer(program->base.world_coords,
                        3, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, world_x)); // pointer

  glEnableVertexAttribArray(program->base.uv_coords);
  glVertexAttribPointer(program->base.uv_coords,
                        2, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, u)); // pointer

  glEnableVertexAttribArray(program->base.color);
  glVertexAttribPointer(program->base.color,
                        4, // size
                        GL_UNSIGNED_BYTE, // type
                        true, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, color)); // pointer
}

void use_program_begin(OpenGLProgram_textured_quads_depth_peeling* program) {
  GLenum error;

  glUseProgram(program->base.program_id);
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.world_coords);
  glVertexAttribPointer(program->base.world_coords,
                        3, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, world_x)); // pointer
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.uv_coords);
  glVertexAttribPointer(program->base.uv_coords,
                        2, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, u)); // pointer
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.color);
  glVertexAttribPointer(program->base.color,
                        4, // size
                        GL_UNSIGNED_BYTE, // type
                        true, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, color)); // pointer
  error = glGetError();
  assert(!error);

  glUniform1i(program->base.depth_sampler, 1);
  error = glGetError();
  assert(!error);
}

void use_program_begin(OpenGLProgram_lines* program) {
  GLenum error;

  glUseProgram(program->base.program_id);
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.world_coords);
  glVertexAttribPointer(program->base.world_coords,
                        3, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(LineVertex), // stride
                        OFFSET_OF(LineVertex, world_x)); // pointer
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.color);
  glVertexAttribPointer(program->base.color,
                        4, // size
                        GL_UNSIGNED_BYTE, // type
                        true, // normalized
                        sizeof(LineVertex), // stride
                        OFFSET_OF(LineVertex, color)); // pointer
  error = glGetError();
  assert(!error);
}

void use_program_begin(OpenGLProgram_lines_depth_peeling* program) {
  GLenum error;
  glUseProgram(program->base.program_id);
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.world_coords);
  glVertexAttribPointer(program->base.world_coords,
                        3, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(LineVertex), // stride
                        OFFSET_OF(LineVertex, world_x)); // pointer
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.color);
  glVertexAttribPointer(program->base.color,
                        4, // size
                        GL_UNSIGNED_BYTE, // type
                        true, // normalized
                        sizeof(LineVertex), // stride
                        OFFSET_OF(LineVertex, color)); // pointer
  error = glGetError();
  assert(!error);

  glUniform1i(program->base.depth_sampler, 1);
  error = glGetError();
  assert(!error);
}

void use_program_begin(OpenGLProgram_peel_composite* program) {
  GLenum error;

  glUseProgram(program->base.program_id);
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.world_coords);
  glVertexAttribPointer(program->base.world_coords,
                        3, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, world_x)); // pointer
  error = glGetError();
  assert(!error);

  glEnableVertexAttribArray(program->base.uv_coords);
  glVertexAttribPointer(program->base.uv_coords,
                        2, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, u)); // pointer
  error = glGetError();
  assert(!error);

  for (i32 i = 0; i < ARRAY_LENGTH(program->peel_samplers); i++) {
    glUniform1i(program->peel_samplers[i], i);
    error = glGetError();
    assert(!error);
  }
}

void use_program_end(OpenGLProgramBase* program) {
  glUseProgram(0);
  glDisableVertexAttribArray(program->world_coords);
  glDisableVertexAttribArray(program->uv_coords);
  glDisableVertexAttribArray(program->color);
  glGetError();
}

void opengl_init(i32 window_width, i32 window_height) {
  compile_program("textured_quads", textured_quads, false);
  init_uniform(textured_quads, texture_sampler);

  compile_program("lines", lines, false);

  compile_program("textured_quads", textured_quads_depth_peeling, true);
  init_uniform(textured_quads_depth_peeling, texture_sampler);

  compile_program("lines", lines_depth_peeling, true);

  compile_program("peel_composite", peel_composite, false);
  auto peel_composite_program = get_program(peel_composite);
  peel_composite_program->peel_samplers[0] = glGetUniformLocation(peel_composite_program->base.program_id, "peel_sampler0");
  peel_composite_program->peel_samplers[1] = glGetUniformLocation(peel_composite_program->base.program_id, "peel_sampler1");
  peel_composite_program->peel_samplers[2] = glGetUniformLocation(peel_composite_program->base.program_id, "peel_sampler2");
  peel_composite_program->peel_samplers[3] = glGetUniformLocation(peel_composite_program->base.program_id, "peel_sampler3");

  GLuint dummy_vao;
  glGenVertexArrays(1, &dummy_vao);
  glBindVertexArray(dummy_vao);

  glGenBuffers(1, &g_opengl_state.vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, g_opengl_state.vertex_buffer);

  for (i32 i = 0; i < ARRAY_LENGTH(g_opengl_state.peel_buffers); i++) {
    g_opengl_state.peel_buffers[i] = create_frame_buffer(window_width, window_height);
  }
}

void opengl_render_commands(RenderCommands* render_commands, i32 window_width, i32 window_height) {
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);

  for (i32 i = 0; i < ARRAY_LENGTH(g_opengl_state.peel_buffers); i++) {
    GLenum error;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_opengl_state.peel_buffers[i].handle);
    glViewport(0, 0, window_width, window_height);
    glScissor(0, 0, window_width, window_height);
    glClearDepth(1.0f);
    glClearColor(render_commands->clear_r, render_commands->clear_g, render_commands->clear_b, 1.0f);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    error = glGetError();
    assert(!error);

    b32 peeling = i > 0;
    f32 x_offset = -2.0f * render_commands->view_center_x / render_commands->view_width;
    f32 x_scale = 2.0f / render_commands->view_width;
    f32 y_scale = 2.0f / render_commands->view_height;
    f32 y_offset = -2.0f * render_commands->view_center_y / render_commands->view_height;
    f32 z_scale = 0.9f / render_commands->view_height;
    f32 z_offset = -0.9f * render_commands->view_center_y / render_commands->view_height + 0.45f;
    GLfloat view_transform[16] = {
      x_scale, 0, 0, x_offset,
      0, y_scale, 0, y_offset,
      0, 0, z_scale, z_offset,
      0, 0, 0, 1.0f
    };

    if (peeling) {
      use_program_begin(get_program(textured_quads_depth_peeling));
    } else {
      use_program_begin(get_program(textured_quads));
    }
    error = glGetError();
    assert(!error);

    glBindBuffer(GL_ARRAY_BUFFER, g_opengl_state.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 render_commands->textured_quad_vertices_count * sizeof(TexturedQuadVertex),
                 render_commands->textured_quad_vertices,
                 GL_STREAM_DRAW);
    error = glGetError();
    assert(!error);

    if (peeling) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, g_opengl_state.peel_buffers[i - 1].depth_buffer);
      glActiveTexture(GL_TEXTURE0);

      auto textured_quads = get_program(textured_quads_depth_peeling);
      glUniform1i(textured_quads->texture_sampler, 0);
      glUniformMatrix4fv(textured_quads->base.view_transform, 1, GL_TRUE, view_transform);
    } else {
      auto textured_quads = get_program(textured_quads);
      glUniform1i(textured_quads->texture_sampler, 0);
      glUniformMatrix4fv(textured_quads->base.view_transform, 1, GL_TRUE, view_transform);
    }
    error = glGetError();
    assert(!error);

    glActiveTexture(GL_TEXTURE0);
    for (int j = 0; j < render_commands->textured_quad_vertices_count; j += 4) {
      void* texture = render_commands->quad_textures[j / 4];
      glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)(texture));
      glDrawArrays(GL_TRIANGLE_STRIP, j, 4);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    error = glGetError();
    assert(!error);

    if (peeling) {
      use_program_end((OpenGLProgramBase*)get_program(textured_quads_depth_peeling));
      error = glGetError();
      assert(!error);
      use_program_begin(get_program(lines_depth_peeling));
      error = glGetError();
      assert(!error);
    } else {
      use_program_end((OpenGLProgramBase*)get_program(textured_quads));
      error = glGetError();
      assert(!error);
      use_program_begin(get_program(lines));
      error = glGetError();
      assert(!error);
    }

    glBindBuffer(GL_ARRAY_BUFFER, g_opengl_state.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 render_commands->line_vertices_count * sizeof(LineVertex),
                 render_commands->line_vertices,
                 GL_STREAM_DRAW);
    error = glGetError();
    assert(!error);

    if (peeling) {
      auto circles = get_program(lines_depth_peeling);
      glUniformMatrix4fv(circles->base.view_transform, 1, GL_TRUE, view_transform);
    } else {
      auto circles = get_program(lines);
      glUniformMatrix4fv(circles->base.view_transform, 1, GL_TRUE, view_transform);
    }
    error = glGetError();
    assert(!error);

    i32 count_index = 0;
    for (i32 j = 0; j < render_commands->line_vertices_count; j += render_commands->line_loop_lengths[count_index++]) {
      glDrawArrays(GL_LINE_LOOP, j, render_commands->line_loop_lengths[count_index]);
    }
    error = glGetError();
    assert(!error);
    if (peeling) {
      use_program_end((OpenGLProgramBase*)get_program(lines_depth_peeling));

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTexture(GL_TEXTURE0);
    } else {
      use_program_end((OpenGLProgramBase*)get_program(lines));
    }
    error = glGetError();
    assert(!error);
  }

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, window_width, window_height);
  glScissor(0, 0, window_width, window_height);
  glClearDepth(1.0f);
  glClearColor(render_commands->clear_r, render_commands->clear_g, render_commands->clear_b, 0.0f);
  glDepthFunc(GL_LESS);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  TexturedQuadVertex vertices[] =
  {
    { 0.0f, 1.0f, -1.0f,  1.0f, 0.0f, 0xFFFFFFFF },
    { 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0xFFFFFFFF },
    { 1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0xFFFFFFFF },
    { 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0xFFFFFFFF },
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

  use_program_begin(get_program(peel_composite));
  for (i32 i = 0; i < ARRAY_LENGTH(g_opengl_state.peel_buffers); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, g_opengl_state.peel_buffers[i].color_buffer);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  for (i32 i = 0; i < ARRAY_LENGTH(g_opengl_state.peel_buffers); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glActiveTexture(GL_TEXTURE0);
  use_program_end((OpenGLProgramBase*)get_program(peel_composite));

}
 