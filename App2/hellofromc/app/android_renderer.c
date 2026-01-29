#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "renderer.h"
#include "atlas.inl"
#include "log.h"

EGLDisplay display;
EGLSurface surface;
EGLContext context;

static int buffer_index;
static int width;
static int height;

static GLuint program;
static GLuint vertex_array_object;
static GLuint vertex_buffer_objects[3];
static GLuint element_buffer_object;
static GLint u_projection_loc;

#define MAX_VERTICES 4096

// Position buffer (2 floats per vertex)
GLsizeiptr buffer_size_pos = MAX_VERTICES * 2 * sizeof(float);

// UV buffer (2 floats per vertex)
GLsizeiptr buffer_size_uv  = MAX_VERTICES * 2 * sizeof(float);

// Color buffer (4 bytes per vertex)
GLsizeiptr buffer_size_color = MAX_VERTICES * 4 * sizeof(uint8_t);

#define BUFFER_SIZE 16384

static GLfloat   texture_buffer[BUFFER_SIZE *  8];
static GLfloat  vertex_buffer[BUFFER_SIZE *  8];
static GLubyte color_buffer[BUFFER_SIZE * 16];
static GLuint  index_buffer[BUFFER_SIZE *  6];

static const char *vertex_shader_src = R"(
#version 300 es
precision mediump float;

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

uniform mat4 u_proj;

out vec2 v_uv;
out vec4 v_color;

void main() {
    gl_Position = u_proj * vec4(a_pos, 0.0, 1.0);
    v_uv = a_uv;
    v_color = a_color;
}
)";

static const char *fragment_shader_src = R"(
#version 300 es
precision mediump float;

in vec2 v_uv;
in vec4 v_color;
uniform sampler2D u_tex;
out vec4 fragColor;

void main() {
    // single-channel atlas texture
    float alpha = texture(u_tex, v_uv).r;
    fragColor = vec4(v_color.rgb, v_color.a * alpha);
}
)";

void r_init(ANativeWindow* window) {
  width  = ANativeWindow_getWidth(window);
  height = ANativeWindow_getHeight(window);

  // Init EGL
  const EGLint attribs[] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_NONE
  };

  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(display, 0, 0);

  EGLConfig config;
  EGLint numConfigs;
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  surface = eglCreateWindowSurface(display, config, window, NULL);

  const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

  eglMakeCurrent(display, surface, surface, context);

  // Init GLES
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);

  glGenVertexArrays(1, &vertex_array_object);
  glBindVertexArray(vertex_array_object);

  glGenBuffers(3, vertex_buffer_objects);

  glGenBuffers(1, &element_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_VERTICES * 6 * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);

  // Position
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[0]);
  glBufferData(GL_ARRAY_BUFFER, buffer_size_pos, NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // UV
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[1]);
  glBufferData(GL_ARRAY_BUFFER, buffer_size_uv, NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  // Color
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[2]);
  glBufferData(GL_ARRAY_BUFFER, buffer_size_color, NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
  glEnableVertexAttribArray(2);
  
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
  glCompileShader(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
  glCompileShader(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Init texture
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE,
			   atlas_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  assert(glGetError() == 0);
}

static void flush(void) {
  if (buffer_index == 0) return;

  glViewport(0, 0, width, height);
  glUseProgram(program);
  
  float ortho_projection[16] = {
	2.0f / width, 0,              0, 0,
	0,            2.0f / height,  0, 0,
	0,            0,             -1, 0,
   -1,           -1,              0, 1
  };

  glUniformMatrix4fv(u_projection_loc, 1, GL_FALSE, ortho_projection);

  glBindVertexArray(vertex_array_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[0]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_index * sizeof(float)*2, vertex_buffer);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[1]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_index * sizeof(float)*2, texture_buffer);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[2]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_index * sizeof(uint32_t)*4, color_buffer);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);

  glDrawElements(GL_TRIANGLES, buffer_index * 6, GL_UNSIGNED_INT, 0);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, buffer_index * 6 * sizeof(uint32_t), index_buffer);

  glBindVertexArray(0);

  buffer_index = 0;
}

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
  if (buffer_index == BUFFER_SIZE) { flush(); }

  int texture_vertex_index = buffer_index * 8;
  int color_index = buffer_index * 16;
  int element_index = buffer_index * 4;
  int index_index = buffer_index * 6;
  buffer_index++;

  // Update texture buffer
  float x = src.x / (float) ATLAS_WIDTH;
  float y = src.y / (float) ATLAS_HEIGHT;
  float w = src.w / (float) ATLAS_WIDTH;
  float h = src.h / (float) ATLAS_HEIGHT;
  texture_buffer[texture_vertex_index + 0] = x;
  texture_buffer[texture_vertex_index + 1] = y;
  texture_buffer[texture_vertex_index + 2] = x + w;
  texture_buffer[texture_vertex_index + 3] = y;
  texture_buffer[texture_vertex_index + 4] = x;
  texture_buffer[texture_vertex_index + 5] = y + h;
  texture_buffer[texture_vertex_index + 6] = x + w;
  texture_buffer[texture_vertex_index + 7] = y + h;

  // Update vertex buffer
  vertex_buffer[texture_vertex_index + 0] = dst.x;
  vertex_buffer[texture_vertex_index + 1] = dst.y;
  vertex_buffer[texture_vertex_index + 2] = dst.x + dst.w;
  vertex_buffer[texture_vertex_index + 3] = dst.y;
  vertex_buffer[texture_vertex_index + 4] = dst.x;
  vertex_buffer[texture_vertex_index + 5] = dst.y + dst.h;
  vertex_buffer[texture_vertex_index + 6] = dst.x + dst.w;
  vertex_buffer[texture_vertex_index + 7] = dst.y + dst.h;

  // Update color buffer
  memcpy(color_buffer + color_index +  0, &color, 4);
  memcpy(color_buffer + color_index +  4, &color, 4);
  memcpy(color_buffer + color_index +  8, &color, 4);
  memcpy(color_buffer + color_index + 12, &color, 4);

  // Update index buffer
  index_buffer[index_index + 0] = element_index + 0;
  index_buffer[index_index + 1] = element_index + 1;
  index_buffer[index_index + 2] = element_index + 2;
  index_buffer[index_index + 3] = element_index + 2;
  index_buffer[index_index + 4] = element_index + 3;
  index_buffer[index_index + 5] = element_index + 1;
}

void r_draw_rect(mu_Rect rect, mu_Color color) {
  push_quad(rect, atlas[ATLAS_WHITE], color);
}

void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  mu_Rect dst = { pos.x, pos.y, 0, 0 };
  for (const char *p = text; *p; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    mu_Rect src = atlas[ATLAS_FONT + chr];
    dst.w = src.w;
    dst.h = src.h;
    push_quad(dst, src, color);
    dst.x += dst.w;
  }
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(mu_rect(x, y, src.w, src.h), src, color);
}

int r_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}

int r_get_text_height(void) {
  return 18;
}

void r_set_clip_rect(mu_Rect rect) {
  flush();
  glScissor(rect.x, height - (rect.y + rect.h), rect.w, rect.h);
}

void r_clear(mu_Color clr) {
  flush();
  glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
  glClear(GL_COLOR_BUFFER_BIT);
}

void r_present(void) {
  flush();
  eglSwapBuffers(display, surface);
}
