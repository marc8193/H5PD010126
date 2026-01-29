#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <stdlib.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "GLES3", __VA_ARGS__))

// Vertex Shader
const char* vertexShaderSrc =
  "#version 300 es\n"
  "layout(location = 0) in vec3 aPos;\n"
  "void main() { gl_Position = vec4(aPos, 1.0); }\n";

// Fragment Shader
const char* fragmentShaderSrc =
  "#version 300 es\n"
  "precision mediump float;\n"
  "out vec4 FragColor;\n"
  "void main() { FragColor = vec4(1.0, 0.5, 0.2, 1.0); }\n";

// OpenGL objects
GLuint shaderProgram, VAO;

// EGL objects
EGLDisplay display;
EGLSurface surface;
EGLContext context;

// Simple triangle setup
void initGLES() {
  float vertices[] = {
    0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f
  };

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
  glEnableVertexAttribArray(0);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
  glCompileShader(fragmentShader);

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  LOGI("GLES3 initialized");
}

// Render a single frame
void renderFrame() {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(shaderProgram);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  eglSwapBuffers(display, surface);
}

// Initialize EGL
int initEGL(ANativeWindow* window) {
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

  LOGI("EGL and GLES3 context initialized");
  return 1;
}

void android_main(struct android_app* app) {
  app->onAppCmd = NULL; // Simplest case

  // Wait until window is available
  while (!app->window) {
    struct android_poll_source* source;
    int events;

    // Poll once (timeout 0 ms)
    int ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }
  }

  initEGL(app->window);
  initGLES();

  // Main loop
  while (1) {
    struct android_poll_source* source;
    int events;

    // Poll once per iteration (timeout 0 ms)
    int ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }

    renderFrame();
  }
}
