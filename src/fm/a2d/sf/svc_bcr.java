
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

public class svc_bcr extends BroadcastReceiver {                        // !! Operates in same process as gui_act !!

  private static int m_obinits = 0;

  public svc_bcr () {                                                   // Need empty constructor since system will start via AndroidManifest.xml, before app ever starts
    m_obinits ++;
    com_uti.logd ("m_obinits: " + m_obinits);

    com_uti.logd ("this: " + this);
  }

    // Media buttons:

  @Override
  public void onReceive (Context context, Intent intent) {
    try {
      com_uti.logd ("gui_act.m_com_api: " + gui_act.m_com_api);

      if (gui_act.m_com_api == null) {
        gui_act.m_com_api = new com_api (context);                      // !! Operates in same process as gui_act !!
        com_uti.logd ("gui_act.m_com_api == null    gui_act.m_com_api: " + gui_act.m_com_api);
      }

      if (gui_act.m_com_api == null) {
        com_uti.loge ("gui_act.m_com_api == null, no action");
        return;
      }

      com_uti.logd ("tuner_state: " + gui_act.m_com_api.tuner_state);

      String action = intent.getAction ();
      com_uti.logd ("context: " + context + "  intent: " + intent + "  action: " + action);
      if (action == null)
        return;

      if (! gui_act.m_com_api.tuner_state.equals ("Start")) {
        com_uti.logd ("tuner_state != Start so ignore");
        return;
      }

                                                                        // Get this at bootup
      if (action.equals (android.media.AudioManager.ACTION_AUDIO_BECOMING_NOISY)) {
        com_uti.logd ("Wired Headset/Antenna Unplugged audio_state: " + gui_act.m_com_api.audio_state);               // Note duplication in svc_aud w/ m_hdst_plgd

        if (gui_act.m_com_api.audio_state.equals ("Start"))             // !! Otherwise get pause timeout when pulling wired headset during phone call
          gui_act.m_com_api.key_set ("audio_state", "Pause");
      }
      else if (action.equals (Intent.ACTION_MEDIA_BUTTON)) {
        handle_key_event (context, (KeyEvent) intent.getExtras ().get (Intent.EXTRA_KEY_EVENT));
      }
      else if (action.equals ("android.media.MASTER_VOLUME_CHANGED_ACTION")) {
        com_uti.loge ("Action android.media.MASTER_VOLUME_CHANGED_ACTION");
      }
      else if (action.equals ("android.media.VOLUME_CHANGED_ACTION")) {
        com_uti.loge ("Action android.media.VOLUME_CHANGED_ACTION");
      }
      else {
        com_uti.loge ("Unknown action  context: " + context + "  intent: " + intent + "  action: " + action);
      }
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }

  private void handle_key_event (Context context, KeyEvent key_event) {
    com_uti.logd ("context: " + context + "  key_event: " + key_event);

    int key_event_action = key_event.getAction ();
    if (key_event_action != KeyEvent.ACTION_DOWN) {
      if (key_event_action == KeyEvent.ACTION_UP)
        com_uti.logd ("Key event action UP: " + key_event_action);
      else
        com_uti.loge ("Key event action not down or up: " + key_event_action);
      return;
    }

    switch (key_event.getKeyCode ()) {

      case KeyEvent.KEYCODE_HEADSETHOOK:
      case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
        gui_act.m_com_api.key_set ("audio_state", "Toggle");
        break;
      case KeyEvent.KEYCODE_MEDIA_PLAY:
        gui_act.m_com_api.key_set ("audio_state", "Start");
        break;
      case KeyEvent.KEYCODE_MEDIA_PAUSE:
        gui_act.m_com_api.key_set ("audio_state", "Pause");
        break;
      case KeyEvent.KEYCODE_MEDIA_STOP:
        gui_act.m_com_api.key_set ("audio_state", "Stop");             // Shut down FM entirely (could pause)
        break;

      case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
        gui_act.m_com_api.key_set ("service_seek_state", "Down");
        break;
      case KeyEvent.KEYCODE_MEDIA_NEXT:
        gui_act.m_com_api.key_set ("service_seek_state", "Up");
        break;
    }
  }

}

