#include <android_native_app_glue.h>
#include <stdlib.h>
#include <string.h>
#include <microui.h>

#include "renderer.h"

static float bg[3] = { 18, 18, 18 };


static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
  return r_get_text_height();
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

  r_init(app->window);
 
  /* init microui */
  mu_Context *ctx = malloc(sizeof(mu_Context));
  mu_init(ctx);
  ctx->text_width = text_width;
  ctx->text_height = text_height;

  /* main loop */
  for (;;) {
	struct android_poll_source* source;
    int events;

    // Poll once per iteration (timeout 0 ms)
    int ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }

    /* process frame */
	mu_begin(ctx);
	mu_end(ctx);

    /* render */
    r_clear(mu_color(bg[0], bg[1], bg[2], 255));
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
}
