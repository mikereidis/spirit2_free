
    // Service

package fm.a2d.sf;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.media.MediaMetadataRetriever;
import android.media.RemoteControlClient;
import android.os.Bundle;
import android.os.IBinder;
import android.os.PowerManager;
import android.widget.Toast;


import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileOutputStream;
import java.lang.reflect.Method;
import java.util.Timer;
import java.util.TimerTask;


public class svc_svc extends Service implements svc_tcb, svc_acb {  // Service class implements Tuner API callbacks & Service Audio API callback

    // Also see AndroidManifest.xml.

    // Action: Commands sent to this service
  //"fm.a2d.sf.action.set"  = com_uti.api_action_id

    // Result: Broadcast multiple status/state data items to all registered listeners; apps, widgets, etc.
  //"fm.a2d.sf.result.get"  = com_uti.api_result_id


    // No constructor

    // Static data:
  private static int            stat_creates= 1;

    // Instance data:
  private Context               m_context   = this;
  private com_api               m_com_api   = null;
  private svc_tap               m_svc_tap   = null;
  private svc_aap               m_svc_aap   = null;

  //private int                 start_type  = START_NOT_STICKY;             // Don't restart if killed; not important enough to restart, which may crash again. ?? But still restarts ??
  private int                   start_type  = START_STICKY;                 // !!!! See if Sticky reduces audio dropouts

  private String []             plst_freq   = new String [com_api.max_presets];
  private String []             plst_name   = new String [com_api.max_presets];
  private int                   preset_curr = 0;
  private int                   preset_num  = 0;

  private Timer                 tuner_state_start_tmr = null;

  private boolean               rfkill_state_set_on = false;

  private AudioManager          m_AM = null;

  private boolean s2_tx             = false;
  private boolean remote_rcc_enable = false;
  private boolean service_update_gui = true;
  private boolean service_update_notification = true;
  private boolean service_update_remote = true;

