#include <stdlib.h>
#include <string.h>

#include <android/input.h>
#include <android_native_app_glue.h>
#include <jni.h>
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

void display_keyboard(struct android_app* app, int p_show) {
  /*
	Based on
	https://stackoverflow.com/questions/5864790/how-to-show-the-soft-keyboard-on-native-activity

	Thanks to cntools/rawdraw for doing the work.
  */
  jint lFlags = 0;

  const struct JNINativeInterface * env = (struct JNINativeInterface*) app->activity->env;
  const struct JNINativeInterface ** envptr = &env;
  const struct JNIInvokeInterface ** jniiptr = app->activity->vm;
  const struct JNIInvokeInterface * jnii = *jniiptr;
  jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
  env = (*envptr);

  jclass activityClass = env->FindClass(envptr, "android/app/NativeActivity");

  /* Retrieves NativeActivity. */
  jobject lNativeActivity = app->activity->clazz;


  /* Retrieves Context.INPUT_METHOD_SERVICE. */
  jclass ClassContext = env->FindClass(envptr, "android/content/Context");
  jfieldID FieldINPUT_METHOD_SERVICE = env->GetStaticFieldID(envptr, ClassContext,
															 "INPUT_METHOD_SERVICE",
															 "Ljava/lang/String;" );

  jobject INPUT_METHOD_SERVICE = env->GetStaticObjectField(envptr, ClassContext,
														   FieldINPUT_METHOD_SERVICE );

  /* Runs getSystemService(Context.INPUT_METHOD_SERVICE). */
  jclass ClassInputMethodManager = env->FindClass(envptr,
												  "android/view/inputmethod/InputMethodManager" );

  jmethodID MethodGetSystemService = env->GetMethodID(envptr, activityClass, "getSystemService",
													  "(Ljava/lang/String;)Ljava/lang/Object;");

  jobject lInputMethodManager = env->CallObjectMethod(envptr, lNativeActivity,
													  MethodGetSystemService,
													  INPUT_METHOD_SERVICE);

  /* Runs getWindow().getDecorView(). */
  jmethodID MethodGetWindow = env->GetMethodID(envptr, activityClass,
											   "getWindow", "()Landroid/view/Window;");

  jobject lWindow = env->CallObjectMethod(envptr, lNativeActivity, MethodGetWindow);
  jclass ClassWindow = env->FindClass(envptr, "android/view/Window");
  jmethodID MethodGetDecorView = env->GetMethodID(envptr, ClassWindow, "getDecorView",
												  "()Landroid/view/View;");

  jobject lDecorView = env->CallObjectMethod(envptr, lWindow, MethodGetDecorView);

  if (p_show) {
	/* Runs lInputMethodManager.showSoftInput(...). */
	jmethodID MethodShowSoftInput = env->GetMethodID(envptr, ClassInputMethodManager,
													 "showSoftInput", "(Landroid/view/View;I)Z");

	/*jboolean lResult = */env->CallBooleanMethod(envptr, lInputMethodManager,
												  MethodShowSoftInput, lDecorView, lFlags);
  } else {
	/* Runs lWindow.getViewToken() */
	jclass ClassView = env->FindClass(envptr, "android/view/View");
	jmethodID MethodGetWindowToken = env->GetMethodID(envptr, ClassView,
													  "getWindowToken",
													  "()Landroid/os/IBinder;");

	jobject lBinder = env->CallObjectMethod(envptr, lDecorView, MethodGetWindowToken);

	/* lInputMethodManager.hideSoftInput(...). */
	jmethodID MethodHideSoftInput = env->GetMethodID(envptr, ClassInputMethodManager,
													 "hideSoftInputFromWindow",
													 "(Landroid/os/IBinder;I)Z");
		
	/*jboolean lRes = */env->CallBooleanMethod(envptr, lInputMethodManager, MethodHideSoftInput,
											   lBinder, lFlags);
  }

  jnii->DetachCurrentThread(jniiptr);
}

