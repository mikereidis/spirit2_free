
/*
!! ?? Can call snd_set_device() in libsnd.so ??

libaudio.so:
__android_log_print
ioctl
_ZN7android13AudioHardware16setFMRadioVolumeEi
...
_ZN7android18A2dpAudioInterface16setFMRadioVolumeEi
...
ZN7android20AudioHardwareGeneric16setFMRadioVolumeEi
...
_ZN7android17AudioHardwareStub16setFMRadioVolumeEi
...
setFMRadioVolume(%d) under 0, assuming 0
setFMRadioVolume(%d) over 20, assuming 20
set_FMRadio_volume_rpc error.
...
FM_HEADSET
FM_SPEAKER


libaudioflinger.so:
_ZN7android12AudioFlinger16setFMRadioVolumeEi


cd ~/Documents/cell/cm7src
time grep * -ri -e snd_set_device 2>/dev/null

bionic/libc/kernel/common/linux/msm_audio.h:                    #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)
bionic/libc/kernel/common/linux/msm_audio_7X30.h:               #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)
development/ndk/platforms/android-3/include/linux/msm_audio.h:  #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)

device/geeksphone/zero/libaudio/msm_audio.h:                    #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)
device/geeksphone/zero/libaudio/AudioHardware.cpp

""
device/geeksphone/one
device/huawei/ideos
device/zte/blade
device/motorola/zeppelin
device/commtiva/z71

external/kernel-headers/original/linux/msm_audio.h:             #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)
hardware/msm7k/libaudio-qsd8k/msm_audio.h:                      #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)
hardware/msm7k/libaudio-qdsp5v2/msm_audio.h:                    #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)

hardware/msm7k/libaudio/AudioHardware.cpp:                      LOGD("rpc_snd_set_device(%d, %d, %d)\n", device, ear_mute, mic_mute);
...

prebuilt/ndk/android-ndk-r4/platforms/android-8/arch-x86/usr/include/linux/msm_audio.h:     #define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)
...

*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

//#include <linux/msm_audio.h>

#include <linux/types.h>
#include <linux/ioctl.h>

  unsigned char logtag [4] = "ssd";
#include <dlfcn.h>

#include <android/log.h>
#define  loge(...)  fm_log_print(ANDROID_LOG_ERROR,logtag,__VA_ARGS__)
#define  logd(...)  fm_log_print(ANDROID_LOG_DEBUG,logtag,__VA_ARGS__)

  int no_log = 0;
  void * log_hndl = NULL;

  int (* do_log) (int prio, const char * tag, const char * fmt, va_list ap);
  #include <stdarg.h>
  int fm_log_print (int prio, const char * tag, const char * fmt, ...) {

    if (no_log)
      return -1;

    va_list ap;
    va_start (ap, fmt); 

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
    do_log (prio, tag, fmt, ap);
    return (0);
  }

  void alt_usleep (uint32_t us) {
    struct timespec delay;
    int err;
    //  return;
    delay.tv_sec = us / 1000000;
    delay.tv_nsec = 1000 * 1000 * (us % 1000000);
        // usleep can't be used because it uses SIGALRM
    do {
      err = nanosleep (& delay, & delay);
    } while (err < 0 && errno == EINTR);
  }

  int ms_sleep (int ms) {
    if (ms > 10)
      loge ("ms_sleep ms: %d", ms);
    usleep (ms * 1000);                                             // ?? Use nanosleep to avoid SIGALRM ??
    return (0);
  }




#define SND_IOCTL_MAGIC 's'
#define SND_GET_NUM_ENDPOINTS _IOR(SND_IOCTL_MAGIC, 4, unsigned *)
struct msm_snd_endpoint {
 int id;
 char name[64];
};
#define SND_GET_ENDPOINT _IOWR(SND_IOCTL_MAGIC, 5, struct msm_snd_endpoint *)
#define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_device_config *)
struct msm_snd_device_config {
 uint32_t device;
 uint32_t ear_mute;
 uint32_t mic_mute;
};
//#define SND_SET_DEVICE _IOW(SND_IOCTL_MAGIC, 2, struct msm_snd_device_config *)
#define AUDIO_IOCTL_MAGIC 'a'
#define AUDIO_SET_VOLUME _IOW(AUDIO_IOCTL_MAGIC, 10, unsigned)

#define SND_SET_FM_RADIO_VOLUME _IOWR(SND_IOCTL_MAGIC, 17, int32_t *)





const char * copyright = "Copyright (c) 2011-2014 Michael A. Reid. All rights reserved.\n";

//SND_SET_DEVICE
//SND_GET_NUM_ENDPOINTS
//SND_GET_ENDPOINT

/*
less ../../cm7src/hardware/msm7k/libaudio-qdsp5v2/msm_audio.h
#define SND_IOCTL_MAGIC 's'
#define SND_SET_VOLUME _IOW(SND_IOCTL_MAGIC, 3, struct msm_snd_volume_config *)
#define SND_GET_NUM_ENDPOINTS _IOR(SND_IOCTL_MAGIC, 4, unsigned *)
struct msm_snd_endpoint {
 int id;
 char name[64];
};
#define SND_GET_ENDPOINT _IOWR(SND_IOCTL_MAGIC, 5, struct msm_snd_endpoint *)

less ../../cm7src/hardware/msm7k/libaudio-qsd8k/msm_audio.h 
#define SND_IOCTL_MAGIC 's'
#define SND_SET_VOLUME _IOW(SND_IOCTL_MAGIC, 3, struct msm_snd_volume_config *)
#define SND_GET_NUM_ENDPOINTS _IOR(SND_IOCTL_MAGIC, 4, unsigned *)
struct msm_snd_endpoint {
 int id;
 char name[64];
};
#define SND_GET_ENDPOINT _IOWR(SND_IOCTL_MAGIC, 5, struct msm_snd_endpoint *)

*/

