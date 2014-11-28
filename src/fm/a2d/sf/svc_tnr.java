
    // Tuner Sub-service

package fm.a2d.sf;

import java.util.TimerTask;
import java.util.Timer;
import android.os.Bundle;
import android.os.Handler;
import android.content.Context;

import java.io.BufferedReader;
import java.io.FileReader;

public class svc_tnr implements svc_tap {

  private static int    stat_constrs = 1;

  private svc_tcb   m_svc_tcb   = null;
  private com_api   m_com_api   = null;
  private Handler   m_hndlr     = new Handler ();                       // Instantiate handler object for events/Service callbacks
  private Context   m_context   = null;

  private boolean need_polling = true;
  private boolean is_polling = false;
  private Timer poll_tmr;
  private int       last_poll_freq      = -1;
  private int       last_poll_rssi      = -1;
  private String    last_poll_most      = "-1";
  private int       last_poll_rds_pi    = -1;
  private int       last_poll_rds_pt    = -1;
  private String    last_poll_rds_ps    = "-1";
  private String    last_poll_rds_rt    = "-1";

  public svc_tnr (Context c, svc_tcb cb_tnr, com_api svc_com_api) {     // Context & Tuner API callback constructor
    com_uti.logd ("stat_constrs: " + stat_constrs++);

    com_uti.logd ("constructor context: " + c + "  cb_tnr: " + cb_tnr);
    m_svc_tcb = cb_tnr;
    m_com_api = svc_com_api;
    m_context = c;
  }


  // Tuner API:

  public String tuner_get (String key) {
    if (key == null)
      return ("");

    else if (key.equalsIgnoreCase ("test"))
      return (com_uti.s2d_get (key));

    else if (key.equalsIgnoreCase ("tuner_state"))
      return (m_com_api.tuner_state);

    else if (m_com_api.tuner_state.equalsIgnoreCase ("start"))
      return (com_uti.s2d_get (key));

        // Else if not on, use cached info:
    else if (key.equalsIgnoreCase ("tuner_band"))
      return (m_com_api.tuner_band);
    else if (key.equalsIgnoreCase ("tuner_freq"))
      return (m_com_api.tuner_freq);
    else if (key.equalsIgnoreCase ("tuner_stereo"))
      return (m_com_api.tuner_stereo);
    else if (key.equalsIgnoreCase ("tuner_thresh"))
      return (m_com_api.tuner_thresh);
    else if (key.equalsIgnoreCase ("tuner_scan_state"))
      return (m_com_api.tuner_scan_state);

    else if (key.equalsIgnoreCase ("tuner_rds_state"))
      return (m_com_api.tuner_rds_state);
    else if (key.equalsIgnoreCase ("tuner_rds_af_state"))
      return (m_com_api.tuner_rds_af_state);
    else if (key.equalsIgnoreCase ("tuner_rds_ta_state"))
      return (m_com_api.tuner_rds_ta_state);

    else if (key.equalsIgnoreCase ("tuner_extra_cmd"))
      return (m_com_api.tuner_extra_cmd);
    else if (key.equalsIgnoreCase ("tuner_extra_resp"))
      return (m_com_api.tuner_extra_resp);

    else if (key.equalsIgnoreCase ("tuner_rssi"))
      return (m_com_api.tuner_rssi);
    else if (key.equalsIgnoreCase ("tuner_qual"))
      return (m_com_api.tuner_qual);
    else if (key.equalsIgnoreCase ("tuner_most"))
      return (m_com_api.tuner_most);

    else if (key.equalsIgnoreCase ("tuner_rds_pi"))
      return (m_com_api.tuner_rds_pi);
    else if (key.equalsIgnoreCase ("tuner_rds_picl"))
      return (m_com_api.tuner_rds_picl);
    else if (key.equalsIgnoreCase ("tuner_rds_pt"))
      return (m_com_api.tuner_rds_pt);
    else if (key.equalsIgnoreCase ("tuner_rds_ptyn"))
      return (m_com_api.tuner_rds_ptyn);
    else if (key.equalsIgnoreCase ("tuner_rds_ps"))
      return (m_com_api.tuner_rds_ps);
    else if (key.equalsIgnoreCase ("tuner_rds_rt"))
      return (m_com_api.tuner_rds_rt);

    else if (key.equalsIgnoreCase ("tuner_rds_af"))
      return (m_com_api.tuner_rds_af);
    else if (key.equalsIgnoreCase ("tuner_rds_ms"))
      return (m_com_api.tuner_rds_ms);
    else if (key.equalsIgnoreCase ("tuner_rds_ct"))
      return (m_com_api.tuner_rds_ct);
    else if (key.equalsIgnoreCase ("tuner_rds_tmc"))
      return (m_com_api.tuner_rds_tmc);
    else if (key.equalsIgnoreCase ("tuner_rds_tp"))
      return (m_com_api.tuner_rds_tp);
    else if (key.equalsIgnoreCase ("tuner_rds_ta"))
      return (m_com_api.tuner_rds_ta);
    else if (key.equalsIgnoreCase ("tuner_rds_taf"))
      return (m_com_api.tuner_rds_taf);

    else
      return ("0");  //return ("");

  }