static int32_t handle_input(struct android_app* app, AInputEvent* event) {
  mu_Context* ctx = (mu_Context*) app->userData;
  
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
	int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
	int32_t x = (int32_t) AMotionEvent_getX(event, 0);
	int32_t y = (int32_t) AMotionEvent_getY(event, 0);

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

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
	int32_t keycode = AKeyEvent_getKeyCode(event);
	int32_t action = AKeyEvent_getAction(event);

	switch (keycode) {
	case AKEYCODE_A:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "a");
	  }

	  break;

	case AKEYCODE_B:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "b");
	  }

	  break;

	case AKEYCODE_C:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "c");
	  }

	  break;

	case AKEYCODE_D:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "d");
	  }

	  break;

	case AKEYCODE_E:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "e");
	  }

	  break;

	case AKEYCODE_F:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "f");
	  }

	  break;

	case AKEYCODE_G:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "g");
	  }

	  break;

	case AKEYCODE_H:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "h");
	  }

	  break;

	case AKEYCODE_I:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "i");
	  }

	  break;

	case AKEYCODE_J:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "j");
	  }

	  break;

	case AKEYCODE_K:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "k");
	  }

	  break;

	case AKEYCODE_L:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "l");
	  }

	  break;

	case AKEYCODE_M:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "m");
	  }

	  break;

	case AKEYCODE_N:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "n");
	  }

	  break;

	case AKEYCODE_O:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "o");
	  }

	  break;

	case AKEYCODE_P:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "p");
	  }

	  break;

	case AKEYCODE_Q:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "q");
	  }

	  break;

	case AKEYCODE_R:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "r");
	  }

	  break;

	case AKEYCODE_S:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "s");
	  }

	  break;

	case AKEYCODE_T:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "t");
	  }

	  break;

	case AKEYCODE_U:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "u");
	  }

	  break;

	case AKEYCODE_V:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "v");
	  }

	  break;

	case AKEYCODE_W:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "w");
	  }

	  break;

	case AKEYCODE_X:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "x");
	  }

	  break;

	case AKEYCODE_Y:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "y");
	  }

	  break;

	case AKEYCODE_Z:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		mu_input_text(ctx, "z");
	  }

	  break;

	case AKEYCODE_BACK:
	  if (action == AKEY_EVENT_ACTION_DOWN) {
		Stack* ui_page_stack = (Stack*) ctx->user_data;

		/* Stack allocation + UI_Window == 6, exit if only one page left in stack */
		if (ui_page_stack->offset == 6) {
		  ANativeActivity_finish(app->activity);
		}
		
		UI_Window* current_window = (UI_Window*) ((uintptr_t) ui_page_stack->buffer +
												  (uintptr_t) ui_page_stack->offset -
												  sizeof(UI_Window));

		stack_free(ui_page_stack, current_window);	
	  }
	  
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

  Stack ui_page_stack = {0};
  unsigned char buffer[256] = {0};
  
  stack_init(&ui_page_stack, buffer, 256);
  ctx->user_data = &ui_page_stack;

  UI_Window* initial_window = (UI_Window*) stack_allocate(&ui_page_stack, sizeof(UI_Window));

  *initial_window = UI_HOME_WINDOW;
   
  /* main loop */
  for (;;) {
    // Poll once (timeout 0 ms)
    ident = ALooper_pollOnce(0, NULL, &events, (void**)&source);
    if (ident >= 0 && source) {
      source->process(app, source);
    }


	UI_Window* current_window = (UI_Window*) ((uintptr_t) ui_page_stack.buffer +
											  (uintptr_t) ui_page_stack.offset - sizeof(UI_Window));
	
    /* process frame */
	mu_begin(ctx);
	
	switch (*current_window) {
	case UI_HOME_WINDOW:
	  ui_home(ctx, &ui_page_stack, mu_vec2(window_width, window_height), 75);
	  break;

	case UI_EDIT_WINDOW:
	  ui_edit(ctx, &ui_page_stack, mu_vec2(window_width, window_height), 75);
	  break;

	case UI_SYNC_WINDOW:
	  printf("Sync page is active\n");
	  break;
	}
	  
	mu_end(ctx);
	
    /* render */
    r_clear(background_color);
    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
      switch (cmd->type) {
      case MU_COMMAND_TEXT:
		r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
		break;

	  case MU_COMMAND_RECT:
		r_draw_rect(cmd->rect.rect, cmd->rect.color, 0);
		break;
		
      case MU_COMMAND_ICON:
		r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
		break;
		
	  case MU_COMMAND_CLIP:
		r_set_clip_rect(cmd->clip.rect);
		break;
      }
    }
	
    r_present();
  }
}