// ?? Generic to m1/m2/m3 ???
int snd_endpoints_disp (int fd) {
  int num_endpoints = -1;
  int ret = ioctl (fd, SND_GET_NUM_ENDPOINTS, & num_endpoints);
  if (ret)
    printf ("snd_endpoints_disp SND_GET_NUM_ENDPOINTS ioctl ret: %d  num_endpoints: %d  errno: %d\n", ret, num_endpoints, errno);
  else
    printf ("snd_endpoints_disp SND_GET_NUM_ENDPOINTS ioctl num_endpoints: %d\n", num_endpoints);
  if (num_endpoints < 1 || num_endpoints > 256) {                       // If unexpected num endpoints or wasn't written
    return (-1);
  }

  struct msm_snd_endpoint snd_endpoint = {0};
  int ctr = 0;
  for (ctr = 0; ctr < num_endpoints; ctr ++) {
    snd_endpoint.id = ctr;
    snd_endpoint.name [0] = 0;
    ret = ioctl (fd, SND_GET_ENDPOINT, & snd_endpoint);
    if (ret)
      printf ("snd_endpoints_disp SND_GET_ENDPOINT ioctl ret: %d  snd_endpoint.id: %d  name: %s  errno: %d\n", ret, snd_endpoint.id, snd_endpoint.name, errno);
    else
      printf ("snd_endpoints_disp SND_GET_ENDPOINT ioctl snd_endpoint.id: %3d  name: %s\n", snd_endpoint.id, snd_endpoint.name);
  }
  return (ret);
}


/*
#define SND_SET_FM_RADIO_VOLUME _IOWR(SND_IOCTL_MAGIC, 17, int *)
struct msm_snd_set_fm_radio_vol_param {
  int32_t volume;};
status_t AudioHardware::setFmVolume (float v) {
  float ratio = 2.5;    // 0.2 in CM
  int volume = (unsigned int) (AudioSystem::logToLinear(v) * ratio);
  struct msm_snd_set_fm_radio_vol_param args;
  args.volume = volume;
  if (ioctl (m7xsnddriverfd, SND_SET_FM_RADIO_VOLUME, &args) < 0) {
    LOGE("set_volume_fm error.");
    return -EIO;
  }
return NO_ERROR;
}*/

int m1_do (int fd, int func_num) {
  int ret = 0;
  char * dev_str = NULL;
  int dev_id = 0;
  struct msm_snd_device_config args;
  args.device = (uint16_t) func_num;

  if (func_num == 256)                                                  // ??
    args.device = 0;
  args.ear_mute = 1; //ear_mute ? SND_MUTE_MUTED : SND_MUTE_UNMUTED;
  args.mic_mute = 1; //mic_mute ? SND_MUTE_MUTED : SND_MUTE_UNMUTED;

  if (func_num == 0) {
    ret = snd_endpoints_disp (fd);
    dev_id = dev_id_get_disp (fd, dev_str);
  }
  else if (func_num < 1000) {                                           // If setting a device...
    errno = 0;
    ret = ioctl (fd, SND_SET_DEVICE, & args); // device, ear_mute, mic_mute = 11 1 1 speaker or 9 1 1 headset    (headset = 10 on LG P500h)
    if (ret)
      printf ("m1_do SND_SET_DEVICE ioctl ret: %d  errno: %d\n", ret, errno);
    else
      printf ("m1_do SND_SET_DEVICE ioctl ret: %d\n", ret);
    //return (ret);
  }
  else if (func_num >= 1000) {
    int32_t volume = func_num - 1000;
    errno = 0;
    ret = ioctl (fd, SND_SET_FM_RADIO_VOLUME, & volume);
    if (ret)
      printf ("m1_do SND_SET_FM_RADIO_VOLUME ioctl ret: %d  errno: %d\n", ret, errno);
    else
      printf ("m1_do SND_SET_FM_RADIO_VOLUME ioctl ret: %d\n", ret);
    //return (ret);
  }
  return (ret);
}


//#define AUDIO_IOCTL_MAGIC 'a'
//#define AUDIO_SET_VOLUME   _IOW(AUDIO_IOCTL_MAGIC, 10, unsigned)
#define AUDIO_SWITCH_DEVICE  _IOW(AUDIO_IOCTL_MAGIC, 32, unsigned)  // m2 both
#define AUDIO_START_VOICE    _IOW(AUDIO_IOCTL_MAGIC, 35, unsigned)  // m2 streak
#define AUDIO_STOP_VOICE     _IOW(AUDIO_IOCTL_MAGIC, 36, unsigned)  // ""

#define AUDIO_START_FM       _IOW(AUDIO_IOCTL_MAGIC, 37, unsigned)  // m2 HTC HD2 etc.
#define AUDIO_STOP_FM        _IOW(AUDIO_IOCTL_MAGIC, 38, unsigned)  // ""

// Sink (RX) devices
#define STREAK_FM_SPKR  0x0208e512
#define STREAK_FM_HDST  0x0208e513
/*
!! Streak

*/
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_STEREO      0x1081514           // m2 qsd8250 / qsd8k Streak
#define SPKR_PHONE_STEREO   ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_STEREO

#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_MONO        0x1081513
#define FM_SPKR             ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_MONO        // m2 qsd8250 / qsd8k (eg HTC HD2)

#define ADSP_AUDIO_DEVICE_ID_HEADSET_SPKR_STEREO    0x107ac8a
#define FM_HEADSET          ADSP_AUDIO_DEVICE_ID_HEADSET_SPKR_STEREO    // m2 qsd8250 / qsd8k (eg HTC HD2)

