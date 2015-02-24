
    // GUI API < Activity

package fm.a2d.sf;

import android.app.Dialog;

import android.content.Intent;

import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import android.os.Bundle;

public interface gui_gap {                                              // GUI API definition:

  public abstract boolean   gap_state_set       (String state);

  public abstract void      gap_service_update  (Intent intent);

  public abstract void      gap_gui_clicked     (View v);

  public abstract Dialog    gap_dialog_create   (int id, Bundle args);

  public abstract boolean   gap_menu_create     (Menu menu);

  public abstract boolean   gap_menu_select     (int itemid);

}