  public String tuner_set (String key, String val) {
    if (key == null)
      return ("");

    else if (key.equalsIgnoreCase ("tuner_state"))                      // If tuner_state...
      return (tuner_state_set (val));

    else if (key.equalsIgnoreCase ("radio_nop"))                        // If radio_nop...
      return (com_uti.s2d_set (key, val));                              // Set via s2d

    //else if (key.equalsIgnoreCase ("tuner_band"))
    //  return (tuner_band_set (val));

    else if (m_com_api.tuner_state.equalsIgnoreCase ("start"))          // If tuner_state = Start...
      return (com_uti.s2d_set (key, val));                                // Set via s2d

        // Else if not on:
    else if (key.equalsIgnoreCase ("tuner_band"))
      return (m_com_api.tuner_band = val);
    else if (key.equalsIgnoreCase ("tuner_freq"))
      return (m_com_api.tuner_freq = val);
    else if (key.equalsIgnoreCase ("tuner_stereo"))
      return (m_com_api.tuner_stereo = val);
    else if (key.equalsIgnoreCase ("tuner_thresh"))
      return (m_com_api.tuner_thresh = val);
    else if (key.equalsIgnoreCase ("tuner_scan_state"))
      return (m_com_api.tuner_scan_state = val);

    else if (key.equalsIgnoreCase ("tuner_rds_state"))
      return (m_com_api.tuner_rds_state = val);
    else if (key.equalsIgnoreCase ("tuner_rds_af_state"))
      return (m_com_api.tuner_rds_af_state = val);
    else if (key.equalsIgnoreCase ("tuner_rds_ta_state"))
      return (m_com_api.tuner_rds_ta_state = val);

    else if (key.equalsIgnoreCase ("tuner_extra_cmd"))
      return (m_com_api.tuner_extra_cmd = val);

    else
      return ("0");  //return ("");

  }

/* Test Freq codes:
boolean hci = false;
int port = 0;
int freq = com_uti.int_get (val);
if (freq >= 90000 && freq < 100000) {
port = freq - 90000;    // port = 0 - 9999
}
if (freq >= 100000 && freq < 108000) {
hci = true;
port = freq - 100000;    // port = 0 - 7999
}
com_uti.logd ("FREQ CODE freq: " + freq + "  hci: " + hci + "  port: " + port);
*/



  private String tuner_state_set (String state) {
    int res_len = 0;
    com_uti.logd ("state: " + state + "  m_com_api.tuner_state: " + m_com_api.tuner_state);

    // START:
    if (state.equalsIgnoreCase ("start")) {                             // START:

      if (! m_com_api.tuner_state.equalsIgnoreCase ("start") && ! m_com_api.tuner_state.equalsIgnoreCase ("starting")) {         // If not already started or starting
        m_com_api.tuner_state = "Starting"; // !! Already set

        int ret = 0;
        //com_uti.file_delete ("s2d_stop");
        if (com_uti.file_get ("/mnt/sdcard/sf/sys_bin"))
          ret = com_uti.sys_run ("killall    s2d    1>/dev/null 2>/dev/null ;                 /system/bin/s2d "    + com_uti.device + "  1>/dev/null 2>/dev/null", true);//  &", true);
        else
          ret = com_uti.sys_run ("killall libs2d.so 1>/dev/null 2>/dev/null ; /data/data/fm.a2d.sf/lib/libs2d.so " + com_uti.device + "  1>/dev/null 2>/dev/null", true);//  &", true);

        com_uti.logd ("daemon kill/start ret: " + ret);

        com_uti.ms_sleep (200);   // !!!! MUST have delay here
        com_uti.loge ("800 ms delay starting..., fix later");
        com_uti.ms_sleep (800);    // Extra stock HTC One M7 ?

        String res = com_uti.s2d_set ("tuner_state", "Start");
        com_uti.logd ("res: " + res);

        m_com_api.tuner_state = res;//"Start";         // State = Start
        com_uti.logd ("start tuner_state: " + m_com_api.tuner_state);
        poll_start ();                                                  // Start polling for changes
      }
    }
    // STOP:
    else if (state.equalsIgnoreCase ("stop")) {                         // STOP:
      if (! m_com_api.tuner_state.equalsIgnoreCase ("stop")) {          // If not already stopped...
        poll_stop ();                                                     // Stop polling for changes
        m_com_api.tuner_state = "Stopping";

        String res = com_uti.s2d_set ("tuner_state", "Stop");
        com_uti.logd ("res: " + res);

        com_uti.ms_sleep (500);                                         // Wait 500 ms for s2d daemon to stop, before killing (which may kill network socket or tuner access)

        m_com_api.tuner_state = "Stop";

        //com_uti.file_create ("s2d_stop");
        //com_uti.sys_run ();
/*
//com_uti.loge ("!!!! Should run killall libs2d.so here, after waiting a bit. But killing may kill UDP port !!!!");
        if (com_uti.file_get ("/mnt/sdcard/sf/sys_bin"))
          com_uti.sys_run ("killall s2d", true);  // To be sure
        else
          com_uti.sys_run ("killall libs2d.so", true);  // To be sure
*/
      }
    }

    m_svc_tcb.cb_tuner_key ("tuner_state", m_com_api.tuner_state);

    return (m_com_api.tuner_state);
  }