int m2_do (int fd, int func_num) {
  uint32_t volume = 127;
  uint32_t path[2];
  int ret = -1;
  char *dev_str = NULL;
  int dev_id = 0;

  switch (func_num) {
    case 0:
      ret = snd_endpoints_disp (fd);
      dev_id = dev_id_get_disp (fd, dev_str);
      return (0);
    case 1:
      ret = ioctl (fd, AUDIO_START_FM, NULL);
      break;
    case 2:
      ret = ioctl (fd, AUDIO_STOP_FM, NULL);
      break;
    case 3:
      ret = ioctl (fd, AUDIO_SET_VOLUME, & volume);
      break;
    case 4:
      path[0] = FM_HEADSET;
      path[1] = 0; //rx_acdb_id;
      ret = ioctl (fd, AUDIO_SWITCH_DEVICE, & path);
      break;
    case 5:
      path[0] = FM_SPKR;
      path[1] = 0; //rx_acdb_id;
      ret = ioctl (fd, AUDIO_SWITCH_DEVICE, & path);
      break;
    //case 6:
    //  path[0] = HEADSET_MIC;
    //  path[1] = 0; //tx_acdb_id;
    //  ret = ioctl (fd, AUDIO_SWITCH_DEVICE, & path);
    //  break;

    case 7:
      ret = ioctl (fd, AUDIO_START_VOICE, NULL);
      break;
    case 8:
      ret = ioctl (fd, AUDIO_STOP_VOICE, NULL);
      break;
    case 9:
      path[0] = SPKR_PHONE_STEREO;
      path[1] = 0; //tx_acdb_id;
      ret = ioctl (fd, AUDIO_SWITCH_DEVICE, & path);
      break;
    case 10:
      path[0] = STREAK_FM_SPKR;
      path[1] = 0; //tx_acdb_id;
      ret = ioctl (fd, AUDIO_SWITCH_DEVICE, & path);
      break;
    case 11:
      path[0] = STREAK_FM_HDST;
      path[1] = 0; //tx_acdb_id;
      ret = ioctl (fd, AUDIO_SWITCH_DEVICE, & path);
      break;
    default:
      if (func_num >= 1000) {
        path[0] = func_num - 1000;
        path[1] = 0;
        ret = ioctl (fd, SND_SET_FM_RADIO_VOLUME, & path);
        //int32_t volume = func_num - 1000;
        //errno = 0;
        //ret = ioctl (fd, SND_SET_FM_RADIO_VOLUME, & volume);
        //if (ret)
        //  printf ("m2_do SND_SET_FM_RADIO_VOLUME ioctl ret: %d  errno: %d\n", ret, errno);
        //else
        //  printf ("m2_do SND_SET_FM_RADIO_VOLUME ioctl ret: %d\n", ret);
      }
  }
  if (ret)
    printf ("m2_do ioctl func_num: %d  ret: %d  errno: %d\n", func_num, ret, errno);
  else
    printf ("m2_do ioctl func_num: %d\n", func_num);
  return (ret);
}

/* M3 for msm7k etc. gdsp5v2        see audio_dev_ctl.c, snddev_data.c (FM_HANDSET_OSR_64, ifmradio_handset_osr64_actions, .capability = (SNDDEV_CAP_RX | SNDDEV_CAP_FM), audio_def.h
#define _MACH_QDSP5_V2_AUDIO_DEF_H

Define sound device capability
#define SNDDEV_CAP_RX 0x1 // RX direction
#define SNDDEV_CAP_TX 0x2 // TX direction
#define SNDDEV_CAP_VOICE 0x4 // Support voice call
#define SNDDEV_CAP_PLAYBACK 0x8 // Support playback
#define SNDDEV_CAP_FM 0x10 // Support FM radio
#define SNDDEV_CAP_TTY 0x20 // Support TTY 
*/
#define SNDDEV_CAP_FM 0x10

//#define AUDIO_IOCTL_MAGIC 'a'
#define AUDIO_GET_NUM_SND_DEVICE    _IOR (AUDIO_IOCTL_MAGIC, 20, unsigned)
#define AUDIO_GET_SND_DEVICES       _IOWR(AUDIO_IOCTL_MAGIC, 21, struct msm_snd_device_list)
#define AUDIO_ENABLE_SND_DEVICE     _IOW (AUDIO_IOCTL_MAGIC, 22, unsigned)
#define AUDIO_DISABLE_SND_DEVICE    _IOW (AUDIO_IOCTL_MAGIC, 23, unsigned)
struct msm_snd_device_info {
      uint32_t dev_id;
      uint32_t dev_cap;                                                 // bitmask describe capability of device. Look for SNDDEV_CAP_FM 0x10
      char dev_name[64];
};
struct msm_snd_device_list {
      uint32_t  num_dev; /* Indicate number of device info to be retrieved */
      struct msm_snd_device_info *list;
};

