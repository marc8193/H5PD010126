#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <allocator.h>
#include <widget.h>
#include <ui.h>

static UI_Action* ui_hotbar(mu_Context* ctx, Arena* arena, const char* button_label_1,
							const char* button_label_2, const char* button_label_3) {
  char* name = "hotbar";
  mu_Id id = mu_get_id(ctx, &name, sizeof(name));
  mu_Rect base = mu_layout_next(ctx);
  
  mu_draw_control_frame(ctx, id, base, MU_COLOR_BASE, 0);

  int margin = 15;
  mu_Rect button_base = mu_rect(base.x + margin, base.y + margin,
								base.w - margin * 2, base.h - margin * 2);

  mu_layout_set_next(ctx, button_base, 0);

  ctx->style->spacing = margin;
  mu_layout_begin_column(ctx); 

  int button_width = (button_base.w - margin * 2) / 3;

  mu_layout_row(ctx, 3, (int[]) { button_width, button_width, button_width }, button_base.h);

  UI_Action* action = NULL;

  const char* button_labels[3] = { button_label_1, button_label_2, button_label_3 };
  for (int i = 0; i < sizeof(button_labels) / sizeof(button_labels[0]); i++) {
	if (mu_button(ctx, button_labels[i])) {
	  action = (UI_Action*) arena_allocate(arena, sizeof(UI_Action));

	  action->widget = mu_get_id(ctx, button_labels[i], strlen(button_labels[i]));
	  action->clicked = 1;
	}
  }
 
  mu_layout_end_column(ctx);
  
  return action;
}

void ui_home(mu_Context* ctx, Stack* ui_page_stack, mu_Vec2 size, int top_margin) {
  if (mu_begin_window_ex(ctx, "Home Screen",
						 mu_rect(0, top_margin, size.x, size.y - top_margin),
						 MU_OPT_NORESIZE | MU_OPT_NOTITLE)) {

	int rect_width = 480;
	int rect_height = 160;
	mu_Rect rect = mu_rect((size.x - rect_width) / 2, (size.y - rect_height) - 200,
						   rect_width, rect_height);
	
	mu_layout_set_next(ctx, rect, 0);

	Arena arena = {0};
	unsigned char buffer[256] = {0};
	
	arena_init(&arena, buffer, sizeof(buffer));
	
	UI_Action* action = ui_hotbar(ctx, &arena, "Home", "Edit", "Sync");
	
	if (action) {
	  if (action->widget == mu_get_id(ctx, "Home", strlen("Home"))) {
		UI_Window* window = (UI_Window*) stack_allocate(ui_page_stack, sizeof(UI_Window));

		*window = UI_HOME_WINDOW;
	  }

	  if (action->widget == mu_get_id(ctx, "Edit", strlen("Edit"))) {
		UI_Window* window = (UI_Window*) stack_allocate(ui_page_stack, sizeof(UI_Window));

		*window = UI_EDIT_WINDOW;
	  }

	  if (action->widget == mu_get_id(ctx, "Sync", strlen("Sync"))) {
		UI_Window* window = (UI_Window*) stack_allocate(ui_page_stack, sizeof(UI_Window));

		*window = UI_SYNC_WINDOW;
	  }	
	}
   	  	
    mu_end_window(ctx);
  }
}

static int ui_scratchpad(mu_Context *ctx, char *buf, int bufsz, int opt) {
  char* name = "scratchpad";
  mu_Id id = mu_get_id(ctx, &name, sizeof(name));
  mu_Rect r = mu_layout_next(ctx);
  
  int res = 0;
  mu_update_control(ctx, id, r, opt | MU_OPT_HOLDFOCUS);

  if (ctx->focus == id) {
    /* handle text input */
    int len = strlen(buf);
    int n = mu_min(bufsz - len - 1, (int) strlen(ctx->input_text));
    if (n > 0) {
      memcpy(buf + len, ctx->input_text, n);
      len += n;
      buf[len] = '\0';
      res |= MU_RES_CHANGE;
    }
    /* handle backspace */
    if (ctx->key_pressed & MU_KEY_BACKSPACE && len > 0) {
      /* skip utf-8 continuation bytes */
      while ((buf[--len] & 0xc0) == 0x80 && len > 0);
      buf[len] = '\0';
      res |= MU_RES_CHANGE;
    }
    /* handle return */
    if (ctx->key_pressed & MU_KEY_RETURN) {
      mu_set_focus(ctx, 0);
      res |= MU_RES_SUBMIT;
    }
  }

  /* draw */
  mu_draw_control_frame(ctx, id, r, MU_COLOR_BASE, opt);
  if (ctx->focus == id) {
    mu_Color color = ctx->style->colors[MU_COLOR_TEXT];
    mu_Font font = ctx->style->font;
    int textw = ctx->text_width(font, buf, -1);
    int texth = ctx->text_height(font);
    int ofx = r.w - ctx->style->padding - textw - 1;
    int textx = r.x + mu_min(ofx, ctx->style->padding);
    int texty = r.y;
    mu_push_clip_rect(ctx, r);
    mu_draw_text(ctx, font, buf, -1, mu_vec2(textx, texty), color);
    mu_draw_rect(ctx, mu_rect(textx + textw, texty, 1, texth), color);
    mu_pop_clip_rect(ctx);
  } else {
    mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, opt);
  }

  return res;
}

void ui_edit(mu_Context* ctx, Stack* ui_page_stack, mu_Vec2 size, int top_margin) {
  if (mu_begin_window_ex(ctx, "Edit Screen",
						 mu_rect(0, top_margin, size.x, size.y - top_margin),
						 MU_OPT_NORESIZE | MU_OPT_NOTITLE)) {

	mu_Rect rect = mu_rect(50, 100, size.x - 50 * 2, size.y - 100 * 2);
	
	mu_layout_set_next(ctx, rect, 0);
	
	static char buffer[256] = {0};

	ui_scratchpad(ctx, buffer, sizeof(buffer), 0);
		
    mu_end_window(ctx);
  }
}
