#include <android/input.h>
#include <android_native_app_glue.h>
#include <stdlib.h>
#include <string.h>
#include <microui.h>

#include "renderer.h"
#include "log.h"

static mu_Context ctx;

static float bg[3] = { 18, 18, 18 };

static void test_window(mu_Context *ctx) {
  /* do window */
  if (mu_begin_window(ctx, "Demo Window", mu_rect(100, 100, 600, 800))) {
    mu_Container *win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

    /* window info */
    if (mu_header(ctx, "Window Info")) {
      mu_Container *win = mu_get_current_container(ctx);
      char buf[64];
      mu_layout_row(ctx, 2, (int[]) { 54, -1 }, 0);
      mu_label(ctx,"Position:");
      sprintf(buf, "%d, %d", win->rect.x, win->rect.y); mu_label(ctx, buf);
      mu_label(ctx, "Size:");
      sprintf(buf, "%d, %d", win->rect.w, win->rect.h); mu_label(ctx, buf);
    }

    /* labels + buttons */
    if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 3, (int[]) { 86, -110, -1 }, 0);
      mu_label(ctx, "Test buttons 1:");
      if (mu_button(ctx, "Button 1")) { LOGI("MICROUI", "Pressed button 1"); }
      if (mu_button(ctx, "Button 2")) { LOGI("MICROUI", "Pressed button 2"); }
      mu_label(ctx, "Test buttons 2:");
      if (mu_button(ctx, "Button 3")) { LOGI("MICROUI", "Pressed button 3"); }
      if (mu_button(ctx, "Popup")) { mu_open_popup(ctx, "Test Popup"); }
      if (mu_begin_popup(ctx, "Test Popup")) {
        mu_button(ctx, "Hello");
        mu_button(ctx, "World");
        mu_end_popup(ctx);
      }
    }

    /* tree */
    if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, (int[]) { 140, -1 }, 0);
      mu_layout_begin_column(ctx);
      if (mu_begin_treenode(ctx, "Test 1")) {
        if (mu_begin_treenode(ctx, "Test 1a")) {
          mu_label(ctx, "Hello");
          mu_label(ctx, "world");
          mu_end_treenode(ctx);
        }
        if (mu_begin_treenode(ctx, "Test 1b")) {
          if (mu_button(ctx, "Button 1")) { LOGI("MICROUI", "Pressed button 1"); }
          if (mu_button(ctx, "Button 2")) { LOGI("MICROUI", "Pressed button 2"); }
          mu_end_treenode(ctx);
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 2")) {
        mu_layout_row(ctx, 2, (int[]) { 54, 54 }, 0);
        if (mu_button(ctx, "Button 3")) { LOGI("MICROUI", "Pressed button 3"); }
        if (mu_button(ctx, "Button 4")) { LOGI("MICROUI", "Pressed button 4"); }
        if (mu_button(ctx, "Button 5")) { LOGI("MICROUI", "Pressed button 5"); }
        if (mu_button(ctx, "Button 6")) { LOGI("MICROUI", "Pressed button 6"); }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 3")) {
        static int checks[3] = { 1, 0, 1 };
        mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        mu_end_treenode(ctx);
      }
      mu_layout_end_column(ctx);

      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 1, (int[]) { -1 }, 0);
      mu_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing "
        "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
        "ipsum, eu varius magna felis a nulla.");
      mu_layout_end_column(ctx);
    }

    /* background color sliders */
    if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, (int[]) { -78, -1 }, 74);
      /* sliders */
      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 2, (int[]) { 46, -1 }, 0);
      mu_label(ctx, "Red:");   mu_slider(ctx, &bg[0], 0, 255);
      mu_label(ctx, "Green:"); mu_slider(ctx, &bg[1], 0, 255);
      mu_label(ctx, "Blue:");  mu_slider(ctx, &bg[2], 0, 255);
      mu_layout_end_column(ctx);
      /* color preview */
      mu_Rect r = mu_layout_next(ctx);
      mu_draw_rect(ctx, r, mu_color(bg[0], bg[1], bg[2], 255));
      char buf[32];
      sprintf(buf, "#%02X%02X%02X", (int) bg[0], (int) bg[1], (int) bg[2]);
      mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
    }

    mu_end_window(ctx);
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

	printf("Motion X: %i, Y: %i\n", x, y);

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
 
  r_init(app->window);
 
  /* init microui */
  mu_init(&ctx);
  ctx.text_width = text_width;
  ctx.text_height = text_height;

  /* main loop */
  for (;;) {
    // Poll once (timeout 0 ms)
    ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }

    /* process frame */
	mu_begin(&ctx);
	test_window(&ctx);
	mu_end(&ctx);

    /* render */
    r_clear(mu_color(bg[0], bg[1], bg[2], 255));
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
