#include <stdlib.h>
#include <string.h>

#include <android/input.h>
#include <android_native_app_glue.h>
#include <renderer.h>
#include <ui.h>
#include <widget.h>

static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
  return r_get_text_height();
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
	int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
	int32_t x = (int32_t) AMotionEvent_getX(event, 0);
	int32_t y = (int32_t) AMotionEvent_getY(event, 0);

	mu_Context* ctx = (mu_Context*) app->userData;

	switch (action) {
	case AMOTION_EVENT_ACTION_DOWN:
	case AMOTION_EVENT_ACTION_POINTER_DOWN:
	  mu_input_mousedown(ctx, x, y, MU_MOUSE_LEFT);
	  break;
	case AMOTION_EVENT_ACTION_UP:
	case AMOTION_EVENT_ACTION_POINTER_UP:
	  mu_input_mouseup(ctx, x, y, MU_MOUSE_LEFT);
	  break;
	case AMOTION_EVENT_ACTION_MOVE:
	  mu_input_mousemove(ctx, x, y);
	  break;
	}
	return 1;
  }

  return 0;
}

void android_main(struct android_app* app) {
  app->onAppCmd = NULL; // Simplest case
  app->onInputEvent = handle_input;

  struct android_poll_source* source;
  int events;
  int ident;

  // Wait until window is available
  while (!app->window) {
    // Poll once (timeout 0 ms)
    ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }
  }

  int window_width  = ANativeWindow_getWidth(app->window);
  int window_height = ANativeWindow_getHeight(app->window);
  
  r_init(app->window, window_width, window_height);
 
  /* init microui */
  mu_Context* ctx = malloc(sizeof(mu_Context));
  app->userData = ctx;
  mu_init(ctx);
  ctx->text_width = text_width;
  ctx->text_height = text_height;

  mu_Color background_color = mu_color(18, 18, 18, 255);
  ctx->style->colors[MU_COLOR_WINDOWBG] = background_color;
  ctx->style->colors[MU_COLOR_BASE] = mu_color(38, 38, 38, 255);

  Arena* arena = malloc(sizeof(Arena));
  unsigned char buffer[256] = {0};

  arena_init(arena, buffer, sizeof(buffer));
  
  /* main loop */
  for (;;) {
    // Poll once (timeout 0 ms)
    ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }

    /* process frame */
	mu_begin(ctx);

	UI_Action* action = ui_home(ctx, arena, mu_vec2(window_width, window_height), 75);
	if (action) {
	  switch (action->widget) {
	  case UI_HOME_HOTBAR_HOME_BUTTON:
		if (action->clicked) printf("Home button clicked\n");
		break;

	  case UI_HOME_HOTBAR_NEW_BUTTON:
		if (action->clicked) printf("New button clicked\n");
		break;

	  case UI_HOME_HOTBAR_SYNC_BUTTON:
		if (action->clicked) printf("Sync button clicked\n");
		break;
	  }
	}

	mu_end(ctx);
	
    /* render */
    r_clear(background_color);
    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
      switch (cmd->type) {
      case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
      case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
      case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
      case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
      }
    }
	
    r_present();
  }

  arena_free(arena);
  free(arena);

  free(ctx);
}
