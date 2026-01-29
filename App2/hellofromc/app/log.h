#ifndef LOG_H
#define LOG_H

#define LOGI(TAG, ...) ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))

#endif


