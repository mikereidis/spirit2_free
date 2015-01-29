
    // GUI Activity:
    // Copyright 2011-2015 Michael A. Reid. All rights reserved.

package fm.a2d.sf;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.media.AudioManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;

public class gui_act extends Activity {     //public class gui_act extends Fragment {       Much work required

  private static    int                 m_obinits   = 0;
  private static    int                 m_creates   = 0;

  public static     Context             m_context   = null;
  public static     com_api             m_com_api   = null;

  private static    BroadcastReceiver   m_api_bcr   = null;
  private           gui_gap             m_gui_gap   = null;

    // Lifecycle:

  public gui_act () {                                                   // empty constructor ?
    m_obinits ++;
    com_uti.logd ("m_obinits: " + m_obinits);

    m_context = this;
    com_uti.logd ("m_context: " + m_context);
    com_uti.logd ("m_com_api: " + m_com_api);
    com_uti.logd ("m_api_bcr: " + m_api_bcr);
    com_uti.logd ("m_gui_gap: " + m_gui_gap);

    if (m_com_api == null) {                                            // If a receiver has not initialized yet...
      m_com_api = new com_api (m_context);                              // Instantiate Common API
      com_uti.logd ("m_com_api: " + m_com_api);
    }

  }

  @Override
  protected void onCreate (Bundle savedInstanceState) {
    m_creates ++;
    com_uti.logd ("m_creates: " + m_creates);

    m_context = this;
    com_uti.logd ("m_context: " + m_context);
    com_uti.logd ("m_com_api: " + m_com_api);
    com_uti.logd ("m_api_bcr: " + m_api_bcr);
    com_uti.logd ("m_gui_gap: " + m_gui_gap);

    com_uti.logd ("SpiritF " + com_uti.app_version_get (m_context));

    com_uti.logd ("savedInstanceState: " + savedInstanceState);
    //com_uti.logd ("dev: " + com_uti.device);

    //com_uti.strict_mode_set (true);

    if (m_com_api == null) {                                            // If a receiver has not initialized yet...
      m_com_api = new com_api (m_context);                              // Instantiate Common API
      com_uti.logd ("m_com_api: " + m_com_api);
    }

    super.onCreate (savedInstanceState);

    if (m_com_api == null) {                                            // If a receiver has not initialized yet...
      m_com_api = new com_api (m_context);                              // Instantiate Common API
      com_uti.logd ("m_com_api: " + m_com_api);
    }

    setVolumeControlStream (AudioManager.STREAM_MUSIC);                 // setVolumeControlStream() must be done from an Activity

    api_bcr_start ();                                                   // Start Common API Broadcast Receiver

    gui_start ();                                                       // Start GUI

  }

    // Create        Start,Resume       Pause,Resume        Pause,Stop,Restart       Start,Resume

    // Launch:   Create      Start       Resume
    // Home:                                         Pause       Stop
    // Return:   Restart     Start       Resume
    // Back:                                         Pause       Stop        Destroy


  // Continuing methods in lifecycle order:
  @Override
  public void onStart () {
    com_uti.logd ("");
    super.onStart ();
    com_uti.logd ("");
  }
  @Override
  public void onResume () {                                             // !! Resume can happen with the FM power off, so try not to do things needing power on
    com_uti.logd ("");
    super.onResume ();
    com_uti.logd ("");
  }
  @Override
  protected void onPause () {
    com_uti.logd ("");
    super.onPause ();
    com_uti.logd ("");
  }
  @Override
  public void onStop () {
    com_uti.logd ("");
    super.onStop ();
    com_uti.logd ("");
  }
  @Override
  public void onRestart () {                                             // Restart comes between Stop and Start or when returning to the app
    com_uti.logd ("");
    super.onRestart ();
    com_uti.logd ("");
  }

  @Override
  public void onDestroy () {
    com_uti.logd ("");
                                   // One of these caused crashes:
    api_bcr_stop ();
    gui_stop ();

    com_uti.logd ("com_uti.num_daemon_get:       " + com_uti.num_daemon_get);
    com_uti.logd ("com_uti.num_daemon_set:       " + com_uti.num_daemon_set);

    if (m_com_api != null) {
      com_uti.logd ("m_com_api.num_key_set:      " + m_com_api.num_key_set);
      com_uti.logd ("m_com_api.num_radio_update: " + m_com_api.num_radio_update);
    }

    // super.onDestroy dismisses any dialogs or cursors the activity was managing. If the logic in onDestroy has something to do with these things, then order may matter.
    super.onDestroy ();

    com_uti.logd ("");
  }


  private void gui_stop () {
    try {
      if (m_gui_gap == null)
        com_uti.loge ("already stopped");
      else if (! m_gui_gap.gap_state_set ("stop"))                          // Stop UI. If error...
        com_uti.loge ("gui_stop error");
      else
        com_uti.logd ("gui_stop OK");
      m_gui_gap = null;
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }
  private void gui_start () {
    try {
      m_gui_gap = new gui_gui (m_context, m_com_api);                       // Instantiate UI
      if (m_gui_gap == null)
        com_uti.loge ("m_gui_gap == null");
      else if (! m_gui_gap.gap_state_set ("start")) {                       // Start UI. If error...
        com_uti.loge ("gui_start error");
        m_gui_gap = null;
      }
      else
        com_uti.logd ("gui_start OK");
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }


  private void api_bcr_stop () {
    if (m_api_bcr != null) {                                            // Remove the State listener
      if (m_context != null)
        m_context.unregisterReceiver (m_api_bcr);
      m_api_bcr = null;
    }
  }

  private void api_bcr_start () {                                       // Common API Intent result & notification Broadcast Receiver
    if (m_api_bcr == null) {
      m_api_bcr = new BroadcastReceiver () {
        @Override
        public void onReceive (Context context, Intent intent) {
          String action = intent.getAction ();

          com_uti.logv ("intent: " + intent + "  action: " + action);
          boolean bypass = false;//true;
          if (bypass || ! action.equalsIgnoreCase ("fm.a2d.sf.result.get"))
            return;

          if (m_com_api != null && m_gui_gap != null) {
            m_com_api.radio_update (intent);

            m_gui_gap.gap_radio_update (intent);
          }
        }
      };

      IntentFilter intent_filter = new IntentFilter ();
      intent_filter.addAction ("fm.a2d.sf.result.get");                  // Can add more actions if needed
      intent_filter.addCategory (Intent.CATEGORY_DEFAULT);

      Intent last_sticky_state_intent = null;
      if (m_context != null)
        last_sticky_state_intent = m_context.registerReceiver (m_api_bcr, intent_filter, null, null);    // No permission, no handler scheduler thread.
      if (last_sticky_state_intent != null) {
        com_uti.logd ("bcast intent last_sticky_state_intent: " + last_sticky_state_intent);
        //m_api_bcr.onReceive (m_context, last_sticky_state_intent);  // Like a resend of last audio status update
      }
    }
  }


    // Dialog methods:

  @Override
  protected Dialog onCreateDialog (int id, Bundle args) {               // Create a dialog by calling specific *_dialog_create function    ; Triggered by showDialog (int id);
    com_uti.logd ("id: " + id + "  args: " + args);
    Dialog ret = m_gui_gap.gap_dialog_create (id, args);
    com_uti.logd ("dialog: " + ret);
    return (ret);
  }


  public void gap_gui_clicked (View v) {
    m_gui_gap.gap_gui_clicked (v);
  }

}
