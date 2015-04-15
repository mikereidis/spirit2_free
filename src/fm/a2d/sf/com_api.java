
    // Radio Service API:

package fm.a2d.sf;

import android.app.PendingIntent;
import android.content.Intent;
import android.content.Context;
import android.os.Bundle;

public class com_api {

  public static Context m_context = null;

    // Public stats:
  public int            num_api_service_update  = 0;
  public int            num_key_set             = 0;


    // Data / Status / Command that form the Common API:

    // Chassis:   Virtual chassis/box/frame that encloses Tuner and Audio functions and brings them together:
        // Tuner:   Radio Frequency Tuner Service & Chip, including audio functions like mute, volume and stereo that are on the FM chip
        // Audio:   Sound Frequency Audio Service & Chip(s), SOC audio and other audio


    // Chassis:

  public  String chass_phase        = "Pre Init";                       // Phase of startup/shutdown for tuner, audio and related components    "Starting", "Stopping" and "ERROR" and "Success" messages
  public  String chass_phtmo        = "0";                              // Phase Timeout seconds if positive, Success if 0, Error code if negative

  public static final int chass_preset_max  = 16;                       // Maximum presets, preset frequencies and names
  public String[]chass_preset_freq  =  {"","","","","","","","","","","","","","","","",};
  public String[]chass_preset_name  =  {"","","","","","","","","","","","","","","","",};

  public  String chass_plug_aud     = "UNK";                            // Audio Plugin
  public  String chass_plug_tnr     = "UNK";                            // Tuner Plugin


    // Audio: Sound Frequency Audio Service & Chip(s)

  public String  audio_state        = "Stop";
  public String  audio_mode         = "Digital";
  public String  audio_output       = "Headset";
  public String  audio_stereo       = "Stereo";
  public String  audio_record_state = "Stop";                       // Stop, Start
  public String  audio_sessid       = "0";


    // Tuner: Radio Frequency Tuner Service & Chip(s)

                                                                        // !!!! Need to update these descriptions:
                                                                        //    CFG = Saved in config
                                                                        //    ... = ephemeral non-volatile
                                                                        //        api = get from FM API
                                                                        //        set = get from set of this variable
                                                                        //        mul = multiple/both
                                                                        //        ... = RO
                                                                        // RW CFG api for tuner_freq & tuner_thresh consistency issues: CFG vs chip current values

  public  String tuner_state        = "Stop";                           // RW ... api States:   stop, start, pause, resume

  public  String tuner_mode         = "Receive";
  public  String tuner_api_mode     = "UART";                           // Default "" or "UART" or "SHIM"
  public  String tuner_api_state    = "Stop";

  public  String tuner_band         = "";                               // RW CFG set Values:   US, EU, UU
  public  String tuner_freq         = "";                               // RW CFG api Values:   String  form: 50.000 - 499.999  (76-108 MHz)
  public int     tuner_freq_int     = 0;                                // ""                 Integer form in kilohertz

  public  String tuner_stereo       = "";                               // RW CFG set Values:   mono, stereo, switch, blend, ... ?
  public  String tuner_thresh       = "";                               // RW CFG api Values:   Seek/scan RSSI threshold
  public  String tuner_seek_state   = "Stop";                           // RW ... set States:   stop, up, down

  public  String tuner_rds_state    = "Stop";                           // RW CFG set States:   on, off
  public  String tuner_rds_af_state = "Stop";                           // RW CFG set States:   on, off
  public  String tuner_rds_ta_state = "Stop";                           // RW CFG set States:   on, off

  public  String tuner_extension    = "";                               // RW ... set Values:   Extension (additional commands)

  public  String tuner_rssi         = "";                               // ro ... ... Values:   RSSI: 0 - 1000
  public  String tuner_qual         = "";                               // ro ... ... Values:   SN 99, SN 30
  public  String tuner_pilot        = "";                               // ro ... ... Values:   mono, stereo, 1, 2, blend, ... ?      1.5 ?

