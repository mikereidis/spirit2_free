
    // Radio Service API:
package fm.a2d.sf;

import android.app.PendingIntent;
import android.content.Intent;
import android.content.Context;
import android.os.Bundle;

public class com_api {

  private static    int                 m_obinits   = 0;
  //private static    int                 m_creates   = 0;

  public  static    Context             m_context   = null;

  private static    int                 m_intent_ctr= 0;

    // Radio statuses:
  public String     radio_phase         = "Pre Init";
  public String     radio_cdown         = "999";
  public String     radio_error         = "";

  public static final int max_presets   = 16;
  public String []  radio_freq_prst     =  {"","","","","","","","","","","","","","","","",};
  public String []  radio_name_prst     =  {"","","","","","","","","","","","","","","","",};

    // Audio:
  public String     audio_state         = "stop";
  public String     audio_output        = "headset";
  public String     audio_stereo        = "Stereo";
  public String     audio_record_state  = "stop";                       // stop, start
  public String     audio_sessid        = "0";

    // Tuner:
                                                                        //    CFG = Saved in config
                                                                        //    ... = ephemeral non-volatile
                                                                        //        api = get from FM API
                                                                        //        set = get from set of this variable
                                                                        //        mul = multiple/both
                                                                        //        ... = RO
                                                                        // RW CFG api for tuner_freq & tuner_thresh consistency issues: CFG vs chip current values
  public  String tuner_state        = "stop";                           // RW ... api States:   stop, start, pause, resume

  public  String tuner_band         = "";//US";                             // RW CFG set Values:   US, EU, JAPAN, CHINA, EU_50K_OFFSET     (Set before Tuner Start)
  public  String tuner_freq         = "";//-1";                             // RW CFG api Values:   String  form: 50.000 - 499.999  (76-108 MHz)
    public int   int_tuner_freq     = 0;//-1;                               // ""                 Integer form in kilohertz
  public  String tuner_stereo       = "";//Stereo";                         // RW CFG set Values:   mono, stereo, switch, blend, ... ?
  public  String tuner_thresh       = "";                               // RW CFG api Values:   Seek/scan RSSI threshold
  public  String tuner_scan_state   = "stop";                           // RW ... set States:   down, up, scan, stop

  public  String tuner_rds_state    = "stop";                           // RW CFG set States:   on, off
  public  String tuner_rds_af_state = "stop";                           // RW CFG set States:   on, off
  public  String tuner_rds_ta_state = "stop";                           // RW CFG set States:   on, off

  public  String tuner_extra_cmd    = "";                               // RW ... set Values:   Extra command
  public  String tuner_extra_resp   = "";                               // ro ... ... Values:   Extra command response

  public  String tuner_rssi         = "";//999";                            // ro ... ... Values:   RSSI: 0 - 1000
  public  String tuner_qual         = "";//SN 99";                          // ro ... ... Values:   SN 99, SN 30
  public  String tuner_most         = "";//Mono";                           // ro ... ... Values:   mono, stereo, 1, 2, blend, ... ?      1.5 ?

  public  String tuner_rds_pi       = "";//-1";                             // ro ... ... Values:   0 - 65535
  public  String tuner_rds_picl     = "";//WKBW";                           // ro ... ... Values:   North American Call Letters or Hex PI for tuner_rds_pi
  public  String tuner_rds_pt       = "";//-1";                             // ro ... ... Values:   0 - 31
  public  String tuner_rds_ptyn     = "";                               // ro ... ... Values:   Describes tuner_rds_pt (English !)
  public  String tuner_rds_ps       = "SpiritF";                               // ro ... ... Values:   RBDS 8 char info or RDS Station
  public  String tuner_rds_rt       = "";//OBNOXIOUS !!     "Analog 2 Digital radio ; Thanks for Your Support... :)";                               // ro ... ... Values:   64 char

  public  String tuner_rds_af       = "";                               // ro ... ... Values:   Space separated array of AF frequencies
  public  String tuner_rds_ms       = "";                               // ro ... ... Values:   0 - 65535   M/S Music/Speech switch code
  public  String tuner_rds_ct       = "";                               // ro ... ... Values:   14 char CT Clock Time & Date

  public  String tuner_rds_tmc      = "";                               // ro ... ... Values:   Space separated array of shorts
  public  String tuner_rds_tp       = "";                               // ro ... ... Values:   0 - 65535   TP Traffic Program Identification code
  public  String tuner_rds_ta       = "";                               // ro ... ... Values:   0 - 65535   TA Traffic Announcement code
  public  String tuner_rds_taf      = "";                               // ro ... ... Values:   0 - 2^32-1  TAF TA Frequency



    // Code:

  public com_api (Context context) {                                    // Context constructor
    m_obinits ++;
    com_uti.logd ("m_obinits: " + m_obinits);
    m_context = context;
    com_uti.logd ("context: " + context);
  }


