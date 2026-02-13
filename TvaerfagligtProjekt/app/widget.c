#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <allocator.h>
#include <widget.h>
#include <ui.h>

static UI_Action* ui_hotbar(mu_Context* ctx, Arena* arena) {
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

  if(mu_button(ctx, "Home")) {
	action = (UI_Action*) arena_allocate(arena, sizeof(UI_Action));
	  
	action->widget = UI_HOME_HOTBAR_HOME_BUTTON;
	action->clicked = 1;
  }

  if(mu_button(ctx, "New")) {
	action = (UI_Action*) arena_allocate(arena, sizeof(UI_Action));
	  
	action->widget = UI_HOME_HOTBAR_NEW_BUTTON;
	action->clicked = 1;
  }

  if(mu_button(ctx, "Sync")) {
	action = (UI_Action*) arena_allocate(arena, sizeof(UI_Action));
	  
	action->widget = UI_HOME_HOTBAR_SYNC_BUTTON;
	action->clicked = 1;
  }
 
  mu_layout_end_column(ctx);
  
  return action;
}

UI_Action* ui_home(mu_Context* ctx, Arena* arena, mu_Vec2 size, int top_margin) {
  UI_Action* action = NULL;
  
  if (mu_begin_window_ex(ctx, "Home Screen",
						 mu_rect(0, top_margin, size.x, size.y - top_margin),
						 MU_OPT_NORESIZE | MU_OPT_NOTITLE)) {

	int rect_width = 480;
	int rect_height = 160;
	mu_Rect rect = mu_rect((size.x - rect_width) / 2, (size.y - rect_height) - 200,
						   rect_width, rect_height);
	
	mu_layout_set_next(ctx, rect, 0);

	action = ui_hotbar(ctx, arena);
	
    mu_end_window(ctx);
  }

  return action;
}
