
package fm.a2d.sf;

import android.content.Intent;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.ListPreference;
import android.text.method.DigitsKeyListener;
import android.widget.EditText;
import android.widget.Toast;

import java.util.Locale;

public class set_act extends PreferenceActivity implements OnSharedPreferenceChangeListener {

  private void dig_sum_set (String key, boolean negative, boolean decimal) {
    digits_key_lstnr_set (key, negative, decimal);         // 0-9 PLUS: no negative, no decimal (positive integer)
    summary_set (key);
  }

  private SharedPreferences m_sp = null;

  private Context m_context = null;

  private com_api com_api_get (Context context) {
    if (gui_act.m_com_api == null) {
      gui_act.m_com_api = new com_api (context);                      // !! Operates in same process as gui_act !!
      com_uti.logd ("gui_act.m_com_api: " + gui_act.m_com_api);
    }
    if (gui_act.m_com_api == null)
      com_uti.loge ("gui_act.m_com_api == null");
    return (gui_act.m_com_api);
  }

    // Lifecycle:
  @Override
  protected void onCreate (Bundle savedInstanceState) {
    com_uti.logd ("savedInstanceState: " + savedInstanceState);     // Results in huge output with many prefs
/*
    if (fm_apln.m_theme != 0) {
      if (fm_apln.m_theme != android.R.style.Theme_Light || com_uti.android_version >= android.os.Build.VERSION_CODES.HONEYCOMB) { // Can't be theme light on GB and earlier
        if (fm_apln.m_theme != android.R.style.Theme_Translucent) {
          com_uti.logd ("onCreate setting theme to: " + fm_apln.m_theme);
          //if (fm_srvc.is_li)
            setTheme (fm_apln.m_theme);
        }
      }
    }
*/
    super.onCreate (savedInstanceState);

    android.preference.PreferenceManager manager = getPreferenceManager ();
    manager.setSharedPreferencesName (com_uti.prefs_file);

    addPreferencesFromResource (R.xml.prefs);

    m_context = this;
    m_sp = com_uti.sp_get (m_context);
    com_uti.logd ("m_sp: " + m_sp);

/*
      summary_set (fm_apln.PREF_BAND_REGION);

      summary_set (fm_apln.PREF_AUDIO_METHOD);
      summary_set (fm_apln.PREF_AUDIO_OUTPUT_DEVICE);
      summary_set (fm_apln.PREF_AUDIO_OUTPUT_MODE);
      dig_sum_set (fm_apln.PREF_AUDIO_VOLUME_SCALE, false, true);       // 0-9 PLUS: no negative, decimal (positive float)

      summary_set (fm_apln.PREF_AF_MODE);
      dig_sum_set (fm_apln.PREF_AF_RSSI_THRESH,     true,  false);      // 0-9 PLUS: OK negative, no decimal (integer)
      dig_sum_set (fm_apln.PREF_AF_PERIOD,          false, false);      // AF rssi polling period  0-9 PLUS: no negative, no decimal (positive integer)
      dig_sum_set (fm_apln.PREF_AF_POLLS,           false, false);      // 0-9 PLUS: no negative, no decimal (positive integer)
      dig_sum_set (fm_apln.PREF_AF_PI_MASK,         false, false);      // 0-9 PLUS: no negative, no decimal (positive integer)
      dig_sum_set (fm_apln.PREF_AF_RSSI_PAUSE,      false, false);      // 0-9 PLUS: no negative, no decimal (positive integer)
      dig_sum_set (fm_apln.PREF_AF_PI_TMO,          false, false);      // 0-9 PLUS: no negative, no decimal (positive integer)
      dig_sum_set (fm_apln.PREF_AF_MAN_AF1,         false, false);      // 0-9 PLUS: no negative, no decimal (positive integer)
      dig_sum_set (fm_apln.PREF_AF_MAN_AF2,         false, false);      // 0-9 PLUS: no negative, no decimal (positive integer)
// Other 20 are Checkbox preferences
*/

  }

