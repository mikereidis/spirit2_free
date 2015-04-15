
  #define LOGTAG "sfjut..."

  #include <stdio.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/resource.h>

  #include <jni.h>

  #define  GENERIC_CLIENT

  #include "utils.c"


  const char * copyright = "Copyright (c) 2011-2015 Michael A. Reid. All rights reserved.";

/*  ~/android/system/hardware/qcom/audio-caf/msm8974/hal/audio_hw.c

static void *offload_thread_loop(void *context)
...
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    set_sched_policy(0, SP_FOREGROUND);
    prctl(PR_SET_NAME, (unsigned long)"Offload Callback", 0, 0, 0);

*/

  jint Java_fm_a2d_sf_com_1uti_native_1priority_1set (JNIEnv * env, jobject thiz, jint new_priority) {
    logd ("native_priority_set new_priority: %d", new_priority);

    utils_init ();

    int priority = 0;
    errno = 0;
    priority = getpriority (PRIO_PROCESS, 0);
    logd ("native_priority_set priority: %d  errno: %d (%s)", priority, errno, strerror (errno));

    errno = 0;
    priority = setpriority (PRIO_PROCESS, 0, new_priority);//-19);
    logd ("native_priority_set priority: %d  errno: %d (%s)", priority, errno, strerror (errno));

    errno = 0;
    priority = getpriority (PRIO_PROCESS, 0);
    logd ("native_priority_set priority: %d  errno: %d (%s)", priority, errno, strerror (errno));
    return (0);
  }

  char prop_buf    [DEF_BUF] = "";

  jint Java_fm_a2d_sf_com_1uti_native_1omni_1get (JNIEnv * env, jobject thiz) {

    utils_init ();

    logd ("native_omni_get sys_prop_modversion: %s", sys_prop_modversion);

    if (! strncasecmp (sys_prop_modversion, "OMNI", strlen ("OMNI")))   // If OmniROM
      return (1);                                                       // Return 1 / true

    return (0);                                                         // Else false
  }


  int native_daemon_cmd (int cmd_len, char * cmd_buf, int res_max, char * res_buf, int net_port, int rx_tmo) {

//    logd ("native_daemon_cmd cmd_len: %d  cmd_buf \"%s\"  res_max: %d  res_buf: %p  net_port: %d  rx_tmo: %d", cmd_len, cmd_buf, res_max, res_buf, net_port, rx_tmo);

    int res_len = gen_client_cmd ((unsigned char *) cmd_buf, cmd_len, (unsigned char *) res_buf, res_max, net_port, rx_tmo);

//    logd ("native_daemon_cmd res_len: %d  res_buf \"%s\"", res_len, res_buf);

    return (res_len);
  }

  //private native int native_daemon_cmd      (int cmd_len, byte [] cmd_buf, int res_len, byte [] res_buf, int rx_tmo);

  jbyte *   jb_cmd_buf          = NULL;
  jbyte     fake_cmd_buf [8192] = {0};

  jbyte *   jb_res_buf          = NULL;
  jbyte     fake_res_buf [8192] = {0};

  jint Java_fm_a2d_sf_com_1uti_native_1daemon_1cmd (JNIEnv * env, jobject thiz, jint cmd_len, jbyteArray cmd_buf, jint res_len, jbyteArray res_buf, jint net_port, jint rx_tmo) {

    utils_init ();

    if (cmd_buf == NULL) {
      loge ("native_daemon_cmd cmd_buf == NULL");
      //return (-1);
      jb_cmd_buf = fake_cmd_buf;
    }
    else
      jb_cmd_buf = (*env)->GetByteArrayElements (env, cmd_buf, NULL);

    if (res_buf == NULL) {
      loge ("native_daemon_cmd res_buf == NULL");
      //return (-1);
      jb_res_buf = fake_res_buf;
    }
    else
      jb_res_buf = (*env)->GetByteArrayElements (env, res_buf, NULL);

    int len = native_daemon_cmd (cmd_len, jb_cmd_buf, res_len, jb_res_buf, net_port, rx_tmo);

    if (cmd_buf != NULL)
      (*env)->ReleaseByteArrayElements (env, cmd_buf, jb_cmd_buf, 0);

    if (res_buf != NULL)
      (*env)->ReleaseByteArrayElements (env, res_buf, jb_res_buf, 0);

    return (len);
  }



