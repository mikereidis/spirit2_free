
    // Spirit2 Tuner Plugin for "Samsung Silicon Labs" API:

    // See /home/m/spirit/kernel/gt-i9305/drivers/samsung/fm_si4709/ for GS1/GS2/Note1      (Si4709 FM tuner chip)
    // See /home/m/spirit/kernel/gt-i9305/drivers/samsung/fm_si47xx/ for     GS3/Note2      (Si4705 FM tuner chip)

  #define LOGTAG "sftnrssl"

  #include <stdio.h>
  #include <errno.h>
  #include <sys/stat.h>
  #include <fcntl.h>

  #include <sys/ioctl.h>

  #include "tnr_tnr.c"

    // Internal functions:

  int dev_hndl      =     -1;

  #define u8  uint8_t
  #define u16 uint16_t
  #define u32 uint32_t


  typedef struct {
    int power_state;
    int seek_state;
  } dev_state_t;

  typedef struct {
    u8 curr_rssi;
    u8 curr_rssi_th;
    u8 curr_snr;
  }  rssi_snr_t;

  typedef struct {
    uint8_t part_number;
    uint16_t manufact_number;
  } device_id_t;

  typedef struct {
    uint8_t  chip_version;
    uint8_t     device;
    uint8_t  firmware_version;
  } chip_id_t;

  struct sys_config2 {
    uint16_t rssi_th;
    uint8_t fm_band;
    uint8_t fm_chan_spac;
    uint8_t fm_vol;
  };

  struct sys_config3 {
    uint8_t smmute;
    uint8_t smutea;
    uint8_t volext;
    uint8_t sksnr;
    uint8_t skcnt;
  };

  typedef struct {
    uint8_t rdsr;
    uint8_t stc;
    uint8_t sfbl;
    uint8_t afcrl;
    uint8_t rdss;
    uint8_t blera;
    uint8_t st;
    uint16_t rssi;
  } status_rssi_t;

  struct power_config {
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

  typedef struct {
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
  
  // dev settings
  // band
  #define BAND_87500_108000_kHz   1
  #define BAND_76000_108000_kHz   2
  #define BAND_76000_90000_kHz    3
  
  // channel spacing
  #define CHAN_SPACING_200_kHz   20        // US
  #define CHAN_SPACING_100_kHz   10        // Europe,Japan
  #define CHAN_SPACING_50_kHz    5
  
  // DE-emphasis Time Constant
  #define DE_TIME_CONSTANT_50   1          // Europe,Japan,Australia
  #define DE_TIME_CONSTANT_75   0          // US
  
  
  // ****************ioctl's******************
  #define Si4709_IOC_MAGIC  0xFA                                          // magic #
  #define Si4709_IOC_NR_MAX 40                                            // max seq no
  
  // commands
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
  #define Si4709_IOC_SEEK_CANCEL                      _IO(Si4709_IOC_MAGIC, 26)                 // VNVS:START 13-OCT'09---- Added ioctl's for reading the device-id,chip-id,power configuration, system configuration2 registers
  #define Si4709_IOC_DEVICE_ID_GET                    _IOR(Si4709_IOC_MAGIC, 27,device_id_t)
  #define Si4709_IOC_CHIP_ID_GET                      _IOR(Si4709_IOC_MAGIC, 28,chip_id_t)
  #define Si4709_IOC_SYS_CONFIG2_GET                  _IOR(Si4709_IOC_MAGIC, 29,sys_config2)
  #define Si4709_IOC_POWER_CONFIG_GET                 _IOR(Si4709_IOC_MAGIC, 30,power_config)   // !! #define Si47xx_IOC_POWER_CONFIG_GET   _IO(Si47xx_IOC_MAGIC, 30)
  #define Si4709_IOC_AFCRL_GET                        _IOR(Si4709_IOC_MAGIC, 31,u8)             // AFCRL bit, to check for a valid channel
  #define Si4709_IOC_DE_SET                           _IOW(Si4709_IOC_MAGIC, 32,uint8_t)        // DE-emphasis Time Constant. DE= 0, TC=50us (Europe,Japan,Australia) and DE=1, TC=75us (USA)
  #define Si4709_IOC_SYS_CONFIG3_GET                  _IOR(Si4709_IOC_MAGIC, 33, sys_config3)
  #define Si4709_IOC_STATUS_RSSI_GET                  _IOR(Si4709_IOC_MAGIC, 34, status_rssi_t)
  #define Si4709_IOC_SYS_CONFIG2_SET                  _IOW(Si4709_IOC_MAGIC, 35, sys_config2)
  #define Si4709_IOC_SYS_CONFIG3_SET                  _IOW(Si4709_IOC_MAGIC, 36, sys_config3)
  #define Si4709_IOC_DSMUTE_ON                        _IO(Si4709_IOC_MAGIC, 37)
  #define Si4709_IOC_DSMUTE_OFF                       _IO(Si4709_IOC_MAGIC, 38)
  #define Si4709_IOC_RESET_RDS_DATA                   _IO(Si4709_IOC_MAGIC, 39)
  #define Si4709_IOC_SEEK_FULL                        _IOR(Si4709_IOC_MAGIC, 40, u32)           // !! New

  char * req_get (int req) {
    switch (req) {
      case Si4709_IOC_BAND_SET:         return ("Si4709_IOC_BAND_SET");
      case Si4709_IOC_CHAN_GET:         return ("Si4709_IOC_CHAN_GET");
      case Si4709_IOC_CHAN_SELECT:      return ("Si4709_IOC_CHAN_SELECT");
      case Si4709_IOC_CHAN_SPACING_SET: return ("Si4709_IOC_CHAN_SPACING_SET");
      case Si4709_IOC_CUR_RSSI_GET:     return ("Si4709_IOC_CUR_RSSI_GET");
      case Si4709_IOC_DE_SET:           return ("Si4709_IOC_DE_SET");
      case Si4709_IOC_DSMUTE_OFF:       return ("Si4709_IOC_DSMUTE_OFF");
      case Si4709_IOC_DSMUTE_ON:        return ("Si4709_IOC_DSMUTE_ON");
      case Si4709_IOC_MONO_SET:         return ("Si4709_IOC_MONO_SET");
      case Si4709_IOC_MUTE_OFF:         return ("Si4709_IOC_MUTE_OFF");
      case Si4709_IOC_MUTE_ON:          return ("Si4709_IOC_MUTE_ON");
      case Si4709_IOC_POWERDOWN:        return ("Si4709_IOC_POWERDOWN");
      case Si4709_IOC_POWERUP:          return ("Si4709_IOC_POWERUP");
      case Si4709_IOC_RDS_DATA_GET:     return ("Si4709_IOC_RDS_DATA_GET");
      case Si4709_IOC_RDS_DISABLE:      return ("Si4709_IOC_RDS_DISABLE");
      case Si4709_IOC_RDS_ENABLE:       return ("Si4709_IOC_RDS_ENABLE");
      case Si4709_IOC_SEEK_CANCEL:      return ("Si4709_IOC_SEEK_CANCEL");
      case Si4709_IOC_SEEK_DOWN:        return ("Si4709_IOC_SEEK_DOWN");
      case Si4709_IOC_SEEK_UP:          return ("Si4709_IOC_SEEK_UP");
      case Si4709_IOC_STATUS_RSSI_GET:  return ("Si4709_IOC_STATUS_RSSI_GET");
      case Si4709_IOC_STEREO_SET:       return ("Si4709_IOC_STEREO_SET");
      case Si4709_IOC_VOLEXT_DISB:      return ("Si4709_IOC_VOLEXT_DISB");
      case Si4709_IOC_VOLEXT_ENB:       return ("Si4709_IOC_VOLEXT_ENB");
      case Si4709_IOC_VOLUME_SET:       return ("Si4709_IOC_VOLUME_SET");
    }
    return ("Unknown !");
  }

  int ssl_ioctl_par (int fd, int req, void * par) {                     // (int d, int request, ...);
    int ret = ioctl (fd, req, par);
    if (ena_log_chip_access)
      logd ("ssl_ioctl_par req: %s", req_get (req));
    return (ret);
  }

  int ssl_ioctl (int fd, int req) {                                     // (int d, int request, ...);
    int ret = ioctl (fd, req);
    if (ena_log_chip_access)
      logd ("ssl_ioctl req: %s", req_get (req));
    return (ret);
  }

  int band_set (int low , int high, int band) {                 // ? Do we need to stop/restart RDS power in reg 0x00 ? Or rbds_set to flush ?
    logd ("band_set low: %d  high: %d  band: %d", low, high, band);
    int ssl_band = 0;
    if (low < 87500 || band == 2)
      ssl_band = BAND_76000_108000_kHz;
      //ssl_band = BAND_76000_90000_kHz;
    else
      ssl_band = BAND_87500_108000_kHz;
    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_BAND_SET, & ssl_band);
    if (ret < 0) {
      loge ("band_set ioctl Si4709_BAND_SET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      return (-1);
    }
    logd ("band_set ioctl Si4709_BAND_SET success");
    return (band);
  }

  int freq_inc_set (int inc) {
    logd ("freq_inc_set: %d", inc);
    inc /= 10;
    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_CHAN_SPACING_SET, & inc);
    if (ret < 0) {
      loge ("freq_inc_set ioctl Si4709_IOC_CHAN_SPACING_SET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      return (-1);
    }
    logd ("freq_inc_set ioctl Si4709_IOC_CHAN_SPACING_SET success");
    return (inc);
  }

  int emph75_set (int band) {
    int sil_emph = 0;
    logd ("emph75_set band: %d", band);
    if (band == 1)                                                      // If US...
      sil_emph = DE_TIME_CONSTANT_75;
    else
      sil_emph = DE_TIME_CONSTANT_50;
    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_DE_SET, & sil_emph);
    if (ret < 0) {
      loge ("emph75_set ioctl Si4709_IOC_DE_SET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      return (-1);
    }
    logd ("emph75_set ioctl Si4709_IOC_DE_SET success");
    return (band);
  }

  int rbds_set (int band) {
    logd ("rbds_set band: %d", band);
    return (band);
  }

  int pwr_off () {
    int ret = 0;
    logd ("pwr_off");
    if (curr_rds_state) {
      errno = 0;
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_RDS_DISABLE);
      if (ret < 0)
        loge ("pwr_off ioctl Si4709_IOC_RDS_DISABLE ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      else
        logd ("pwr_off ioctl Si4709_IOC_RDS_DISABLE success");
    }

    chip_imp_mute_sg (1);                                               // Mute

    errno = 0;
    ret = ssl_ioctl (dev_hndl, Si4709_IOC_POWERDOWN);
    if (ret < 0)
     loge ("pwr_off ioctl Si4709_IOC_POWERDOWN ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else
      logd ("pwr_off ioctl Si4709_IOC_POWERDOWN success");
    curr_state = 0;
    return (curr_state);
  }

  int vol_get () {
    int vol = curr_vol;
    logd ("chip_imp_vol_get: %d", vol);
    return (vol);
  }

  int freq_get () {//freq_sg
    int ssl_freq = 8850;
    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_CHAN_GET, & ssl_freq);
    if (ret < 0) {
      loge ("freq_get ioctl Si4709_IOC_CHAN_GET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      return (-1);
    }
    curr_freq_int = ssl_freq * 10;
    if (ena_log_tnr_extra)
      logd ("freq_get ioctl Si4709_IOC_CHAN_GET success: %d", curr_freq_int);
    return (curr_freq_int);
  }

  int sls_status_chip_imp_pilot_sg_cnt = 0;

  //int sls_status_chip_imp_event_sg_cnt = 0;

  int seek_stop () {
    logd ("seek_stop");
    errno = 0;
    int ret = ssl_ioctl (dev_hndl, Si4709_IOC_SEEK_CANCEL);
    if (ret < 0)
      loge ("seek_stop ioctl Si4709_SEEK_CANCEL ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else
      logd ("seek_stop ioctl Si4709_SEEK_CANCEL success");

    curr_seek_state = 0;
    return (curr_seek_state);
  }


    // Chip API:

//#define  SUPPORT_RDS
#ifdef  SUPPORT_RDS
  #include "rds_ssl.c"
#else
  int rds_poll (unsigned char * rds_grpd) {return (EVT_GET_NONE);}
#endif

  int chip_imp_event_sg (unsigned char * rds_grpd) {                    // Polling function called every event_sg_ms milliseconds. Not used remotely but could be in future.
    int ret = 0;
    ret = rds_poll (rds_grpd);
    return (ret);
  }

  int chip_imp_api_mode_sg (int api_mode) {
    if (api_mode == GET)
      return (curr_api_mode);
    curr_api_mode = api_mode;
    logd ("chip_imp_api_mode_sg curr_api_mode: %d", curr_api_mode);
    return (curr_api_mode);
  }

/* Kernel symbols GS2/Note1:

lsmod
Si4709_driver 27844 1 - Live 0x00000000

GS3 CM12    -
NO2 CM12    -
GS2 CM12    Si4709_driver 27844 1 - Live 0x00000000
NO1 CM11    Si4709_driver 25134 1 - Live 0x00000000
GS1 CM11    -

a "ls -R sys|grep -i -e fm -e sil -e si4 -e 470"
a "find sys|grep -i -e fm -e sil -e si4 -e 470"

GS3 / NO2 CM12:
sys/devices/virtual/misc/fmradio
sys/devices/virtual/misc/fmradio/uevent
sys/devices/virtual/misc/fmradio/dev
sys/devices/virtual/misc/fmradio/subsystem
sys/devices/virtual/misc/fmradio/power
sys/devices/virtual/misc/fmradio/power/runtime_status
sys/devices/virtual/misc/fmradio/power/control
sys/devices/virtual/misc/fmradio/power/runtime_suspended_time
sys/devices/virtual/misc/fmradio/power/runtime_active_time
sys/devices/virtual/misc/fmradio/power/autosuspend_delay_ms

sys/bus/i2c/drivers/Si47xx
sys/bus/i2c/drivers/Si47xx/19-0011
sys/bus/i2c/drivers/Si47xx/uevent
sys/bus/i2c/drivers/Si47xx/unbind
sys/bus/i2c/drivers/Si47xx/bind

    Link    sys/class/misc/fmradio

GS3 CM12:
sys/kernel/debug/asoc/Midas_WM1811/wm8994-codec/dapm/FM In
NO2 CM12:
sys/kernel/debug/asoc/T0_WM1811/wm8994-codec/dapm/FM In


GS2 CM12 / NO2 CM11: Above except: (and no explicitly "FM" named control under sys/kernel/debug/asoc)
sys/bus/i2c/drivers/Si4709
sys/bus/i2c/drivers/Si4709/16-0010
sys/bus/i2c/drivers/Si4709/module
sys/bus/i2c/drivers/Si4709/uevent
sys/bus/i2c/drivers/Si4709/unbind
sys/bus/i2c/drivers/Si4709/bind

sys/module/Si4709_driver
sys/module/Si4709_driver/holders
sys/module/Si4709_driver/initstate
sys/module/Si4709_driver/refcnt
sys/module/Si4709_driver/sections
sys/module/Si4709_driver/sections/.text
sys/module/Si4709_driver/sections/.exit.text
sys/module/Si4709_driver/sections/.init.text
sys/module/Si4709_driver/sections/.rodata
sys/module/Si4709_driver/sections/.rodata.str1.4
sys/module/Si4709_driver/sections/.data
sys/module/Si4709_driver/sections/.gnu.linkonce.this_module
sys/module/Si4709_driver/sections/.note.gnu.build-id
sys/module/Si4709_driver/sections/.bss
sys/module/Si4709_driver/sections/.symtab
sys/module/Si4709_driver/sections/.strtab
sys/module/Si4709_driver/notes
sys/module/Si4709_driver/notes/.note.gnu.build-id
sys/module/Si4709_driver/drivers
sys/module/Si4709_driver/drivers/i2c:Si4709

GS1 CM11:
sys/devices/virtual/misc/voodoo_sound/fm_radio_headset_restore_bass
sys/devices/virtual/misc/voodoo_sound/fm_radio_headset_restore_highs
sys/devices/virtual/misc/voodoo_sound/fm_radio_headset_normalize_gain

sys/bus/i2c/drivers/Si4709
sys/bus/i2c/drivers/Si4709/8-0010
sys/bus/i2c/drivers/Si4709/uevent
sys/bus/i2c/drivers/Si4709/unbind
sys/bus/i2c/drivers/Si4709/bind


sys/bus/i2c/drivers/Si47xx

sys/bus/i2c/drivers/Si4709
    sys/module/Si4709_driver

*/

  char * gs3_ssl = "/sys/bus/i2c/drivers/Si47xx";
  char * gs2_ssl = "/sys/bus/i2c/drivers/Si4709";
  char * gs1_ssl = "/sys/module/Si4709_driver";

  char * gs2_mod1 = "/sys/bus/platform/drivers/iris_fm/";
  char * gs2_mod2 = "/sys/bus/platform/drivers/iris_fm/uevent";
  char * gs2_mod3 = "/sys/module/radio_iris/";
  char * gs2_mod4 = "/sys/module/radio_iris/parameters/sig_blend";

  int api_ready_get () {

    if (file_get (gs3_ssl))
      logd ("YES: gs3_ssl: %s", gs3_ssl);
    else
      logd ("NO:  gs3_ssl: %s", gs3_ssl);

    if (file_get (gs2_ssl))
      logd ("YES: gs2_ssl: %s", gs2_ssl);
    else
      logd ("NO:  gs2_ssl: %s", gs2_ssl);

    if (file_get (gs1_ssl))
      logd ("YES: gs1_ssl: %s", gs1_ssl);
    else
      logd ("NO:  gs1_ssl: %s", gs1_ssl);


    if (file_get (gs2_mod1))
      logd ("YES: gs2_mod1: %s", gs2_mod1);
    else
      logd ("NO:  gs2_mod1: %s", gs2_mod1);

    if (file_get (gs2_mod2))
      logd ("YES: gs2_mod2: %s", gs2_mod2);
    else
      logd ("NO:  gs2_mod2: %s", gs2_mod2);

    if (file_get (gs2_mod3))
      logd ("YES: gs2_mod3: %s", gs2_mod3);
    else
      logd ("NO:  gs2_mod3: %s", gs2_mod3);

    if (file_get (gs2_mod4))
      logd ("YES: gs2_mod4: %s", gs2_mod4);
    else
      logd ("NO:  gs2_mod4: %s", gs2_mod4);


    return (1);
  }

  int chip_imp_api_state_sg (int state) {
    logd ("chip_imp_api_state_sg state: %d", state);
    if (state == GET)
      return (curr_api_state);

    if (state == 0) {
      if (dev_hndl >= 0) {
        close (dev_hndl);
      }
      curr_api_state = 0;
      return (curr_api_state);
    }

    if (file_get ("/system/lib/modules/Si4709_driver.ko"))
      util_insmod ("/system/lib/modules/Si4709_driver.ko");

    errno = 0;
    dev_hndl = open ("/dev/fmradio", O_RDONLY);
    if (dev_hndl < 0) {
      logd ("chip_imp_api_state_sg error opening samsung /dev/fmradio errno: %d (%s)", errno, strerror (errno));
      errno = 0;
      dev_hndl = open ("/dev/radio0", O_RDONLY);
      if (dev_hndl < 0) {
        loge ("chip_imp_api_state_sg error opening samsung /dev/radio0 errno: %d (%s)", errno, strerror (errno));
        curr_api_state = 0;
        return (curr_api_state);
      }
    }
    logd ("chip_imp_api_state_sg samsung /dev/fmradio or /dev/radio0: %d", dev_hndl);

    curr_api_state = 1;
    return (curr_api_state);
  }

  int chip_imp_mode_sg (int mode) {
    if (mode == GET)
      return (curr_mode);
    curr_mode = mode;
    logd ("chip_imp_mode_sg curr_mode: %d", curr_mode);
    return (curr_mode);
  }

  int chip_imp_state_sg (int state) {
    if (state == GET)
      return (curr_state);

    logd ("chip_imp_state_sg state: %d", state);
    if (state == 0)
      return (pwr_off ());

    int ret = 0;
    errno = 0;
    ret = ssl_ioctl (dev_hndl, Si4709_IOC_POWERUP);
    if (ret < 0) {
        loge ("chip_imp_state_sg ioctl Si4709_IOC_POWERUP ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
        curr_state = 0;
        loge ("chip_imp_state_sg curr_state: %d", curr_state);
        return (curr_state);
    }

    logd ("chip_imp_state_sg ioctl Si4709_IOC_POWERUP success");
    //chip_info_log ();

    chip_imp_mute_sg (1);                                               // Mute for now

    if (curr_rds_state) {
      errno = 0;
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_RDS_ENABLE);
      if (ret < 0)
        loge ("sl_chip_imp_state_sg ioctl Si4709_IOC_RDS_ENABLE ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      else
        logd ("sl_chip_imp_state_sg ioctl Si4709_IOC_RDS_ENABLE success");
    }
    else {
      errno = 0;
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_RDS_DISABLE);
      if (ret < 0)
        loge ("sl_chip_imp_state_sg ioctl Si4709_IOC_RDS_DISABLE ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      else
        logd ("sl_chip_imp_state_sg ioctl Si4709_IOC_RDS_DISABLE success");
    }

    //chip_imp_band_sg (0);

    if (api_ready_get () == 0) {
      curr_state = 0;
      loge ("API Not ready");
      return (curr_state);
    }

    chip_imp_vol_sg (65535);

    curr_state = 1;
    logd ("chip_imp_state_sg curr_state: %d", curr_state);
    return (curr_state);
  }

  int chip_imp_antenna_sg (int antenna) {
    if (antenna == GET)
      return (curr_antenna);
/*
    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_ANTENNA, antenna) < 0)     // 0 = common external, 1 = Sony Z/Z1 internal
      loge ("chip_imp_antenna_sg ANTENNA error");
    else
      logd ("chip_imp_antenna_sg ANTENNA success");
*/
    curr_antenna = antenna;
    logd ("chip_imp_antenna_sg curr_antenna: %d", curr_antenna);
    return (curr_antenna);
  }

  int chip_imp_band_sg (int band) {                                     //  0:EU    1:US    2:UU
    if (band == GET)
      return (curr_band);

    logd ("chip_imp_band_sg band: %d", band);

    curr_band = band;

    curr_freq_hi = 108000;

    curr_freq_lo = 87500;
    if (band == 2)                                                      // If Wide
      curr_freq_lo = 76000;//65000;

    curr_freq_inc = 100;
    if (band == 1)                                                      // If US
      curr_freq_inc = 200;

    band_set (curr_freq_lo, curr_freq_hi, band);

    freq_inc_set (curr_freq_inc);

    emph75_set (band);

    rbds_set (band);

    return (band);
  }

  int chip_imp_freq_sg (int freq) {        // 10 KHz resolution    76 MHz = 7600, 108 MHz = 10800
    if (freq == GET)
      return (freq_get ());

    logd ("chip_imp_freq_sg: %d", freq);
    int ssl_freq = freq / 10 ;
    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_CHAN_SELECT, & ssl_freq);
    if (ret < 0) {
      loge ("chip_imp_freq_sg ioctl Si4709_IOC_CHAN_SELECT ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      return (curr_freq_int);
    }
    curr_freq_int = freq;
    logd ("chip_imp_freq_sg ioctl Si4709_IOC_CHAN_SELECT success");
    rds_init ();
    return (curr_freq_int);
  }

  int chip_imp_vol_sg (int vol) {
    if (vol == GET)
      return (curr_vol);

    int ret = 0;

    int vol_ext = 0;                                                    // 0 = Default
    errno = 0;
    if (vol_ext == 2 || file_get ("/sdcard/spirit/ssl_volext_ena"))            // If Volume Extension enabled...       GS3: ioctl Si4709_IOC_VOLEXT error: -1 25  vol_ext: 0
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_VOLEXT_ENB);
    else if (vol_ext == 1 || file_get ("/sdcard/spirit/ssl_volext_dis"))       // Else if Volume Extension disabled...
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_VOLEXT_DISB);

    if (ret < 0)
      loge ("chip_imp_vol_sg ioctl Si4709_IOC_VOLEXT ret: %d  errno: %d (%s)  vol_ext: %d", ret, errno, strerror (errno), vol_ext);
    else if (vol_ext >= 1)
      logd ("chip_imp_vol_sg ioctl Si4709_IOC_VOLEXT success vol_ext: %d", vol_ext);

    uint8_t vol_reg = vol / 4096;                                         // vol_reg = 0 - 15 from vol = 0 - 65535
    if (vol && ! vol_reg)
      vol_reg = 1;
    if (vol_reg > 15)
      vol_reg = 15;
    logd ("chip_imp_vol_sg: %d  %d", vol, vol_reg);

    errno = 0;
    ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_VOLUME_SET, & vol_reg);
    if (ret < 0)
      loge ("chip_imp_vol_sg ioctl Si4709_VOLUME_SET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else
      logd ("chip_imp_vol_sg ioctl Si4709_VOLUME_SET success");

    curr_vol = vol;
    logd ("chip_imp_vol_sg curr_vol: %d", curr_vol);
    return (curr_vol);
  }

  int chip_imp_thresh_sg (int thresh) {
    if (thresh == GET)
      return (curr_thresh);

    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_RSSI_SEEK_TH_SET, & thresh);
    if (ret < 0)
      loge ("chip_imp_thresh_sg ioctl Si4709_RSSI_SEEK_TH_SET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else
      logd ("chip_imp_thresh_sg ioctl Si4709_RSSI_SEEK_TH_SET success");


    curr_thresh = thresh;
    logd ("chip_imp_thresh_sg curr_thresh: %d", curr_thresh);
    return (curr_thresh);
  }

  int chip_imp_mute_sg (int mute) {
    if (mute == GET)
      return (curr_mute);

    int ret = 0;
    errno = 0;
    if (mute)
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_MUTE_ON);
    else
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_MUTE_OFF);
    if (ret < 0)
      loge ("chip_imp_mute_sg ioctl Si4709_IOC_MUTE error: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else
      logd ("chip_imp_mute_sg ioctl Si4709_IOC_MUTE success");

    curr_mute = mute;
    logd ("chip_imp_mute_sg curr_mute: %d", curr_mute);
    return (curr_mute);
  }

  int chip_imp_softmute_sg (int softmute) {
    if (softmute == GET)
      return (curr_softmute);

    int ret = 0;
    errno = 0;
    if (softmute)
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_DSMUTE_ON);                 // !!!! Backwards on GS3 !!!!
    else
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_DSMUTE_OFF);
    if (ret < 0)
      loge ("chip_imp_softmute_sg ioctl Si4709_IOC_DSMUTE error: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else
      logd ("chip_imp_softmute_sg ioctl Si4709_IOC_DSMUTE success");

    curr_softmute = softmute;
    logd ("chip_imp_softmute_sg curr_softmute: %d", curr_softmute);
    return (curr_softmute);
  }

  int chip_imp_stereo_sg (int stereo) {                                 //
    if (stereo == GET)
      return (curr_stereo);

    int ret = 0;
    errno = 0;
    if (stereo)
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_STEREO_SET);
    else
      ret = ssl_ioctl (dev_hndl, Si4709_IOC_MONO_SET);
    if (ret < 0)
      loge ("chip_imp_stereo_sg ioctl Si4709_IOC_STEREO_SET/Si4709_IOC_MONO_SET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else
      logd ("chip_imp_stereo_sg ioctl Si4709_IOC_STEREO_SET/Si4709_IOC_MONO_SET success");

    curr_stereo = stereo;
    logd ("chip_imp_stereo_sg curr_stereo: %d", curr_stereo);
    return (curr_stereo);
  }

  int chip_imp_seek_state_sg (int seek_state) {
    if (seek_state == GET)
      return (curr_seek_state);

    if (seek_state == 0)
      return (seek_stop ());

    int ret = 0;
    logd ("chip_imp_seek_state_sg seek_state: %d", seek_state);
    //seek_stop ();
    int seek_up = 1;
    if (seek_state == 2)
      seek_up = 0;

    int freq = 0, ctr = 0;
    int this_freq = 88500;
    for (ctr = 0; ctr < 2; ctr ++) {                                      // Do up to 2 seeks, 1st to freq or end of band, 2nd from end of band if at end of band
      if (ctr > 0) {
        if (seek_up)
          this_freq = curr_freq_lo;
        else
          this_freq = curr_freq_hi;
        chip_imp_freq_sg (this_freq);                                   // Set new frequency
      }
      errno = 0;
      if (seek_up)
        ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_SEEK_UP, & freq);
      else
        ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_SEEK_DOWN, & freq);
      if (ret < 0)
        loge ("chip_imp_seek_state_sg ioctl Si4709_SEEK ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
      else
        logd ("chip_imp_seek_state_sg ioctl Si4709_SEEK success freq: %d", freq);
      if (freq)
        break;
    }
    this_freq = freq * 10;
    curr_freq_int = this_freq;

    curr_seek_state = 0;                                                // !!!! Still running ????      SSL only
    rds_init ();
    return (curr_freq_int);//curr_seek_state);
  }

  int chip_imp_rds_state_sg (int rds_state) {
    if (rds_state == GET)
      return (curr_rds_state);

    curr_rds_state = rds_state;
    logd ("chip_imp_rds_state_sg curr_rds_state: %d", curr_rds_state);
    return (curr_rds_state);
  }

  int chip_imp_rds_af_state_sg (int rds_af_state) {
    if (rds_af_state == GET)
      return (curr_rds_af_state);
    curr_rds_af_state = rds_af_state;
    logd ("chip_imp_rds_af_state_sg curr_rds_af_state: %d", curr_rds_af_state);
    return (curr_rds_af_state);
  }

  int chip_imp_rssi_sg (int fake_rssi) {
    rssi_snr_t rssi_snr = {0};
    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_CUR_RSSI_GET, & rssi_snr);
    if (ret < 0)
      loge ("chip_imp_rssi_sg ioctl Si4709_IOC_CUR_RSSI_GET ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
    else if (ena_log_tnr_extra)
      logd ("chip_imp_rssi_sg ioctl Si4709_IOC_CUR_RSSI_GET success: %d %d %d", rssi_snr.curr_rssi, rssi_snr.curr_rssi_th, rssi_snr.curr_snr);

    curr_rssi = rssi_snr.curr_rssi;
    return (curr_rssi);
  }

  int chip_imp_pilot_sg (int fake_pilot) {

//return (curr_pilot = 1);      // !!!! May interfere with RDS, so disable for now !!


    status_rssi_t sr = {0};
    errno = 0;
    int ret = ssl_ioctl_par (dev_hndl, Si4709_IOC_STATUS_RSSI_GET, & sr);
    if (ret < 0) {
      loge ("chip_imp_pilot_sg ioctl Si4709_IOC_STATUS_RSSI_GET ret: %d  errno: %d (%s)  rds_ready: %d  rds_synced: %d  seek_tune_complete: %d  seekfail_bandlimit: %d\
          afc_railed: %d  block_error_a: %d stereo: %d  rssi: %d", ret, errno, strerror (errno), sr.rdsr, sr.rdss, sr.stc, sr.sfbl, sr.afcrl, sr.blera, sr.st, sr.rssi);
     curr_pilot  = 0;
     return (curr_pilot);
    }
    if (sls_status_chip_imp_pilot_sg_cnt ++ % 1200 == 0)                           // Every 2 minutes
      logd ("chip_imp_pilot_sg Si4709_IOC_STATUS_RSSI_GET success: %d  rds_ready: %d  rds_synced: %d  seek_tune_complete: %d  seekfail_bandlimit: %d\
          afc_railed: %d  block_err_a: %d stereo: %d  rssi: %d", ret, sr.rdsr, sr.rdss, sr.stc, sr.sfbl, sr.afcrl, sr.blera, sr.st, sr.rssi);
    if (sr.st)                                                            // If stereo detected
      return (curr_pilot = 1);
    else
      return (curr_pilot = 0);
  }

  int chip_imp_rds_pi_sg (int rds_pi) {
    if (rds_pi == GET)
      return (curr_rds_pi);
    int ret = -1;
    //ret = rds_pi_set (rds_pi);
    curr_rds_pi = rds_pi;
    return (curr_rds_pi);
  }
  int chip_imp_rds_pt_sg (int rds_pt) {
    if (rds_pt == GET)
      return (curr_rds_pt);
    int ret = -1;
    //ret = rds_pt_set (rds_pt);
    curr_rds_pt = rds_pt;
    return (curr_rds_pt);
  }
  char * chip_imp_rds_ps_sg (char * rds_ps) {
    if (rds_ps == GETP)
      return (curr_rds_ps);
    int ret = -1;
    //ret = rds_ps_set (rds_ps);
    strlcpy (curr_rds_ps, rds_ps, sizeof (curr_rds_ps));
    return (curr_rds_ps);
  }
  char * chip_imp_rds_rt_sg (char * rds_rt) {
    if (rds_rt == GETP)
      return (curr_rds_rt);
    int ret = -1;
    //ret = rds_rt_set (rds_rt);
    strlcpy (curr_rds_rt, rds_rt, sizeof (curr_rds_rt));
    return (curr_rds_rt);
  }

  char * chip_imp_extension_sg (char * reg) {
    if (reg == GETP)
      return (curr_extension);
    int ret = -1;
    strlcpy (curr_extension, reg, sizeof (curr_extension));
    return (curr_extension);
  }

