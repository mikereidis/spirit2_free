
    // GUI API < Activity

package fm.a2d.sf;

import android.view.View;
import android.os.Bundle;
import android.app.Dialog;
import android.content.Intent;

public interface gui_gap {                                              // GUI API definition:

  public abstract boolean   gap_state_set      (String state);
  public abstract Dialog    gap_dialog_create  (int id, Bundle args);
  public abstract void      gap_radio_update   (Intent intent);
  public abstract void      gap_gui_clicked    (View v);

}