// ?? Generic to m1/m2/m3 ???
int dev_id_get_disp (int fd, char *dev_str) {
  int num_dev = 0, dev_id = -1;
  int ret = ioctl (fd, AUDIO_GET_NUM_SND_DEVICE, &num_dev);
  if (ret) {
    printf ("dev_id_get_disp AUDIO_GET_NUM_SND_DEVICE ioctl error ret: %d  num_dev: %d  errno: %d\n", ret, num_dev, errno);
    if (num_dev <= 0 || num_dev > 256)
      return (-1);
  }
  int size = sizeof (struct msm_snd_device_info) * num_dev;
  printf ("dev_id_get_disp sizeof (struct msm_snd_device_info): %d  num_dev: %d\n", sizeof (struct msm_snd_device_info), num_dev);
  struct msm_snd_device_list devlist = {0};
  devlist.num_dev = num_dev;
  devlist.list = malloc (size);
  if (! devlist.list) {
    printf ("dev_id_get_disp malloc errno: %d\n", errno);
    return (-2);
  }
  ret = ioctl (fd, AUDIO_GET_SND_DEVICES, &devlist);
  if (ret) {
    printf ("dev_id_get_disp AUDIO_GET_SND_DEVICES ioctl error ret: %d errno: %d\n", ret, errno);
    if (devlist.list)
      free (devlist.list);
    return (-3);
  }
  int ctr;
  for (ctr = 0; ctr < num_dev; ctr++) {
    if (dev_str && dev_id == -1) {                                      // If want to get dev_id and don't have it yet...
      if (! strncmp(devlist.list[ctr].dev_name, dev_str, 64))
        dev_id = devlist.list[ctr].dev_id;
    }
    if (devlist.list[ctr].dev_cap & SNDDEV_CAP_FM)
      printf ("dev_id: %2.2d  dev_cap: 0x%8.8x  dev_name: %64.64s  FM Capability\n", devlist.list[ctr].dev_id, devlist.list[ctr].dev_cap, devlist.list[ctr].dev_name);
    else
      printf ("dev_id: %2.2d  dev_cap: 0x%8.8x  dev_name: %64.64s\n", devlist.list[ctr].dev_id, devlist.list[ctr].dev_cap, devlist.list[ctr].dev_name);
  }
  if (devlist.list)
    free (devlist.list);
  if (dev_str  && dev_id == -1) {                                       // If want to get dev_id and don't have it...
    printf ("dev_id_get_disp device not found: %s\n", dev_str);
    return (-2);
  }
  else if (dev_str) {                                                   // If want to get dev_id and DO have it...
    printf ("dev_id_get_disp device: %d\n", dev_id);
    return (dev_id);
  }
  return (0);
}
int m3_enab_disab (int fd, int disable, int dev_id) {
  int ret = 0;
  if (! disable) {
    ret = ioctl (fd, AUDIO_ENABLE_SND_DEVICE, &dev_id);     // 0x40046117
    if (ret)
      printf ("m3_enab_disab enable dev_id: %d error errno: %d\n", dev_id, errno);
    else
      printf ("m3_enab_disab enable dev_id: %d success\n", dev_id);
  }
  else {
    ret = ioctl (fd, AUDIO_DISABLE_SND_DEVICE, &dev_id);    // 0x40046116
    if (ret)
      printf ("m3_enab_disab disable dev_id: %d error errno: %d\n", dev_id, errno);
    else
      printf ("m3_enab_disab disable dev_id: %d success\n", dev_id);
  }
  if (ret)
    return (-1);
  return (0);
}

int m3_do (int fd, int func_num, char *optm3_str) {
  int ret = 0;
  char *dev_str = "fmradio_handset_rx";
  int dev_id = 0;

  if (func_num == 1 || func_num == 2) {
    dev_str = "fmradio_headset_rx";
  }
  else if (func_num == 3 || func_num == 4) {
    dev_str = "fmradio_speaker_rx";
  }

  if (optm3_str)
    dev_str = optm3_str;
  dev_id = dev_id_get_disp (fd, dev_str);
  if (dev_id < 0) {
    printf ("m3_do dev_id_get_disp error\n");
    //return (-1);
  }

  if (func_num == 0) {
    ret = snd_endpoints_disp (fd);
    //dev_str = NULL;
    dev_id = dev_id_get_disp (fd, NULL);//dev_str);
  }
  else if (func_num == 1 || func_num == 3) {                            // If enable...
    ret = m3_enab_disab (fd, 0, dev_id);
  }
  else if (func_num == 2 || func_num == 4) {                            // If disable...
    ret = m3_enab_disab (fd, 1, dev_id);
  }
  else if (func_num >= 1000) {
    int32_t volume = func_num - 1000;
    errno = 0;
    ret = ioctl (fd, SND_SET_FM_RADIO_VOLUME, & volume);
    if (ret)
      printf ("m1_do SND_SET_FM_RADIO_VOLUME ioctl ret: %d  errno: %d\n", ret, errno);
    else
      printf ("m1_do SND_SET_FM_RADIO_VOLUME ioctl ret: %d\n", ret);
  }
  return (ret);
}


#include "alsa.c"   // int m4_do (int fd, int func_num)
 
char *audio_dev_name_m1 = "/dev/msm_snd";
char *audio_dev_name_m2 = "/dev/msm_audio_ctl";
char *audio_dev_name_m3 = "/dev/msm_audio_dev_ctrl";
char *audio_dev_name_m4 = "/dev/snd/controlC0";
char *audio_dev_name_m5 = "/dev/snd/pcmC0D0c";//controlC0";//pcmC0D0p";
char *audio_dev_name_m6 = "/dev/snd/pcmC0D6c";//mixer";
char *audio_dev_name    = NULL;//audio_dev_name_m1;

// QSD8x50 family of devices (Nexus One, HTC Evo, etc)

void help_print () {
    printf ("Require at least 2 arguments for mode & function/type:\n\n");

    printf ("For /dev/msm_snd            = msm7x30 (eg HTC Legend, LG P500, SE X10mini)\n");
    printf ("1 0 = info\n1 9 = FM Headset\n1 10 = FM Headset B\n1 11 = FM Speaker\n1 39 = SE Headset\n1 43 = SE Headphone\n1 1007 = Volume 7\n\n");

    printf ("For /dev/msm_audio_ctl      = qsd8250 / qsd8k (eg HTC Desire/HD2/Dell Streak)\n");
    printf ("2 0 = info\n2 1 = Start FM\n2 2 = Stop FM\n2 3 = Set mid volume\n2 4 = FM Headset\n2 5 = FM Speaker\n2 1008 = Volume 8\n\n");
    printf ("Streak:\n2 7 = Start Voice\n2 8 = Stop Voice\n2 9 = SPKR_PHONE_STEREO\n2 10 = FM Speaker\n2 11 = FM Headset\n\n");

    printf ("For /dev/msm_audio_dev_ctrl = qdsp5v2 (eg HTC Desire HD/Z)\n");
    printf ("3 0 = info\n3 1 = FM Headset enable\n3 2 = FM Headset disable\n3 3 = FM Speaker enable\n3 4 = FM Speaker disable\n3 1 devname = devname enable\n3 2 devname = devname disable\n3 1009 = Volume 9\n\n");

    printf ("For /dev/snd/controlC0      = ALSA (eg Samsung GalaxyS)\n");
    printf ("4 0 = info\n4 type id value = write <=4 value(s) to id\n");
    printf ("(type 1 = Boolean, 2 = Integer32, 3 = Enumerated, 6 = Integer64)\n\n");
}

int snd_fd = -1;