  public  String tuner_rds_pi       = "";                               // ro ... ... Values:   0 - 65535
  public  String tuner_rds_picl     = "";                               // ro ... ... Values:   North American Call Letters or Hex PI for tuner_rds_pi
  public  String tuner_rds_pt       = "";                               // ro ... ... Values:   0 - 31
  public  String tuner_rds_ptyn     = "";                               // ro ... ... Values:   Describes tuner_rds_pt (English !)
  public  String tuner_rds_ps       = "SpiritF";                               // ro ... ... Values:   RBDS 8 char info or RDS Station
  public  String tuner_rds_rt       = "";                               // ro ... ... Values:   64 char

  public  String tuner_rds_af       = "";                               // ro ... ... Values:   Space separated array of AF frequencies
  public  String tuner_rds_ms       = "";                               // ro ... ... Values:   0 - 65535   M/S Music/Speech switch code
  public  String tuner_rds_ct       = "";                               // ro ... ... Values:   14 char CT Clock Time & Date

  public  String tuner_rds_tmc      = "";                               // ro ... ... Values:   Space separated array of shorts
  public  String tuner_rds_tp       = "";                               // ro ... ... Values:   0 - 65535   TP Traffic Program Identification code
  public  String tuner_rds_ta       = "";                               // ro ... ... Values:   0 - 65535   TA Traffic Announcement code
  public  String tuner_rds_taf      = "";                               // ro ... ... Values:   0 - 2^32-1  TAF TA Frequency


    // Code:

  private static int m_obinits = 0;
  public com_api (Context context) {                                    // Context constructor
    m_obinits ++;
    com_uti.logd ("m_obinits: " + m_obinits);
    m_context = context;
    com_uti.logd ("context: " + context);
  }

  private static int request_code = 777;
  public static PendingIntent pend_intent_get (Context context, String key, String val) {
    Intent intent = new Intent (com_uti.api_action_id);
    com_uti.logv ("context: " + context + "  m_context: " + m_context + "  intent: " + intent + "  key: " + key + "  val: " + val);
    intent.setClass (context, svc_svc.class);                           // !! Note possible different context and m_context !!
    intent.putExtra (key, val);
    request_code ++;                                                    // Need a unique value to identify
    PendingIntent pi = PendingIntent.getService (context, request_code, intent, PendingIntent.FLAG_UPDATE_CURRENT);
    return (pi);
  }
///*
  public void key_set (String key, String val, String key2, String val2) {  // Presets currently require simultaneous preset frequency and name
    num_key_set ++;
    com_uti.logd ("key: " + key + "  val: " + val + "  key2: " + key2 + "  val2: " + val2);
    Intent intent = new Intent (com_uti.api_action_id);
    //if (intent == null) {
    //  com_uti.loge ("intent == null");
    //  return;
    //}
    intent.setClass (m_context, svc_svc.class);
    intent.putExtra (key, val);
    intent.putExtra (key2, val2);
    m_context.startService (intent);
  }
//*/
  public void key_set (String key, String val) {
    num_key_set ++;
    try {
      com_uti.logd ("key: " + key + "  val: " + val);
      Intent intent = new Intent (com_uti.api_action_id);
      /*if (intent == null) {
        com_uti.loge ("intent == null");
        return;
      }*/

      //intent.setClass (m_context, svc_svc.class);
      intent.setComponent (new android.content.ComponentName ("fm.a2d.sf", "fm.a2d.sf.svc_svc"));
      intent.putExtra (key, val);
      if (m_context != null)
        m_context.startService (intent);
      else
        com_uti.loge ("m_context == null");
    }
    catch (Throwable e) {
      e.printStackTrace ();
    };
  }

                                                                        // Send at minimum: phase, cdown and status update
  public Intent service_update_send (Intent intent, String phase, String cdown) {
    if (intent == null)                                                 // If for svc_svc and not for phase/cdown/count
      com_uti.logd ("phase: " + phase +  "  cdown: " + cdown);

    chass_phase = phase;
    chass_phtmo = cdown;

    if (intent == null)                                                 // If no Intent (with more info) passed...
      intent = new Intent (com_uti.api_result_id);                      // Create a new broadcast result Intent

    intent.putExtra ("chass_phase",  chass_phase);
    intent.putExtra ("chass_phtmo",  chass_phtmo);

    try {
      m_context.sendStickyBroadcast (intent);                           // Send Sticky Broadcast w/ all info
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }

    com_uti.quiet_ms_sleep (10);                                        // Sleep a bit to give receivers a chance to process. Makes debugging with log messages easier regarding order of state changes

    return (intent);
  }

