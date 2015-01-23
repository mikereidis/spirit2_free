
#define LOGTAG "sfjutjni"

#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>

//#define _GNU_SOURCE
#include <unistd.h>


#define NELEM(x)                    (sizeof (x)/sizeof (*(x)))
#include <math.h>
#include <fcntl.h>

#include <android/log.h>
//#define  loge(...)  __android_log_print(ANDROID_LOG_ERROR, LOGTAG,__VA_ARGS__)
//#define  logd(...)  __android_log_print(ANDROID_LOG_DEBUG, LOGTAG,__VA_ARGS__)
#define  loge(...)  fm_log_print(ANDROID_LOG_ERROR, LOGTAG,__VA_ARGS__)
#define  logd(...)  fm_log_print(ANDROID_LOG_DEBUG, LOGTAG,__VA_ARGS__)

//int __android_log_print(int prio, const char *tag, const char *fmt, ...);
//int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap);

  int no_log = 0;
  void * log_hndl = NULL;

  int (* do_log) (int prio, const char * tag, const char * fmt, va_list ap);
  #include <stdarg.h>
  int fm_log_print (int prio, const char * tag, const char * fmt, ...) {

    if (no_log)
      return -1;

    va_list ap;
    va_start (ap, fmt); 

    //if (prio == ANDROID_LOG_DEBUG)
    //  printf ("D/jut ");
    //else
    //  printf ("E/jut ");
    //vprintf (fmt, ap);
    //printf ("\n");

    if (log_hndl == NULL) {
      log_hndl = dlopen ("liblog.so", RTLD_LAZY);
      if (log_hndl == NULL) {
        no_log = 1;                                                       // Don't try again
        return (-1);
      }
      do_log = dlsym (log_hndl, "__android_log_vprint");
      if (do_log == NULL) {
        no_log = 1;                                                       // Don't try again
        return (-1);
      }
    }
    //__android_log_vprint (prio, tag, fmt, ap);
    do_log (prio, tag, fmt, ap);
    return (0);
  }


  const char * copyright = "Copyright (c) 2011-2015 Michael A. Reid. All rights reserved.";

  #include <sys/time.h>
  #include <sys/resource.h>

  jint Java_fm_a2d_sf_svc_1aud_native_1priority_1set (JNIEnv * env, jobject thiz, jint new_priority) {
    logd ("native_priority_set new_priority: %d", new_priority);
    int priority = 0;
    priority = getpriority (PRIO_PROCESS, 0);
    logd ("native_priority_set priority: %d  errno: %d", priority, errno);

    priority = setpriority (PRIO_PROCESS, 0, new_priority);//-19);
    logd ("native_priority_set priority: %d  errno: %d", priority, errno);

    priority = getpriority (PRIO_PROCESS, 0);
    logd ("native_priority_set priority: %d  errno: %d", priority, errno);
    return (0);
  }

  #define DEF_BUF 256
  char prop_buf    [DEF_BUF] = "";

  jint Java_fm_a2d_sf_svc_1aud_native_1prop_1get (JNIEnv * env, jobject thiz, jint prop) {
    __system_property_get ("ro.modversion", prop_buf);
    logd ("native_prop_get %d: %s", prop, prop_buf);
    if (! strncasecmp (prop_buf, "omni", 4))                            // If OmniROM
      return (1);                                                       // Return 1 / true
    return (0);
  }


