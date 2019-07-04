#ifndef __QUP_LOG_H__
#define __QUP_LOG_H__

 #include <android/log.h>
 #define QUP_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "LMG", __VA_ARGS__))


#endif // __QUP_LOG_H__