int main (int argc, char * argv []) {
  int ret;
  char * opt1_str = NULL, * opt2_str = NULL;
  char * file_str = NULL, * time_str = NULL, * bufs_str = NULL;
  printf ("Spirit FM Radio audio utility version 2014, Feb 14\n");//: %x\n", AUDIO_GET_SND_DEVICES);
  printf ("%s", copyright);
  if (argc < 3) {
    help_print ();
    return  (-1);
  }
  int mode_num = atoi (argv [1]);                                        // Param 1 = Mode Number: 1 = /dev/msm_snd, ... 4 = ALSA
  printf ("Mode: \"%s\" (%d)\n", argv [1], mode_num);
  if (mode_num < 1 || mode_num > 7) {
    printf ("Mode must be a number from 1 to 7:\n\n");
    help_print ();
    return  (-1);
  }
  int func_num = atoi (argv [2]);                                        // Param 2 = Function Number: 0 = Info, For ALSA: 1 = Write Boolean, 2 = Integer, 3 = Enum
  printf ("Function: \"%s\" (%d)\n", argv [2], func_num);
  int max_func_num = 1;
  switch (mode_num) {
    case 1:
      audio_dev_name = audio_dev_name_m1;
      max_func_num = 65535;
      break;
    case 2:
      audio_dev_name = audio_dev_name_m2;
      max_func_num = 65535;//6 + 5;
      break;
    case 3:
      audio_dev_name = audio_dev_name_m3;
      max_func_num = 65535;//4;
      break;
    case 4:
      audio_dev_name = audio_dev_name_m4;
      max_func_num = 6;
      break;
    case 5:
      audio_dev_name = audio_dev_name_m5;
      max_func_num = 999999999;
      break;
    case 6:
      audio_dev_name = audio_dev_name_m6;
      max_func_num = 999999999;
      break;
    case 7:
      audio_dev_name = "/dev/null";//audio_dev_name_m7;
      max_func_num = 999999999;
      break;
  }
  if (argc > 3) {
    opt1_str = argv [3];
  }
  if (argc > 4) {
    opt2_str = argv [4];
  }
  if (argc > 5) {                                                       // If at least 5 parameters...
    audio_dev_name = argv [5];                                           // Set audio device name
  }
  if (func_num < 0 || func_num > max_func_num) {
    printf ("For mode: %d  Function must be a number from 0 to %d:\n\n", mode_num, max_func_num);
    help_print ();
    return  (-1);
  }
  printf ("Opening device %s\n", audio_dev_name);
  snd_fd = open (audio_dev_name, O_NONBLOCK | O_RDWR ); //O_RDWR, 0);//S_IRWXU | S_IRWXG | S_IRWXO);// // O_RDONLY, O_WRONLY, or O_RDWR
  if (snd_fd < 0) {
    printf ("Error opening device1 %s errno: %s (%d)\n", audio_dev_name, strerror (errno), errno);
    return (-2);
  }
  printf ("snd_fd: %d\n", snd_fd);
  char *id = "1";//int id = 1;
  long value = 0;
  unsigned flags = 0;
  switch (mode_num) {
    case 1:
      ret = m1_do (snd_fd, func_num);
      break;
    case 2:
      ret = m2_do (snd_fd, func_num);
      break;
    case 3:
      ret = m3_do (snd_fd, func_num, opt1_str);
      break;
    case 4:
      if (func_num > 0) {                                               // If anything but display (ssd 4 0)
        if (argc < 5) { // (opt2_str == NULL)
          printf ("Require at least 4 arguments for mode (4), type, ALSA id & value:\n\n");
          help_print ();
          close (snd_fd);
          return (-1);
        }
        value = atol (argv [4]);  // opt2_str                                   // Value to write to control // !! ?? Type only needed for Int64 ??
      }
      ret = m4_do (snd_fd, func_num, opt1_str, value);                          // Func, ID, value
      break;

    case 5: // 5 loops forever, while 6 doesn't         "5 9" is only combination used by Spirit(1)
      if (argc > 6)                                                             // If at least 6 parameters...
        file_str = argv [6];
      else
        file_str = NULL;//"/sdcard/fm.pcm";
      if (argc > 7)                                                             // If at least 7 parameters...
        time_str = argv [7];
      if (argc > 8)                                                             // If at least 8 parameters...
        bufs_str = argv [8];
      do_pcm_rec (func_num, opt1_str, opt2_str, file_str, time_str, bufs_str);  // Func, Samplerate, Channels, (device), (filename), (Time), (Bufsize)

      while (func_num == 8 || func_num == 9)                                    // Func Num = 9 for loop forever
        sleep (999999);
      break;

    case 6:
      if (argc > 6)                                                                 // If at least 6 parameters...
        file_str = argv [6];
      else
        file_str = NULL;//"/sdcard/fm.pcm";
      if (argc > 7)                                                                 // If at least 7 parameters...
        time_str = argv [7];
      if (argc > 8)                                                                 // If at least 8 parameters...
        bufs_str = argv [8];
      do_pcm_rec (func_num, opt1_str, opt2_str, file_str, time_str, bufs_str);      // Func, Samplerate, Channels, (device), (filename), (Time), (Bufsize)
      break;
  }

  close (snd_fd);
  return 0;
}

/*
/sys/devices/platform/soc-audio/sound/card0/pcmC0D0p/power:
-rw-r--r-- root     root         4096 2012-10-21 02:45 autosuspend_delay_ms
-rw-r--r-- root     root         4096 2012-10-21 02:45 control
-r--r--r-- root     root         4096 2012-10-21 02:45 runtime_active_time
-r--r--r-- root     root         4096 2012-10-21 02:45 runtime_status
-r--r--r-- root     root         4096 2012-10-21 02:45 runtime_suspended_time

/sys/kernel/debug/asoc:
drwxr-xr-x root     root              1969-12-31 19:00 Midas_WM1811
-r--r--r-- root     root            0 1969-12-31 19:00 codecs
-r--r--r-- root     root            0 1969-12-31 19:00 dais
-r--r--r-- root     root            0 1969-12-31 19:00 platforms

/sys/kernel/debug/asoc/Midas_WM1811:
drwxr-xr-x root     root              1969-12-31 19:00 dapm
-rw-r--r-- root     root            0 1969-12-31 19:00 dapm_pop_time
drwxr-xr-x root     root              1969-12-31 19:00 wm8994-codec

/sys/kernel/debug/asoc/Midas_WM1811/dapm:
-r--r--r-- root     root            0 1969-12-31 19:00 bias_level

*/
      //tifm_test ();
      //system ("nohup arec -D hw:0,6 > /sdcard/nh &");