  void digits_key_lstnr_set (String key, boolean sign, boolean decimal) {
    EditTextPreference etp = (EditTextPreference) findPreference (key);                             // Get Preference object for key
    EditText et = (EditText) etp.getEditText ();
    et.setKeyListener (DigitsKeyListener.getInstance (sign, decimal));  // 0-9 PLUS sign, decimal
    return;
  }

  void summary_set (String key) {

    Preference pref = findPreference (key);                             // Get Preference object for key

    if (pref instanceof ListPreference) {                               // If a ListPreference...
      ListPreference lp = (ListPreference) pref;
      CharSequence summ = "";
      try {
        summ = lp.getEntry ();                                          // ArrayIndexOutOfBoundsException
      }
      catch (Throwable e) {
        e.printStackTrace ();
      }
      com_uti.logd ("key: " + key + "  summ: " + summ);

      if (summ == null || summ.equals ("")) {                           // If no selection yet...
/*
        if (key.equals (fm_apln.PREF_AUDIO_VCS))                        // !!!! Manual change for Index !!!!
          lp.setValueIndex (4);                                         // 4 / 3 = Music    !!!! Manual change for Index !!!!

        else if (key.equals (fm_apln.PREF_REC_SOURCE))
          lp.setValueIndex (8);
        else if (key.equals (fm_apln.PREF_REC_FORMAT))
          lp.setValueIndex (1);
        else if (key.equals (fm_apln.PREF_REC_ENCODER))
          lp.setValueIndex (3);

        else
*/
          lp.setValueIndex (0);                                         // Use first value as default for all other list preferences (Audio output device, audio output mode etc.)

        try {
          summ = lp.getEntry ();                                        // Get name of entry
        }
        catch (Throwable e) {
          e.printStackTrace ();
        }
        com_uti.logd ("summ: " + summ);
      }
      pref.setSummary (summ);
    }
    else if (pref instanceof EditTextPreference) {
      //EditTextPreference etp = (EditTextPreference) pref;
      String summ;
/*
      if (key.equals (fm_apln.PREF_UI_DISPLAY_WEIGHT))
        summ = m_sp.getString (key, "3");

      else if (key.equals (fm_apln.PREF_AUDIO_VOLUME_SCALE))
        summ = m_sp.getString (key, "100");
      else if (key.equals (fm_apln.PREF_DEBUG_FIXED_VOLUME))
        summ = m_sp.getString (key, "0");
      else if (key.equals (fm_apln.PREF_AUDIO_EXTERNAL_VOLUME))
        summ = m_sp.getString (key, "0");

      else if (key.equals (fm_apln.PREF_REC_SAMPLE_RATE))
        summ = m_sp.getString (key, "44100"); // 48000 ?

      else
*/
        summ = m_sp.getString (key, "");                                  // Default default is empty string

      pref.setSummary (summ);                                           // !! ?? Why below and not to right ??
    }

    else if (pref instanceof CheckBoxPreference) {
      CheckBoxPreference cbp = (CheckBoxPreference) pref;
      Boolean summ;
/*
      if ((key.equals (fm_apln.PREF_UI_BOLD_PRESETS)) || (key.equals (fm_apln.PREF_UI_FREQ_BTN_1))  || (key.equals (fm_apln.PREF_UI_FREQ_BTN_2)) || (key.equals (fm_apln.PREF_UI_PRST_BTN_1))
            || (key.equals (fm_apln.PREF_UI_PRST_BTN_2)) || (key.equals (fm_apln.PREF_UI_PRST_BTN_3)) || (key.equals (fm_apln.PREF_UI_MODE_BTN_1)) )
        summ = m_sp.getBoolean (key, true);

      else if ((key.equals (fm_apln.PREF_UI_DISP_STAT)) || (key.equals (fm_apln.PREF_UI_DISP_FREQ)) || (key.equals (fm_apln.PREF_UI_DISP_RDS))
            || (key.equals (fm_apln.PREF_UI_DISP_VOL)) || (key.equals (fm_apln.PREF_UI_DISP_RT)) || (key.equals (fm_apln.PREF_UI_DISP_VISU))
            || (key.equals (fm_apln.PREF_REC_STEREO)) )
        summ = m_sp.getBoolean (key, true);

      else
        summ = m_sp.getBoolean (key, false);                              // Default default is false

      cbp.setChecked (summ);                                            // Write to config ! (No other way ?)
*/
    }

  }