  public void api_service_update (Intent intent) {
    num_api_service_update ++;
    com_uti.logv ("num_api_service_update: " + num_api_service_update + "  intent: " + intent);

    Bundle extras = intent.getExtras ();

    String  new_chass_phase         = extras.getString  ("chass_phase",         "");//"extra_detect");
    //if (! new_chass_phase.equals ("extra_detect"))
      chass_phase                     = new_chass_phase;

    String  new_chass_phtmo         = extras.getString  ("chass_phtmo",         "");//"extra_detect");
    //if (! new_chass_phtmo.equals ("extra_detect"))
      chass_phtmo                     = new_chass_phtmo;

    for (int ctr = 0; ctr < com_api.chass_preset_max; ctr ++) {
      String  new_chass_preset_freq = extras.getString  ("chass_preset_freq_" + ctr,   "extra_detect");//88500");
      String  new_chass_preset_name = extras.getString  ("chass_preset_name_" + ctr,   "extra_detect");//885");
      if (! new_chass_preset_freq.equals ("extra_detect"))
        chass_preset_freq [ctr]               = new_chass_preset_freq;
      if (! new_chass_preset_name.equals ("extra_detect"))
        chass_preset_name [ctr]               = new_chass_preset_name;
    }

    String  new_audio_state         = extras.getString  ("audio_state",         "extra_detect");//stop");
    String  new_audio_output        = extras.getString  ("audio_output",        "extra_detect");//headset");
    String  new_audio_stereo        = extras.getString  ("audio_stereo",        "extra_detect");//Stereo");
    String  new_audio_record_state  = extras.getString  ("audio_record_state",  "extra_detect");//stop");
    String  new_audio_sessid        = extras.getString  ("audio_sessid",        "extra_detect");
    if (! new_audio_state.equals ("extra_detect"))
      audio_state                     = new_audio_state;
    if (! new_audio_output.equals ("extra_detect"))
      audio_output                    = new_audio_output;
    if (! new_audio_stereo.equals ("extra_detect"))
      audio_stereo                    = new_audio_stereo;
    if (! new_audio_record_state.equals ("extra_detect"))
      audio_record_state              = new_audio_record_state;
    if (! new_audio_sessid.equals ("extra_detect"))
      audio_sessid                    = new_audio_sessid;

    String  new_tuner_state         = extras.getString  ("tuner_state",         "extra_detect");
    if (! new_tuner_state.equals ("extra_detect"))
      tuner_state                     = new_tuner_state;

    String  new_tuner_band          = extras.getString  ("tuner_band",          "extra_detect");
    String  new_tuner_freq          = extras.getString  ("tuner_freq",          "extra_detect");
    String  new_tuner_stereo        = extras.getString  ("tuner_stereo",        "extra_detect");
    String  new_tuner_thresh        = extras.getString  ("tuner_thresh",        "extra_detect");
    String  new_tuner_seek_state    = extras.getString  ("tuner_seek_state",    "extra_detect");

    if (! new_tuner_band.equals ("extra_detect"))
      tuner_band                      = new_tuner_band;
    if (! new_tuner_freq.equals ("extra_detect"))
      tuner_freq                      = new_tuner_freq;
    if (! new_tuner_stereo.equals ("extra_detect"))
      tuner_stereo                    = new_tuner_stereo;
    if (! new_tuner_thresh.equals ("extra_detect"))
      tuner_thresh                    = new_tuner_thresh;
    if (! new_tuner_seek_state.equals ("extra_detect"))
      tuner_seek_state                = new_tuner_seek_state;

    String  new_tuner_rds_state     = extras.getString  ("tuner_rds_state",     "extra_detect");
    String  new_tuner_rds_af_state  = extras.getString  ("tuner_rds_af_state",  "extra_detect");
    String  new_tuner_rds_ta_state  = extras.getString  ("tuner_rds_ta_state",  "extra_detect");
    if (! new_tuner_rds_state.equals ("extra_detect"))
      tuner_rds_state                 = new_tuner_rds_state;
    if (! new_tuner_rds_af_state.equals ("extra_detect"))
      tuner_rds_af_state              = new_tuner_rds_af_state;
    if (! new_tuner_rds_ta_state.equals ("extra_detect"))
      tuner_rds_ta_state              = new_tuner_rds_ta_state;

    String  new_tuner_extension           = extras.getString  ("tuner_extension",           "extra_detect");
    if (! new_tuner_extension.equals ("extra_detect"))
      tuner_extension                       = new_tuner_extension;

    String  new_tuner_rssi          = extras.getString  ("tuner_rssi",          "extra_detect");
    String  new_tuner_qual          = extras.getString  ("tuner_qual",          "extra_detect");
    String  new_tuner_pilot         = extras.getString  ("tuner_pilot ",          "extra_detect");
    if (! new_tuner_rssi.equals ("extra_detect"))
      tuner_rssi                      = new_tuner_rssi;
    if (! new_tuner_qual.equals ("extra_detect"))
      tuner_qual                      = new_tuner_qual;
    if (! new_tuner_pilot.equals ("extra_detect"))
      tuner_pilot                     = new_tuner_pilot ;

    String  new_tuner_rds_pi        = extras.getString  ("tuner_rds_pi",        "extra_detect");
    String  new_tuner_rds_picl      = extras.getString  ("tuner_rds_picl",      "extra_detect");
    String  new_tuner_rds_pt        = extras.getString  ("tuner_rds_pt",        "extra_detect");
    String  new_tuner_rds_ptyn      = extras.getString  ("tuner_rds_ptyn",      "extra_detect");
    String  new_tuner_rds_ps        = extras.getString  ("tuner_rds_ps",        "extra_detect");
    String  new_tuner_rds_rt        = extras.getString  ("tuner_rds_rt",        "extra_detect");
    if (! new_tuner_rds_pi.equals ("extra_detect"))
      tuner_rds_pi                    = new_tuner_rds_pi;
    if (! new_tuner_rds_picl.equals ("extra_detect"))
      tuner_rds_picl                  = new_tuner_rds_picl;
    if (! new_tuner_rds_pt.equals ("extra_detect"))
      tuner_rds_pt                    = new_tuner_rds_pt;
    if (! new_tuner_rds_ptyn.equals ("extra_detect"))
      tuner_rds_ptyn                  = new_tuner_rds_ptyn;
    if (! new_tuner_rds_ps.equals ("extra_detect"))
      tuner_rds_ps                    = new_tuner_rds_ps;
    if (! new_tuner_rds_rt.equals ("extra_detect"))
      tuner_rds_rt                    = new_tuner_rds_rt;

    String  new_tuner_rds_af        = extras.getString  ("tuner_rds_af",        "extra_detect");
    String  new_tuner_rds_ms        = extras.getString  ("tuner_rds_ms",        "extra_detect");
    String  new_tuner_rds_ct        = extras.getString  ("tuner_rds_ct",        "extra_detect");
    String  new_tuner_rds_tmc       = extras.getString  ("tuner_rds_tmc",       "extra_detect");
    String  new_tuner_rds_tp        = extras.getString  ("tuner_rds_tp",        "extra_detect");
    String  new_tuner_rds_ta        = extras.getString  ("tuner_rds_ta",        "extra_detect");
    String  new_tuner_rds_taf       = extras.getString  ("tuner_rds_taf",       "extra_detect");
    if (! new_tuner_rds_af.equals ("extra_detect"))
      tuner_rds_af                    = new_tuner_rds_af;
    if (! new_tuner_rds_ms.equals ("extra_detect"))
      tuner_rds_ms                    = new_tuner_rds_ms;
    if (! new_tuner_rds_ct.equals ("extra_detect"))
      tuner_rds_ct                    = new_tuner_rds_ct;
    if (! new_tuner_rds_tmc.equals ("extra_detect"))
      tuner_rds_tmc                   = new_tuner_rds_tmc;
    if (! new_tuner_rds_tp.equals ("extra_detect"))
      tuner_rds_tp                    = new_tuner_rds_tp;
    if (! new_tuner_rds_ta.equals ("extra_detect"))
      tuner_rds_ta                    = new_tuner_rds_ta;
    if (! new_tuner_rds_taf.equals ("extra_detect"))
      tuner_rds_taf                   = new_tuner_rds_taf;

  }

}