//      system ("arec -D hw:0,6 -P");
      //system ("kill -s SIGTSTP `pidof arec`");



  void wav_write_bytes (char * buf, int idx, int bytes, int value) {
    if (bytes > 0)
      buf [idx + 0] = (char) (value>> 0 & 0xFF);
    if (bytes > 1)
      buf [idx + 1] = (char) (value>> 8 & 0xFF);
    if (bytes > 2)
      buf [idx + 2] = (char) (value>>16 & 0xFF);
    if (bytes > 3)
      buf [idx + 3] = (char) (value>>24 & 0xFF);
  }

  int wav_data_size = 0x00000000;
  char wav_header [45] = "RIFF....WAVEfmt sc1safncsamrbytrbabsdatasc2s";
  int rec_fd = 0;

// WAV file size limit = 4 GB. At 10 MB/minute (44.1 Stereo) this is 400 minutes = 6 hours, 40 minutes
  int wav_write_header (int sample_rate, int channels) {

    //printf ("wav_write_header\n");

    wav_write_bytes (wav_header, 0x04, 4, wav_data_size + 36);      // Chunksize = total filesize - 8 = DataSize + 36
    wav_write_bytes (wav_header, 0x10, 4, 0x00000010);                        // Subchunk1Size = 16 for PCM
    wav_write_bytes (wav_header, 0x14, 2, 0x0001);                            // AudioFormat = PCM = 1
    wav_write_bytes (wav_header, 0x16, 2, channels);                          // NumChannels = 1 = Mono
    wav_write_bytes (wav_header, 0x18, 4, sample_rate);                 // SampleRate
    wav_write_bytes (wav_header, 0x1c, 4, sample_rate * channels * 2);  // ByteRate = SampleRate * NumChannels * BitsPerSample/8
    wav_write_bytes (wav_header, 0x20, 2, channels * 2);                      // BlockAlign = NumChannels * BitsPerSample/8
    wav_write_bytes (wav_header, 0x22, 2, 16);                                // BitsPerSample (no regard for channels)

    wav_write_bytes (wav_header, 0x28, 4, wav_data_size);           // DataSize = Subchunk2Size = NumSamples * NumChannels * BitsPerSample/8 = Filesize - 44
//lseek
    if (rec_fd > 0) {
      int written = write (rec_fd, wav_header, 44);                   // Write the wav header
      if (written != 44) {                                             // Small buffers under ? bytes should not be segmented ?
        printf ("wav_write_header write errno: %d\n", errno);
        close (rec_fd);
        rec_fd = -1;
        return (-1);
      }
    }
    return (0);
  }

  int wav_write_final (int sample_rate, int channels) {
    if (rec_fd > 0) {
      if (lseek (rec_fd, 0, SEEK_SET)) {
        printf ("wav_write_final seek errno: %d\n", errno);
          return (-1);
      }
      if (wav_write_header (sample_rate, channels)) {
          return (-1);
        }
      close (rec_fd);
      rec_fd = -1;
    }
    return (0);
  }


  int pcm_read_ctr = 0;

  void display_pcm_stat (int increment, int offset, int len, signed char * buf) {
    int max = -32768, min = 32769, avg = 0, max_amp_avg = 0;
    int i = 0;
    for (i = 0; i < len; i += increment) {
      signed short short_sample = *((signed short *) & buf [i + 0 + offset]);
      int sample = short_sample;
      if (sample > max)
        max = sample;
      if (sample < min)
        min = sample;
      avg += sample;
    }
    //avg = (max + min) / 2;      // Doesn't work well due to spikes
    avg /= (len / 2);             // This smooths average to about -700 on GS3.
    if ( (max - avg) > max_amp_avg)   // Avg means we don't have to test min
      max_amp_avg = max - avg; 
    printf ("display_pcm_stat readi frames: %d  len: %d  max: %d  min: %d  avg: %d  max_amp_avg: %d\n", pcm_read_ctr, len, max, min, avg, max_amp_avg);
  }

  void display_pcm_stats (int channels, int len, signed char * buf) {
    if (channels == 1) {
      display_pcm_stat (2, 0, len, buf);                                // Treat as mono
    }
    else {
      display_pcm_stat (4, 0, len, buf);                                // Left is first
      display_pcm_stat (4, 2, len, buf);                                // Right is second
    }
  }


struct snd_pcm_info_copy {
         unsigned int device;            /* RO/WR (control): device number */
         unsigned int subdevice;         /* RO/WR (control): subdevice number */
         int stream;                     /* RO/WR (control): stream direction */
         int card;                       /* R: card number */
         unsigned char id[64];           /* ID (user selectable) */
         unsigned char name[80];         /* name of this device */
         unsigned char subname[32];      /* subdevice name */
         int dev_class;                  /* SNDRV_PCM_CLASS_* */
         int dev_subclass;               /* SNDRV_PCM_SUBCLASS_* */
         unsigned int subdevices_count;
         unsigned int subdevices_avail;
         union snd_pcm_sync_id sync;     /* hardware synchronization ID */
         unsigned char reserved[64];     /* reserved for future... */
};

