
    // Media button and other remote controls: Lockscreen, AVRCP & future components
package fm.a2d.sf;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.view.KeyEvent;


    // Broadcast Intent handler:
    //      android.media.AUDIO_BECOMING_NOISY:     Headphones disconnected
    //      android.intent.action.MEDIA_BUTTON:     Headphone Media button pressed
    // Declared as receiver in AndroidManifest.xml and passed to (un)registerMediaButtonEventReceiver

public class svc_bcr extends BroadcastReceiver {

  private static int    stat_constrs = 1;
  //private static com_api gui_act.m_com_api = null;                              // Static: Need to maintain state across broadcasts !!

  private static final boolean extra_log = true;//false;

  public svc_bcr () {                                                   // Need empty constructor since system will start via AndroidManifest.xml, before app ever starts
    com_uti.logx ("");    // Regular creation

    com_uti.logd ("stat_constrs: " + stat_constrs++);
  }



    // Media buttons:
  @Override
  public void onReceive (Context context, Intent intent) {
    try {
      if (extra_log)
        com_uti.logd ("gui_act.m_com_api: " + gui_act.m_com_api);

      if (gui_act.m_com_api == null) {
        gui_act.m_com_api = new com_api (context);
        com_uti.logd ("gui_act.m_com_api: " + gui_act.m_com_api);
      }

      if (gui_act.m_com_api == null) {
        com_uti.loge ("gui_act.m_com_api == null, no action");
        return;
      }
      if (extra_log)
        com_uti.logd ("tuner_state: " + gui_act.m_com_api.tuner_state);

      String action = intent.getAction ();
      if (extra_log)
        com_uti.logd ("context: " + context + "  intent: " + intent + "  action: " + action);
      if (action == null)
        return;
//      if (action.equalsIgnoreCase ("fm.a2d.sf.result.get")) {
//        radio_update (context, intent);
//        return;
//      }

        // radio_update above must happen before state checking to see if media button events can be sent to svc_svc
      if (gui_act.m_com_api.tuner_state.equalsIgnoreCase ("stop")) {
        com_uti.logd ("tuner_state == stop, no action");
        return;
      }

      if (action.equalsIgnoreCase (android.media.AudioManager.ACTION_AUDIO_BECOMING_NOISY)) {   // !!!! Get this at bootup !!!!  ; Disabled for now
        com_uti.logd ("audio noisy");
        //gui_act.m_com_api.key_set ("audio_state", "pause");
      }
      else if (action.equalsIgnoreCase (Intent.ACTION_MEDIA_BUTTON)) {
        handle_key_event (context, (KeyEvent) intent.getExtras ().get (Intent.EXTRA_KEY_EVENT));
      }
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }
/*
  private void radio_update (Context context, Intent intent) {
    if (extra_log)
      com_uti.logd ("");

    if (gui_act.m_com_api == null) {
      gui_act.m_com_api = new com_api (context);
      com_uti.logd ("gui_act.m_com_api: " + gui_act.m_com_api);
    }

    gui_act.m_com_api.radio_update (intent);                                    // Context change issues between broadcasts ???

    if (extra_log)
      com_uti.logd ("tuner_state: " + gui_act.m_com_api.tuner_state + "  audio_state: " + gui_act.m_com_api.audio_state);
  }
*/

  private void handle_key_event (Context context, KeyEvent key_event) {
  }

}
