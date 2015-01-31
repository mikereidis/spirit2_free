
  #define LOGTAG "sftnrgen"

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tnr_tnr.h"
#include "tnr_tnr.c"

    // FM Chip specific code

  int dev_hndl      =     -1;
  extern int curr_freq_val;
  extern int curr_freq_inc;
  extern int curr_freq_lo;
  extern int curr_freq_hi;

  int chip_imp_api_on (int freq_lo, int freq_hi, int freq_inc) {
    logd ("chip_imp_api_on freq_lo: %d  freq_hi: %d  freq_inc: %d", freq_lo, freq_hi, freq_inc);
    curr_freq_lo = freq_lo;
    curr_freq_hi = freq_hi;
    curr_freq_inc= freq_inc;
/*
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
*/
    return (0);
  }

  int chip_imp_api_off () {
/*
    if (dev_hndl >= 0) {
      close (dev_hndl);
    }
*/
    return (0);
  }

    // Internal functions:

  int band_setup ();


    // Chip API:
  int chip_imp_pwr_on (int pwr_rds) {
    int ret = 0;
    logd ("chip_imp_pwr_on");
/*
    ret = ioctl (dev_hndl, Si4709_IOC_POWERUP);
    if (ret < 0) {
      loge ("chip_imp_pwr_on IOCTL Si4709_IOC_POWERUP error: %3.3d", ret);
      return (-1);
    }
    logd ("chip_imp_pwr_on IOCTL Si4709_IOC_POWERUP success");
    //chip_info_log ();
    //chip_imp_mute_set (1);                                                       // Mute for now
    ms_sleep (100);
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
        loge ("sl_chip_imp_pwr_on IOCTL Si4709_IOC_RDS_DISABLE error: %d", ret);
      else
        logd ("sl_chip_imp_pwr_on IOCTL Si4709_IOC_RDS_DISABLE success");
    }
*/
    ms_sleep (200);
    band_setup ();

//chip_imp_vol_set (65535); Test volext; doesn't work on stock 4.3

    logd ("chip_imp_pwr_on done");
    return (0);
  }

  int chip_imp_pwr_off (int pwr_rds) {
    int ret = 0;
    logd ("chip_imp_pwr_off");
/*
    if (pwr_rds) {
      ms_sleep (100);
      ret = ioctl (dev_hndl, Si4709_IOC_RDS_DISABLE);
      if (ret < 0) {
        loge ("chip_imp_pwr_off IOCTL Si4709_IOC_RDS_DISABLE error: %3.3d", ret);
     }
      else {
        logd ("chip_imp_pwr_off IOCTL Si4709_IOC_RDS_DISABLE success");
      }
      ms_sleep (100);
    }
    chip_imp_mute_set (1);                                                         // Mute
    ret = ioctl (dev_hndl, Si4709_IOC_POWERDOWN);
    if (ret < 0) {
     loge ("chip_imp_pwr_off IOCTL Si4709_IOC_POWERDOWN error: %3.3d", ret);
      return (-1);
    }
    logd ("chip_imp_pwr_off IOCTL Si4709_IOC_POWERDOWN success");
*/
    return (0);
  }

    // Set:
  int chip_imp_freq_set (int freq) {        // 10 KHz resolution    76 MHz = 7600, 108 MHz = 10800
    logd ("chip_imp_freq_set: %3.3d", freq);
/*
    freq = freq / 10 ;
    int ret = ioctl (dev_hndl, Si4709_IOC_CHAN_SELECT, & freq);
    if (ret < 0) {
      loge ("chip_imp_freq_set IOCTL Si4709_IOC_CHAN_SELECT error: %3.3d", ret);
      return (-1);
    }
*/
    curr_freq_val = freq;
    logd ("chip_imp_freq_set gen success");
    return (0);
  }

  int chip_imp_mute_set (int mute) {
    int ret = 0;
    logd ("chip_imp_mute_set: %3.3d", mute);
/*
    if (mute) {
      ret = ioctl (dev_hndl, Si4709_IOC_MUTE_ON);
    }
    else {
      ret = ioctl (dev_hndl, Si4709_IOC_MUTE_OFF);
    }
    if (ret < 0) {
      loge ("chip_imp_mute_set IOCTL Si4709_IOC_MUTE error: %3.3d", ret);
      return (-1);
    }
    logd ("chip_imp_mute_set IOCTL Si4709_IOC_MUTE success");
*/
    return (0);
  }

  int chip_imp_stro_set (int stereo) {                                        //
    int ret = 0;
    logd ("chip_imp_stro_set: %3.3d", stereo);
/*
    if (stereo)
      ret = ioctl (dev_hndl, Si4709_IOC_STEREO_SET);
    else
      ret = ioctl (dev_hndl, Si4709_IOC_MONO_SET);
    if (ret < 0) {
      loge ("chip_imp_stro_set IOCTL Si4709_MOST_SET error: %3.3d  errno: %3.3d", ret, errno);
      return (-1);
    }
    logd ("chip_imp_stro_set IOCTL Si4709_MOST_SET success");
*/
    return (0);
  }

  int vol_ext = 0;                                                     // 0 = Automatic
  int chip_imp_vol_set (int vol) {
    int ret = 0;
/*
    if (vol_ext == 2) {                                                // If enabled
      ret = ioctl (dev_hndl, Si4709_IOC_VOLEXT_ENB);
    }
    else if (vol_ext == 1) {                                           // If disabled
      ret = ioctl (dev_hndl, Si4709_IOC_VOLEXT_DISB);
    }
    if (ret < 0) {
      loge ("chip_imp_vol_set IOCTL Si4709_IOC_VOLEXT error: %3.3d  vol_ext: %3.3d", ret, vol_ext);
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
      loge ("chip_imp_vol_set IOCTL Si4709_VOLUME_SET error: %3.3d  errno: %3.3d", ret, errno);
    else
      logd ("chip_imp_vol_set IOCTL Si4709_VOLUME_SET success");
*/
    return (0);
  }

    // Get:
  int chip_imp_freq_get () {
    int freq = 88500;
/*
    int ret = ioctl (dev_hndl, Si4709_IOC_CHAN_GET, & freq);
    if (ret < 0) {
      loge ("chip_imp_freq_get IOCTL Si4709_IOC_CHAN_GET error: %3.3d", ret);
      return (-1);
    }
    freq *= 10;
*/
    curr_freq_val = freq;
    if (ena_log_tnr_extra)
      logd ("chip_imp_freq_get gen success: %3.3d", freq);
    return (freq);
  }

  int chip_imp_rssi_get () {
/*
    rssi_snr_t rssi_snr = {0};
    int ret = ioctl (dev_hndl, Si4709_IOC_CUR_RSSI_GET, & rssi_snr);
    if (ret < 0) {
      loge ("chip_imp_rssi_get IOCTL Si4709_IOC_CUR_RSSI_GET error: %3.3d", ret);
      return (-1);
    }
    if (ena_log_tnr_extra)
      logd ("chip_imp_rssi_get IOCTL Si4709_IOC_CUR_RSSI_GET success: %3.3d %3.3d %3.3d",rssi_snr.curr_rssi,rssi_snr.curr_rssi_th,rssi_snr.curr_snr);
    return (rssi_snr.curr_rssi);
*/
    return (-1);
  }

  int sls_status_chip_imp_rssi_get_cnt = 0;
  int chip_imp_stro_get () {
/*
    status_rssi_t sr = {0};
    int ret = ioctl (dev_hndl, Si4709_IOC_STATUS_RSSI_GET, & sr);
    if (ret < 0) {
      loge ("chip_imp_stro_get IOCTL                    Si4709_IOC_STATUS_RSSI_GET error: %3.3d   rds_ready: %3.3d  rds_synced: %3.3d  seek_tune_complete: %3.3d  seekfail_bandlimit: %3.3d\
          afc_railed: %3.3d  block_error_a: %3.3d stereo: %3.3d  rssi: %3.3d", ret, sr.rdsr, sr.rdss, sr.stc, sr.sfbl, sr.afcrl, sr.blera, sr.st, sr.rssi);
     return (0);
    }
    if (sls_status_chip_imp_rssi_get_cnt ++ % 1200 == 0)                           // Every 2 minutes
      logd ("chip_imp_stro_get                          Si4709_IOC_STATUS_RSSI_GET success: %3.3d  rds_ready: %3.3d  rds_synced: %3.3d  seek_tune_complete: %3.3d  seekfail_bandlimit: %3.3d\
          afc_railed: %3.3d  block_err_a: %3.3d stereo: %3.3d  rssi: %3.3d", ret, sr.rdsr, sr.rdss, sr.stc, sr.sfbl, sr.afcrl, sr.blera, sr.st, sr.rssi);
    if (sr.st)                                                            // If stereo detected
      return (1);
*/
    return (0);
  }

  int chip_imp_events_process (unsigned char * rds_grpd) {
    return (-1);    // No RDS Data
  }

  int chip_imp_seek_start (int dir) {
    int ret = 0;
    logd ("chip_imp_seek_start dir: %3.3d", dir);
    //chip_imp_seek_stop ();
    int this_freq = 88500;
/*
    int freq = 0, ctr = 0;
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
        loge ("chip_imp_seek_start IOCTL Si4709_SEEK error: %3.3d", ret);
        return (-1);
      }
      logd ("chip_imp_seek_start IOCTL Si4709_SEEK success freq: %3.3d", freq);
      if (freq)
        break;
    }
    this_freq = freq * 10;
*/
    return (this_freq);
  }

  int chip_imp_seek_stop () {
    logd ("chip_imp_seek_stop");
/*
   int ret = ioctl (dev_hndl, Si4709_IOC_SEEK_CANCEL);
    if (ret < 0) {
      loge ("chip_imp_seek_stop IOCTL Si4709_SEEK_CANCEL error: %3.3d", ret);
      return (-1);
    }
    logd ("chip_imp_seek_stop IOCTL Si4709_SEEK_CANCEL success");
*/
    return (0);
  }


  int band_set (int low , int high) {                 // ? Do we need to stop/restart RDS power in reg 0x00 ? Or rbds_set to flush ?
    int band = 0;
    logd ("band_set low: %3.3d  high: %3.3d", low, high);
/*
    if (low < 87500)
      //band = BAND_76000_108000_kHz;
      band = BAND_76000_90000_kHz;
    else
      band = BAND_87500_108000_kHz;
    int ret = ioctl (dev_hndl, Si4709_IOC_BAND_SET, & band);
    if (ret < 0) {
      loge ("band_set IOCTL Si4709_BAND_SET error: %3.3d  errno: %3.3d", ret, errno);
      return (-1);
    }
    logd ("band_set IOCTL Si4709_BAND_SET success");
*/
    return (0);
  }
  int freq_inc_set (int inc) {
    logd ("freq_inc_set: %3.3d", inc);
/*
    inc /= 10;
    int ret = ioctl (dev_hndl, Si4709_IOC_CHAN_SPACING_SET, & inc);
*/
    return (0);
  }
  int emph75_set (int emph75) {
    int sil_emph = 0;  // actually only u8
    logd ("emph75_set: %3.3d", emph75);
/*
    if (emph75)
      sil_emph = DE_TIME_CONSTANT_75;
    else
      sil_emph = DE_TIME_CONSTANT_50;
    int ret = ioctl (dev_hndl, Si4709_IOC_DE_SET, & sil_emph);
*/
    return (0);
  }
  int rbds_set (int rbds) {
    logd ("rbds_set: %3.3d",rbds);
    return (0);
  }
  int band_setup () {
    band_set (curr_freq_lo, curr_freq_hi);
    freq_inc_set (curr_freq_inc);
    if (curr_freq_inc >= 200) {                                         // If US
      emph75_set (1);
      rbds_set (1);
    }
    else {
      emph75_set (0);
      rbds_set (0);
    }
  }

  int chip_imp_extra_cmd (const char * command, char ** parameters) {
    if (command == NULL)
      return (-1);
    return (0);
  }

