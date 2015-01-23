
  #define LOGTAG "sftnrssl"

#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include <android/log.h>
#include "jni.h"

#include "tnr_tnr.h"

#define EVT_LOCK_BYPASS
            // Locking causes problems after a while; blocks in events_process()    (   ioctl (dev_hndl, Si4709_IOC_RDS_DATA_GET, & rd);    )
  #include "tnr_tnr.c"


    // Functions called from this chip specific code to generic code:

  long ms_sleep (long ms);

  #define  loge(...)  fm_log_print(ANDROID_LOG_ERROR, LOGTAG,__VA_ARGS__)
  #define  logd(...)  fm_log_print(ANDROID_LOG_DEBUG, LOGTAG,__VA_ARGS__)

  int fm_log_print (int prio, const char * tag, const char * fmt, ...);

  extern int extra_log;// = 0;


  int dev_hndl      =     -1;

    // FM Chip specific code:


#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t


typedef struct
{
    int power_state;
    int seek_state;
}dev_state_t;

typedef struct
{
  u8 curr_rssi;
  u8 curr_rssi_th;
  u8 curr_snr;
}rssi_snr_t;

typedef struct {
    uint8_t part_number;
    uint16_t manufact_number;
} device_id_t;

typedef struct {
    uint8_t  chip_version;
    uint8_t     device;
    uint8_t  firmware_version;
} chip_id_t;

struct sys_config2
{
    uint16_t rssi_th;
    uint8_t fm_band;
    uint8_t fm_chan_spac;
    uint8_t fm_vol;
};

struct sys_config3
{
    uint8_t smmute;
    uint8_t smutea;
    uint8_t volext;
    uint8_t sksnr;
    uint8_t skcnt;
};

typedef struct
{
    uint8_t rdsr;
    uint8_t stc;
    uint8_t sfbl;
    uint8_t afcrl;
    uint8_t rdss;
    uint8_t blera;
    uint8_t st;
    uint16_t rssi;
} status_rssi_t;

struct power_config
{
    uint16_t dsmute :1;
    uint16_t dmute:1;
    uint16_t mono:1;
    uint16_t rds_mode:1;
    uint16_t sk_mode:1;
    uint16_t seek_up:1;
    uint16_t seek:1;
    uint16_t power_disable:1;
    uint16_t power_enable:1;
};

typedef struct
{
	u16 rdsa;
	u16 rdsb;
	u16 rdsc;
	u16 rdsd;
	u8  curr_rssi;
	u32 curr_channel;
	u8 blera;
	u8 blerb;
	u8 blerc;
	u8 blerd;
} radio_data_t;

#define NUM_SEEK_PRESETS    20

#define WAIT_OVER           0
#define SEEK_WAITING        1
#define NO_WAIT             2
#define TUNE_WAITING        4
#define RDS_WAITING         5
#define SEEK_CANCEL         6

/*dev settings*/
/*band*/
#define BAND_87500_108000_kHz   1
#define BAND_76000_108000_kHz   2
#define BAND_76000_90000_kHz    3

/*channel spacing*/
#define CHAN_SPACING_200_kHz   20        /*US*/
#define CHAN_SPACING_100_kHz   10        /*Europe,Japan*/
#define CHAN_SPACING_50_kHz    5

/*DE-emphasis Time Constant*/
#define DE_TIME_CONSTANT_50   1          /*Europe,Japan,Australia*/
#define DE_TIME_CONSTANT_75   0          /*US*/


// ****************IOCTLS******************
#define Si4709_IOC_MAGIC  0xFA                                          // magic #
#define Si4709_IOC_NR_MAX 40                                            // max seq no