  @Override
  protected void onResume() {
    com_uti.logd ("m_sp: " + m_sp);
    super.onResume ();
    if (m_sp != null)
      m_sp.registerOnSharedPreferenceChangeListener (this);          // Register our change listener
  }
  @Override
  protected void onPause() {
    com_uti.logd ("m_sp: " + m_sp);
    super.onPause ();
    if (m_sp != null)
      m_sp.unregisterOnSharedPreferenceChangeListener (this);        // UnRegister our change listener
  }

  public void onSharedPreferenceChanged (SharedPreferences sp, String key) {
    com_uti.logd ("m_sp: " + m_sp + "  sp: " + sp + "  key: " + key);

    if (com_api_get (m_context) == null)
      return;

    String val = sp.getString (key, "");
    if (val == null || val.equals (""))
      return;

    String lo_key = key.toLowerCase (Locale.getDefault ());

    if (lo_key.startsWith ("gui"))
      com_uti.logd ("ignore gui key");
    else
      gui_act.m_com_api.key_set (key, val);

/*
    if (key.equals ("audio_output"))
      gui_act.m_com_api.key_set ("audio_output", val);
    else if (key.equals ("audio_stereo"))
      gui_act.m_com_api.key_set ("audio_stereo", val);
    else if (key.equals ("tuner_band"))
      gui_act.m_com_api.key_set ("tuner_band", val);
    else if (key.equals ("tuner_stereo"))
      gui_act.m_com_api.key_set ("tuner_stereo", val);
    else if (key.equals ("tuner_rds_state"))
      gui_act.m_com_api.key_set ("tuner_rds_state", val);
    else if (key.equals ("tuner_rds_af_state"))
      gui_act.m_com_api.key_set ("tuner_rds_af_state", val);
    //else if (key.equals ("tuner_"))
    //  gui_act.m_com_api.key_set ("tuner_", val);
    else if (key.equals ("audio_digital_amp"))
      gui_act.m_com_api.key_set ("audio_digital_amp", val);
*/

/*
    if (! key.equals (fm_apln.PREF_AUDIO_METHOD))                       // Only if not audio method (Galaxy S issue !)
      summary_set (key);                                                // !! What about pay options ! ??

    fm_srvc m_srvc = fm_apln.m_srvc;
    if (m_srvc == null)
      return;

    if (key.equals (fm_apln.PREF_BAND_REGION)) {                         // Band Region
      com_uti.logd ("onSharedPreferenceChanged Region Change: " + idx);
      fm_apln.region_set (idx);
    }

    else if (key.equals (fm_apln.PREF_DEBUG_EXTRA_LOGS)) {
      boolean debug_extra_logs = sp.getBoolean (key, false);
      com_uti.logd ("onSharedPreferenceChanged Debug Extra Logs Change: " + debug_extra_logs);
      if (debug_extra_logs) {
        m_srvc.misc_set_get (2, "Debug extra logs on",1);               // Debug extra logs on
      }
      else {
        m_srvc.misc_set_get (2, "Debug extra logs off", 0);             // Debug extra logs off
      }
    }


    else if (
        (key.equals (fm_apln.PREF_UI_RDS)) ||
        (key.equals (fm_apln.PREF_UI_THEME)) ||
        (key.equals (fm_apln.PREF_UI_TEXT_COLOR)) ||
        (key.equals (fm_apln.PREF_UI_MODE_BTN_1)) ) {

      Toast.makeText (this, "Restart to Apply", Toast.LENGTH_LONG).show ();
    }
*/
    return;
  }

}

