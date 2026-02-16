#ifndef WIDGET_H
#define WIDGET_H

#include <ui.h>
#include <allocator.h>

typedef enum {
  UI_HOME_WINDOW,
  UI_EDIT_WINDOW,
  UI_SYNC_WINDOW,
} UI_Window;

typedef struct {
  mu_Id widget;
  uint8_t clicked;
} UI_Action;

void ui_home(mu_Context* ctx, Stack* ui_page_stack, mu_Vec2 size, int top_margin);
void ui_edit(mu_Context* ctx, Stack* ui_page_stack, mu_Vec2 size, int top_margin);

#endif
