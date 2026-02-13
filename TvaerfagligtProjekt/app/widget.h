#ifndef WIDGET_H
#define WIDGET_H

#include <ui.h>
#include <allocator.h>

typedef enum {
  UI_HOME_HOTBAR_HOME_BUTTON,
  UI_HOME_HOTBAR_NEW_BUTTON,
  UI_HOME_HOTBAR_SYNC_BUTTON,
  UI_WIDGETS_MAX
} Widget_ID;

typedef struct {
  Widget_ID widget;
  uint8_t clicked;
} UI_Action;

UI_Action* ui_home(mu_Context* ctx, Arena* arena, mu_Vec2 size, int top_margin);

#endif