  private void poll_start () {
    if (! need_polling)
      return;
    if (is_polling)
      return;
    poll_tmr = new Timer ("Poll", true);                                // Start poll timer so volume will be set before FM chip power up, and applied at chip power up.
    if (poll_tmr != null) {
      poll_tmr.schedule (new poll_tmr_hndlr (), 3000, 500);             // After 3 seconds every 500 ms
    }
    //ms_sleep (10);                                                    // Wait 10 milliseconds.
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

  private class poll_tmr_hndlr extends TimerTask {
    public void run () {
      if (! m_com_api.tuner_state.equalsIgnoreCase ("start"))                                             // Done if state not started
        return;

        // Freq:
      int min_freq = 65000; //50000;    // !! Broadcom sometimes returns 64000, so suppress this as it writes frequency to settings
      String temp_freq_str = com_uti.s2d_get ("tuner_freq");
      int    temp_freq_int = com_uti.int_get (temp_freq_str);
      if (temp_freq_int >= min_freq) {
        m_com_api.tuner_freq = temp_freq_str;
        m_com_api.int_tuner_freq = com_uti.int_get (m_com_api.tuner_freq);
        if (m_com_api.int_tuner_freq >= min_freq && last_poll_freq != m_com_api.int_tuner_freq) {
          last_poll_freq = m_com_api.int_tuner_freq;
          m_svc_tcb.cb_tuner_key ("tuner_freq", m_com_api.tuner_freq);   // Inform change
        }
      }

        // RSSI:
      m_com_api.tuner_rssi = com_uti.s2d_get ("tuner_rssi");
      if (last_poll_rssi != (last_poll_rssi = com_uti.int_get (m_com_api.tuner_rssi)))
        m_svc_tcb.cb_tuner_key ("tuner_rssi", m_com_api.tuner_rssi);                        // Inform change

        // MOST:
      m_com_api.tuner_most = com_uti.s2d_get ("tuner_most");
      if (! last_poll_most.equals (m_com_api.tuner_most))
        m_svc_tcb.cb_tuner_key ("tuner_most", last_poll_most = m_com_api.tuner_most);       // Inform change

        // RDS ps:
      m_com_api.tuner_rds_ps = com_uti.s2d_get ("tuner_rds_ps");
      if (! last_poll_rds_ps.equals (m_com_api.tuner_rds_ps))
        m_svc_tcb.cb_tuner_key ("tuner_rds_ps", last_poll_rds_ps = m_com_api.tuner_rds_ps); // Inform change

        // RDS rt:
      m_com_api.tuner_rds_rt = com_uti.s2d_get ("tuner_rds_rt                                                                    ").trim ();    // !!!! Must have ~ 64 characters due to s2d design.
      if (! last_poll_rds_rt.equals (m_com_api.tuner_rds_rt))
        m_svc_tcb.cb_tuner_key ("tuner_rds_rt", last_poll_rds_rt = m_com_api.tuner_rds_rt); // Inform change

        // RDS pi:
      m_com_api.tuner_rds_pi = com_uti.s2d_get ("tuner_rds_pi");
      int rds_pi = com_uti.int_get (m_com_api.tuner_rds_pi);
      if (last_poll_rds_pi != rds_pi) {
        last_poll_rds_pi = rds_pi;
        m_com_api.tuner_rds_picl = com_uti.tnru_rds_picl_get (m_com_api.tuner_band, rds_pi);
        m_svc_tcb.cb_tuner_key ("tuner_rds_pi", m_com_api.tuner_rds_pi);                    // Inform change
      }

        // RDS pt:
      m_com_api.tuner_rds_pt = com_uti.s2d_get ("tuner_rds_pt");
      int rds_pt = com_uti.int_get (m_com_api.tuner_rds_pt);
      if (last_poll_rds_pt != rds_pt) {
        last_poll_rds_pt = rds_pt;
        m_com_api.tuner_rds_pt = m_com_api.tuner_rds_ptyn = com_uti.tnru_rds_ptype_get (m_com_api.tuner_band, rds_pt);
        m_svc_tcb.cb_tuner_key ("tuner_rds_pt", m_com_api.tuner_rds_pt);                    // Inform change
      }
    }
  }

}

