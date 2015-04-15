
    // Audio calibration related functions

#define ACDB_STUFF
#ifdef  ACDB_STUFF


//"/system/etc/audio_platform_info.xml"






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

  int acdb_set_init () {
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


  char * acdb_set_flag = "/dev/msm_acdb_s2";

  int acdb_set (int acdb_id) {
/*
    if (file_get (acdb_set_flag)) {
      loge ("!!!!!!!!!!!!!!!!!!! Already disabled");
      return (-1);
    }

    //sys_run ("touch /dev/msm_acdb_s2");
    file_write (acdb_set_flag, "", 0, O_CREAT | O_RDWR);                   // Signal acdb disabled

    if (android_version < 21) {                                   // For pre-Lollipop
      loge ("!!!!!!!!!!!!!!!!!!! android_version: %d", android_version);
      return (-1);
    }
*/
    if (! file_get ("/dev/msm_acdb")) {
      loge ("!!!!!!!!!!!!!!!!!!! No /dev/msm_acdb");
      return (-1);
    }

    if (! file_get ("/system/vendor/lib/libacdbloader.so") && ! file_get ("/system/lib/libacdbloader.so")) {
      loge ("!!!!!!!!!!!!!!!!!!! No /system/vendor/lib/libacdbloader.so & No /system/lib/libacdbloader.so");
      return (-1);
    }
/*
    if (! file_get ("/system/etc/acdbdata/MTP/MTP_General_cal.acdb")) {
      loge ("!!!!!!!!!!!!!!!!!!! No /system/etc/acdbdata/MTP/MTP_General_cal.acdb");
      return (-1);
    }
*/

    acdb_set_init ();   // Get function pointers

//ms_sleep (1000);
    int ret = 0;
    if (acdb_init != NULL) {
      logd ("about to call acdb_init");
      ret = acdb_init ();
      logd ("acdb_init ret: %d", ret);
    }
    else
      loge ("acdb_init == NULL !!!!!!!!!!!!!!!!!!!!!!1");

ms_sleep (1000);
///*
//ms_sleep (700);
#define ACDB_DEV_TYPE_OUT 1
#define ACDB_DEV_TYPE_IN 2

    int acdb_dev_id     = acdb_id;//0;//acdb_device_table [snd_device];       [SND_DEVICE_IN_HANDSET_MIC] = 4,        [SND_DEVICE_IN_CAPTURE_FM] = 0,
    int acdb_dev_type   = ACDB_DEV_TYPE_IN;
    //int snd_device = 47;

    if (acdb_send_audio_cal != NULL) {
      logd ("about to call acdb_send_audio_cal acdb_id: %d", acdb_id);
      acdb_send_audio_cal (acdb_dev_id, acdb_dev_type);
      logd ("acdb_send_audio_cal done");
    }
    else
      loge ("acdb_send_audio_cal == NULL !!!!!!!!!!!!!!!!!!!!!!1");

ms_sleep (2000);
//*/
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

/*
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
*/

/*
      errno = 0;
      int fd = open ("/dev/msm_acdb", O_RDWR);
      if (fd < 0) {
        loge ("Open errno: %d (%s)", errno, strerror (errno));
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
      errno = 0;
      ret = ioctl (fd, AUDIO_DEREGISTER_PMEM, & arg);
      if (ret < 0)
        loge ("Ioctl errno: %d (%s)", errno, strerror (errno));
      else
        loge ("Ioctl success");
      close (fd);


#define AUDIO_MAX_COMMON_IOCTL_NUM      100

a strace -f -e ioctl -t -p 282 | grep "ioctl(19"

[pid   886] 19:38:41 ioctl(19, 0x40046171, 0xb41ffa5c) = 0     13   AUDIO_SET_ADM_RX_TOPOLOGY       Int32
[pid   886] 19:38:41 ioctl(19, 0x40046167, 0xb41ffa60) = 0      3   AUDIO_SET_AUDPROC_RX_CAL        SamePtr2       ENI_SETMULT
[pid   886] 19:38:41 ioctl(19, 0x40046169, 0xb41ffa60) = 0      5   AUDIO_SET_AUDPROC_RX_VOL_CAL    SamePtr2
[pid   886] 19:38:41 ioctl(19, 0x40046175, 0xb41ffa18) = 0     11   AUDIO_SET_VOICE_RX_TOPOLOGY     Ptr3

[pid 14260] 19:38:42 ioctl(19, 0x40046172, 0xa49f1ac4) = 0     14   AUDIO_SET_ADM_TX_TOPOLOGY       Int32
[pid 14260] 19:38:42 ioctl(19, 0x4004616a, 0xa49f1ac8) = 0      6   AUDIO_SET_AUDPROC_TX_CAL        SamePtr5
[pid 14260] 19:38:42 ioctl(19, 0x4004616c, 0xa49f1ac8) = 0      8   AUDIO_SET_AUDPROC_TX_VOL_CAL    SamePtr5
[pid 14260] 19:38:42 ioctl(19, 0x40046174, 0xa49f1a80) = 0     16   AUDIO_SET_AFE_TX_CAL            Ptr6



strace -f -e ioctl -t -p 282 2 > &1 | grep "ioctl(19"

                                                                    b4003000-b4100000 rw-p 00000000 00:00 0          [stack:886]
[pid   886] 23:46:02 ioctl(19, 0x40046171, 0xb40ffa5c) = 0
[pid   886] 23:46:02 ioctl(19, ENI_SETMULT, 0xb40ffa60) = 0
[pid   886] 23:46:02 ioctl(19, 0x40046169, 0xb40ffa60) = 0
[pid   886] 23:46:02 ioctl(19, 0x40046175, 0xb40ffa18) = 0

                                                                    a587f000-a597c000 rw-p 00000000 00:00 0          [stack:6187]

[pid  6187] 23:46:03 ioctl(19, 0x40046172, 0xa597bac4) = 0      2000
[pid  6187] 23:46:03 ioctl(19, 0x4004616a, 0xa597bac8) = 0      2, b6efbf9c     > 1, 10
[pid  6187] 23:46:03 ioctl(19, 0x4004616c, 0xa597bac8) = 0
[pid  6187] 23:46:03 ioctl(19, 0x40046174, 0xa597ba80) = 0      a9b5b0f0    ->  


*/
    return (0);
  }
#endif  //#ifdef  ACDB_STUFF

