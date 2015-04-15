
    // Tuner Sub-service

package fm.a2d.sf;

//import android.app.PendingIntent;
import android.content.Intent;
import java.util.TimerTask;
import java.util.Timer;
import android.os.Bundle;
import android.os.Handler;
import android.content.Context;

import java.io.BufferedReader;
import java.io.FileReader;

public class svc_tnr implements svc_tap {

  private static int    m_obinits = 1;

  private svc_tcb   m_svc_tcb   = null;
  private com_api   m_com_api   = null;
  private Handler   m_hndlr     = new Handler ();                       // Instantiate handler object for events/Service callbacks
  private Context   m_context   = null;

  private boolean   need_polling        = true;
  private boolean   is_polling          = false;
  private Timer     poll_tmr            = null;

  private String    last_poll_state     = "-1";
  private int       last_poll_freq      = -1;
  private int       last_poll_rssi      = -1;
  private String    last_poll_pilot      = "-1";
  private int       last_poll_rds_pi    = -1;
  private int       last_poll_rds_pt    = -1;
  private String    last_poll_rds_ps    = "-1";
  private String    last_poll_rds_rt    = "-1";

  public svc_tnr (Context c, svc_tcb cb_tnr, com_api svc_com_api) {     // Context & Tuner API callback constructor
    com_uti.logd ("m_obinits: " + m_obinits++);

    com_uti.logd ("constructor context: " + c + "  cb_tnr: " + cb_tnr);
    m_svc_tcb = cb_tnr;
    m_com_api = svc_com_api;
    m_context = c;
  }


  // Tuner API:

  public String tuner_get (String key) {
    if (key == null)
      return ("");
/*
    else if (key.equals ("test"))
      return (com_uti.daemon_get (key));

    else if (key.equals ("tuner_state"))
      return (m_com_api.tuner_state);

*/

    else if (m_com_api.tuner_state.equals ("Start"))          // If tuner started...
      return (com_uti.daemon_get (key));                                // Get value from daemon

        // Else if not on, use cached info:
    else if (key.equals ("tuner_band"))
      return (m_com_api.tuner_band);
    else if (key.equals ("tuner_freq"))
      return (m_com_api.tuner_freq);
    else if (key.equals ("tuner_stereo"))
      return (m_com_api.tuner_stereo);
    else if (key.equals ("tuner_thresh"))
      return (m_com_api.tuner_thresh);
    else if (key.equals ("tuner_seek_state"))
      return (m_com_api.tuner_seek_state);

    else if (key.equals ("tuner_rds_state"))
      return (m_com_api.tuner_rds_state);
    else if (key.equals ("tuner_rds_af_state"))
      return (m_com_api.tuner_rds_af_state);
    else if (key.equals ("tuner_rds_ta_state"))
      return (m_com_api.tuner_rds_ta_state);

    else if (key.equals ("tuner_extension"))
      return (m_com_api.tuner_extension);

    else if (key.equals ("tuner_rssi"))
      return (m_com_api.tuner_rssi);
    else if (key.equals ("tuner_qual"))
      return (m_com_api.tuner_qual);
    else if (key.equals ("tuner_pilot "))
      return (m_com_api.tuner_pilot);

    else if (key.equals ("tuner_rds_pi"))
      return (m_com_api.tuner_rds_pi);
    else if (key.equals ("tuner_rds_picl"))
      return (m_com_api.tuner_rds_picl);
    else if (key.equals ("tuner_rds_pt"))
      return (m_com_api.tuner_rds_pt);
    else if (key.equals ("tuner_rds_ptyn"))
      return (m_com_api.tuner_rds_ptyn);
    else if (key.equals ("tuner_rds_ps"))
      return (m_com_api.tuner_rds_ps);
    else if (key.equals ("tuner_rds_rt"))
      return (m_com_api.tuner_rds_rt);

    else if (key.equals ("tuner_rds_af"))
      return (m_com_api.tuner_rds_af);
    else if (key.equals ("tuner_rds_ms"))
      return (m_com_api.tuner_rds_ms);
    else if (key.equals ("tuner_rds_ct"))
      return (m_com_api.tuner_rds_ct);
    else if (key.equals ("tuner_rds_tmc"))
      return (m_com_api.tuner_rds_tmc);
    else if (key.equals ("tuner_rds_tp"))
      return (m_com_api.tuner_rds_tp);
    else if (key.equals ("tuner_rds_ta"))
      return (m_com_api.tuner_rds_ta);
    else if (key.equals ("tuner_rds_taf"))
      return (m_com_api.tuner_rds_taf);

    else
      return ("0");  //return ("");

  }