void pcm_info_dump () {
  //#include <linux/ioctl.h>
  //#define SNDRV_PCM_IOCTL_INFO            _IOR('A', 0x01, struct snd_pcm_info)

  struct snd_pcm_info info;
  struct snd_pcm_info * i = & info;

  int ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_INFO, & info);   //-2128592639 0x81204101  size: 288 // 2166374657     //printf ("pcm_info_dump info: %d %x  size: %d\n", SNDRV_PCM_IOCTL_INFO, SNDRV_PCM_IOCTL_INFO, sizeof (info));
  if (ret) {
    printf ("pcm_info_dump info error ret: %d  errno: %d\n", ret, errno);
    return;
  }

  printf ("pcm_info_dump device: %d  subdevice: %d  stream: %d  card: %d\n", i->device, i->subdevice, i->stream, i->card);
  //if (i->id[0])
  printf ("pcm_info_dump id: %s  name: %s  subname: %s\n", i->id, i->name, i->subname);

  printf ("pcm_info_dump dev_class: %d  dev_subclass: %d  subdevices_count: %d  subdevices_avail: %d\n", i->dev_class, i->dev_subclass, i->subdevices_count, i->subdevices_avail);
  int ctr = 0;
  printf ("pcm_info_dump sync:");
  //char * ptr = (char *) & i->sync;
  for (ctr = 0; ctr < 16; ctr ++) {
    //char dat = * ptr ++;
    //printf (" %x", dat);
    printf (" %x", i->sync.id [ctr]);
  }
  printf ("\n");
}

int pcm_prepare (int func_num, int sample_rate, int channels) {
  int ret = 0;
  struct snd_pcm_hw_params params;
  //#define SNDRV_PCM_IOCTL_HW_REFINE       _IOWR('A', 0x10, struct snd_pcm_hw_params)
  //#define SNDRV_PCM_IOCTL_HW_PARAMS       _IOWR('A', 0x11, struct snd_pcm_hw_params)
  param_init (& params);

/* Don't need a refine
//  ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_HW_REFINE, & params);
  if (ret)
    printf ("pcm_prepare set hw refine error ret: %d  errno: %d\n", ret, errno);
  else
    printf ("pcm_prepare set hw refine OK\n");
*/

  int sample_bits = 16;

  param_set_mask (& params, SNDRV_PCM_HW_PARAM_ACCESS,      SNDRV_PCM_ACCESS_RW_INTERLEAVED);
  param_set_mask (& params, SNDRV_PCM_HW_PARAM_FORMAT,      SNDRV_PCM_FORMAT_S16_LE);
  param_set_mask (& params, SNDRV_PCM_HW_PARAM_SUBFORMAT,   SNDRV_PCM_SUBFORMAT_STD);

  param_set_int  (& params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, sample_bits);
  param_set_int  (& params, SNDRV_PCM_HW_PARAM_FRAME_BITS,  sample_bits * channels);
  param_set_int  (& params, SNDRV_PCM_HW_PARAM_CHANNELS,    channels);
  param_set_int  (& params, SNDRV_PCM_HW_PARAM_RATE,        sample_rate);

/*
  if (func_num != 9) {
//    param_set_int  (& params, SNDRV_PCM_HW_PARAM_PERIODS,     4);     // Interrupts per buffer
//    param_set_min  (& params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, 1024);  // Frames between interrupts
  }
  else {
//    param_set_min  (& params, SNDRV_PCM_HW_PARAM_PERIOD_TIME, 1000);  // Microseconds between interrupts
    //param_set_min  (& params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, bufs / 16);
    //param_set_min  (& params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, 500);

//param_set_int  (& params, SNDRV_PCM_HW_PARAM_PERIODS,     16);     // Interrupts per buffer
//param_set_min  (& params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, 160 / channels);  // Frames between interrupts

  }
*/

    //int cctr = 0;
    //char * cptr = (char *) & params;
    //for (cctr = 0; cctr < sizeof (struct snd_pcm_hw_params); cctr ++) {
    //  printf ("params %3.3d: x%2.2x\n", cctr, cptr [cctr]);
    //}

  ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_HW_PARAMS, & params);
  if (ret) {
    printf ("pcm_prepare set hw params error ret: %d  errno: %d\n", ret, errno);
    return (-1);
  }
  //else
  //  printf ("pcm_prepare set hw params OK\n");



///*  Don't need to set software params

//?? Still need ????
  if (func_num == 8 || func_num == 9) {
    struct snd_pcm_sw_params sparams;
    //#define SNDRV_PCM_IOCTL_SW_PARAMS       _IOWR('A', 0x13, struct snd_pcm_sw_params)
    memset (& sparams, 0, sizeof (sparams));

    sparams.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;//ENABLE;
    sparams.period_step =         1;
    sparams.avail_min =           80;         //1;                                                // Act when a minimum of 1 frame is ready
    sparams.start_threshold =     1;          //1024*8/2;//period_cnt * period_sz / 2;                 // 2048
    sparams.stop_threshold =      2147483647; //1024*8*10;//period_cnt * period_sz * 10;                 // 40960
    sparams.xfer_align =          1;//80;         //512;//period_sz / 2;                                   // 512      needed for old kernels
    sparams.silence_size =        0;
    sparams.silence_threshold =   0;

    ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_SW_PARAMS, & sparams);
    if (ret)
      printf ("pcm_prepare set sw params error ret: %d  errno: %d\n", ret, errno);
    else
      printf ("pcm_prepare set sw params OK\n");
  }
//*/


    // Prepare for transfer
  ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_PREPARE);//, snd_fd);//0x4140, 0x7);
  if (ret) {
    printf ("pcm_prepare prepare error ret: %d  errno: %d\n", ret, errno);
    return (-1);
  }
  //else
  //  printf ("pcm_prepare 0x4140, 0x7 OK\n");

  return (0);
}

