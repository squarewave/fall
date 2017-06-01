#include "gl/glew.h"

#include "platform.h"
#include "renderer_opengl.h"
#include "render_commands.h"

OpenGLState g_opengl_state = {0};

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

#define compile_program(type) (OpenGLProgram_##type*)create_program_(#type, sizeof(OpenGLProgram_##type), OpenGLProgramType_##type)
void* create_program_(const char* type_str, size_t size, OpenGLProgramType type) {
  assert(!g_opengl_state.programs[type]);
  g_opengl_state.programs[type] = (OpenGLProgramBase*)g_opengl_state.program_memory + g_opengl_state.program_memory_used;
  g_opengl_state.program_memory_used += size;
  OpenGLProgramBase* result = g_opengl_state.programs[type];

  char filename_buffer[64];
  sprintf(filename_buffer, "shaders/%s.glsl", type_str);
  auto common_source = g_platform.DEBUG_read_entire_file("shaders/common.glsl");
  auto program_source = g_platform.DEBUG_read_entire_file(filename_buffer);

  const char* vertex_defines = R"(
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

  const char* fragment_defines = R"(
    #version 330
    #define FRAGMENT_SHADER 1
  )";
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

  return result;
}

void use_program_begin(OpenGLProgramBase* program) {
  glUseProgram(program->program_id);

  glEnableVertexAttribArray(program->world_coords);
  glVertexAttribPointer(program->world_coords,
                        2, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, world_x)); // pointer

  glEnableVertexAttribArray(program->uv_coords);
  glVertexAttribPointer(program->uv_coords,
                        2, // size
                        GL_FLOAT, // type
                        false, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, u)); // pointer

  glEnableVertexAttribArray(program->color);
  glVertexAttribPointer(program->color,
                        4, // size
                        GL_UNSIGNED_BYTE, // type
                        true, // normalized
                        sizeof(TexturedQuadVertex), // stride
                        OFFSET_OF(TexturedQuadVertex, color)); // pointer
}

void use_program_end(OpenGLProgramBase* program) {
  glUseProgram(0);
  glDisableVertexAttribArray(program->world_coords);
  glDisableVertexAttribArray(program->uv_coords);
  glDisableVertexAttribArray(program->color);
}

void opengl_init(i32 window_width, i32 window_height) {
  compile_program(textured_quads);
  init_uniform(textured_quads, texture_sampler);

  GLuint dummy_vao;
  glGenVertexArrays(1, &dummy_vao);
  glBindVertexArray(dummy_vao);

  glGenBuffers(1, &g_opengl_state.vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, g_opengl_state.vertex_buffer);
}

void opengl_render_commands(RenderCommands* render_commands, i32 window_width, i32 window_height) {
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glEnable(GL_SCISSOR_TEST);

  glViewport(0, 0, window_width, window_height);
  glScissor(0, 0, window_width, window_height);
  glClearDepth(1.0f);
  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  use_program_begin((OpenGLProgramBase*)get_program(textured_quads));

  glBindBuffer(GL_ARRAY_BUFFER, g_opengl_state.vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER,
               render_commands->vertex_count * sizeof(TexturedQuadVertex),
               render_commands->vertex_array,
               GL_STREAM_DRAW);

  auto textured_quads = get_program(textured_quads);
  glUniform1i(textured_quads->texture_sampler, 0);
  GLfloat view_transform[9] = {
    2.0f / render_commands->view_width, 0, -2.0f * render_commands->view_center_x / render_commands->view_width,
    0, 2.0f / render_commands->view_height, -2.0f * render_commands->view_center_y / render_commands->view_height,
    0, 0, 1.0f
  };
  glUniformMatrix3fv(textured_quads->base.view_transform, 1, GL_TRUE, view_transform);

  glActiveTexture(GL_TEXTURE0);
  for (int i = 0; i < render_commands->vertex_count; i += 4) {
    void* texture = render_commands->quad_textures[i / 4];
    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)(texture));
    glDrawArrays(GL_TRIANGLE_STRIP, i, 4);
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  use_program_end((OpenGLProgramBase*)get_program(textured_quads));
}
