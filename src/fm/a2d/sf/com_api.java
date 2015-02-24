
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


    // Data that forms the Common API:

    // Service:
  public String     service_phase       = "Pre Init";
  public String     service_cdown       = "0";//999";

  public static final int max_presets   = 16;
  public String []  service_preset_freq =  {"","","","","","","","","","","","","","","","",};
  public String []  service_preset_name =  {"","","","","","","","","","","","","","","","",};

  public String     service_device      = com_uti.DEV_UNK_STR;          // Device ID
    public int      service_device_int  = com_uti.DEV_UNK;              // ""        Integer form

    // Audio:
  public String     audio_state         = "Stop";
  public String     audio_mode          = "Digital";
  public String     audio_output        = "Headset";
  public String     audio_stereo        = "Stereo";
  public String     audio_record_state  = "Stop";                       // Stop, Start
  public String     audio_sessid        = "0";

    // Tuner:                                                           // !!!! Need to update these descriptions:
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

  public  String tuner_band         = "";                               // RW CFG set Values:   US, EU, JAPAN, CHINA, EU_50K_OFFSET     (Set before Tuner Start)
  public  String tuner_freq         = "";                               // RW CFG api Values:   String  form: 50.000 - 499.999  (76-108 MHz)
    public int   tuner_freq_int     = 0;                                // ""                 Integer form in kilohertz

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

  public void key_set (String key, String val, String key2, String val2) {  // Presets currently require simultaneous preset frequency and name
    num_key_set ++;
    com_uti.logd ("key: " + key + "  val: " + val + "  key2: " + key2 + "  val2: " + val2);
    Intent intent = new Intent (com_uti.api_action_id);
    /*if (intent == null) {
      com_uti.loge ("intent == null");
      return;
    }*/
    intent.setClass (m_context, svc_svc.class);
    intent.putExtra (key, val);
    intent.putExtra (key2, val2);
    m_context.startService (intent);
  }

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
      intent.setComponent (new android.content.ComponentName ("fm.a2d.sf",
            "fm.a2d.sf.svc_svc"));  // Seperate lines for single pass sed

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

    service_phase = phase;
    service_cdown = cdown;

    if (intent == null)                                                 // If no Intent (with more info) passed...
      intent = new Intent (com_uti.api_result_id);                      // Create a new broadcast result Intent

    intent.putExtra ("service_phase",  service_phase);
    intent.putExtra ("service_cdown",  service_cdown);

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
    com_uti.logw ("num_api_service_update: " + num_api_service_update + "  intent: " + intent);

    Bundle extras = intent.getExtras ();

    String  new_service_phase         = extras.getString  ("service_phase",         "");//"extra_detect");
    //if (! new_service_phase.equalsIgnoreCase ("extra_detect"))
      service_phase                     = new_service_phase;

    String  new_service_cdown         = extras.getString  ("service_cdown",         "");//"extra_detect");
    //if (! new_service_cdown.equalsIgnoreCase ("extra_detect"))
      service_cdown                     = new_service_cdown;

    for (int ctr = 0; ctr < max_presets; ctr ++) {
      String  new_service_preset_freq = extras.getString  ("service_preset_freq_" + ctr,   "extra_detect");//88500");
      String  new_service_preset_name = extras.getString  ("service_preset_name_" + ctr,   "extra_detect");//885");
      if (! new_service_preset_freq.equalsIgnoreCase ("extra_detect"))
        service_preset_freq [ctr]               = new_service_preset_freq;
      if (! new_service_preset_name.equalsIgnoreCase ("extra_detect"))
        service_preset_name [ctr]               = new_service_preset_name;
    }

    String  new_audio_state         = extras.getString  ("audio_state",         "extra_detect");//stop");
    String  new_audio_output        = extras.getString  ("audio_output",        "extra_detect");//headset");
    String  new_audio_stereo        = extras.getString  ("audio_stereo",        "extra_detect");//Stereo");
    String  new_audio_record_state  = extras.getString  ("audio_record_state",  "extra_detect");//stop");
    String  new_audio_sessid        = extras.getString  ("audio_sessid",        "extra_detect");
    if (! new_audio_state.equalsIgnoreCase ("extra_detect"))
      audio_state                     = new_audio_state;
    if (! new_audio_output.equalsIgnoreCase ("extra_detect"))
      audio_output                    = new_audio_output;
    if (! new_audio_stereo.equalsIgnoreCase ("extra_detect"))
      audio_stereo                    = new_audio_stereo;
    if (! new_audio_record_state.equalsIgnoreCase ("extra_detect"))
      audio_record_state              = new_audio_record_state;
    if (! new_audio_sessid.equalsIgnoreCase ("extra_detect"))
      audio_sessid                    = new_audio_sessid;

    String  new_tuner_state         = extras.getString  ("tuner_state",         "extra_detect");
    if (! new_tuner_state.equalsIgnoreCase ("extra_detect"))
      tuner_state                     = new_tuner_state;

    String  new_tuner_band          = extras.getString  ("tuner_band",          "extra_detect");
    String  new_tuner_freq          = extras.getString  ("tuner_freq",          "extra_detect");
    String  new_tuner_stereo        = extras.getString  ("tuner_stereo",        "extra_detect");
    String  new_tuner_thresh        = extras.getString  ("tuner_thresh",        "extra_detect");
    String  new_tuner_seek_state    = extras.getString  ("tuner_seek_state",    "extra_detect");

    if (! new_tuner_band.equalsIgnoreCase ("extra_detect"))
      tuner_band                      = new_tuner_band;
    if (! new_tuner_freq.equalsIgnoreCase ("extra_detect"))
      tuner_freq                      = new_tuner_freq;
    if (! new_tuner_stereo.equalsIgnoreCase ("extra_detect"))
      tuner_stereo                    = new_tuner_stereo;
    if (! new_tuner_thresh.equalsIgnoreCase ("extra_detect"))
      tuner_thresh                    = new_tuner_thresh;
    if (! new_tuner_seek_state.equalsIgnoreCase ("extra_detect"))
      tuner_seek_state                = new_tuner_seek_state;

    String  new_tuner_rds_state     = extras.getString  ("tuner_rds_state",     "extra_detect");
    String  new_tuner_rds_af_state  = extras.getString  ("tuner_rds_af_state",  "extra_detect");
    String  new_tuner_rds_ta_state  = extras.getString  ("tuner_rds_ta_state",  "extra_detect");
    if (! new_tuner_rds_state.equalsIgnoreCase ("extra_detect"))
      tuner_rds_state                 = new_tuner_rds_state;
    if (! new_tuner_rds_af_state.equalsIgnoreCase ("extra_detect"))
      tuner_rds_af_state              = new_tuner_rds_af_state;
    if (! new_tuner_rds_ta_state.equalsIgnoreCase ("extra_detect"))
      tuner_rds_ta_state              = new_tuner_rds_ta_state;

    String  new_tuner_extension           = extras.getString  ("tuner_extension",           "extra_detect");
    if (! new_tuner_extension.equalsIgnoreCase ("extra_detect"))
      tuner_extension                       = new_tuner_extension;

    String  new_tuner_rssi          = extras.getString  ("tuner_rssi",          "extra_detect");
    String  new_tuner_qual          = extras.getString  ("tuner_qual",          "extra_detect");
    String  new_tuner_pilot         = extras.getString  ("tuner_pilot ",          "extra_detect");
    if (! new_tuner_rssi.equalsIgnoreCase ("extra_detect"))
      tuner_rssi                      = new_tuner_rssi;
    if (! new_tuner_qual.equalsIgnoreCase ("extra_detect"))
      tuner_qual                      = new_tuner_qual;
    if (! new_tuner_pilot.equalsIgnoreCase ("extra_detect"))
      tuner_pilot                     = new_tuner_pilot ;

    String  new_tuner_rds_pi        = extras.getString  ("tuner_rds_pi",        "extra_detect");
    String  new_tuner_rds_picl      = extras.getString  ("tuner_rds_picl",      "extra_detect");
    String  new_tuner_rds_pt        = extras.getString  ("tuner_rds_pt",        "extra_detect");
    String  new_tuner_rds_ptyn      = extras.getString  ("tuner_rds_ptyn",      "extra_detect");
    String  new_tuner_rds_ps        = extras.getString  ("tuner_rds_ps",        "extra_detect");
    String  new_tuner_rds_rt        = extras.getString  ("tuner_rds_rt",        "extra_detect");
    if (! new_tuner_rds_pi.equalsIgnoreCase ("extra_detect"))
      tuner_rds_pi                    = new_tuner_rds_pi;
    if (! new_tuner_rds_picl.equalsIgnoreCase ("extra_detect"))
      tuner_rds_picl                  = new_tuner_rds_picl;
    if (! new_tuner_rds_pt.equalsIgnoreCase ("extra_detect"))
      tuner_rds_pt                    = new_tuner_rds_pt;
    if (! new_tuner_rds_ptyn.equalsIgnoreCase ("extra_detect"))
      tuner_rds_ptyn                  = new_tuner_rds_ptyn;
    if (! new_tuner_rds_ps.equalsIgnoreCase ("extra_detect"))
      tuner_rds_ps                    = new_tuner_rds_ps;
    if (! new_tuner_rds_rt.equalsIgnoreCase ("extra_detect"))
      tuner_rds_rt                    = new_tuner_rds_rt;

    String  new_tuner_rds_af        = extras.getString  ("tuner_rds_af",        "extra_detect");
    String  new_tuner_rds_ms        = extras.getString  ("tuner_rds_ms",        "extra_detect");
    String  new_tuner_rds_ct        = extras.getString  ("tuner_rds_ct",        "extra_detect");
    String  new_tuner_rds_tmc       = extras.getString  ("tuner_rds_tmc",       "extra_detect");
    String  new_tuner_rds_tp        = extras.getString  ("tuner_rds_tp",        "extra_detect");
    String  new_tuner_rds_ta        = extras.getString  ("tuner_rds_ta",        "extra_detect");
    String  new_tuner_rds_taf       = extras.getString  ("tuner_rds_taf",       "extra_detect");
    if (! new_tuner_rds_af.equalsIgnoreCase ("extra_detect"))
      tuner_rds_af                    = new_tuner_rds_af;
    if (! new_tuner_rds_ms.equalsIgnoreCase ("extra_detect"))
      tuner_rds_ms                    = new_tuner_rds_ms;
    if (! new_tuner_rds_ct.equalsIgnoreCase ("extra_detect"))
      tuner_rds_ct                    = new_tuner_rds_ct;
    if (! new_tuner_rds_tmc.equalsIgnoreCase ("extra_detect"))
      tuner_rds_tmc                   = new_tuner_rds_tmc;
    if (! new_tuner_rds_tp.equalsIgnoreCase ("extra_detect"))
      tuner_rds_tp                    = new_tuner_rds_tp;
    if (! new_tuner_rds_ta.equalsIgnoreCase ("extra_detect"))
      tuner_rds_ta                    = new_tuner_rds_ta;
    if (! new_tuner_rds_taf.equalsIgnoreCase ("extra_detect"))
      tuner_rds_taf                   = new_tuner_rds_taf;

  }

}