  public String tuner_set (String key, String val) {
    if (key == null)
      return ("");

    else if (key.equals ("tuner_state"))                      // If tuner_state...
      return (tuner_state_set (val));                                   // Set Tuner State in local function

    else if (m_com_api.tuner_state.equals ("Start"))          // If tuner_state = Start (If s2d tuner daemon is running)...
      return (com_uti.daemon_set (key, val));                           // Send key and value to s2d daemon

                                                                        // Else if s2d daemon is not running: just set the local variable
    else if (key.equals ("tuner_band"))
      return (m_com_api.tuner_band = val);
    else if (key.equals ("tuner_freq"))
      return (m_com_api.tuner_freq = val);
    else if (key.equals ("tuner_stereo"))
      return (m_com_api.tuner_stereo = val);
    else if (key.equals ("tuner_thresh"))
      return (m_com_api.tuner_thresh = val);
    else if (key.equals ("tuner_seek_state"))
      return (m_com_api.tuner_seek_state = val);

    else if (key.equals ("tuner_rds_state"))
      return (m_com_api.tuner_rds_state = val);
    else if (key.equals ("tuner_rds_af_state"))
      return (m_com_api.tuner_rds_af_state = val);
    else if (key.equals ("tuner_rds_ta_state"))
      return (m_com_api.tuner_rds_ta_state = val);

    else if (key.equals ("tuner_extension"))
      return (m_com_api.tuner_extension = val);

    else
      return ("0");  //return ("");

  }


  private static final String s2d_running_file = "/dev/s2d_running";

  private boolean s2d_running_wait (boolean running, int tmo_ms) {
    com_uti.logd ("poll for s2d_running");
    int sleep_ms = 100;
    int max_ctr = tmo_ms / sleep_ms;
    int ctr = 0;
    for (ctr = 0; ctr < max_ctr; ctr ++) {
      if (running) {
        if (com_uti.quiet_file_get (s2d_running_file))
          break;
      }
      else {
        if (! com_uti.quiet_file_get (s2d_running_file))
          break;
      }
      com_uti.quiet_ms_sleep (sleep_ms);
    }
    if (ctr < max_ctr) {
      com_uti.logd ("Done ctr: " + ctr);
      return (true);
    }
    com_uti.loge ("Done timeout ctr: " + ctr);
    return (false);
  }


  public static final int service_timeout_daemon_start  = 16;           // Wait up to 16 seconds for daemon to start
  private int service_timeout_tuner_api_start           = 12;           // Wait up to 12 seconds for tuner_api to start
  private int service_timeout_tuner_start               = 8;
  private int service_timeout_general                   = 2;
  private int service_timeout_tuner_stop                = 4;
  private int service_timeout_tuner_api_stop            = 6;

