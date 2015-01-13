
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

  public  static com_api m_com_api    = null;
  private static int    stat_creates = 1;
  private static BroadcastReceiver m_bcast_lstnr = null;

  private   gui_gap     m_gui       = null;
  private   Context     m_context   = null;

    // Lifecycle:

  @Override
  protected void onCreate (Bundle savedInstanceState) {
    com_uti.logd ("stat_creates: " + stat_creates++);

    m_context = this;

    //com_uti.logd ("SpiritF " + com_uti.app_version_get (m_context) + " " + copyright);

    com_uti.logd ("savedInstanceState: " + savedInstanceState);
    //com_uti.logd ("dev: " + com_uti.device);

    //com_uti.strict_mode_set (true);

    super.onCreate (savedInstanceState);

    if (m_com_api == null) {
      m_com_api = new com_api (m_context);
      com_uti.logd ("m_com_api: " + m_com_api);
    }

    setVolumeControlStream (AudioManager.STREAM_MUSIC);                      // Must be done from an Activity
    bcast_lstnr_start ();
    gui_start ();
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
    bcast_lstnr_stop ();
    gui_stop ();

    // super.onDestroy dismisses any dialogs or cursors the activity was managing. If the logic in onDestroy has something to do with these things, then order may matter.
    super.onDestroy ();

    com_uti.logd ("");
  }


  private void gui_stop () {
    try {
      if (m_gui == null)
        com_uti.loge ("already stopped");
      else if (! m_gui.gap_state_set ("stop"))                          // Stop UI. If error...
        com_uti.loge ("gui_stop error");
      else
        com_uti.logd ("gui_stop OK");
      m_gui = null;
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }
  private void gui_start () {
    try {
      m_gui = new gui_gui (m_context, m_com_api);                       // Instantiate UI
      if (m_gui == null)
        com_uti.loge ("m_gui == null");
      else if (! m_gui.gap_state_set ("start")) {                       // Start UI. If error...
        com_uti.loge ("gui_start error");
        m_gui = null;
      }
      else
        com_uti.logd ("gui_start OK");
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }


  public void bcast_lstnr_stop () {
    if (m_bcast_lstnr != null) {                                        // Remove the State listener
      if (m_context != null)
        m_context.unregisterReceiver (m_bcast_lstnr);
      m_bcast_lstnr = null;
    }
  }

  public void bcast_lstnr_start () {
    if (m_bcast_lstnr == null) {
      m_bcast_lstnr = new BroadcastReceiver () {
        @Override
        public void onReceive (Context context, Intent intent) {
          String action = intent.getAction ();

          com_uti.logx ("intent: " + intent + "  action: " + action);
          boolean bypass = false;//true;
          if (bypass || ! action.equalsIgnoreCase ("fm.a2d.sf.result.get"))
            return;

          if (m_com_api != null && m_gui != null) {
            m_com_api.radio_update (intent);

            m_gui.gap_radio_update (intent);
          }
        }
      };

      IntentFilter intent_filter = new IntentFilter ();
      intent_filter.addAction ("fm.a2d.sf.result.get");                  // Can add more actions if needed
      intent_filter.addCategory (Intent.CATEGORY_DEFAULT);

      Intent last_sticky_state_intent = null;
      if (m_context != null)
        last_sticky_state_intent = m_context.registerReceiver (m_bcast_lstnr, intent_filter, null, null);    // No permission, no handler scheduler thread.
      if (last_sticky_state_intent != null) {
        com_uti.logd ("bcast intent last_sticky_state_intent: " + last_sticky_state_intent);
        //m_bcast_lstnr.onReceive (m_context, last_sticky_state_intent);  // Like a resend of last audio status update
      }
    }
  }


    // Dialog methods:

  @Override
  protected Dialog onCreateDialog (int id, Bundle args) {                            // Create a dialog by calling specific *_dialog_create function    ; Triggered by showDialog (int id);
    com_uti.logd ("id: " + id + "  args: " + args);
    Dialog ret = m_gui.gap_dialog_create (id, args);
    com_uti.logd ("dialog: " + ret);
    return (ret);
  }


  public void gap_gui_clicked (View v) {
    m_gui.gap_gui_clicked (v);
  }

}