/*commands*/
#define Si4709_IOC_POWERUP                          _IO(Si4709_IOC_MAGIC, 0)
#define Si4709_IOC_POWERDOWN                        _IO(Si4709_IOC_MAGIC, 1)
#define Si4709_IOC_BAND_SET                         _IOW(Si4709_IOC_MAGIC, 2, int)
#define Si4709_IOC_CHAN_SPACING_SET                 _IOW(Si4709_IOC_MAGIC, 3, int)
#define Si4709_IOC_CHAN_SELECT                      _IOW(Si4709_IOC_MAGIC, 4, uint32_t)
#define Si4709_IOC_CHAN_GET                         _IOR(Si4709_IOC_MAGIC, 5, uint32_t)
#define Si4709_IOC_SEEK_UP                          _IOR(Si4709_IOC_MAGIC, 6, uint32_t)
#define Si4709_IOC_SEEK_DOWN                        _IOR(Si4709_IOC_MAGIC, 7, uint32_t)
//#define Si4709_IOC_SEEK_AUTO                       _IOR(Si4709_IOC_MAGIC, 8, u32)         // VNVS:28OCT'09---- Si4709_IOC_SEEK_AUTO is disabled as of now
#define Si4709_IOC_RSSI_SEEK_TH_SET                 _IOW(Si4709_IOC_MAGIC, 9, u8)
#define Si4709_IOC_SEEK_SNR_SET                     _IOW(Si4709_IOC_MAGIC, 10, u8)
#define Si4709_IOC_SEEK_CNT_SET                     _IOW(Si4709_IOC_MAGIC, 11, u8)
#define Si4709_IOC_CUR_RSSI_GET                     _IOR(Si4709_IOC_MAGIC, 12, rssi_snr_t)
#define Si4709_IOC_VOLEXT_ENB                       _IO(Si4709_IOC_MAGIC, 13)
#define Si4709_IOC_VOLEXT_DISB                      _IO(Si4709_IOC_MAGIC, 14)
#define Si4709_IOC_VOLUME_SET                       _IOW(Si4709_IOC_MAGIC, 15, uint8_t)
#define Si4709_IOC_VOLUME_GET                       _IOR(Si4709_IOC_MAGIC, 16, u8)
#define Si4709_IOC_MUTE_ON                          _IO(Si4709_IOC_MAGIC, 17)
#define Si4709_IOC_MUTE_OFF                         _IO(Si4709_IOC_MAGIC, 18)
#define Si4709_IOC_MONO_SET                         _IO(Si4709_IOC_MAGIC, 19)
#define Si4709_IOC_STEREO_SET                       _IO(Si4709_IOC_MAGIC, 20)
#define Si4709_IOC_RSTATE_GET                       _IOR(Si4709_IOC_MAGIC, 21, dev_state_t)
#define Si4709_IOC_RDS_DATA_GET                     _IOR(Si4709_IOC_MAGIC, 22, radio_data_t)
#define Si4709_IOC_RDS_ENABLE                       _IO(Si4709_IOC_MAGIC, 23)
#define Si4709_IOC_RDS_DISABLE                      _IO(Si4709_IOC_MAGIC, 24)
#define Si4709_IOC_RDS_TIMEOUT_SET                  _IOW(Si4709_IOC_MAGIC, 25, u32)
#define Si4709_IOC_SEEK_CANCEL                      _IO(Si4709_IOC_MAGIC, 26)               // VNVS:START 13-OCT'09---- Added IOCTLs for reading the device-id,chip-id,power configuration, system configuration2 registers
#define Si4709_IOC_DEVICE_ID_GET                    _IOR(Si4709_IOC_MAGIC, 27,device_id_t)
#define Si4709_IOC_CHIP_ID_GET                      _IOR(Si4709_IOC_MAGIC, 28,chip_id_t)
#define Si4709_IOC_SYS_CONFIG2_GET                  _IOR(Si4709_IOC_MAGIC, 29,sys_config2)
#define Si4709_IOC_POWER_CONFIG_GET                 _IOR(Si4709_IOC_MAGIC, 30,power_config)
#define Si4709_IOC_AFCRL_GET                        _IOR(Si4709_IOC_MAGIC, 31,u8)            // AFCRL bit, to check for a valid channel
#define Si4709_IOC_DE_SET                           _IOW(Si4709_IOC_MAGIC, 32,uint8_t)       // DE-emphasis Time Constant. DE= 0, TC=50us (Europe,Japan,Australia) and DE=1, TC=75us (USA)
#define Si4709_IOC_SYS_CONFIG3_GET                  _IOR(Si4709_IOC_MAGIC, 33, sys_config3)
#define Si4709_IOC_STATUS_RSSI_GET                  _IOR(Si4709_IOC_MAGIC, 34, status_rssi_t)
#define Si4709_IOC_SYS_CONFIG2_SET                  _IOW(Si4709_IOC_MAGIC, 35, sys_config2)
#define Si4709_IOC_SYS_CONFIG3_SET                  _IOW(Si4709_IOC_MAGIC, 36, sys_config3)
#define Si4709_IOC_DSMUTE_ON                        _IO(Si4709_IOC_MAGIC, 37)
#define Si4709_IOC_DSMUTE_OFF                       _IO(Si4709_IOC_MAGIC, 38)
#define Si4709_IOC_RESET_RDS_DATA                   _IO(Si4709_IOC_MAGIC, 39)


    // Internally used (by chip_*()) functions:

  int intern_band     =      0;   // 0 = EU, 1 = US
  extern int curr_freq_val;
  extern int curr_freq_inc;
  extern int curr_freq_lo;
  extern int curr_freq_hi;

  int band_set (int low , int high, int band) {                 // ? Do we need to stop/restart RDS power in reg 0x00 ? Or rbds_set to flush ?
    logd ("band_set low: %3.3d  high: %3.3d  band: %3.3d", low, high, band);
    int ssl_band = 0;
    if (low < 87500)
      //ssl_band = BAND_76000_108000_kHz;
      ssl_band = BAND_76000_90000_kHz;
    else
      ssl_band = BAND_87500_108000_kHz;
    int ret = ioctl (dev_hndl, Si4709_IOC_BAND_SET, & ssl_band);
    if (ret < 0) {
      loge ("band_set IOCTL Si4709_BAND_SET error: %d %d", ret, errno);
      return (-1);
    }
    logd ("band_set IOCTL Si4709_BAND_SET success");
    return (0);
  }
  int freq_inc_set (int inc) {
    logd ("freq_inc_set: %3.3d", inc);
    inc /= 10;
    int ret = ioctl (dev_hndl, Si4709_IOC_CHAN_SPACING_SET, & inc);
    if (ret < 0) {
      loge ("freq_inc_set IOCTL Si4709_IOC_CHAN_SPACING_SET error: %d %d", ret, errno);
      return (-1);
    }
    logd ("freq_inc_set IOCTL Si4709_IOC_CHAN_SPACING_SET success");
    return (0);
  }
  int emph75_set (int emph75) {
    int sil_emph = 0;  // actually only u8
    logd ("emph75_set: %3.3d", emph75);
    if (emph75)
      sil_emph = DE_TIME_CONSTANT_75;
    else
      sil_emph = DE_TIME_CONSTANT_50;
    int ret = ioctl (dev_hndl, Si4709_IOC_DE_SET, & sil_emph);
    if (ret < 0) {
      loge ("emph75_set IOCTL Si4709_IOC_DE_SET error: %d %d", ret, errno);
      return (-1);
    }
    logd ("emph75_set IOCTL Si4709_IOC_DE_SET success");
    return (0);
  }
  int rbds_set (int rbds) {
    logd ("rbds_set: %3.3d",rbds);
    return (0);
  }
  int band_setup () {
    if (intern_band)
      curr_freq_inc = 200;
    else
      curr_freq_inc = 100;
    band_set (curr_freq_lo, curr_freq_hi, intern_band);
    freq_inc_set (curr_freq_inc);
    emph75_set (intern_band);
    rbds_set (intern_band);
  }

    // chip_*() functions used by plug.c

  int sys_run (char * cmd) {
    int ret = system (cmd);
    logd ("sys_run ret: %d  cmd: \"%s\"", ret, cmd);
    return (ret);
  }

  int chip_imp_api_on (int freq_lo, int freq_hi, int freq_inc) {
    logd ("chip_imp_api_on freq_lo: %d  freq_hi: %d  freq_inc: %d", freq_lo, freq_hi, freq_inc);
    curr_freq_lo = freq_lo;
    curr_freq_hi = freq_hi;
    curr_freq_inc= freq_inc;

    if (file_get ("/system/lib/modules/Si4709_driver.ko"))
      sys_run ("insmod /system/lib/modules/Si4709_driver.ko    >/dev/null 2>/dev/null ; modprobe Si4709_driver >/dev/null 2>/dev/null");
    else if (file_get ("/lib/modules/Si4709_driver.ko"))
      sys_run ("insmod        /lib/modules/Si4709_driver.ko    >/dev/null 2>/dev/null ; modprobe Si4709_driver >/dev/null 2>/dev/null");
    else if (file_get ("/system/lib/modules/radio-si4709-i2c.ko"))
      sys_run ("insmod /system/lib/modules/radio-si4709-i2c.ko >/dev/null 2>/dev/null ; modprobe radio-si4709-i2c >/dev/null 2>/dev/null");
    else if (file_get ("/lib/modules/radio-si4709-i2c.ko"))
      sys_run ("insmod        /lib/modules/radio-si4709-i2c.ko >/dev/null 2>/dev/null ; modprobe radio-si4709-i2c >/dev/null 2>/dev/null");
    else
      sys_run ("modprobe Si4709_driver >/dev/null 2>/dev/null ; modprobe radio_si4709_i2c >/dev/null 2>/dev/null");

    dev_hndl = open ("/dev/fmradio", O_RDONLY);
    if (dev_hndl < 0) {
      logd ("chip_imp_api_on error opening samsung /dev/fmradio: %3.3d", errno);
      dev_hndl = open ("/dev/radio0", O_RDONLY);
      if (dev_hndl < 0) {
        loge ("chip_imp_api_on error opening samsung /dev/fmradio or /dev/radio0: %3.3d", errno);
        return (-1);
      }
    }
    logd ("chip_imp_api_on samsung /dev/fmradio or /dev/radio0: %3.3d", dev_hndl);

    return (0);
  }

  int chip_imp_api_off () {
    if (dev_hndl >= 0) {
      close (dev_hndl);
    }
    return (0);
  }


    // Internal functions:

    // Chip API:
  int chip_imp_pwr_on (int pwr_rds) {
    int ret = 0;
    logd ("chip_imp_pwr_on");

    ret = ioctl (dev_hndl, Si4709_IOC_POWERUP);
    if (ret < 0) {
      loge ("chip_imp_pwr_on IOCTL Si4709_IOC_POWERUP error: %d %d", ret, errno);
      return (-1);
    }

    logd ("chip_imp_pwr_on IOCTL Si4709_IOC_POWERUP success");
    //chip_info_log ();
    //chip_imp_mute_set (1);                                                       // Mute for now

ms_sleep (101); // !!!! NEED !!!! ??
ms_sleep (101);

    if (pwr_rds) {
      ret = ioctl (dev_hndl, Si4709_IOC_RDS_ENABLE);
      if (ret < 0)
        loge ("sl_chip_imp_pwr_on IOCTL Si4709_IOC_RDS_ENABLE error: %d %d", ret, errno);
      else
        logd ("sl_chip_imp_pwr_on IOCTL Si4709_IOC_RDS_ENABLE success");
    }
    else {
      ret = ioctl (dev_hndl, Si4709_IOC_RDS_DISABLE);
      if (ret < 0)
        loge ("sl_chip_imp_pwr_on IOCTL Si4709_IOC_RDS_DISABLE error: %d %d", ret, errno);
      else
        logd ("sl_chip_imp_pwr_on IOCTL Si4709_IOC_RDS_DISABLE success");
    }
    ms_sleep (101);
    ms_sleep (101);
    band_setup ();

//chip_imp_vol_set (65535); Test volext; doesn't work on stock 4.3

    logd ("chip_imp_pwr_on done");
    return (0);
  }

  int chip_imp_pwr_off (int pwr_rds) {
    int ret = 0;
    logd ("chip_imp_pwr_off");
    if (pwr_rds) {
//      ms_sleep (101);
      ret = ioctl (dev_hndl, Si4709_IOC_RDS_DISABLE);
      if (ret < 0) {
        loge ("chip_imp_pwr_off IOCTL Si4709_IOC_RDS_DISABLE error: %d %d", ret, errno);
     }
      else {
        logd ("chip_imp_pwr_off IOCTL Si4709_IOC_RDS_DISABLE success");
      }
      ms_sleep (101);
    }
    chip_imp_mute_set (1);                                                         // Mute
    ret = ioctl (dev_hndl, Si4709_IOC_POWERDOWN);
    if (ret < 0) {
     loge ("chip_imp_pwr_off IOCTL Si4709_IOC_POWERDOWN error: %d %d", ret, errno);
      return (-1);
    }
    logd ("chip_imp_pwr_off IOCTL Si4709_IOC_POWERDOWN success");
    return (0);
  }

    // Set:
  int chip_imp_freq_set (int freq) {        // 10 KHz resolution    76 MHz = 7600, 108 MHz = 10800
    logd ("chip_imp_freq_set: %3.3d", freq);
    freq = freq / 10 ;
    int ret = ioctl (dev_hndl, Si4709_IOC_CHAN_SELECT, & freq);
    if (ret < 0) {
      loge ("chip_imp_freq_set IOCTL Si4709_IOC_CHAN_SELECT error: %d %d", ret, errno);
      return (-1);
    }
    curr_freq_val = freq;
    logd ("chip_imp_freq_set IOCTL Si4709_IOC_CHAN_SELECT success");
    return (0);
  }

  int chip_imp_mute_set (int mute) {
    int ret = 0;
    logd ("chip_imp_mute_set: %3.3d", mute);
    if (mute) {
      ret = ioctl (dev_hndl, Si4709_IOC_MUTE_ON);
    }
    else {
      ret = ioctl (dev_hndl, Si4709_IOC_MUTE_OFF);
    }
    if (ret < 0) {
      loge ("chip_imp_mute_set IOCTL Si4709_IOC_MUTE error: %d %d", ret, errno);
      return (-1);
    }
    logd ("chip_imp_mute_set IOCTL Si4709_IOC_MUTE success");
    return (0);
  }

  int chip_imp_stro_set (int stereo) {                                        //
    int ret = 0;
    logd ("chip_imp_stro_set: %3.3d", stereo);
    if (stereo)
      ret = ioctl (dev_hndl, Si4709_IOC_STEREO_SET);
    else
      ret = ioctl (dev_hndl, Si4709_IOC_MONO_SET);
    if (ret < 0) {
      loge ("chip_imp_stro_set IOCTL Si4709_MOST_SET error: %d %d", ret, errno);
      return (-1);
    }
    logd ("chip_imp_stro_set IOCTL Si4709_MOST_SET success");
    return (0);
  }

  int vol_ext = 0;                                                     // 0 = Automatic
  int chip_imp_vol_set (int vol) {
    int ret = 0;
    if (vol_ext == 2) {                                                // If enabled
      ret = ioctl (dev_hndl, Si4709_IOC_VOLEXT_ENB);
    }
    else if (vol_ext == 1) {                                           // If disabled
      ret = ioctl (dev_hndl, Si4709_IOC_VOLEXT_DISB);
    }
    if (ret < 0) {
      loge ("chip_imp_vol_set IOCTL Si4709_IOC_VOLEXT error: %d %d  vol_ext: %d", ret, errno, vol_ext);
//      return (-1);
    }
    if (vol_ext >= 1)
      logd ("chip_imp_vol_set IOCTL Si4709_IOC_VOLEXT success vol_ext: %3.3d", vol_ext);
    uint8_t vol_reg = vol / 4096;                                         // vol_reg = 0 - 15 from vol = 0 - 65535
    if (vol && ! vol_reg)
      vol_reg = 1;
    if (vol_reg > 15)
      vol_reg = 15;
    logd ("chip_imp_vol_set: %3.3d  %3.3d", vol, vol_reg);
    ret = ioctl (dev_hndl, Si4709_IOC_VOLUME_SET, & vol_reg);
    if (ret < 0)
      loge ("chip_imp_vol_set IOCTL Si4709_VOLUME_SET error: %d %d", ret, errno);
    else
      logd ("chip_imp_vol_set IOCTL Si4709_VOLUME_SET success");
    return (0);
  }

    // Get:
  int chip_imp_freq_get () {
    int freq = 88500;
    int ret = ioctl (dev_hndl, Si4709_IOC_CHAN_GET, & freq);
    if (ret < 0) {
      loge ("chip_imp_freq_get IOCTL Si4709_IOC_CHAN_GET error: %d %d", ret, errno);
      return (-1);
    }
    freq *= 10;
    curr_freq_val = freq;
    if (extra_log)
      logd ("chip_imp_freq_get IOCTL Si4709_IOC_CHAN_GET success: %3.3d", freq);
    return (freq);
  }

  int chip_imp_rssi_get () {
    rssi_snr_t rssi_snr = {0};
    int ret = ioctl (dev_hndl, Si4709_IOC_CUR_RSSI_GET, & rssi_snr);
    if (ret < 0) {
      loge ("chip_imp_rssi_get IOCTL Si4709_IOC_CUR_RSSI_GET error: %d %d", ret, errno);
      return (-1);
    }
    if (extra_log)
      logd ("chip_imp_rssi_get IOCTL Si4709_IOC_CUR_RSSI_GET success: %3.3d %3.3d %3.3d",rssi_snr.curr_rssi,rssi_snr.curr_rssi_th,rssi_snr.curr_snr);
    return (rssi_snr.curr_rssi);
  }

  int sls_status_chip_imp_stro_get_cnt = 0;
  int chip_imp_stro_get () {
    status_rssi_t sr = {0};
    int ret = ioctl (dev_hndl, Si4709_IOC_STATUS_RSSI_GET, & sr);
    if (ret < 0) {
      loge ("chip_imp_stro_get IOCTL                    Si4709_IOC_STATUS_RSSI_GET error: %d %d  rds_ready: %3.3d  rds_synced: %3.3d  seek_tune_complete: %3.3d  seekfail_bandlimit: %3.3d\
          afc_railed: %3.3d  block_error_a: %3.3d stereo: %3.3d  rssi: %3.3d", ret, errno, sr.rdsr, sr.rdss, sr.stc, sr.sfbl, sr.afcrl, sr.blera, sr.st, sr.rssi);
     return (0);
    }
    if (sls_status_chip_imp_stro_get_cnt ++ % 1200 == 0)                           // Every 2 minutes
      logd ("chip_imp_stro_get                          Si4709_IOC_STATUS_RSSI_GET success: %3.3d  rds_ready: %3.3d  rds_synced: %3.3d  seek_tune_complete: %3.3d  seekfail_bandlimit: %3.3d\
          afc_railed: %3.3d  block_err_a: %3.3d stereo: %3.3d  rssi: %3.3d", ret, sr.rdsr, sr.rdss, sr.stc, sr.sfbl, sr.afcrl, sr.blera, sr.st, sr.rssi);
    if (sr.st)                                                            // If stereo detected
      return (1);
    return (0);
  }


    // RDS:

