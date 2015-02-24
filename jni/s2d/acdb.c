
//#define ACDB_STUFF
#ifdef  ACDB_STUFF
/* Audio calibration related functions */
typedef void (*acdb_deallocate_t)();
typedef int  (*acdb_init_t)();
typedef void (*acdb_send_audio_cal_t)(int, int);
typedef void (*acdb_send_voice_cal_t)(int, int);
typedef int (*acdb_reload_vocvoltable_t)(int);

typedef int (*acdb_loader_get_calibration_t)(char *attr, int size, void *data);
acdb_loader_get_calibration_t acdb_loader_get_calibration;

    void                       *acdb_handle;
    //int                        voice_feature_set;
    acdb_init_t                acdb_init;
    acdb_deallocate_t          acdb_deallocate;
    acdb_send_audio_cal_t      acdb_send_audio_cal;
    acdb_send_voice_cal_t      acdb_send_voice_cal;
    acdb_reload_vocvoltable_t  acdb_reload_vocvoltable;

#define LIB_ACDB_LOADER "libacdbloader.so"

    #define AUDIO_IOCTL_MAGIC 'a'
    #define AUDIO_DEREGISTER_PMEM _IOW(AUDIO_IOCTL_MAGIC, 16, unsigned)

  int acdb_disable_init () {
    acdb_handle = dlopen (LIB_ACDB_LOADER, RTLD_NOW);
    if (acdb_handle == NULL) {
      loge ("%s: DLOPEN failed for %s", __func__, LIB_ACDB_LOADER);
      return (-1);
    }

    logd ("%s: DLOPEN successful for %s", __func__, LIB_ACDB_LOADER);
    acdb_deallocate = (acdb_deallocate_t)dlsym(acdb_handle,                                                         "acdb_loader_deallocate_ACDB");
    if (!acdb_deallocate)
      loge ("%s: Could not find the symbol acdb_loader_deallocate_ACDB from %s", __func__, LIB_ACDB_LOADER);

    acdb_send_audio_cal = (acdb_send_audio_cal_t)dlsym(acdb_handle,                                                 "acdb_loader_send_audio_cal");
    if (!acdb_send_audio_cal)
      loge ("%s: Could not find the symbol acdb_send_audio_cal from %s",         __func__, LIB_ACDB_LOADER);

    acdb_send_voice_cal = (acdb_send_voice_cal_t)dlsym(acdb_handle,                                                 "acdb_loader_send_voice_cal");
    if (!acdb_send_voice_cal)
      loge ("%s: Could not find the symbol acdb_loader_send_voice_cal from %s",  __func__, LIB_ACDB_LOADER);

    acdb_reload_vocvoltable = (acdb_reload_vocvoltable_t)dlsym(acdb_handle,                                         "acdb_loader_reload_vocvoltable");
    if (!acdb_reload_vocvoltable)
      loge ("%s: Could not find the symbol acdb_loader_reload_vocvoltable from %s", __func__, LIB_ACDB_LOADER);

    acdb_init = (acdb_init_t)dlsym(acdb_handle,                                                                     "acdb_loader_init_ACDB");
    if (acdb_init == NULL)
      loge ("%s: dlsym error %s for acdb_loader_init_ACDB", __func__, dlerror());


    logd ("acdb_init success");
    return (0);
  }


  char * acdb_disable_flag = "/dev/msm_acdb_s2";

  int acdb_disable () {
    if (file_get (acdb_disable_flag)) {
      loge ("!!!!!!!!!!!!!!!!!!! Already disabled");
      return (-1);
    }

    //sys_run ("touch /dev/msm_acdb_s2");
    file_write (acdb_disable_flag, "", 0, O_CREAT | O_RDWR);                   // Signal acdb disabled

    prop_buf_get ("ro.build.version.sdk",    version_sdk_prop_buf); // Android 4.4 KitKat = 19
    version_sdk = atoi (version_sdk_prop_buf);
    if (version_sdk < 21) {                                   // For pre-Lollipop
      loge ("!!!!!!!!!!!!!!!!!!! version_sdk: %d", version_sdk);
      return (-1);
    }

    if (! file_get ("/dev/msm_acdb")) {
      loge ("!!!!!!!!!!!!!!!!!!! No /dev/msm_acdb");
      return (-1);
    }

    if (! file_get ("/system/vendor/lib/libacdbloader.so") && ! file_get ("/system/lib/libacdbloader.so")) {
      loge ("!!!!!!!!!!!!!!!!!!! No /system/vendor/lib/libacdbloader.so & No /system/lib/libacdbloader.so");
      return (-1);
    }

    if (! file_get ("/system/etc/acdbdata/MTP/MTP_General_cal.acdb")) {
      loge ("!!!!!!!!!!!!!!!!!!! No /system/etc/acdbdata/MTP/MTP_General_cal.acdb");
      return (-1);
    }

    acdb_disable_init ();   // Get function pointers

//ms_sleep (1000);
    int ret = 0;
    if (acdb_init != NULL) {
      logd ("about to call acdb_init");
      ret = acdb_init ();
      logd ("acdb_init ret: %d", ret);
    }
    else
      loge ("acdb_init == NULL !!!!!!!!!!!!!!!!!!!!!!1");

//ms_sleep (1000);
/*
//ms_sleep (700);
#define ACDB_DEV_TYPE_OUT 1
#define ACDB_DEV_TYPE_IN 2

    int acdb_dev_id     = 0;//acdb_device_table [snd_device];       [SND_DEVICE_IN_HANDSET_MIC] = 4,        [SND_DEVICE_IN_CAPTURE_FM] = 0,
    int acdb_dev_type   = ACDB_DEV_TYPE_IN;

    if (acdb_send_audio_cal != NULL) {
      logd ("about to call acdb_send_audio_cal");
      acdb_send_audio_cal (acdb_dev_id, acdb_dev_type);
      logd ("acdb_send_audio_cal done");
    }
    else
      loge ("acdb_send_audio_cal == NULL !!!!!!!!!!!!!!!!!!!!!!1");

ms_sleep (1000);
*/
/*
    if (acdb_deallocate != NULL) {
      logd ("about to call acdb_deallocate");
      acdb_deallocate ();
      logd ("acdb_deallocate done");
    }
    else
      loge ("acdb_deallocate == NULL !!!!!!!!!!!!!!!!!!!!!!1");

ms_sleep (1000);
*/
if (0) {
    acdb_loader_get_calibration = (acdb_loader_get_calibration_t)
          dlsym(acdb_handle, "acdb_loader_get_calibration");

    if (acdb_loader_get_calibration == NULL) {
        loge ("%s: ERROR. dlsym Error:%s acdb_loader_get_calibration", __func__,
           dlerror());
        return;
    }
    if (send_codec_cal(acdb_loader_get_calibration, acdb_handle) < 0)
        loge ("%s: Could not send anc cal", __FUNCTION__);
}

/*
      int fd = open ("/dev/msm_acdb", O_RDWR);
      if (fd < 0) {
        loge ("Open errno: %s", strerror (errno));
        return -1;
      }
      else
        loge ("Open success");
      struct acdb_ioctl {
        unsigned int size;
        char data [0x10];
      };
      struct acdb_ioctl arg;
      arg.size = 0x10;
      int i = 0;
      for (i = 0; i < arg.size; i ++)
        arg.data [i] = 0;
      ret = ioctl (fd, AUDIO_DEREGISTER_PMEM, & arg);
      if (ret < 0)
        loge ("Ioctl errno: %s", strerror (errno));
      else
        loge ("Ioctl success");
      close (fd);
*/
    return (0);
  }
#endif  //#ifdef  ACDB_STUFF

