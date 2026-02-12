#include <android/input.h>
#include <android_native_app_glue.h>
#include <stdlib.h>
#include <string.h>
#include <microui.h>

#include "renderer.h"
#include "log.h"

static mu_Context ctx;

static int window_width;
static int window_height;

static int topbar_margin = 75;
static mu_Color background_color = { .r = 18, .g = 18, .b = 18, .a = 255 };

int mu_hotbar(mu_Context *ctx) {
  char* name = "hotbar";
  mu_Id id = mu_get_id(ctx, &name, sizeof(name));
  mu_Rect base = mu_layout_next(ctx);
  int result = 0;

  mu_draw_control_frame(ctx, id, base, MU_COLOR_BASE, 0);

  int margin = 10;
  int button_width = (base.w / 3) - margin - (margin / 3);

  for (int i = 0; i < 3; i++) {
	char button_name[256] = {0};
	sprintf(button_name, "Button %i", i);
	mu_Id id = mu_get_id(ctx, button_name, sizeof(button_name));	
	mu_Rect button = mu_rect((base.x + margin) + (button_width + margin) * i, base.y + margin,
							 button_width, base.h - (margin * 2));

	mu_update_control(ctx, id, button, 0);
	if (ctx->mouse_pressed == MU_MOUSE_LEFT && ctx->focus == id) {
	  result |= MU_RES_SUBMIT + i;
	}

	mu_draw_control_frame(ctx, id, button, MU_COLOR_BUTTON, 0);
	mu_draw_control_text(ctx, button_name, button, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
  };

  return result;
}

static void home_window() {
  /* do window */
  if (mu_begin_window_ex(&ctx, "home",
						 mu_rect(0, topbar_margin, window_width, window_height - topbar_margin),
						 MU_OPT_NORESIZE | MU_OPT_NOTITLE)) {

	int rect_width = 480;
	int rect_height = 160;
	mu_Rect rect = mu_rect((window_width - rect_width) / 2, (window_height - rect_height) - 200,
						   rect_width, rect_height);
	
	mu_layout_set_next(&ctx, rect, 0);

	switch (mu_hotbar(&ctx)) {
	case MU_RES_SUBMIT + 0: printf("Clicked button 1\n"); break;
	case MU_RES_SUBMIT + 1: printf("Clicked button 2\n"); break;
	case MU_RES_SUBMIT + 2: printf("Clicked button 3\n"); break;
	}
	
    mu_end_window(&ctx);
  }
}

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
	int32_t x = (int32_t)AMotionEvent_getX(event, 0);
	int32_t y = (int32_t)AMotionEvent_getY(event, 0);

	switch (action) {
	case AMOTION_EVENT_ACTION_DOWN:
	case AMOTION_EVENT_ACTION_POINTER_DOWN:
	  mu_input_mousedown(&ctx, x, y, MU_MOUSE_LEFT);
	  break;
	case AMOTION_EVENT_ACTION_UP:
	case AMOTION_EVENT_ACTION_POINTER_UP:
	  mu_input_mouseup(&ctx, x, y, MU_MOUSE_LEFT);
	  break;
	case AMOTION_EVENT_ACTION_MOVE:
	  mu_input_mousemove(&ctx, x, y);
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

  window_width  = ANativeWindow_getWidth(app->window);
  window_height = ANativeWindow_getHeight(app->window);
  
  r_init(app->window, window_width, window_height);
 
  /* init microui */
  mu_init(&ctx);
  ctx.text_width = text_width;
  ctx.text_height = text_height;
  ctx.style->colors[MU_COLOR_WINDOWBG] = background_color;
  ctx.style->colors[MU_COLOR_BASE] = mu_color(38, 38, 38, 255);

  ctx.style->size = mu_vec2(128, 64);
  
  /* main loop */
  for (;;) {
    // Poll once (timeout 0 ms)
    ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }

    /* process frame */
	mu_begin(&ctx);
	home_window();
	mu_end(&ctx);

    /* render */
    r_clear(background_color);
    mu_Command *cmd = NULL;
    while (mu_next_command(&ctx, &cmd)) {
      switch (cmd->type) {
      case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
      case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
      case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
      case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
      }
    }
	
    r_present();
  }
}