  private String tuner_state_set (String desired_state) {               // Turn FM chip power on (Start) or off (Stop)
    int res_len = 0;
    int ret = 0;
    String cmd_line = "";
    com_uti.logd ("desired_state: " + desired_state + "  m_com_api.tuner_state: " + m_com_api.tuner_state);

    // START:
    if (desired_state.equals ("Start")) {                     // START:

      if (m_com_api.tuner_state.equals ("Stop")) {            // If tuner is stopped
        m_com_api.tuner_state = "Starting";                             // Starting...

        if (com_uti.file_get (s2d_running_file)) {                      // If "s2d daemon running" file already exists (if still running or was unexpectedly killed)
          com_uti.file_delete (s2d_running_file);
          com_uti.loge ("HAVE s2d_running_file: " + s2d_running_file);
          if (com_uti.file_get (s2d_running_file))                      // If we can't delete the "s2d daemon running" file
            com_uti.loge ("STILL HAVE after file_delete s2d_running_file: " + s2d_running_file);
        }
        else {
          com_uti.logd ("No s2d_running_file: " + s2d_running_file);
        }

                                                                        // Code for Xperia L issue; files under lib like lib/libs2d.so are permission 644 which works OK for libs but not exe/binaries
        String daemon_lib = "/data/data/fm.a2d.sf/lib/libs2d.so";
        String daemon_exe = "/data/data/fm.a2d.sf/files/s2d";

        cmd_line  = "chmod 755 " + daemon_lib + " ;";                   // Set permission 755 for libs2d.so
        cmd_line += "chmod 755 /data/data/fm.a2d.sf/lib/* ;";           // Set permission 755 for all libs
        cmd_line += "cp " + daemon_lib + " " + daemon_exe + " ;";       // Copy lib/lib2sd.so to files/s2d
        cmd_line += "chmod 755 " + daemon_exe + " ;";                   // Set permission 755 for files/s2d
        ret = com_uti.sys_run (cmd_line, false);
        com_uti.logd ("daemon setup ret: " + ret);


                                                                        // Send Phase Update
        m_com_api.service_update_send (null, "Testing Daemon", "" + service_timeout_general);

        String test1 = "/data/data/fm.a2d.sf/files/test1";
        String test2 = "/data/data/fm.a2d.sf/files/test2";

        if (com_uti.file_get (test1))
          com_uti.file_delete (test1);
        if (com_uti.file_get (test2))
          com_uti.file_delete (test2);

        if (com_uti.file_get (test1))
          com_uti.loge ("Can not delete " + test1);
        if (com_uti.file_get (test2))
          com_uti.loge ("Can not delete " + test2);

                                                                        // Start libs2d.so daemon for test in test mode (Not daemon server mode)
        cmd_line = daemon_lib + " test test 1>" + test1 + " 2>" + test2;
        ret = com_uti.sys_run (cmd_line, false);                        // Start s2d daemon for testing; Don't need SU
        com_uti.logd ("daemon test ret: " + ret);

        long size1 = 0;
        if (com_uti.file_get (test1)) {
          size1 = com_uti.file_size_get (test1);
          com_uti.logd ("Have file " + test1 + " size1: " + size1);
          if (size1 > 0) {
            byte [] ba = com_uti.file_read_16k (test1);
            String str = com_uti.ba_to_str (ba);
            com_uti.logd ("str: \"" + str + "\"");
          }
        }
        else {
          com_uti.loge ("No file " + test2);
        }

        long size2 = 0;
        if (com_uti.file_get (test2)) {
          size2 = com_uti.file_size_get (test2);
          com_uti.logd ("Have file " + test2 + " size2: " + size2);
          if (size2 > 0) {
            byte [] ba = com_uti.file_read_16k (test2);
            String str = com_uti.ba_to_str (ba);
            com_uti.logd ("str: \"" + str + "\"");
          }
        }
        else {
          com_uti.loge ("No file " + test2);
        }

        String daemon_bin = daemon_lib;
        if (size1 <= 0 && com_uti.file_get (daemon_exe)) {
          daemon_bin = daemon_exe;
        }

                                                                        // Send Phase Update
        m_com_api.service_update_send (null, "Starting Daemon", "" + service_timeout_daemon_start);

                                                                        // Start libs2d.so daemon
        cmd_line = daemon_bin + " server_mode_via_argv1 1>/dev/null 2>/dev/null";
        ret = com_uti.sys_run (cmd_line, true);                         // Start s2d daemon
        com_uti.logd ("daemon kill/start ret: " + ret);

                                                                        // Wait for daemon to start
        boolean started = s2d_running_wait (true, service_timeout_daemon_start * 1000);

        if (! started) {                                                // If s2d daemon NOT started... Send Error Update
          m_com_api.service_update_send (null, "ERROR Starting Daemon", "-1");    
          m_com_api.tuner_state = "Stop";
        }
        else if (started) {                                             // Else if s2d daemon started successfully...

                                                                        // Set Chassis Plugin Audio:
          m_com_api.service_update_send (null, "Setting Chassis Plugin Audio", "" + service_timeout_general);
          String new_chass_plug_aud = com_uti.daemon_set ("chass_plug_aud", m_com_api.chass_plug_aud);
          com_uti.logd ("m_com_api.chass_plug_aud: " + m_com_api.chass_plug_aud + "  new_chass_plug_aud: " + new_chass_plug_aud);

                                                                        // Set Chassis Plugin Tuner:
          m_com_api.service_update_send (null, "Setting Chassis Plugin Tuner", "" + service_timeout_general);
          String new_chass_plug_tnr = com_uti.daemon_set ("chass_plug_tnr", m_com_api.chass_plug_tnr);
          com_uti.logd ("m_com_api.chass_plug_tnr: " + m_com_api.chass_plug_tnr + "  new_chass_plug_tnr: " + new_chass_plug_tnr);

                                                                        // Set Tuner API Mode: UART, SHIM
          m_com_api.service_update_send (null, "Setting Tuner API Mode", "" + service_timeout_general);
          String new_tuner_api_mode = com_uti.daemon_set ("tuner_api_mode", m_com_api.tuner_api_mode);
          com_uti.logd ("m_com_api.tuner_api_mode: " + m_com_api.tuner_api_mode + "  new_tuner_api_mode: " + new_tuner_api_mode);

                                                                        // Start Tuner API
          m_com_api.service_update_send (null, "Starting Tuner API", "" + service_timeout_tuner_api_start);
          String new_tuner_api_state = com_uti.daemon_set ("tuner_api_state", "Start");
          com_uti.logd ("m_com_api.tuner_api_state: " + m_com_api.tuner_api_state + "  new_tuner_api_state: " + new_tuner_api_state);

          if (! new_tuner_api_state.equals ("Start")) {       // If error starting Tuner API...
                                                                        // Send Error update
            m_com_api.service_update_send (null, "ERROR Starting Tuner API", "-1");
            m_com_api.tuner_api_state = "Stop";
            m_com_api.tuner_state = "Stop";
          }
          else {                                                        // Else if Tuner API started successfully...
            m_com_api.tuner_api_state = "Start";

            m_com_api.tuner_mode = "Receive";                           // Default mode is Receive
            if (com_uti.s2_tx_apk () && m_com_api.chass_plug_tnr.equals ("QCV")) {  // If Transmit APK and tuner is QCV...
              m_com_api.tuner_mode = "Transmit";                        // Default mode is Transmit
            }
                                                                        // Allow Settings to have the final say
            m_com_api.tuner_mode = com_uti.prefs_get (m_context, "tuner_mode", m_com_api.tuner_mode);

                                                                        // Set Tuner Mode: Rx or Tx
            m_com_api.service_update_send (null, "Setting Tuner Mode", "" + service_timeout_general);
            String new_tuner_mode = com_uti.daemon_set ("tuner_mode", m_com_api.tuner_mode);
            com_uti.logd ("m_com_api.tuner_mode: " + m_com_api.tuner_mode + "  new_tuner_mode: " + new_tuner_mode);


                                                                        // Start Tuner
            m_com_api.service_update_send (null, "Starting Tuner", "" + service_timeout_tuner_start);
            String new_tuner_state = com_uti.daemon_set ("tuner_state", "Start");
            com_uti.logd ("m_com_api.tuner_state: " + m_com_api.tuner_state + "  new_tuner_state: " + new_tuner_state);


            if (! new_tuner_state.equals ("Start")) {         // If error starting tuner...
                                                                        // Send Error Update
              m_com_api.service_update_send (null, "ERROR Starting Tuner", "-1");
              m_com_api.tuner_state = "Stop";
            }
            else {                                                      // Else if Tuner started successfully...
                                                                        // Send Success Update
              m_com_api.service_update_send (null, "Success Starting Tuner", "0");

                // Not fully working yet: ?
              if (com_uti.s2_tx_apk () && m_com_api.tuner_mode.equals ("Transmit")) {   // If Transmit APK and TX mode...
                //com_uti.daemon_set ("tuner_rds_pi", "" + com_uti.prefs_get (m_context, "tuner_rds_pi", 34357));
                //com_uti.daemon_set ("tuner_rds_pt", "" + com_uti.prefs_get (m_context, "tuner_rds_pt", 9));
                //com_uti.daemon_set ("tuner_rds_ps",      com_uti.prefs_get (m_context, "tuner_rds_ps", "SpiritTX"));//"Spirit Transmit Program Service"));
                //com_uti.daemon_set ("tuner_rds_rt",      com_uti.prefs_get (m_context, "tuner_rds_rt", "Radiotext"));//"Spirit Transmit RadioText.......................................56789012345678901234567890123456789012345678901234567890123456701234567890abcdef"));

                need_tx_rds_init = true;    // Need to delay this in order to work... How much ?
              }
              else
                need_tx_rds_init = false;

              int poll_ms  = 1000;//800;//500;                          // Called every 1000/800/500 ms
              int wait_ms = 3000;//6000;                                // Wait 3/6 seconds before polling starts
              poll_start (wait_ms, poll_ms);                            // Start polling for changes & RDS

              m_com_api.tuner_state = "Start";                          // Started now
            }
          }
        }
      }
    }

    // STOP:
    else if (desired_state.equals ("Stop")) {                 // STOP:
      if (m_com_api.tuner_state.equals ("Start")) {           // If tuner is started...
        poll_stop ();                                                   // Stop polling for changes
        m_com_api.tuner_state = "Stopping";

        m_com_api.service_update_send (null, "Stopping Tuner", "" + service_timeout_tuner_stop);    // Send Phase Update

        String new_tuner_state = com_uti.daemon_set ("tuner_state", "Stop");// Stop Tuner
        com_uti.logd ("new_tuner_state: " + new_tuner_state);

        if (new_tuner_state.equals ("Stop")) {                // If Tuner stopped successfully...
          m_com_api.service_update_send (null, "Success Stopping Tuner", "0"); // Send Phase Update
          com_uti.logd ("Success Stopping Tuner");
        }
        else {
          m_com_api.service_update_send (null, "ERROR Stopping tuner", "-1");    // Send Error Update
          com_uti.loge ("ERROR Stopping tuner new_tuner_state: " + new_tuner_state);
        }

                                                                        // Stop Tuner API / Daemon
        m_com_api.service_update_send (null, "Stopping Tuner API", "" + service_timeout_tuner_api_stop);  // Send Phase Update

        String new_tuner_api_state = com_uti.daemon_set ("tuner_api_state", "Stop");
        com_uti.logd ("m_com_api.tuner_api_state: " + m_com_api.tuner_api_state + "  new_tuner_api_state: " + new_tuner_api_state);

        boolean stopped = s2d_running_wait (false, 2000);               // Wait up to 2 seconds for daemon to stop

        if (stopped && new_tuner_api_state.equals ("Stop")) { // If Tuner API stopped successfully...
          m_com_api.service_update_send (null, "Success Stopping Tuner API / Daemon", "0");  // Send Phase update
          com_uti.logd ("Success Stopping Tuner API / Daemon");
        }
        else {                                                        // Else if error stopping Tuner API...
          m_com_api.service_update_send (null, "ERROR Stopping Tuner API / Daemon", "-1");  // Send Error update
          com_uti.loge ("ERROR Stopping Tuner API / Daemon stopped: " + stopped + "  new_tuner_api_state: " + new_tuner_api_state);
        }
        m_com_api.tuner_api_state = "Stop";

        m_com_api.tuner_state = "Stop";
      }
    }
    else {
      com_uti.loge ("Unexpected desired_state: " + desired_state + "  setting m_com_api.tuner_state: " + m_com_api.tuner_state);
      return (m_com_api.tuner_state);
    }

    if (desired_state.equals (m_com_api.tuner_state)) {
      com_uti.logd ("Success setting m_com_api.tuner_state: " + m_com_api.tuner_state + "  to desired_state: " + desired_state);
      m_svc_tcb.cb_tuner_key ("tuner_state", m_com_api.tuner_state);
    }
    else {
      com_uti.loge ("ERROR setting m_com_api.tuner_state: " + m_com_api.tuner_state + "  to desired_state: " + desired_state);
      m_svc_tcb.cb_tuner_key ("tuner_state", m_com_api.tuner_state);
    }

    return (m_com_api.tuner_state);
  }