//#define  SUPPORT_RDS
#ifdef  SUPPORT_RDS
  #include "ssl_rds.c"
#else
  int rds_events_process (unsigned char * rds_grpd) {return (-1);}
#endif

  //int sls_status_chip_imp_events_process_cnt = 0;
  int chip_imp_events_process (unsigned char * rds_grpd) {
    int ret = 0;

    ret = rds_events_process (rds_grpd);
    return (ret);
  }

    // Seek:

  int chip_imp_seek_start (int dir) {
    int ret = 0;
    logd ("chip_imp_seek_start dir: %3.3d", dir);
    //chip_imp_seek_stop ();
    int freq = 0, ctr = 0;
    int this_freq = 88500;
    for (ctr = 0; ctr < 2; ctr ++) {                                      // Do up to 2 seeks, 1st to freq or end of band, 2nd from end of band if at end of band
      if (ctr > 0) {
        if (dir)
          this_freq = curr_freq_lo;
        else
          this_freq = curr_freq_hi;
        chip_imp_freq_set (this_freq);
      }
      if (dir)
        ret = ioctl (dev_hndl, Si4709_IOC_SEEK_UP, & freq);
      else
        ret = ioctl (dev_hndl, Si4709_IOC_SEEK_DOWN, & freq);
      if (ret < 0) {
        loge ("chip_imp_seek_start IOCTL Si4709_SEEK error: %d %d", ret, errno);
        return (-1);
      }
      logd ("chip_imp_seek_start IOCTL Si4709_SEEK success freq: %3.3d", freq);
      if (freq)
        break;
    }
    this_freq = freq * 10;
    return (this_freq);
  }

  int chip_imp_seek_stop () {
    logd ("chip_imp_seek_stop");
   int ret = ioctl (dev_hndl, Si4709_IOC_SEEK_CANCEL);
    if (ret < 0) {
      loge ("chip_imp_seek_stop IOCTL Si4709_SEEK_CANCEL error: %d %d", ret, errno);
      return (-1);
    }
    logd ("chip_imp_seek_stop IOCTL Si4709_SEEK_CANCEL success");
    return (0);
  }

  int chip_imp_extra_cmd (const char * command, char ** parameters) {
    if (command == NULL)
      return (-1);

    int full_val = atoi (command);              // full_val = -2^31 ... +2^31 - 1       = Over 9 digits
    int ctrl = (full_val / 1000) - 200;         // ctrl = hi 3 digits - 200     (control to write to)
    int val  = (full_val % 1000);               // val  = lo 3 digits           (value to write)

      if (val == 990) {
        intern_band = 0;  // EU
        band_setup ();
        logd ("chip_imp_extra_cmd command (EU Band set): %s  full_val: %d  ctrl: %d  val: %d", command, full_val, ctrl, val);
        return (0);
      }
      else if (val == 991) {
        intern_band = 1;  // US
        band_setup ();
        logd ("chip_imp_extra_cmd command (US Band set): %s  full_val: %d  ctrl: %d  val: %d", command, full_val, ctrl, val);
        return (0);
      }
      else {
        loge ("chip_imp_extra_cmd command (?): %s  full_val: %d  ctrl: %d  val: %d", command, full_val, ctrl, val);
        return (-1);
      }

    return (0);
  }


//  #include "plug.c"