  public static PendingIntent pend_intent_get (Context context, String key, String val) {
    Intent intent = new Intent ("fm.a2d.sf.action.set");
    com_uti.logv ("context: " + context + "  m_context: " + m_context + "  intent: " + intent + "  key: " + key + "  val: " + val);
    /*if (intent == null) {
      com_uti.loge ("intent == null");
      return (null);
    }*/
    intent.setClass (context, svc_svc.class);                         // !! Note possible different context and m_context !!
    intent.putExtra (key, val);
    m_intent_ctr ++;
    PendingIntent pi = PendingIntent.getService (context, m_intent_ctr, intent, PendingIntent.FLAG_UPDATE_CURRENT);// Different 2nd parameter
    return (pi);
  }
  public void key_set (String key, String val, String key2, String val2) {  // Presets currently require simultaneous preset frequency and name
    com_uti.logd ("key: " + key + "  val: " + val + "  key2: " + key2 + "  val2: " + val2);
    Intent intent = new Intent ("fm.a2d.sf.action.set");
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
    try {
    com_uti.logd ("key: " + key + "  val: " + val);
    Intent intent = new Intent ("fm.a2d.sf.action.set");
    /*if (intent == null) {
      com_uti.loge ("intent == null");
      return;
    }*/

//    intent.setClass (m_context, svc_svc.class);
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


  public void radio_update (Intent intent) {
    com_uti.logv ("intent: " + intent);

    Bundle extras = intent.getExtras ();

    String  new_radio_phase         = extras.getString  ("radio_phase",         "default_detect");
    if (! new_radio_phase.equalsIgnoreCase ("default_detect"))
      radio_phase                     = new_radio_phase;
    String  new_radio_cdown         = extras.getString  ("radio_cdown",         "default_detect");
    if (! new_radio_cdown.equalsIgnoreCase ("default_detect"))
      radio_cdown                     = new_radio_cdown;
    String  new_radio_error         = extras.getString  ("radio_error",         "default_detect");
    if (! new_radio_error.equalsIgnoreCase ("default_detect"))
      radio_error                     = new_radio_error;

    for (int ctr = 0; ctr < max_presets; ctr ++) {
      String  new_radio_freq_prst = extras.getString  ("radio_freq_prst_" + ctr,   "default_detect");//88500");
      String  new_radio_name_prst = extras.getString  ("radio_name_prst_" + ctr,   "default_detect");//885");
      if (! new_radio_freq_prst.equalsIgnoreCase ("default_detect"))
        radio_freq_prst [ctr]               = new_radio_freq_prst;
      if (! new_radio_name_prst.equalsIgnoreCase ("default_detect"))
        radio_name_prst [ctr]               = new_radio_name_prst;
    }

    String  new_audio_state         = extras.getString  ("audio_state",         "default_detect");//stop");
    String  new_audio_output        = extras.getString  ("audio_output",        "default_detect");//headset");
    String  new_audio_stereo        = extras.getString  ("audio_stereo",        "default_detect");//Stereo");
    String  new_audio_record_state  = extras.getString  ("audio_record_state",  "default_detect");//stop");
    String  new_audio_sessid        = extras.getString  ("audio_sessid",        "default_detect");
    if (! new_audio_state.equalsIgnoreCase ("default_detect"))
      audio_state                     = new_audio_state;
    if (! new_audio_output.equalsIgnoreCase ("default_detect"))
      audio_output                    = new_audio_output;
    if (! new_audio_stereo.equalsIgnoreCase ("default_detect"))
      audio_stereo                    = new_audio_stereo;
    if (! new_audio_record_state.equalsIgnoreCase ("default_detect"))
      audio_record_state              = new_audio_record_state;
    if (! new_audio_sessid.equalsIgnoreCase ("default_detect"))
      audio_sessid                    = new_audio_sessid;

    String  new_tuner_state         = extras.getString  ("tuner_state",         "default_detect");
    if (! new_tuner_state.equalsIgnoreCase ("default_detect"))
      tuner_state                     = new_tuner_state;

    String  new_tuner_band          = extras.getString  ("tuner_band",          "default_detect");
    String  new_tuner_freq          = extras.getString  ("tuner_freq",          "default_detect");
    String  new_tuner_stereo        = extras.getString  ("tuner_stereo",        "default_detect");
    String  new_tuner_thresh        = extras.getString  ("tuner_thresh",        "default_detect");
    String  new_tuner_scan_state    = extras.getString  ("tuner_scan_state",    "default_detect");

    if (! new_tuner_band.equalsIgnoreCase ("default_detect"))
      tuner_band                      = new_tuner_band;
    if (! new_tuner_freq.equalsIgnoreCase ("default_detect"))
      tuner_freq                      = new_tuner_freq;
    if (! new_tuner_stereo.equalsIgnoreCase ("default_detect"))
      tuner_stereo                    = new_tuner_stereo;
    if (! new_tuner_thresh.equalsIgnoreCase ("default_detect"))
      tuner_thresh                    = new_tuner_thresh;
    if (! new_tuner_scan_state.equalsIgnoreCase ("default_detect"))
      tuner_scan_state                = new_tuner_scan_state;

    String  new_tuner_rds_state     = extras.getString  ("tuner_rds_state",     "default_detect");
    String  new_tuner_rds_af_state  = extras.getString  ("tuner_rds_af_state",  "default_detect");
    String  new_tuner_rds_ta_state  = extras.getString  ("tuner_rds_ta_state",  "default_detect");
    if (! new_tuner_rds_state.equalsIgnoreCase ("default_detect"))
      tuner_rds_state                 = new_tuner_rds_state;
    if (! new_tuner_rds_af_state.equalsIgnoreCase ("default_detect"))
      tuner_rds_af_state              = new_tuner_rds_af_state;
    if (! new_tuner_rds_ta_state.equalsIgnoreCase ("default_detect"))
      tuner_rds_ta_state              = new_tuner_rds_ta_state;

    String  new_tuner_extra_cmd     = extras.getString  ("tuner_extra_cmd",     "default_detect");
    String  new_tuner_extra_resp    = extras.getString  ("tuner_extra_resp",    "default_detect");
    if (! new_tuner_extra_cmd.equalsIgnoreCase ("default_detect"))
      tuner_extra_cmd                 = new_tuner_extra_cmd;
    if (! new_tuner_extra_resp.equalsIgnoreCase ("default_detect"))
      tuner_extra_resp                = new_tuner_extra_resp;

    String  new_tuner_rssi          = extras.getString  ("tuner_rssi",          "default_detect");
    String  new_tuner_qual          = extras.getString  ("tuner_qual",          "default_detect");
    String  new_tuner_most          = extras.getString  ("tuner_most",          "default_detect");
    if (! new_tuner_rssi.equalsIgnoreCase ("default_detect"))
      tuner_rssi                      = new_tuner_rssi;
    if (! new_tuner_qual.equalsIgnoreCase ("default_detect"))
      tuner_qual                      = new_tuner_qual;
    if (! new_tuner_most.equalsIgnoreCase ("default_detect"))
      tuner_most                      = new_tuner_most;

    String  new_tuner_rds_pi        = extras.getString  ("tuner_rds_pi",        "default_detect");
    String  new_tuner_rds_picl      = extras.getString  ("tuner_rds_picl",      "default_detect");
    String  new_tuner_rds_pt        = extras.getString  ("tuner_rds_pt",        "default_detect");
    String  new_tuner_rds_ptyn      = extras.getString  ("tuner_rds_ptyn",      "default_detect");
    String  new_tuner_rds_ps        = extras.getString  ("tuner_rds_ps",        "default_detect");
    String  new_tuner_rds_rt        = extras.getString  ("tuner_rds_rt",        "default_detect");
    if (! new_tuner_rds_pi.equalsIgnoreCase ("default_detect"))
      tuner_rds_pi                    = new_tuner_rds_pi;
    if (! new_tuner_rds_picl.equalsIgnoreCase ("default_detect"))
      tuner_rds_picl                  = new_tuner_rds_picl;
    if (! new_tuner_rds_pt.equalsIgnoreCase ("default_detect"))
      tuner_rds_pt                    = new_tuner_rds_pt;
    if (! new_tuner_rds_ptyn.equalsIgnoreCase ("default_detect"))
      tuner_rds_ptyn                  = new_tuner_rds_ptyn;
    if (! new_tuner_rds_ps.equalsIgnoreCase ("default_detect"))
      tuner_rds_ps                    = new_tuner_rds_ps;
    if (! new_tuner_rds_rt.equalsIgnoreCase ("default_detect"))
      tuner_rds_rt                    = new_tuner_rds_rt;

    String  new_tuner_rds_af        = extras.getString  ("tuner_rds_af",        "default_detect");
    String  new_tuner_rds_ms        = extras.getString  ("tuner_rds_ms",        "default_detect");
    String  new_tuner_rds_ct        = extras.getString  ("tuner_rds_ct",        "default_detect");
    String  new_tuner_rds_tmc       = extras.getString  ("tuner_rds_tmc",       "default_detect");
    String  new_tuner_rds_tp        = extras.getString  ("tuner_rds_tp",        "default_detect");
    String  new_tuner_rds_ta        = extras.getString  ("tuner_rds_ta",        "default_detect");
    String  new_tuner_rds_taf       = extras.getString  ("tuner_rds_taf",       "default_detect");
    if (! new_tuner_rds_af.equalsIgnoreCase ("default_detect"))
      tuner_rds_af                    = new_tuner_rds_af;
    if (! new_tuner_rds_ms.equalsIgnoreCase ("default_detect"))
      tuner_rds_ms                    = new_tuner_rds_ms;
    if (! new_tuner_rds_ct.equalsIgnoreCase ("default_detect"))
      tuner_rds_ct                    = new_tuner_rds_ct;
    if (! new_tuner_rds_tmc.equalsIgnoreCase ("default_detect"))
      tuner_rds_tmc                   = new_tuner_rds_tmc;
    if (! new_tuner_rds_tp.equalsIgnoreCase ("default_detect"))
      tuner_rds_tp                    = new_tuner_rds_tp;
    if (! new_tuner_rds_ta.equalsIgnoreCase ("default_detect"))
      tuner_rds_ta                    = new_tuner_rds_ta;
    if (! new_tuner_rds_taf.equalsIgnoreCase ("default_detect"))
      tuner_rds_taf                   = new_tuner_rds_taf;

  }

}