  private void poll_start (int wait_ms, int poll_ms) {
    if (! need_polling)
      return;
    if (is_polling)
      return;
    poll_tmr = new Timer ("Poll", true);                                // Start poll timer so volume will be set before FM chip power up, and applied at chip power up.
    if (poll_tmr != null) {
      poll_tmr.schedule (new poll_tmr_hndlr (), wait_ms, poll_ms);      // After wait_ms milliseconds every poll_ms ms     (3s because audio_state needs to complete ; could start after it completes)
    }
    is_polling = true;
  }
  private void poll_stop () {
    if (! need_polling)
      return;
    if (! is_polling)
      return;
    if (poll_tmr != null)
      poll_tmr.cancel ();                                               // Stop poll timer
    is_polling = false;
  }

  private static final  int min_freq = 65000;                           // !! Broadcom sometimes returns 64000, so suppress this as it writes frequency to settings

  private int       new_rds_pi      = -1;
  private int       new_rds_pt      = -1;
  private String    new_freq_str    = "-1";
  private int       new_freq_int    = -1;
  private boolean   need_tx_rds_init = false;

  //private static final  boolean tuner_state_callbacks = false;
  private class poll_tmr_hndlr extends TimerTask {
    public void run () {

/*    if (tuner_state_callbacks) {                                      // NOT ready for prime-time !
        m_com_api.tuner_state = com_uti.daemon_get ("tuner_state");
        if (! last_poll_state.equals (m_com_api.tuner_state))
          m_svc_tcb.cb_tuner_key ("tuner_state", last_poll_state = m_com_api.tuner_state);
      }*/

      if (! m_com_api.tuner_state.equals ("Start"))                     // Done if state not started
        return;



                // Not fully working yet: ?
      if (need_tx_rds_init && com_uti.s2_tx_apk () && m_com_api.tuner_mode.equals ("Transmit")) {   // If Transmit APK and TX mode...
        need_tx_rds_init = false;
        com_uti.daemon_set ("tuner_rds_pi", "" + com_uti.prefs_get (m_context, "tuner_rds_pi", 34357));
        com_uti.daemon_set ("tuner_rds_pt", "" + com_uti.prefs_get (m_context, "tuner_rds_pt", 9));
        com_uti.daemon_set ("tuner_rds_ps",      com_uti.prefs_get (m_context, "tuner_rds_ps", "SpiritTX"));//"Spirit Transmit Program Service"));
        com_uti.daemon_set ("tuner_rds_rt",      com_uti.prefs_get (m_context, "tuner_rds_rt", "Radiotext"));//"Spirit Transmit RadioText.......................................56789012345678901234567890123456789012345678901234567890123456701234567890abcdef"));
      }

      try {
        String bulk = com_uti.daemon_get ("tuner_bulk");
        com_uti.logv ("bulk: \"" + bulk + "\"");

        String [] buar = {""};
        buar = bulk.split ("mArK", 7);                                  // Split special tuner_bulk response

        if (buar.length <= 1)                                           // Done if nothing
          return;
        if (buar.length > 0) {
          new_freq_str          = buar [0];
          com_uti.logv ("new_freq_str: \"" + new_freq_str + "\"");
        }
        if (buar.length > 1) {
          m_com_api.tuner_rssi  = buar [1];
          com_uti.logv ("m_com_api.tuner_rssi: \"" + m_com_api.tuner_rssi + "\"");
        }
        if (buar.length > 2) {
          m_com_api.tuner_pilot  = buar [2];
          com_uti.logv ("m_com_api.tuner_pilot : \"" + m_com_api.tuner_pilot + "\"");
        }
        if (buar.length > 3) {
          m_com_api.tuner_rds_pi= buar [3];
          com_uti.logv ("m_com_api.tuner_rds_pi: \"" + m_com_api.tuner_rds_pi + "\"");
        }
        if (buar.length > 4) {
          m_com_api.tuner_rds_pt= buar [4];
          com_uti.logv ("m_com_api.tuner_rds_pt: \"" + m_com_api.tuner_rds_pt + "\"");
        }
        if (buar.length > 5) {
          m_com_api.tuner_rds_ps= buar [5];
          com_uti.logv ("m_com_api.tuner_rds_ps: \"" + m_com_api.tuner_rds_ps + "\"");
        }
        if (buar.length > 6) {
          m_com_api.tuner_rds_rt= buar [6].trim ();
          com_uti.logv ("m_com_api.tuner_rds_rt: \"" + m_com_api.tuner_rds_rt + "\"");
        }
      }
      catch (Throwable e) {
        com_uti.loge ("Throwable: " + e);
        e.printStackTrace ();
      }

      boolean need_bulk_displays_update = false;

        // Freq:
      new_freq_int = com_uti.int_get (new_freq_str);
      if (new_freq_int >= min_freq) {
        if (last_poll_freq != new_freq_int) {
          m_com_api.tuner_freq = new_freq_str;
          m_com_api.tuner_freq_int = new_freq_int;
          last_poll_freq = new_freq_int;
          m_svc_tcb.cb_tuner_key ("tuner_freq", m_com_api.tuner_freq);  // Inform change    !!!! SEPERATELY than other bulk results; svc_svc:cb_tuner_freq() clears a lot of RDS info etc.
        }
      }

        // RSSI:
      if (last_poll_rssi != (last_poll_rssi = com_uti.int_get (m_com_api.tuner_rssi)))
        need_bulk_displays_update = true;//m_svc_tcb.cb_tuner_key ("tuner_rssi", m_com_api.tuner_rssi);                        // Inform change

        // Pilot:                                                       // NOT ready for prime-time !
      //if (! last_poll_pilot.equals (m_com_api.tuner_pilot)) {
      //  need_bulk_displays_update = true;//m_svc_tcb.cb_tuner_key ("tuner_pilot ", last_poll_pilot = m_com_api.tuner_pilot); // Inform change
      //  last_poll_pilot = m_com_api.tuner_pilot;
      //}

        // RDS ps:
      if (! last_poll_rds_ps.equals (m_com_api.tuner_rds_ps)) {
        need_bulk_displays_update = true;//m_svc_tcb.cb_tuner_key ("tuner_rds_ps", last_poll_rds_ps = m_com_api.tuner_rds_ps); // Inform change
        last_poll_rds_ps = m_com_api.tuner_rds_ps;
      }

        // RDS rt:
      if (! last_poll_rds_rt.equals (m_com_api.tuner_rds_rt)) {
        need_bulk_displays_update = true;//m_svc_tcb.cb_tuner_key ("tuner_rds_rt", last_poll_rds_rt = m_com_api.tuner_rds_rt); // Inform change
        last_poll_rds_rt = m_com_api.tuner_rds_rt;
      }

        // RDS pi:
      new_rds_pi = com_uti.int_get (m_com_api.tuner_rds_pi);
      if (last_poll_rds_pi != new_rds_pi) {
        last_poll_rds_pi = new_rds_pi;
        m_com_api.tuner_rds_picl = com_uti.tnru_rds_picl_get (m_com_api.tuner_band, new_rds_pi);
        need_bulk_displays_update = true;//m_svc_tcb.cb_tuner_key ("tuner_rds_pi", m_com_api.tuner_rds_pi);                    // Inform change
      }

        // RDS pt:
      new_rds_pt = com_uti.int_get (m_com_api.tuner_rds_pt);
      if (last_poll_rds_pt != new_rds_pt) {
        last_poll_rds_pt = new_rds_pt;
        m_com_api.tuner_rds_pt = m_com_api.tuner_rds_ptyn = com_uti.tnru_rds_ptype_get (m_com_api.tuner_band, new_rds_pt);
        need_bulk_displays_update = true;//m_svc_tcb.cb_tuner_key ("tuner_rds_pt", m_com_api.tuner_rds_pt);                    // Inform change
      }

      if (need_bulk_displays_update) {
        m_svc_tcb.cb_tuner_key ("tuner_bulk", "");
      }

    }
  }

}