  @Override
  public void onCreate () {                                             // When service newly created...
    com_uti.logd ("stat_creates: " + stat_creates++);

    try {

      com_uti.devnum_set (m_context);                                   // Set/get device number

      //com_uti.strict_mode_set (true);                                 // Enable strict mode; disabled for now
      com_uti.strict_mode_set (false);                                  // !!!! Disable strict mode so we can send network packets from Java

      m_AM = (AudioManager) m_context.getSystemService (Context.AUDIO_SERVICE);

      s2_tx = com_uti.s2_tx_get ();
      if (s2_tx)
        com_uti.logd ("s2_tx");

      if (s2_tx) {                                                      // If Transmit mode
        remote_rcc_enable = false;
      }
      else if (com_uti.android_version <  21) {                          // Else if receive mode and Kitkat or earlier
// && com_uti.bt_get () ) {     // Needed for lockscreen and AVRCP on BT devices
        remote_rcc_enable = true;
      }
      else if (com_uti.android_version >= 21) {                          // Else if receive mode and Lollipop or later
// Needed for AVRCP on BT devices only ?
        remote_rcc_enable = true;
      }


      if (m_com_api == null) {
        m_com_api = new com_api (this);                                 // Instantiate Common API   class
        com_uti.logd ("m_com_api: " + m_com_api);
      }

      m_svc_aap = new svc_aud (this, this, m_com_api);                  // Instantiate audio        class

      m_svc_tap = new svc_tnr (this, this, m_com_api);                  // Instantiate tuner        class

      String update_gui = com_uti.prefs_get (m_context, "service_update_gui", "");
      if (update_gui.equalsIgnoreCase ("Disable"))
        service_update_gui = false;
      else
        service_update_gui = true;
      String update_notification = com_uti.prefs_get (m_context, "service_update_notification", "");
      if (update_notification.equalsIgnoreCase ("Disable"))
        service_update_notification = false;
      else
        service_update_notification = true;
      String update_remote = com_uti.prefs_get (m_context, "service_update_remote", "");
      if (update_remote.equalsIgnoreCase ("Disable"))
        service_update_remote = false;
      else
        service_update_remote = true;

      //foreground_start ();

    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }

    // Continuing methods in lifecycle order:  (Some don't apply to services, more for activities)

    /*@Override
  public void onStart (Intent intent, int param_int) {
    com_uti.logd ("");
    super.onStart (intent, param_int);
    com_uti.logd ("");
  }*/

  @Override
  public void onDestroy () {
    com_uti.logd ("");

    com_uti.logd ("com_uti.num_daemon_get:              " + com_uti.num_daemon_get);
    com_uti.logd ("com_uti.num_daemon_set:              " + com_uti.num_daemon_set);

    if (m_com_api != null) {
      com_uti.logd ("m_com_api.num_key_set:             " + m_com_api.num_key_set);
      com_uti.logd ("m_com_api.num_api_service_update:  " + m_com_api.num_api_service_update);
    }

    if (! m_com_api.audio_state.equalsIgnoreCase ("stop"))
      com_uti.loge ("destroy with m_com_api.audio_state: " + m_com_api.audio_state);

    if (! m_com_api.tuner_state.equalsIgnoreCase ("stop"))
      com_uti.loge ("destroy with m_com_api.tuner_state: " + m_com_api.tuner_state);

    if (! m_com_api.tuner_api_state.equalsIgnoreCase ("stop"))
      com_uti.loge ("destroy with m_com_api.tuner_api_state: " + m_com_api.tuner_api_state);

    //tuner_state_set ("stop");                                         // This might take a while, and hang, since presumably tuner should already be stopped when onDestroy() is called
                                                                        // Just settle for the error logging above

    foreground_stop ();                                                 // !! match startForeground()


    m_svc_tap = null;
  }


  private void foreground_start () {
    //if (service_update_remote || service_update_notification) // Stops media buttons ?

      com_uti.logd ("Calling startForeground");
      startForeground (com_uti.s2_notif_id, new Notification.Builder (m_context).build ());
  }

  private void foreground_stop () {
      com_uti.logd ("Calling stopForeground");
      stopForeground (true);                                            // Stop Foreground (for audio) and remove notification (true)       !! match startForeground()

  }

  @Override
  public IBinder onBind (Intent arg0) {
   com_uti.logd ("");

   return (null);                                                       // Binding not allowed ; no direct call API, must use Intents
  }

  @Override                                                             // Handle command intents sent via startService()
  public int onStartCommand (Intent intent, int flags, int startId) {   //
    com_uti.logd ("intent: " + intent + "  flags: " + flags + "  startId: " + startId);

    try {

    if (intent == null) {
      com_uti.loge ("intent == null");
      return (start_type);
    }
    String action = intent.getAction ();
    if (action == null) {
      com_uti.loge ("action == null");
      return (start_type);
    }
    if (! action.equalsIgnoreCase (com_uti.api_action_id)) {//"fm.a2d.sf.action.set")) {
      com_uti.loge ("action: " + action);
      return (start_type);
    }

    Bundle extras = intent.getExtras ();
    if (extras == null) {
      com_uti.loge ("NULL extras");
      return (start_type);
    }
    com_uti.logd ("extras: " + extras.describeContents ());

    String val = "";

// audio_android_smo    // ?? setMode == setPhoneState ???
    if (! (val = extras.getString ("audio_android_smo", "")).equalsIgnoreCase (""))
      m_AM.setMode (com_uti.int_get (val));

// audio_android_sso
    if (! (val = extras.getString ("audio_android_sso", "")).equalsIgnoreCase ("")) {
      if (com_uti.int_get (val) == 0)
        m_AM.setSpeakerphoneOn (false);
      else
        m_AM.setSpeakerphoneOn (true);
    }

// audio_android_spa
    if (! (val = extras.getString ("audio_android_spa", "")).equalsIgnoreCase (""))
      m_AM.setParameters (val);

// audio_android_gpa
    if (! (val = extras.getString ("audio_android_gpa", "")).equalsIgnoreCase (""))
      com_uti.logd ("m_AM.getParameters (): " + m_AM.getParameters (val));

// audio_android_sfc
    if (! (val = extras.getString ("audio_android_sfc", "")).equalsIgnoreCase (""))
      com_uti.logd ("com_uti.setForceUse (FOR_COMMUNICATION): " + com_uti.setForceUse (com_uti.FOR_COMMUNICATION, com_uti.int_get (val)));
// audio_android_sfm
    if (! (val = extras.getString ("audio_android_sfm", "")).equalsIgnoreCase (""))
      com_uti.logd ("com_uti.setForceUse (FOR_MEDIA): "         + com_uti.setForceUse (com_uti.FOR_MEDIA,         com_uti.int_get (val)));
// audio_android_sfr
    if (! (val = extras.getString ("audio_android_sfr", "")).equalsIgnoreCase (""))
      com_uti.logd ("com_uti.setForceUse (FOR_RECORD): "        + com_uti.setForceUse (com_uti.FOR_RECORD,        com_uti.int_get (val)));
// audio_android_sfd
    if (! (val = extras.getString ("audio_android_sfd", "")).equalsIgnoreCase (""))
      com_uti.logd ("com_uti.setForceUse (FOR_DOCK): "          + com_uti.setForceUse (com_uti.FOR_DOCK,          com_uti.int_get (val)));


// audio_android_gdc
    if (! (val = extras.getString ("audio_android_gdc", "")).equalsIgnoreCase (""))
      com_uti.logd ("com_uti.getDeviceConnectionState (): " + com_uti.getDeviceConnectionState (com_uti.int_get (val), ""));
// audio_android_sdu
    if (! (val = extras.getString ("audio_android_sdu", "")).equalsIgnoreCase (""))
      com_uti.logd ("com_uti.setDeviceConnectionState (UNAVAILABLE): " + com_uti.setDeviceConnectionState (com_uti.int_get (val), com_uti.DEVICE_STATE_UNAVAILABLE, ""));
// audio_android_sda
    if (! (val = extras.getString ("audio_android_sda", "")).equalsIgnoreCase (""))
      com_uti.logd ("com_uti.setDeviceConnectionState (AVAILABLE): " + com_uti.setDeviceConnectionState (com_uti.int_get (val), com_uti.DEVICE_STATE_AVAILABLE, ""));



// tuner_seek_state
    val = extras.getString ("tuner_seek_state", "");
    if (! val.equals ("")) {
      //com_uti.loge ("preset related tuner_seek_state val: " + val);
      m_svc_tap.tuner_set ("tuner_seek_state", val);
    }

// service_seek_state : preset or seek
    val = extras.getString ("service_seek_state", "");
    //com_uti.loge ("preset or seek extras.getString (\"service_seek_state\", \"\") val: " + val);
    //com_uti.loge ("preset_num: " + preset_num);
    //com_uti.loge ("preset_curr: " + preset_curr);
    if (val.equalsIgnoreCase ("down")) {
      if (preset_num <= 1) {
        m_svc_tap.tuner_set ("tuner_seek_state", val);
      }
      else
        preset_next (false);
    }
    else if (val.equalsIgnoreCase ("up")) {
      if (preset_num <= 1)
        m_svc_tap.tuner_set ("tuner_seek_state", val);
      else
        preset_next (true);
    }
    else if (val.equalsIgnoreCase ("")) {
    }
    else {
    }

    // Tuner:
    val = extras.getString ("tuner_state", "");
    if (! val.equals (""))
      tuner_state_set (val);

    val = extras.getString ("tuner_freq", "");
    if (! val.equals (""))
      tuner_freq_set (val);

    val = extras.getString ("tuner_band", "");
    if (! val.equals ("")) {
      m_svc_tap.tuner_set ("tuner_band", val);
      com_uti.prefs_set (m_context, "tuner_band", val);
      m_com_api.tuner_band = val;
      com_uti.tnru_band_set (m_com_api.tuner_band);
    }

    val = extras.getString ("tuner_stereo", "");
    if (! val.equals ("")) {
      m_svc_tap.tuner_set ("tuner_stereo", val);
      com_uti.prefs_set (m_context, "tuner_stereo", val);
    }

    val = extras.getString ("tuner_extension", "");
    if (! val.equals ("")) {
      m_svc_tap.tuner_set ("tuner_extension", val);
    }

    val = extras.getString ("tuner_rds_state", "");
    if (! val.equals ("")) {
      tuner_rds_state_set (val);
    }

    val = extras.getString ("tuner_rds_af_state", "");
    if (! val.equals ("")) {
      tuner_rds_af_state_set (val);
    }

    val = extras.getString ("tuner_vol", "");
    if (! val.equals (""))
      com_uti.daemon_set ("tuner_vol", val);

    com_uti.extras_daemon_set ("tuner_rds_pi", extras);
    com_uti.extras_daemon_set ("tuner_rds_pt", extras);
    com_uti.extras_daemon_set ("tuner_rds_ps", extras);
    com_uti.extras_daemon_set ("tuner_rds_rt", extras);

    com_uti.extras_daemon_set ("tuner_acdb", extras);


    // Audio:
    val = extras.getString ("audio_mode", "");
    if (! val.equals ("")) {
      m_svc_aap.audio_mode_set (val);
      com_uti.prefs_set (m_context, "audio_mode", m_com_api.audio_mode);
    }

    val = extras.getString ("audio_state", "");
    if (! val.equals (""))
      audio_state_set (val);

    val = extras.getString ("audio_digital_amp", "");
    if (! val.equals ("")) {
      m_svc_aap.audio_digital_amp_set ();//val);
      //com_uti.prefs_set (m_context, "audio_digital_amp", m_com_api.audio_digital_amp);
    }

    val = extras.getString ("audio_output", "");
    if (! val.equals ("")) {
      if (m_com_api.audio_state.equalsIgnoreCase ("start"))             // If audio is started...
        m_svc_aap.audio_output_set (val, true);                         // Set new audio output with restart
                                                                        // Save audio output preference     !! Toggle may be converted
      com_uti.prefs_set (m_context, "audio_output", m_com_api.audio_output);
    }

    val = extras.getString ("audio_stereo", "");
    if (! val.equals ("")) {
      m_svc_aap.audio_stereo_set (val);
      com_uti.prefs_set (m_context, "audio_stereo", val);
    }

    val = extras.getString ("audio_record_state", "");
    if (! val.equals (""))
      m_svc_aap.audio_record_state_set (val);

    val = extras.getString ("service_update_gui", "");
    if (val.equalsIgnoreCase ("Disable"))
      service_update_gui = false;
    else
      service_update_gui = true;
    val = extras.getString ("service_update_notification", "");
    if (val.equalsIgnoreCase ("Disable"))
      service_update_notification = false;
    else
      service_update_notification = true;
    val = extras.getString ("service_update_remote", "");
    if (val.equalsIgnoreCase ("Disable"))
      service_update_remote = false;
    else
      service_update_remote = true;

    service_update_send ();                                             // Update GUI/Widget/Remote/Notification with latest data

    }
    catch (Throwable e) {
      e.printStackTrace ();
    }

    return (start_type);
  }


    // For state and status updates:

  private PowerManager pmgr = null;

  private void displays_update (String caller) {                        // Update all "displays"

    if (pmgr == null)
      pmgr = (PowerManager) m_context.getSystemService (Context.POWER_SERVICE);

    boolean screen_on = pmgr.isScreenOn ();
    if (! screen_on)
      return;

    com_uti.logw ("caller: " + caller);

    if (screen_on && service_update_gui) {
      Intent service_update_intent = service_update_send ();            // Send Intent to send and Update widgets, GUI(s), other components and 
      m_com_api.api_service_update (service_update_intent);             // Update our copy of data in Radio API using Intent
    }

    if (screen_on && service_update_notification) {
    }

  }


  private void tuner_extras_put (Intent intent) {

    intent.putExtra ("tuner_state",        m_com_api.tuner_state);      // m_svc_tap.tuner_get ("tuner_state"));
    intent.putExtra ("tuner_band",         m_com_api.tuner_band);       // m_svc_tap.tuner_get ("tuner_band"));

    // Just Use cached value now; don't go to daemon
    //    String freq_khz = m_svc_tap.tuner_get ("tuner_freq");

    String freq_khz = m_com_api.tuner_freq;

    int ifreq = com_uti.int_get (freq_khz);

    //!! ifreq = com_uti.tnru_freq_fix (ifreq + 25);

    if (ifreq >= 50000 && ifreq < 500000) {
      m_com_api.tuner_freq = ("" + (double) ifreq / 1000);
      m_com_api.tuner_freq_int = ifreq;
    }
    com_uti.logv ("m_com_api.tuner_freq: " + m_com_api.tuner_freq + "  m_com_api.tuner_freq_int: " + m_com_api.tuner_freq_int);
    intent.putExtra ("tuner_freq",         m_com_api.tuner_freq);



    //intent.putExtra ("tuner_stereo",       m_svc_tap.tuner_get ("tuner_stereo"));
    //intent.putExtra ("tuner_thresh",       m_svc_tap.tuner_get ("tuner_thresh"));
    //intent.putExtra ("tuner_seek_state",   m_svc_tap.tuner_get ("tuner_seek_state"));

    //intent.putExtra ("tuner_rds_state",    m_svc_tap.tuner_get ("tuner_rds_state"));
    //intent.putExtra ("tuner_rds_af_state", m_svc_tap.tuner_get ("tuner_rds_af_state"));
    //intent.putExtra ("tuner_rds_ta_state", m_svc_tap.tuner_get ("tuner_rds_ta_state"));

    //intent.putExtra ("tuner_extension",          m_svc_tap.tuner_get ("tuner_extension"));

    intent.putExtra ("tuner_rssi",         m_com_api.tuner_rssi);      //m_svc_tap.tuner_get ("tuner_rssi"));
    //intent.putExtra ("tuner_qual",         m_svc_tap.tuner_get ("tuner_qual"));
    intent.putExtra ("tuner_pilot ",         m_com_api.tuner_pilot);      //m_svc_tap.tuner_get ("tuner_pilot "));

    intent.putExtra ("tuner_rds_pi",       m_com_api.tuner_rds_pi);    //m_svc_tap.tuner_get ("tuner_rds_pi"));
    intent.putExtra ("tuner_rds_picl",     m_com_api.tuner_rds_picl);  //m_svc_tap.tuner_get ("tuner_rds_picl"));
    //intent.putExtra ("tuner_rds_pt",       m_com_api.tuner_rds_pt);  //m_svc_tap.tuner_get ("tuner_rds_pt"));
    intent.putExtra ("tuner_rds_ptyn",     m_com_api.tuner_rds_ptyn);  //m_svc_tap.tuner_get ("tuner_rds_ptyn"));
    intent.putExtra ("tuner_rds_ps",       m_com_api.tuner_rds_ps);    //m_svc_tap.tuner_get ("tuner_rds_ps"));
    intent.putExtra ("tuner_rds_rt",       m_com_api.tuner_rds_rt);    //m_svc_tap.tuner_get ("tuner_rds_rt"));

    //intent.putExtra ("tuner_rds_af",       m_svc_tap.tuner_get ("tuner_rds_af"));
    //intent.putExtra ("tuner_rds_ms",       m_svc_tap.tuner_get ("tuner_rds_ms"));
    //intent.putExtra ("tuner_rds_ct",       m_svc_tap.tuner_get ("tuner_rds_ct"));
    //intent.putExtra ("tuner_rds_tmc",      m_svc_tap.tuner_get ("tuner_rds_tmc"));
    //intent.putExtra ("tuner_rds_tp",       m_svc_tap.tuner_get ("tuner_rds_tp"));
    //intent.putExtra ("tuner_rds_ta",       m_svc_tap.tuner_get ("tuner_rds_ta"));
    //intent.putExtra ("tuner_rds_taf",      m_svc_tap.tuner_get ("tuner_rds_taf"));
  }

  private Intent service_update_send () {                               // Send all radio state & status info

    m_svc_aap.audio_sessid_get ();                                      // Better to update here or in svc_tnr where polling takes place ?

    com_uti.logv ("audio_state: " + m_com_api.audio_state + "  audio_output: " + m_com_api.audio_output + "  audio_stereo: " + m_com_api.audio_stereo + "  audio_record_state: " + m_com_api.audio_record_state);

    Intent intent = new Intent (com_uti.api_result_id);//"fm.a2d.sf.result.get");           // Create a new broadcast result Intent
                                                                        // Send service data
    for (int ctr = 0; ctr < preset_num; ctr ++) {                       // Send preset list
      intent.putExtra ("service_preset_freq_" + ctr, plst_freq [ctr]);
      intent.putExtra ("service_preset_name_" + ctr, plst_name [ctr]);
    }
                                                                        // Send audio data
    intent.putExtra ("audio_state",        m_com_api.audio_state);
    intent.putExtra ("audio_output",       m_com_api.audio_output);
    intent.putExtra ("audio_stereo",       m_com_api.audio_stereo);
    intent.putExtra ("audio_record_state", m_com_api.audio_record_state);
    intent.putExtra ("audio_sessid",       m_com_api.audio_sessid);
                                                                        // Send tuner data
    if (m_svc_tap == null)
      intent.putExtra ("tuner_state",      "stop");
    else
      tuner_extras_put (intent);

    m_com_api.service_update_send (intent, "", "");//m_com_api.service_phase, m_com_api.service_cdown); // Send Phase Update + More

    return (intent);
  }


    // Presets:



  private int preset_curr_set (String freq) {                           // Set preset_curr to a valid preset index for passed frequency
    com_uti.logd ("freq: " + freq);
    if (preset_num > 0) {                                               // If we have any presets...
      if (freq.equals (plst_freq [preset_curr])) {                      // If current preset and passed frequency matches...
        com_uti.logd ("freq.equals (plst_freq [preset_curr] returning current preset_curr: " + preset_curr);
        return (preset_curr);
      }
      for (int ctr = 0; ctr < preset_num; ctr ++) {
        if (freq.equals (plst_freq [ctr])) {
          preset_curr = ctr;
          com_uti.logd ("freq.equals (plst_freq [ctr] returning preset_curr = ctr: " + preset_curr);
          return (preset_curr);
        }
      }
    }
    preset_curr = 0;                                                    // Just use 0; may not be associated with a preset
    com_uti.logd ("Frequency not found so returning preset_curr = 0: " + preset_curr);
    return (preset_curr);
  }

  private int preset_curr_fix () {
    if (preset_num < 0)                                                 // First ensure number of presets is sane
      preset_num = 0;
    if (preset_num >= com_api.max_presets)
      preset_num = com_api.max_presets;

    if (preset_curr < 0)                                                // Ensure preset_curr wraps to valid values
      preset_curr = preset_num - 1;
    if (preset_curr > preset_num - 1)
      preset_curr = 0;
    return (preset_curr);
  }

  private void preset_next (boolean up) {                               // Tune to next preset, up (true) or down
    com_uti.logd ("start preset_curr: " + preset_curr + "  preset_num: " + preset_num);
    if (preset_num <= 0)                                                // If no presets, done
      return;
    preset_curr_fix ();                                                 // First fix any problems
    preset_curr_set (m_com_api.tuner_freq);
    if (up)
      preset_curr ++;
    else
      preset_curr --;
    preset_curr_fix ();                                                 // Fix any problems
    String freq = plst_freq [preset_curr];
    double freq_dou = com_uti.double_get (freq);
    int freq_int = (int) (freq_dou * 1000);
    com_uti.logd ("freq: " + freq + "  freq_int: " + freq_int + "  freq_dou: " + freq_dou + "  preset_curr: " + preset_curr + "  preset_num: " + preset_num);
    if (! freq.equals (""))// && freq_int >= 50000 && freq_int <= 108000)
      tuner_freq_set (freq);
  }


    // AUDIO:

  private boolean need_audio_start_after_tuner_start = false;

  private String audio_state_set (String desired_state) {               // Called only by onStartCommand()
    com_uti.logd ("desired_state: " + desired_state);
    if (desired_state.equalsIgnoreCase ("toggle")) {                    // TOGGLE:
      if (m_com_api.audio_state.equalsIgnoreCase ("start"))
        desired_state = "pause";
      else
        desired_state = "start";
    }
                                                                        // If Audio Stop or Pause...
    if (desired_state.equalsIgnoreCase ("Stop") || desired_state.equalsIgnoreCase ("Pause")) {
      m_svc_aap.audio_state_set (desired_state);                        // Set Audio State synchronously
      return (m_com_api.audio_state);                                   // Return current audio state
    }

    if (desired_state.equalsIgnoreCase ("Start")) {
    }
    else {
      com_uti.loge ("Unexpected desired_state: " + desired_state);
      return (m_com_api.audio_state);                                   // Return current audio state
    }

                                                                        // Else if Audio Start desired...
    if (m_com_api.audio_state.equalsIgnoreCase ("start"))               // If audio already started...
      return (m_com_api.audio_state);                                   // Return current audio state

                                                                        // Else if audio stopped or paused and we want to start audio...
    String mode = com_uti.prefs_get (m_context, "audio_mode", "Digital");
    m_svc_aap.audio_mode_set (mode);                                    // Set audio mode from prefs, before audio is started

    String stereo = com_uti.prefs_get (m_context, "audio_stereo", "Stereo");
    m_svc_aap.audio_stereo_set (stereo);                                // Set audio stereo from prefs, before audio is started

    if (m_com_api.tuner_state.equalsIgnoreCase ("start")) {             // If tuner started...
      m_svc_aap.audio_state_set ("Start");                              // Set Audio State immediately & synchronously
    }
    else {                                                              // Else if tuner not started...
      need_audio_start_after_tuner_start = true;                        // Signal tuner state callback that audio needs to be started after tuner finishes starting
      tuner_state_set ("Start");                                        // Start tuner first, audio will start later via callback
    }

    return (m_com_api.audio_state);                                     // Return current audio state
  }


    // Audio State callback called only by svc_aud: audio_state_set()

  public void cb_audio_state (String new_audio_state) {                 // Audio state changed callback from svc_aud
    com_uti.logd ("new_audio_state: " + new_audio_state);

    if (new_audio_state.equalsIgnoreCase ("start")) {                   // If new audio state = Start...
//      service_update_send ();                                           // Update GUI/Widget/Remote/Notification with latest data
    }
    else if (new_audio_state.equalsIgnoreCase ("stop")) {               // Else if new audio state = Stop...
//      service_update_send ();                                           // Update GUI/Widget/Remote/Notification with latest data
    }
    else if (new_audio_state.equalsIgnoreCase ("pause")) {              // Else if new audio state = Pause...
//      service_update_send ();                                           // Update GUI/Widget/Remote/Notification with latest data
      // Remote State = Still Playing
    }
    else {
      com_uti.loge ("Unexpected new_audio_state: " + new_audio_state);
      return;
    }

    displays_update ("cb_audio_state");                                 // Update all displays/data sinks
  }


    // TUNER:

  private String tuner_state_set (String desired_state) {               // Called only by onStartCommand(), (maybe onDestroy in future)
    com_uti.logd ("desired_state: " + desired_state);
    if (desired_state.equalsIgnoreCase ("toggle")) {                    // If Toggle...
      if (m_com_api.tuner_state.equalsIgnoreCase ("start"))
        desired_state = "stop";
      else
        desired_state = "start";
    }

    if (desired_state.equalsIgnoreCase ("start")) {                     // If Start...
      tuner_state_start_tmr = new Timer ("tuner start", true);          // One shot Poll timer for file creates, SU commands, Bluedroid Init, then start tuner
      if (tuner_state_start_tmr == null) {                              // If error...
        com_uti.loge ("Fatal error");
        return (m_com_api.tuner_state);
      }                                                                 // Else, Once after 0.01 seconds.
      tuner_state_start_tmr.schedule (new tuner_state_start_tmr_hndlr (), 10);
      return (m_com_api.tuner_state);
    }
    else if (desired_state.equalsIgnoreCase ("stop")) {                 // If Stop...
      m_svc_aap.audio_state_set ("Stop");                               // Set Audio State  synchronously to Stop
      m_svc_tap.tuner_set ("tuner_state", "stop");                      // Set Tuner State asynchronously to Stop
      return (m_com_api.tuner_state);                                   // Return new tuner state
    }

    com_uti.loge ("Unexpected desired_state: " + desired_state);
    return (m_com_api.tuner_state);                                     // Return current tuner state
  }


    // Tuner State callback called only by cb_tuner_key ("tuner_state") which is called for tuner_state only by:
        // svc_tnr:tuner_state_set()        Start, Stop
        // svc_tnr:poll_tmr_hndlr:run()     Start, Stop from daemon (still disabled)

  private void cb_tuner_state (String new_tuner_state) {
    com_uti.logd ("new_tuner_state: " + new_tuner_state);
    if (new_tuner_state.equalsIgnoreCase ("start")) {                   // If tuner state is Start...
      service_update_send ();                                           // Update GUI/Widget/Remote/Notification with latest data
      foreground_start ();
      tuner_prefs_init ();                                              // Load tuner prefs
      if (need_audio_start_after_tuner_start) {                         // If Audio starting...
        need_audio_start_after_tuner_start = false;
        m_svc_aap.audio_state_set ("Start");                            // Set Audio State synchronously
      }
      return;
    }
    else if (new_tuner_state.equalsIgnoreCase ("stop")) {               // Else if tuner state is Stop...
      service_update_send ();                                           // Update GUI/Widget/Remote/Notification with latest data
      foreground_stop ();                                                 // !! match startForeground()

        // Optional, but helps clear out old data/problems:
      com_uti.logd ("Before stopSelf()");
      stopSelf ();                                                      // Stop this service entirely
      com_uti.logd ("After  stopSelf()");

    }
    else {
      com_uti.loge ("Unexpected new_tuner_state: " + new_tuner_state);
      return;
    }

//    displays_update ("cb_tuner_state");                                 // Update all displays/data sinks
  }

  private class tuner_state_start_tmr_hndlr extends TimerTask {

    public void run () {
      int ret = 0;

      ret = files_init ();                                              // /data/data/fm.a2d.sf/files/: busybox, ssd, s.wav, b1.bin, b2.bin
      if (ret != 0)
        com_uti.loge ("files_init IGNORE Errors: " + ret);

      if (com_uti.devnum == com_uti.DEV_OM7 || com_uti.devnum == com_uti.DEV_LG2 || com_uti.devnum == com_uti.DEV_XZ2) {
        m_com_api.service_update_send (null, "Broadcom Bluetooth Init", "10");// Send Phase Update
        ret = bcom_bluetooth_init ();
        if (ret != 0) {
          m_com_api.service_update_send (null, "ERROR Broadcom Bluetooth Init", "-1");// Send Error Update
          return;
        }
      }
      com_uti.logd ("Starting Tuner...");

      m_svc_tap.tuner_set ("tuner_state", "Start");                     // This starts the daemon

      if (tuner_state_start_tmr != null)
        tuner_state_start_tmr.cancel ();                                // Stop one shot poll timer (Not periodic so don't need ?)

      if (m_com_api.tuner_state.equalsIgnoreCase ("start"))             // If tuner started...
        com_uti.logd ("Done Success with m_com_api.tuner_state: " + m_com_api.tuner_state);
      else
        com_uti.loge ("Done Error with m_com_api.tuner_state: " + m_com_api.tuner_state);
    }
  }


    // Other non state machine tuner stuff:

  private void tuner_rds_state_set (String val) {
    m_svc_tap.tuner_set ("tuner_rds_state", val);
    com_uti.prefs_set (m_context, "tuner_rds_state", val);
  }
  private void tuner_rds_af_state_set (String val) {
    m_svc_tap.tuner_set ("tuner_rds_af_state", val);
    com_uti.prefs_set (m_context, "tuner_rds_af_state", val);
  }

  private void tuner_prefs_init () {                                    // Load tuner prefs
    String band = com_uti.prefs_get (m_context, "tuner_band", "EU");
    m_svc_tap.tuner_set ("tuner_band", band);
    m_com_api.tuner_band = band;
    com_uti.tnru_band_set (band);

    String stereo = com_uti.prefs_get (m_context, "tuner_stereo", "Stereo");
    m_svc_tap.tuner_set ("tuner_stereo", stereo);

    tuner_rds_state_set     (com_uti.prefs_get (m_context, "tuner_rds_state",    "Start")); // !! Always rewrites pref
    tuner_rds_af_state_set  (com_uti.prefs_get (m_context, "tuner_rds_af_state", "Stop"));  // !! Always rewrites pref


    int freq = com_uti.prefs_get (m_context, "tuner_freq", 88500);
    tuner_freq_set ("" + freq);                                         // Set initial frequency
  }


  private void tuner_freq_set (String freq) {//int freq) {                              // To fix float problems w/ 106.1 becoming 106099
    com_uti.logd ("freq: " + freq);
    int ifreq = com_uti.tnru_band_new_freq_get (freq, m_com_api.tuner_freq_int); // Deal with up, down, etc.

    //int ifreq = com_uti.tnru_khz_get (freq);

    ifreq += 25;                                                        // Round up...
    ifreq = ifreq / 50;                                                 // Nearest 50 KHz
    ifreq *= 50;                                                        // Back to KHz scale
    com_uti.logd ("ifreq: " + ifreq);

    m_svc_tap.tuner_set ("tuner_freq", "" + ifreq);                     // Set frequency
  }



    // Tuner API callbacks:


    // Single Tuner Sub-Service callback expands to other functions:

  public void cb_tuner_key (String key, String val) {
    com_uti.logv ("key: " + key + "  val: " + val);

    if (key == null)
      com_uti.loge ("key: " + key);
    else if (key.equalsIgnoreCase ("tuner_state"))
      cb_tuner_state (val);
    else if (key.equalsIgnoreCase ("tuner_freq"))
      cb_tuner_freq (val);
    else if (key.equalsIgnoreCase ("tuner_rssi"))
      cb_tuner_rssi (val);
    else if (key.equalsIgnoreCase ("tuner_pilot "))
      cb_tuner_pilot(val);
    else if (key.equalsIgnoreCase ("tuner_qual"))
      cb_tuner_qual (val);
    else if (key.equalsIgnoreCase ("tuner_rds_pi"))
      cb_tuner_rds_pi (val);
    else if (key.equalsIgnoreCase ("tuner_rds_pt"))
      cb_tuner_rds_pt (val);
    else if (key.equalsIgnoreCase ("tuner_rds_ps"))
      cb_tuner_rds_ps (val);
    else if (key.equalsIgnoreCase ("tuner_rds_rt"))
      cb_tuner_rds_rt (val);
    else
      com_uti.loge ("key: " + key);
  }

// Freq:
  private void cb_tuner_freq (String freq) {

    com_uti.logd ("freq: " + freq);

    m_com_api.tuner_stereo = "";

    m_com_api.tuner_rssi         = "";//999";                            // ro ... ... Values:   RSSI: 0 - 1000
    m_com_api.tuner_qual         = "";//SN 99";                          // ro ... ... Values:   SN 99, SN 30
    m_com_api.tuner_pilot        = "";//Mono";                           // ro ... ... Values:   mono, stereo, 1, 2, blend, ... ?      1.5 ?

    m_com_api.tuner_rds_pi       = "";//-1";                             // ro ... ... Values:   0 - 65535
    m_com_api.tuner_rds_picl     = "";//WKBW";                           // ro ... ... Values:   North American Call Letters or Hex PI for tuner_rds_pi
    m_com_api.tuner_rds_pt       = "";//-1";                             // ro ... ... Values:   0 - 31
    m_com_api.tuner_rds_ptyn     = "";//";                               // ro ... ... Values:   Describes tuner_rds_pt (English !)
    m_com_api.tuner_rds_ps       = "";//SpiritF";                        // ro ... ... Values:   RBDS 8 char info or RDS Station
    m_com_api.tuner_rds_rt       = "";//Thanks for Your Support... :)";  // ro ... ... Values:   64 char

    m_com_api.tuner_rds_af       = "";//";                               // ro ... ... Values:   Space separated array of AF frequencies
    m_com_api.tuner_rds_ms       = "";//";                               // ro ... ... Values:   0 - 65535   M/S Music/Speech switch code
    m_com_api.tuner_rds_ct       = "";//";                               // ro ... ... Values:   14 char CT Clock Time & Date

    m_com_api.tuner_rds_tmc      = "";//";                               // ro ... ... Values:   Space separated array of shorts
    m_com_api.tuner_rds_tp       = "";//";                               // ro ... ... Values:   0 - 65535   TP Traffic Program Identification code
    m_com_api.tuner_rds_ta       = "";//";                               // ro ... ... Values:   0 - 65535   TA Traffic Announcement code
    m_com_api.tuner_rds_taf      = "";//";                               // ro ... ... Values:   0 - 2^32-1  TAF TA Frequency

    int new_freq = com_uti.tnru_freq_fix (25 + com_uti.tnru_khz_get (freq));
    m_com_api.tuner_freq = com_uti.tnru_mhz_get (new_freq);
    m_com_api.tuner_freq_int = new_freq;

    displays_update ("cb_tuner_freq");

    com_uti.prefs_set (m_context, "tuner_freq", new_freq);
  }
// RSSI:
  private void cb_tuner_rssi (String rssi) {
    //com_uti.loge ("rssi: " + rssi);
    //displays_update ("cb_tuner_rssi");
  }
// pilot:
  private void cb_tuner_pilot (String pilot) {
    //com_uti.loge ("pilot: " + pilot);
    //displays_update ("cb_tuner_pilot ");
  }
// Qual:
  private void cb_tuner_qual (String qual) {
    com_uti.loge ("qual: " + qual);
    //displays_update ("cb_tuner_qual");
  }
// RDS:
  private void cb_tuner_rds_pi (String pi) {
    displays_update ("cb_tuner_rds_pi");
  }
  private void cb_tuner_rds_pt (String pt) {
    displays_update ("cb_tuner_rds_pt");
  }
  private void cb_tuner_rds_ps (String ps) {
    displays_update ("cb_tuner_rds_ps");
  }
  private void cb_tuner_rds_rt (String rt) {
    displays_update ("cb_tuner_rds_rt");
  }


  private String lib_name_get () {
    switch (com_uti.devnum) {
      case com_uti.DEV_UNK: return ("libs2t_gen.so");
      case com_uti.DEV_GEN: return ("libs2t_gen.so");
      case com_uti.DEV_GS1: return ("libs2t_ssl.so");
      case com_uti.DEV_GS2: return ("libs2t_ssl.so");
      case com_uti.DEV_GS3: return ("libs2t_ssl.so");
      case com_uti.DEV_QCV: return ("libs2t_qcv.so");
      case com_uti.DEV_OM7: return ("libs2t_bch.so");
      case com_uti.DEV_LG2: return ("libs2t_bch.so");
      case com_uti.DEV_XZ2: return ("libs2t_bch.so");
    }
    return ("libs2t_gen.so");
  }

    // Hardware / API dependent part of svc_svc:

  private int files_init () {
    com_uti.logd ("starting...");

    //String wav_full_filename = com_uti.file_create (m_context, R.raw.s_wav,    "s.wav",         false);             // Not executable
    //String bb1_full_filename = com_uti.file_create (m_context, R.raw.b1_bin,     "b1.bin",        false);             // Not executable
    //String bb2_full_filename = com_uti.file_create (m_context, R.raw.b2_bin,     "b2.bin",        false);             // Not executable

    String add_full_filename = com_uti.file_create (m_context, R.raw.spirit_sh,  "99-spirit.sh",  true);
        // Check:
    int ret = 0;

      if (! com_uti.access_get (add_full_filename, false, false, true)) { // rwX
        com_uti.loge ("error unexecutable addon.d script 99-spirit.sh");
        ret ++;
      }
/*
      if (! com_uti.access_get (bb1_full_filename, true, false, false)) { // Rwx
        com_uti.loge ("error inaccessible bb1 file");
        ret ++;
      }
      if (! com_uti.access_get (bb2_full_filename, true, false, false)) { // Rwx
        com_uti.loge ("error inaccessible bb2 file");
        ret ++;
      }
*/
      com_uti.logd ("done ret: " + ret);
    return (ret);
  }

  private int bcom_bluetooth_init () {    // Install shim if needed (May change BT state & warm restart). Determine UART or SHIM Mode & create/delete "use_shim" flag file for s2d. Turn BT off if needed.
    com_uti.logd ("start");

    m_com_api.tuner_api_mode = "UART";                                           // Default = UART MODE
/*
    String short_filename = "use_shim";
    String full_filename = m_context.getFilesDir () + "/" + short_filename;

    if (com_uti.file_get (full_filename)) {                             // If use_shim flag is set...
      com_uti.logd ("Removing file: " + full_filename);
      //File.delete (full_filename); //com_uti.sys_WAS_run ("rm " + full_filename, true);                    // Remove file/flag

      File dir = m_context.getFilesDir ();
      File file = new File (dir, short_filename);
      boolean deleted = file.delete ();
      if (deleted)
        com_uti.logd ("deleted: " + deleted);
      else
        com_uti.loge ("deleted: " + deleted);
    }
*/
    if (com_uti.shim_files_possible_get ()) {                           // If Bluedroid...
      com_uti.logd ("Bluedroid support");

      boolean fresh_shim_install = false;                               // !!!! Need code to update shim when it changes !!!!   (Stable Jan 1, 2014 -> July 31/Nov 30, 2014)

      boolean unfriendly_auto_install_and_reboot = false;
      if (unfriendly_auto_install_and_reboot) {
      if (! com_uti.shim_files_operational_get ()) {                    // If shim files not operational...
        if (com_uti.bt_get ()) {                                        // July 31, 2014: only install shim if BT is on
          com_uti.bt_set (false, true);                                 // Bluetooth off, and wait for off
          com_uti.logd ("Start 4 second delay after BT Off");
          com_uti.ms_sleep (4000);                                      // Extra 4 second delay to ensure BT is off !!
          com_uti.logd ("End 4 second delay after BT Off");

          com_uti.shim_install ();                                      // Install shim

          com_uti.bt_set (true, true);                                  // Bluetooth on, and wait for on  (Need to set BT on so reboot has it on.)

            //Toast.makeText (m_context, "WARM RESTART PENDING FOR SHIM INSTALL !!", Toast.LENGTH_LONG).show ();  java.lang.RuntimeException: Can't create handler inside thread that has not called Looper.prepare()
        /* Don't need a delay before reboot because BT is on enough to stay on after reboot ?? (And we waited for On to be detected anyway)
            com_uti.logd ("Start 4 second delay after BT On");
            com_uti.ms_sleep (4000);                                    // Extra 4 second delay to ensure BT is on
            com_uti.logd ("End 4 second delay after BT On"); */
            //Toast.makeText (m_context, "WARM RESTART !!", Toast.LENGTH_LONG).show ();
            //com_uti.sys_WAS_run ("kill `pidof system_server`", true);

          com_uti.sys_run ("reboot now", true);                         // M7 GPE requires reboot

          fresh_shim_install = true;
        }
      }
      }//if (unfriendly_auto_install_and_reboot) {

      if (! fresh_shim_install && com_uti.shim_files_operational_get () && com_uti.bt_get ()) {     // If not fresh shim install, and shim files operational, and BT is on...
        com_uti.logd ("Bluedroid shim installed & BT on");
        try {
          /*FileOutputStream fos = m_context.openFileOutput (short_filename, Context.MODE_PRIVATE);   // | MODE_WORLD_WRITEABLE      // Somebody got a NullPointerException here
          if (fos != null) {
            fos.close ();*/                                               // Create "use_shim" flag file for lower level
            m_com_api.tuner_api_mode = "SHIM";                                          // SHIM MODE if success
          //}
        }
        catch (Throwable e) {
          com_uti.loge ("Exception, try UART mode");
          e.printStackTrace ();
        }
      }
    }

    if (m_com_api.tuner_api_mode.equalsIgnoreCase ("UART")) {
      if (com_uti.bt_get ()) {
        com_uti.logd ("UART mode needed but BT is on; turn BT Off");
        com_uti.bt_set (false, true);                                   // Bluetooth off, and wait for off
        com_uti.logd ("Start 4 second delay after BT Off");
        com_uti.ms_sleep (4000);                                        // Extra 4 second delay to ensure BT is off !!
        com_uti.logd ("End 4 second delay after BT Off");
      }
/* Back in acc_hci
      rfkill_state_set (1);                                             // BT UART Power = ON
      rfkill_state_set (1);                                             // Original code did twice
*/
    }
// BT is? KitKat:   service call bluetooth_manager 4                                                TRANSACTION_isEnabled
// BT On  KitKat:   service call bluetooth_manager 6        Older:  service call bluetooth 3
// BT On- KitKat:   service call bluetooth_manager 7                                                TRANSACTION_enableNoAutoConnect
// BT Off KitKat:   service call bluetooth_manager 8        Older:  service call bluetooth 4


    com_uti.logd ("done m_com_api.tuner_api_mode: " + m_com_api.tuner_api_mode);
    return (0);
  }

/*
  private String rfkill_state_get () {
    String state = "0";
    try {
      state = (new BufferedReader (new FileReader ("/sys/class/rfkill/rfkill0/state"))).readLine ();
      com_uti.logd ("Read rfkill0/state: " + state);
    }
    catch (Throwable e) {
      e.printStackTrace ();
      com_uti.loge ("Read rfkill0/state Exception");
    }
    return (state);
  }

  private int rfkill_state_set (int state) {
    com_uti.logd ("state: " + state);
    if (state != 0) {                                                   // If turning on...
      //rfkill_state_set_on = true;
    }
    else {                                                              // Else if turning off...
      if (! rfkill_state_set_on)                                        // If was not previously off...
        return (0);                                                     // Done
    }
    rfkill_state_get ();                                                // Display rfkill state
    String [] cmds = {("")};
    cmds [0] = ("echo " + state + " > /sys/class/rfkill/rfkill0/state");
    com_uti.sys_WAS_run (cmds, true);                                         // Set rfkill state WITH SU/Root
    rfkill_state_get ();                                                // Display rfkill state
    if (state != 0)                                                     // If turning on...
      rfkill_state_set_on = true;
    else
      rfkill_state_set_on = false;
    return (0);
  }
*/

}