/* GS3: /dev/snd/pcmC0D3c
pcm_info_dump device: 3  subdevice: 0  stream: 1  card: 0
pcm_info_dump id: Pri_Dai wm8994-aif1-3  name:   subname: subdevice #0
pcm_info_dump dev_class: 0  dev_subclass: 0  subdevices_count: 1  subdevices_avail: 0
pcm_info_dump sync: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*/
int do_pcm_rec (int func_num, char * samprate_str, char * channels_str, char * file_str, char * time_str, char * bufs_str) {    // Func, Samplerate, Channels, (device), (Time), (Bufsize)

  int ret = 0;
  int time = 10;                                                        // Default 10 seconds
  int bufs = 4096;
  int sample_rate = 44100;
  int channels = 2;
  if (samprate_str != NULL)
    sample_rate = atol (samprate_str);
  if (channels_str != NULL)
    channels = atol (channels_str);
  if (time_str != NULL)
    time = atol (time_str);
  if (bufs_str != NULL)
    bufs = atol (bufs_str);
  //if (_str != NULL)
  //   = atol (_str);

  printf ("do_pcm_rec sample_rate: %d  channels: %d\n", sample_rate, channels);

  //ret = fcntl (snd_fd, F_GETFL);                                        // !! Don't use flags, only to match
  ret = fcntl (snd_fd, F_SETFL, O_RDWR);    // !! Need this !

  pcm_info_dump ();

  if (pcm_prepare (func_num, sample_rate, channels)) {
    return (-1);
  }

  if (func_num == 8) {

    // Start
    ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_START);
    if (ret) {
      printf ("do_pcm_rec start error ret: %d  errno: %d\n", ret, errno);
      return (-1);
    }
    if (file_str == NULL)
      //file_str = "/dev/snd/pcmC0D17c";
      return (-1);

    int snd2_fd = open (file_str, O_NONBLOCK | O_RDWR);

    if (snd2_fd < 0) {
      printf ("Error opening device2 %s errno: %s (%d)\n", file_str, strerror (errno), errno);
      return (-2);
    }
    printf ("snd2_fd: %d\n", snd2_fd);
    ret = fcntl (snd2_fd, F_SETFL, O_RDWR);    // !! Need this !

    int temp_fd = snd_fd;
    snd_fd = snd2_fd;

    pcm_prepare (func_num, sample_rate, channels);
    //ms_sleep (10);
    snd_fd = temp_fd;

    // Start
    ret = ioctl (snd2_fd, SNDRV_PCM_IOCTL_START);
    if (ret) {
      printf ("do_pcm_rec start2 error ret: %d  errno: %d\n", ret, errno);
      return (-1);
    }


    return (0);
  }

  if (file_str != NULL) {
    rec_fd = open (file_str, O_CREAT | O_TRUNC | O_NONBLOCK | O_WRONLY); //        // O_RDONLY, O_WRONLY, or O_RDWR
    if (rec_fd < 0) {
      printf ("do_pcm_rec open record file error rec_fd: %d  errno: %d\n", rec_fd, errno);
      return (-1);
    }
    wav_data_size = 0;
    if (wav_write_header (sample_rate, channels)) {
      return (-1);
    }
  }


    // Start transfer
  struct snd_xferi xfer;

  //#define REC_BUF_SIZE  8192 //640 //5120    //4096//65536   // 4096
  //signed char buf [REC_BUF_SIZE] = {0};

    // @ 48K, stereo, 16 bps, 320 bytes = 80 frames = 80 / 48000 seconds = 1.67 ms

    // OXL OK:  8000 (25 * 320), 320, 640
    // OXL Err: 4000 (12.5*320), 480
  
  signed char * buf = malloc (bufs);
  if (buf == NULL)
    return (-1);

  //#define SNDRV_PCM_IOCTL_READI_FRAMES    _IOR('A', 0x51, struct snd_xferi)
  snd_pcm_sframes_t result = 1; // ??
  xfer.result = result;
  xfer.buf = buf;
  snd_pcm_uframes_t frames = bufs / (2 * channels);//1024;//2048;//sizeof (buf) / (16 / 8);   // 16 = channels * bits per sample
  xfer.frames = frames;
/*
  int ctr = 0;
  for (ctr = 0; ctr < bufs; ctr += 2) {
    buf [ctr] = 0;//ctr % 2;//ctr % 16;
    if (func_num == 1)                                                  // If 1 = Silence
      buf [ctr + 1] =  0;
    else                                                                // Else if 0/other = triangle wave
      buf [ctr + 1] =  (ctr / 2) % 128;//2048;//1024;
  }
*/

  int errors = 0;

  while (wav_data_size < 2 * channels * sample_rate * time) {           // seconds
    if (func_num == 0 || func_num == 9)                                 // If 0/9 = read/record
      ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_READI_FRAMES, & xfer);       // Read  interleaved
    else                                                                // Else if other = write/play
      ret = ioctl (snd_fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, & xfer);      // Write interleaved
    if (ret) {
      printf ("do_pcm_rec readi frames errors: %d  ret: %d  errno: %d\n", ++ errors, ret, errno);
      //return (-1);

      if (func_num == 9)    // For Spirit audio, just need to return after error and loop on long sleep
        return (-1);


      close (snd_fd);                                                   // Close and re-open to attempt to recover:
      snd_fd = open (audio_dev_name, O_NONBLOCK ); // // O_RDONLY, O_WRONLY, or O_RDWR
      if (snd_fd < 0) {
        printf ("Error opening device3 %s errno: %s (%d)\n", audio_dev_name, strerror (errno), errno);
        return (-2);
      }
      printf ("snd_fd: %d\n", snd_fd);
      ret = fcntl (snd_fd, F_SETFL, O_RDWR);    // !! Need this !

      pcm_prepare (func_num, sample_rate, channels);
      ms_sleep (10);
    }
    else {
      int len = xfer.result * 2 * channels;
      if (pcm_read_ctr ++ % 1 == 0) {                                   // Every buffer
        display_pcm_stats (channels, len, buf);
      }

      if (rec_fd > 0) {
        int written = write (rec_fd, buf, len);                           // Write the PCM data
        if (written != len) {                                             // Small buffers under ? bytes should not be segmented ?
          printf ("do_pcm_rec readi write errno: %d\n", errno);
          close (rec_fd);
          rec_fd = -1;
          return (-1);
        }
      }
      wav_data_size += len;

    }
    // Next read/write
  }

  wav_write_final (sample_rate, channels);

  printf ("errors: %d\n", errors);

  return (0);
}


