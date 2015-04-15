
    // FM Chip non-specific generic code, #include'd in tnr_bch.cpp, tnr_ssl.cpp, tnr_qcv.cpp

  const char * copyright = "Copyright (c) 2011-2015 Michael A. Reid. All rights reserved.";

  #include "utils.c"

  #include "tnr_tnr.h"

    // Plugin support data:

    // Prototypes
  int af_count_get ();
  int af_confidence_get (int idx);
  int current_event_get (int just_poll);

    // Callbacks that plugin calls:

  void (* cb_tuner_state)   (int reason);
  void (* cb_tuner_rssi)    (int new_level);
  void (* cb_tuner_pilot)   (int is_stereo);
  void (* cb_tuner_rds)     (rds_struct_t * rds_struct);
  void (* cb_tuner_rds_af)  (int new_freq);


    // Current value variables:

  int curr_api_mode     = 0;                                            // 0 = UART/default, 1 = SHIM
  int curr_api_state    = 0;                                            // 0 = Stop, 1 = Start
  int curr_mode         = 0;                                            // 0 = Receive, 1 = Transmit
  int curr_state        = 0;

  int curr_antenna      = 0;                                            // 0 = Default External/Wired headset antenna, 1 = Internal for many Sony Xperia Z/Z1 class Qualcomm chips
  int curr_band         = 0;                                            // 0 = EU

  int curr_freq_int     =  88500;
    int curr_freq_lo      =  87500;
    int curr_freq_hi      = 108000;
    int curr_freq_inc     =    100;

  int curr_vol          = 7;
  int curr_thresh       = 0;
  int curr_mute         = 0;
  int curr_softmute     = 1;                                            // 1 for default SoftMute or RF Mute, which lowers audio volume when signal is weak. 0 to disable to listen to weak signals.

  int curr_stereo       = 1;

  int curr_seek_state   = 0;
  int curr_rds_state    = 1;                                            // Default 1 = RDS on
  int curr_rds_af_state = 0;                                            // 0 = Disabled, 1 = Manual, 2 = RDS, 3 = Allow Regional  !! > 1 means need RDS so leave on even if screen off

  int curr_rssi         = -7;
  int curr_pilot        = 0  ;                                          // 0 = Mono, 1 = Stereo: read only indication of pilot 19 KHz / stereo


    // RDS Values: Current (under construction), Candidate (complete, but must be repeated before confirmed), Confirmed (Displayable, with few if any visual/logical defects.)

  int cand_rds_pi_ctr = 0;                                              // RDS PI
  int curr_rds_pi = 0;
  int cand_rds_pi = 0;
  int conf_rds_pi = 0;

  int cand_rds_pt_ctr = 0;                                              // RDS PT
  int curr_rds_pt = -1;
  int cand_rds_pt = -1;
  int conf_rds_pt = -1;

  char curr_rds_ps [9] = "        ";                                    // Current   RDS PS: Assembled here to start.
  char cand_rds_ps [9] = "        ";                                    // Candidate RDS PS: When all bytes are set, curr_rds_ps is copied to cand_rds_ps.
  char conf_rds_ps [9] = "        ";                                    // Confirmed RDS PS: When a new Current PS matches Candidate PS, the candidate is considered confirmed and copied here where the App can retrieve it.

  char curr_rds_rt [65] = "                                                                ";     // Current   RDS RT.
  char cand_rds_rt [65] = "                                                                ";     // Candidate RDS RT.
  char conf_rds_rt [65] = "                                                                ";     // Confirmed RDS RT.
  char conf_rds_rf [65] = {0};                                          // Confirmed RDS RT with no trailing spaces.


    // AF:
  int curr_af_num       = 0;                                            // Current number of AF table entries
  char curr_extension[64]= "";                                          // Current Extension data

  int last_af_count_get_s = 0;
  int af_regional_count = 0;

    // RDS:
  //unsigned char rds_grpd [8] = {0};
  unsigned char evt_rbuf [8] = {0};                                     // For RDS, but can be extended for other uses


    // Previous value variables:
  int prev_freq = 0;
  int prev_pilot  = 0;
  int pre2_pilot  = 0;

    // General flags:
  int stro_evt_enable = 0;      // !! Too much activity on SSL / QCV        BCM never gets here ?
  int rssi_evt_enable = 0;

    // Event flags requiring callback:
  int need_freq_chngd     = 0;
  int need_seek_cmplt     = 0;
  int need_pi_chngd       = 0;
  int need_pt_chngd       = 0;
  int need_ps_chngd       = 0;
  int need_rt_chngd       = 0;

    // Stats:
  int tuner_event_sg_ctr     = 0;
  int af_common_error_num     = 0;
  int af_general_error_num    = 0;
  int next_rssi_sg_per = 5000;         // Next RSSI get period: Get new RSSI every 5 seconds
  int next_rssi_sg_ms = 0;             // Time for next rssi_sg in ms_get () milliseconds
  int next_display_test_per = 10000;    // Next RDS test periond: New rds test every 1 seconds   1 hour
  int next_display_em_ms = 0;           // For emulator mode


    // Frequency Utilities:

  int freq_fix (int freq) {               // Ensures the frequency returned is fixed in regards to curr_freq_inc and curr_freq_odd
    logd ("freq_fix: %d", freq);
                                          // Only called by freq_enforce()
    // Tests/Use cases:
    // w/ Odd:  108099-107900 -> 107900     = Add 100, Divide by 200, then multiply by 200, then subtract 100
    // w/ Even: 108199-108000 -> 108000     = Divide by 200, then multiply by 200  (curr_freq_inc)
    // w/ Odd:  87500-87699 -> 87500     = Add 100, Divide by 200, then multiply by 200, then subtract 100
    // w/ Even: 87600-87799 -> 87600     = Divide by 200, then multiply by 200  (curr_freq_inc)

    if (curr_freq_inc >= 200) {//curr_freq_odd) {                       // curr_freq_odd: Should only be true if 200 KHz curr_freq_inc (North America/US/Korea)
      freq += curr_freq_inc / 2;               // Add half of curr_freq_inc (100) so that odd becomes even and usable for round down
      freq /= curr_freq_inc;
      freq *= curr_freq_inc;                   // Round down freq to closest frequency
      freq -= curr_freq_inc / 2;               // Subtract half of curr_freq_inc (100) so that even becomes original odd again
    }
    else {
      freq /= curr_freq_inc;
      freq *= curr_freq_inc;                   // Round down freq to closest frequency
    }
    if (freq < curr_freq_lo)                   // If rounding down caused freq to go below freq_lo...
      freq += curr_freq_inc;                   // Next channel up should be good.
    return (freq);
  }

  int freq_enforce (int freq, int fix) {  // Enforces limits curr_freq_lo and curr_freq_hi. If fixed due to limits, or argument fix != 0 then also enforces curr_freq_inc/curr_freq_odd
    logd ("freq_enforce: %d %d", freq, fix);
                                          // Only called by chip_imp_freq_sg () w/ fix = 0
                                          // Only called by freq_up_get ()/freq_dn_get () w/ fix = 1
                                          // Parameter fix set if caller requests or limit fixed so we can apply curr_freq_inc/curr_freq_odd rules. Unset to set frequency as close as possible.
    if (freq < curr_freq_lo) {
      freq = curr_freq_hi;
      fix = 1;                            // Force a curr_freq_inc/curr_freq_odd fix due to limit fix
    }
    if (freq > curr_freq_hi) {
      freq = curr_freq_lo;
      fix = 1;                            // Force a curr_freq_inc/curr_freq_odd fix due to limit fix
    }
    if (fix) {
      freq = freq_fix (freq);
    }
    return (freq);
  }

  int freq_up_get (int freq) {                                            // Called only by chip_imp_seek_state_sg ()
    logd ("freq_up_get: %d", freq);
    int save_freq_inc = curr_freq_inc;
    if (curr_freq_inc < 100)                                              // !!!! Hack for seek problems w/ 50 KHz offsets on TI and BC Low level APIs.
      curr_freq_inc = 100;

    //if (curr_freq_inc < 100)
    //  freq += 100;
    //else
      freq += curr_freq_inc;
    freq = freq_enforce (freq, 1);

    curr_freq_inc = save_freq_inc;
    return (freq);
  }

  int freq_dn_get (int freq) {                                            // Called only by chip_imp_seek_state_sg ()
    int save_freq_inc = curr_freq_inc;
    if (curr_freq_inc < 100)                                              // !!!! Hack for seek problems w/ 50 KHz offsets on TI and BC Low level APIs.
      curr_freq_inc = 100;

    //if (curr_freq_inc < 100)
    //  freq -= 100;
    //else
      freq -= curr_freq_inc;
    freq = freq_enforce (freq, 1);

    curr_freq_inc = save_freq_inc;
    return (freq);
  }


    // RDS:

  #ifdef  SUPPORT_RDS
    #include "rds_tnr.c"
  #else
  int rds_init () {
    return (-1);
  }
  int af_count_get () {
    return (0);
  }
  int rds_group_process (unsigned char * grp) {                         // For tnr_bch only
    return (-1);
  }
  void af_switch_if_needed () {
  }
  void rds_callback () {
  }
  #endif


    // Event getter / poll:

  #define   EVT_GET_NONE    0
  //#define   EVT_GET_RSSI    1
  //#define   EVT_GET_pilot     2
  #define   EVT_GET_RDS_PI  3
  #define   EVT_GET_RDS_PT  4
  #define   EVT_GET_RDS_PS  5
  #define   EVT_GET_RDS_RT  6
  #define   EVT_GET_RDS_RAW 7

  #define   EVT_GET_FREQ_OFFSET               0
  //#define   EVT_GET_FREQ_LO               65000
  //#define   EVT_GET_FREQ_HI              108000

  #define   EVT_GET_SEEK_FREQ_OFFSET    1000000
  //#define   EVT_GET_SEEK_FREQ_LO        1065000
  //#define   EVT_GET_SEEK_FREQ_HI        1108000

  int rds_total_polls = 0;
  int rds_data_polls = 0;

  int current_event_get (int just_poll) {                               // Called only from: tuner_event_sg()   w/ just_poll=0 (to get events)
                                                                            // And     from: af_switch()        w/ just_poll=1 (to avoid getting events, which we don't want while AF switching)
                                                                            // Need to keep polling during Alternate Frequency switching in order to get RDS info like PI
    if (ena_log_tnr_evt)
      logd ("current_event_get start just_poll: %d", just_poll);

    int current_event = -1;
    int pilot = 0; 

    if (! curr_state | curr_seek_state)                                 // If no power or seeking...
      return (EVT_GET_NONE);                                            // Return w/ no event

    if (ena_log_tnr_evt)
      logd ("current_event_get before af_count_get");

    int curr_s = ms_get () / 1000;
    if (last_af_count_get_s + 60 < curr_s)
      af_count_get ();                                                  // Ensure at least one AF aging every 60 seconds

    if (ena_log_tnr_evt)
      logd ("current_event_get after  af_count_get");
                                                                        // !!!! stro_get() before event_sg() to avoid si4709 problem ?
    if (stro_evt_enable) {                                              // If stereo events enabled...
      pilot = chip_imp_pilot_sg (GET);                                    // Get Mono/Stereo indication...
      //logd ("current_event_get pilot: %d  curr_pilot : %d  prev_pilot : %d  pre2_pilot : %d", pilot, curr_pilot, prev_pilot, pre2_pilot);

                                                                        // Only change stereo indication if two consecutive of old value followed by two consecutive of new (different)
      if (! pilot && ! curr_pilot  && prev_pilot  && pre2_pilot)
        cb_tuner_pilot  (0);                                              // Mono callback
      else if (pilot && curr_pilot  && ! prev_pilot  && ! pre2_pilot)
        cb_tuner_pilot  (1);                                              // Stereo callback

      pre2_pilot  = prev_pilot ;                                            // Age Previous + 1
      prev_pilot  = curr_pilot ;                                            // Age Previous
      curr_pilot  = pilot;                                                 // Previous = current
    }

    if (ena_log_tnr_evt)
      logd ("current_event_get after  if (stro_evt_enable)");

    if (curr_rds_state) {                                               // If RDS is on..
      int rds_grp_allowance_left = 2;//3;//4;//8;                                   // Only process maximum of 8 rds groups (about 0.16 seconds worth ?) per polling period

      int ret = EVT_GET_RDS_RAW;                                        // Ensure while loop starts

      while (ret != EVT_GET_NONE && rds_grp_allowance_left -- > 0) {    // While we have any 8 byte RDS groups available...
                                                                        // (Only one is processed per call to event_sg(), except QCV which blocks and handles it's own low level processing)

        ret = chip_imp_event_sg (evt_rbuf);                             // RDS Process/get (except during low power mode or no rds mode)
        rds_total_polls ++;

        if (ena_log_tnr_evt)
          logd ("current_event_get chip_imp_event_sg ret: %d", ret);

// !! SSL is the only one that returns raw data here !!
        if (ret == EVT_GET_RDS_RAW) {                                   // If have new raw RDS data...
          rds_data_polls ++;
          rds_group_process (evt_rbuf);                                 // Pass ptr to 8 bytes of group data, Most significant byte first (big endian)
        }
        if (rds_total_polls % 800 == 0)
          logd ("rds_total_polls: %d  rds_data_polls: %d", rds_total_polls, rds_data_polls);
      }
    }

    if (ena_log_tnr_evt)
      logd ("current_event_get after  if (curr_rds_state)");

    if (rssi_evt_enable && ! curr_seek_state) {                         // If RSSI events enabled and not seeking...

      if (ms_get () >= next_rssi_sg_ms) {                               // If time for another RSSI check...
        next_rssi_sg_ms = ms_get () + next_rssi_sg_per;                 // Set next RSSI check time
        int old_rssi = curr_rssi;                                       // Save old RSSI to compare

        curr_rssi = chip_imp_rssi_sg (GET);                             // Get current RSSI
        if (curr_rssi != old_rssi) {                                    // If RSSI changed
          if (ena_log_tnr_evt)
            logd ("current_event_get  new rssi: %d", curr_rssi);
          cb_tuner_rssi (curr_rssi);                                    // Signal RSSI changed event
        }
        else {
          if (ena_log_tnr_evt)
            logd ("current_event_get same rssi: %d", curr_rssi);
        }
      }
    }

    if (! curr_state) {
      if (ena_log_tnr_evt)
        logd ("current_event_get ! curr_state");
      current_event = EVT_GET_NONE;
    }
    else if (just_poll) {
      if (ena_log_tnr_evt)
        logd ("current_event_get just_poll");
      current_event = EVT_GET_NONE;
    }
    else if (curr_seek_state && need_seek_cmplt) {
      if (ena_log_tnr_evt)
        logd ("current_event_get seek_in_progress && need_seek_cmplt");
      curr_seek_state = 0;
      need_seek_cmplt = 0;
      current_event = curr_freq_int + EVT_GET_SEEK_FREQ_OFFSET;         // Seek Complete Event
    }
    else if (need_freq_chngd) {
      if (ena_log_tnr_evt)
        logd ("current_event_get need_freq_chngd");
      need_freq_chngd = 0;
      current_event = curr_freq_int + EVT_GET_FREQ_OFFSET;              // Frequency Changed Event
    }
    else if (need_pi_chngd) {
      if (ena_log_tnr_evt)
        logd ("current_event_get need_pi_chngd");
      need_pi_chngd = 0;
      current_event = EVT_GET_RDS_PI;
    }
    else if (need_pt_chngd) {
      if (ena_log_tnr_evt)
        logd ("current_event_get need_pt_chngd");
      need_pt_chngd = 0;
      current_event = EVT_GET_RDS_PT;
    }
    else if (need_ps_chngd) {
      if (ena_log_tnr_evt)
        logd ("current_event_get need_ps_chngd");
      need_ps_chngd = 0;
      current_event = EVT_GET_RDS_PS;
    }
    else if (need_rt_chngd) {
      if (ena_log_tnr_evt)
        logd ("current_event_get need_rt_chngd");
      need_rt_chngd = 0;
      current_event = EVT_GET_RDS_RT;
    }
    else {
      if (ena_log_tnr_evt)
        logd ("current_event_get no event");
    }

    if (ena_log_tnr_evt)
      logd ("current_event_get done current_event: %d", current_event);

    return (current_event);
  }


    // CHIP API: / Tuner functions:         // 21 Remote sget Set/Get:

  int tuner_event_sg (unsigned char * evt_rbuf) { //int event_sg_ms) {  // Polling function called every event_sg_ms milliseconds. Not used remotely but could be in future.
    if (ena_log_verbose_tshoot)
      logd ("tuner_event_sg evt_rbuf: %p", evt_rbuf);
    int event_allowance_left = 1;//2;//8;                                       // Only process maximum of 8 events per polling period
    int current_event = 1;                                              // Start current_event = 1 to ensure while() loop starts

                                                                        // While tuner is on and we processed an event and less than 8 events processed and not seeking...
    while (curr_state && current_event > EVT_GET_NONE && event_allowance_left -- > 0 && ! curr_seek_state) {
      current_event = current_event_get (0);                            // Get next event
      if (ena_log_verbose_tshoot)
        logd ("tuner_event_sg current_event: %d", current_event);
      if (curr_state && current_event >= EVT_GET_RDS_PI && current_event <= EVT_GET_RDS_RT)
        rds_callback ();                                                // If tuner on and this is an RDS event Do RDS Callback
    }
    if (ena_log_verbose_tshoot)
      logd ("tuner_event_sg done events event_allowance_left: %d", event_allowance_left);
    if (curr_state && curr_af_num && ! curr_seek_state)                 // If tuner on and we have AFs and not seeking...
      if (tuner_event_sg_ctr % 10 == 0)                                 // Every 1 second...
        af_switch_if_needed ();                                         // Check RSSI and switch to AF if needed

    if (tuner_event_sg_ctr ++ % 600 == 0)                                // Log every 60 seconds to show we are still alive and not blocked...
      logd ("tuner_event_sg_ctr: %d  current_event: %d", tuner_event_sg_ctr, current_event);

    if (ena_log_verbose_tshoot)
      logd ("tuner_event_sg done af_switch_if_needed and returning");

    return (current_event);
  }

  plugin_funcs_t local_funcs = {                                        // !! Must match order with plugin_funcs_t in tnr_tnr.h

        // 21 sget Set/Get:

    // Integer functions:
                                                        // Not used remotely by app, but could be. App could control polling and get "tuner_bulk" response
    tuner_event_sg,             //chip_imp_event_sg,                     // Polling function called every 100 ms or so when daemon receive times out regularly.

    chip_imp_api_mode_sg,                                                // 0 / 1 = Default+UART / SHIM
    chip_imp_api_state_sg,                                               // 0 / 1 = Stop         / Start
    chip_imp_mode_sg,                                                    // 0 / 1 = Receive      / Transmit
    chip_imp_state_sg,                                                   // 0 / 1 = Stop         / Start

    chip_imp_antenna_sg,                                                 // 0 / 1 = External     / Internal
    chip_imp_band_sg,                                                    // 0 / 1 = EU           / US
    chip_imp_freq_sg,                                                    // 65/76/87.5 - 108,..., 65000/76000/87500 - 108000
    chip_imp_vol_sg,                                                     // 0 - 65535
    chip_imp_thresh_sg,                                                  // 0 - ?

    chip_imp_mute_sg,                                                    // 0 / 1 = Unmute       / Mute
    chip_imp_softmute_sg,                                                // 0 / 1 = SoftMute dis / SoftMute enable
    chip_imp_stereo_sg,                                                  // 0 / 1 = Mono         / Stereo    (Stereo request)
    chip_imp_seek_state_sg,                                              // 0 / 1 / 2 = Stop, Up, Down
    chip_imp_rds_state_sg,                                               // 0 / 1 = Stop         / Start
    chip_imp_rds_af_state_sg,                                            // 0 / 1 = Stop         / Start

    chip_imp_rssi_sg,                                                    // 0 - ?
    chip_imp_pilot_sg,                                                    // 0 / 1 = Mono         / Stereo    (Stereo/19 KHz pilot indication)

    chip_imp_rds_pi_sg,                                                  // 0 - 65535
    chip_imp_rds_pt_sg,                                                  // 0 - 65535

    // Character array functions:   May be ASCIIZ strings, but for Euro language RDS, 0 is the common 'á'   ; Could translate to rare character however...

    chip_imp_rds_ps_sg,                                                  // Characters: 0 - 8 or 0 - 64 / ?  for multiple consecutive preloaded PS strings to cycle through
    chip_imp_rds_rt_sg,                                                  // Characters: 0 - 64 / ?           for multiple consecutive preloaded RT strings to cycle through ?

    chip_imp_extension_sg,                                               // Characters: 0 - ? For chip specific extensions such as register access or other functions
  };


    // Initial C level registration function each plugin library must implement:

  int plugin_reg (unsigned int * sig, plugin_funcs_t * funcs, plugin_cbs_t * cbs) {
    logd ("plugin_reg sig: %p  funcs: %p  cbs: %p", sig, funcs, cbs);

    utils_init ();

    //logd ("plugin_reg sizeof (rds_struct_t): %d",  sizeof (rds_struct_t));  // plugin_reg sizeof (rds_struct_t): 216

    if (sig)                                                            // If valid sig pointer passed...
      * sig = PLUGIN_SIG;                                               // Return signature identifying a valid plugin

    if (funcs)                                                          // If valid plugin functions pointer passed...
      * funcs = local_funcs;                                            // Return our local functions structure pointer

    if (cbs) {                                                          // If valid callbacks pointer passed...
                                                                        // Save callback function pointers that we (the plugin) call in the main s2d daemon binary
      cb_tuner_state = cbs->cb_tuner_state;                             // cb_tuner_state not used yet, but could inform app about fatal/unexpected errors that set tuner state to Stop
      cb_tuner_rssi  = cbs->cb_tuner_rssi;                              // RSSI
      cb_tuner_pilot   = cbs->cb_tuner_pilot ;                              // MOST: Mono/Stereo reception indication
      cb_tuner_rds   = cbs->cb_tuner_rds;                               // RDS Data
      cb_tuner_rds_af= cbs->cb_tuner_rds_af;                            // RDS AF Alternate frequencies
    }

    return (0);
  }

