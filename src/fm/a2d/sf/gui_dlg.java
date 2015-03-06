
    // GUI Dialog: Aborted attempt at using fragments

package fm.a2d.sf;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;

public class gui_dlg { //extends Dialog implements DialogInterface.OnClickListener {
}

//  Had: public class gui_gui implements gui_gap, gui_dlg.gui_dlg_lstnr {
//       public class gui_dlg extends DialogFragment implements DialogInterface.OnClickListener {
/*
  private gui_dlg_lstnr listener;

  //public gui_dlg () {       //Context,boolean,OnCancelListener
  //public gui_dlg () {       //Context,int
  public gui_dlg (Context context) {       //Context
  }

  public static gui_dlg init (Context context, int icon, String title, String message, String positive, String neutral, String negative) {
    gui_dlg frag = new gui_dlg (context);
    Bundle args = new Bundle ();
    args.putInt ("icon", icon);
    args.putString ("title", title);
    args.putString ("message", message);
    args.putString ("positive", positive);
    args.putString ("neutral", neutral);
    args.putString ("negative", negative);
//    frag.setArguments (args);
    return (frag);
  }

  public interface gui_dlg_lstnr {
    public void on_pos ();
    public void on_neu ();
    public void on_neg ();
  }

  public void setgui_dlg_lstnr (gui_dlg_lstnr listener) {
    this.listener = listener;
  }

  //public void dismiss () {
  //  getDialog ().dismiss ();
  //}

  @Override
  public Dialog onCreateDialog (Bundle savedInstanceState) {
//    int     icon    = getArguments ().getInt    ("icon");
    String title    = "title";      //getArguments ().getString ("title");
    String message  = "message";    //getArguments ().getString ("message");
    String positive = "positive";   //getArguments ().getString ("positive");
    String neutral  = "neutral";    //getArguments ().getString ("neutral");
    String negative = "nagative";   //getArguments ().getString ("negative");

    return new AlertDialog.Builder (getActivity ())
        //.setIcon (icon)
        .setMessage (message)
        .setTitle (title)
        .setPositiveButton (positive, this)
        .setNeutralButton  (neutral, this)
        .setNegativeButton (negative, this)
        .create ();
  }

  @Override
  public void onClick (DialogInterface dialog, int which) {
    if (listener != null) {
      switch (which) {
      case DialogInterface.BUTTON_POSITIVE:
        listener.on_pos ();
      case DialogInterface.BUTTON_NEUTRAL:
        listener.on_neu ();
      case DialogInterface.BUTTON_NEGATIVE:
      default:
        listener.on_neg ();
      }
    }
  }

}
*/

    // implements gui_dlg_lstnr :
/*
  public void on_pos () {
    com_uti.logd ("");
  }
  public void on_neu () {
    com_uti.logd ("");
  }
  public void on_neg () {
    com_uti.logd ("");
  }
*/
/*
  private gui_dlg start_dlg_frag = null;
  private gui_dlg stop_dlg_frag  = null;
  private boolean start_gui_dlg_active = false;
  private boolean stop_gui_dlg_active = false;

  private void start_gui_dlg_show (boolean start) {
    //start_dlg_frag.dismiss ();    // Dismiss previous

    // DialogFragment.show() will take care of adding the fragment in a transaction.  We also want to remove any currently showing dialog, so make our own transaction and take care of that here.
    FragmentTransaction ft = m_gui_act.getFragmentManager ().beginTransaction ();
    Fragment prev = m_gui_act.getFragmentManager ().findFragmentByTag ("start_stop_dialog");
    if (prev != null) {
        ft.remove (prev);
    }
    ft.addToBackStack (null);

    if (start) {
      start_dlg_frag = gui_dlg.init (R.drawable.img_icon_128, m_com_api.service_phase, m_com_api.service_error, null, null, m_context.getString (android.R.string.cancel));//"", m_context.getString (android.R.string.ok), "", null);//"");
      start_dlg_frag.setgui_dlg_lstnr (this);//m_gui_act);
      start_gui_dlg_active = true;
      start_dlg_frag.show (m_gui_act.getFragmentManager (), "start_stop_dialog");
      start_dlg_frag.show (ft, "start_stop_dialog");
    }
    else
      start_gui_dlg_active = false;
  }
*/
/*
  private void stop_gui_dlg_show (boolean start) {
    // DialogFragment.show() will take care of adding the fragment in a transaction.  We also want to remove any currently showing dialog, so make our own transaction and take care of that here.
    FragmentTransaction ft = m_gui_act.getFragmentManager ().beginTransaction ();
    Fragment prev = m_gui_act.getFragmentManager ().findFragmentByTag ("start_stop_dialog");
    if (prev != null) {
        ft.remove (prev);
    }
    ft.addToBackStack (null);

    if (start) {
      stop_dlg_frag = gui_dlg.init (android.R.drawable.stat_sys_headset, "Stop", null, null, null, null);//"", m_context.getString (android.R.string.ok), "", null);//"");
      stop_dlg_frag.setgui_dlg_lstnr (this);//m_gui_act);
      stop_gui_dlg_active = true;
      //stop_dlg_frag.show (m_gui_act.getFragmentManager (), "start_stop_dialog");
      stop_dlg_frag.show (ft, "start_stop_dialog");
    }
    else
      stop_gui_dlg_active = false;
  }
*/

