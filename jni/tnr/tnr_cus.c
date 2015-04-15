
    // Spirit2 Tuner Plugin for "Custom" API:

    // Does nothing at present, but expected to become a "Custom" demonstration API that allows binaries / scripts to access a real or emulated FM tuner chip

  #define LOGTAG "sftnrcus"

  #include <stdio.h>
  #include <errno.h>
  #include <sys/stat.h>
  #include <fcntl.h>

  #include "tnr_tnr.c"

    // Internal functions:

  int pwr_off () {
    curr_state = 0;
    return (curr_state);
  }

  int seek_stop () {
    logd ("seek_stop");
    curr_seek_state = 0;
    return (curr_seek_state);
  }

  int band_set (int low , int high, int band) {
    logd ("band_set low: %d  high: %d  band: %d", low, high, band);
    return (band);
  }

  int freq_inc_set (int inc) {
    logd ("freq_inc_set inc: %d", inc);
    return (inc);
  }

  int emph75_set (int band) {
    logd ("emph75_set band: %d", band);
    return (band);
  }

  int rbds_set (int band) {
    logd ("rbds_set band: %d", band);
    return (band);
  }


    // Chip API:

  int rds_poll (unsigned char * rds_grpd) {
    return (EVT_GET_NONE);                                              // No RDS
  }
                                                                        // Polling function called every event_sg_ms milliseconds to process RDS, RSSI and MOST event.
                                                                        // Not used remotely but could be in future.

  int chip_imp_event_sg (unsigned char * rds_grpd) {                    // Called  by tnr_tnr:current_event_get (get_event) mainly called by tnr_tnr:tuner_event_sg()
                                                                            //                current_event_get (just_poll) also   called by rds_tnr:af_switch ()
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

  int chip_imp_api_state_sg (int api_state) {
    logd ("chip_imp_api_state_sg state: %d", api_state);
    curr_api_state = api_state;
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

    chip_imp_band_sg (curr_band);                                       // Set Band

    chip_imp_vol_sg (65535);

    curr_state = 1;
    logd ("chip_imp_state_sg curr_state: %d", curr_state);
    return (curr_state);
  }

  int chip_imp_antenna_sg (int antenna) {
    if (antenna == GET)
      return (curr_antenna);

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
      return (curr_freq_int);

    logd ("chip_imp_freq_sg: %d", freq);
    curr_freq_int = freq;
    logd ("chip_imp_freq_sg success");
    return (curr_freq_int);
  }

  int chip_imp_vol_sg (int vol) {
    if (vol == GET)
      return (curr_vol);

    curr_vol = vol;
    logd ("chip_imp_vol_sg curr_vol: %d", curr_vol);
    return (curr_vol);
  }

  int chip_imp_thresh_sg (int thresh) {
    if (thresh == GET)
      return (curr_thresh);
    curr_thresh = thresh;
    logd ("chip_imp_thresh_sg curr_thresh: %d", curr_thresh);
    return (curr_thresh);
  }

  int chip_imp_mute_sg (int mute) {
    if (mute == GET)
      return (curr_mute);

    curr_mute = mute;
    logd ("chip_imp_mute_sg curr_mute: %d", curr_mute);
    return (curr_mute);
  }

  int chip_imp_softmute_sg (int softmute) {
    if (softmute == GET)
      return (curr_softmute);

    curr_softmute = softmute;
    logd ("chip_imp_softmute_sg curr_softmute: %d", curr_softmute);
    return (curr_softmute);
  }

  int chip_imp_stereo_sg (int stereo) {                                 //
    if (stereo == GET)
      return (curr_stereo);

    curr_stereo = stereo;
    logd ("chip_imp_stereo_sg curr_stereo: %d", curr_stereo);
    return (curr_stereo);
  }

  int chip_imp_seek_state_sg (int seek_state) {
    if (seek_state == GET)
      return (curr_seek_state);

    if (seek_state == 0)
      return (seek_stop ());

    curr_seek_state = 0;
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
    return (curr_rssi);
  }

  int chip_imp_pilot_sg (int fake_pilot) {
    return (curr_pilot);
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

