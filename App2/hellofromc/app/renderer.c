#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "renderer.h"
#include "atlas.inl"
#include "log.h"

#define MAX_VERTICES 4096
#define MAX_QUADS (MAX_VERTICES / 4)
#define MAX_INDICES (MAX_QUADS * 6)

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;

static int width;
static int height;

static GLuint vertex_array_object;
static GLuint vertex_buffer_object;
static GLuint element_buffer_object;

static GLuint shader_program;
static GLuint texture;

typedef struct {
  GLfloat position[2];
  GLfloat texture_coordinates[2];
  GLubyte color[4];
} Vertex;

static Vertex vertices[MAX_VERTICES];
static GLuint indices[MAX_INDICES];
static int buffer_index;

static const char* vertex_shader_source = R"(#version 300 es
precision mediump float;

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texture_coordinates;
layout (location = 2) in vec4 a_color;

uniform mat4 u_projection;

out vec2 texture_coordinates;
out vec4 color;

void main()
{
  gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
  texture_coordinates = a_texture_coordinates;
  color = a_color;
}
)";

static const char* fragment_shader_source = R"(#version 300 es
precision mediump float;

in vec2 texture_coordinates;
in vec4 color;

uniform sampler2D u_texture;

out vec4 o_color;

void main()
{
  float alpha = texture(u_texture, texture_coordinates).r; 
  o_color = vec4(color.rgb, color.a * alpha);
} 
)";

void r_init(ANativeWindow* window) {
  width  = ANativeWindow_getWidth(window);
  height = ANativeWindow_getHeight(window);
  
  /* Init EGL */
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

  /* Init GLES */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  
  glViewport(0, 0, width, height);
   
  glGenBuffers(1, &vertex_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);

  glGenVertexArrays(1, &vertex_array_object);
  glBindVertexArray(vertex_array_object);

  /* Position */
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glEnableVertexAttribArray(0);

  /* Texture Coordinates */
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						(void*)offsetof(Vertex, texture_coordinates));
  
  glEnableVertexAttribArray(1);

  /* Color */
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
  glEnableVertexAttribArray(2);

  glGenBuffers(1, &element_buffer_object);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLuint), NULL, GL_STATIC_DRAW);
  
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);

  GLint success;
  size_t message_length = 512;
  char message[message_length];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
	glGetShaderInfoLog(vertex_shader, message_length, NULL, message);
	fprintf(stderr, "Shader compilation failed: %s\n", message);
	assert(0);
  }

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
	glGetShaderInfoLog(fragment_shader, message_length, NULL, message);
	fprintf(stderr, "Shader compilation failed: %s\n", message);
	assert(0);
  }
  
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
	glGetProgramInfoLog(shader_program, message_length, NULL, message);
	fprintf(stderr, "Shader program linking failed: %s\n", message);
	assert(0);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE,
			   atlas_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  assert(glGetError() == 0);
}

static void flush(void) {
  if (buffer_index == 0) return;

  glUseProgram(shader_program);
  
  float ortho_projection[16] = {
	2.0f / width, 0,              0, 0,
	0,           -2.0f / height,  0, 0,
	0,            0,             -1, 0,
   -1,            1,              0, 1
  };

  GLint u_projection_location = glGetUniformLocation(shader_program, "u_projection");
  glUniformMatrix4fv(u_projection_location, 1, GL_FALSE, ortho_projection);

  glBindTexture(GL_TEXTURE_2D, texture);
  
  glBindVertexArray(vertex_array_object);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_index * 4 * sizeof(Vertex), vertices);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, buffer_index * 6 * sizeof(GLuint), indices);

  glDrawElements(GL_TRIANGLES, buffer_index * 6, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);

  buffer_index = 0;
}

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
  if (buffer_index == MAX_QUADS) {
	flush();
  }
  
  int base_vertex = buffer_index * 4; /* 4 vertices per quad */
  int base_index  = buffer_index * 6; /* 6 indices per quad */

  /* Top left */
  vertices[base_vertex + 0] = (Vertex){ .position = { dst.x, dst.y },
										.texture_coordinates = { src.x / (float) ATLAS_WIDTH,
																 src.y / (float) ATLAS_HEIGHT },
										
										.color = { color.r, color.g, color.b, color.a }};

  /* Top right */
  vertices[base_vertex + 1] = (Vertex){ .position = { dst.x + dst.w, dst.y },
										.texture_coordinates = { (src.x + src.w) /
																 (float) ATLAS_WIDTH,
																 src.y / (float) ATLAS_HEIGHT },
										
										.color = { color.r, color.g, color.b, color.a }};

  /* Bottom left */
  vertices[base_vertex + 2] = (Vertex){ .position = { dst.x, dst.y + dst.h },
										.texture_coordinates = { src.x / (float) ATLAS_WIDTH,
																 (src.y + src.h) /
																 (float) ATLAS_HEIGHT },
										
										.color = { color.r, color.g, color.b, color.a }};

  /* Bottom right */
  vertices[base_vertex + 3] = (Vertex){ .position = { dst.x + dst.w, dst.y + dst.h },
										.texture_coordinates = { (src.x + src.w) /
																 (float) ATLAS_WIDTH,
																 (src.y + src.h) /
																 (float) ATLAS_HEIGHT },
										
										.color = { color.r, color.g, color.b, color.a }};

  indices[base_index + 0] = base_vertex + 0;
  indices[base_index + 1] = base_vertex + 1;
  indices[base_index + 2] = base_vertex + 2;
  indices[base_index + 3] = base_vertex + 2;
  indices[base_index + 4] = base_vertex + 1;
  indices[base_index + 5] = base_vertex + 3;
  
  buffer_index++;
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
