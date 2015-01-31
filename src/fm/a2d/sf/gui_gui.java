
    // GUI

package fm.a2d.sf;

import java.util.Locale;

import android.bluetooth.BluetoothAdapter;

import android.app.Activity;
import android.widget.Toast;
    
import android.media.AudioManager;
import android.media.audiofx.AudioEffect;

import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;

import android.util.DisplayMetrics;
import android.widget.ImageView;
import android.os.Bundle;
import android.net.Uri;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.DialogInterface;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.text.TextUtils;

import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;

import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;

import java.util.Timer;
import java.util.TimerTask;
import java.util.ArrayList;

public class gui_gui implements gui_gap {

  private static int    m_obinits = 1;
  private int           audio_stream = AudioManager.STREAM_MUSIC;

  private Activity      m_gui_act    = null;
  private Context       m_context    = null;
  private com_api       m_com_api    = null;



    // User Interface:
  private Animation     m_ani_button= null;

  // Text:
  private TextView      m_tv_rssi   = null;
  private TextView      m_tv_state   = null;
  private TextView      m_tv_most   = null;
  private TextView      m_tv_band   = null;
  private TextView      m_tv_freq   = null;

  // RDS data:
  private TextView      m_tv_picl   = null;
  private TextView      m_tv_ps     = null;
  private TextView      m_tv_ptyn   = null;
  private TextView      m_tv_rt     = null;

    // ImageView Buttons:
  private ImageView     m_iv_seekup= null;
  private ImageView     m_iv_seekdn= null;

  private ImageView     m_iv_prev   = null;
  private ImageView     m_iv_next   = null;

  private ImageView     m_iv_paupla = null;
  private ImageView     m_iv_stop   = null;
  private ImageView     m_iv_pause  = null;
  private ImageView     m_iv_mute   = null;
  private ImageView     m_iv_record = null;
  private ImageView     m_iv_menu   = null;
  private ImageView     m_iv_out    = null;                             // ImageView for Speaker/Headset toggle
  private ImageView     m_iv_pwr    = null;

    // Radio Group/Buttons:
  private RadioGroup    m_rg_band   = null;;
  private RadioButton   rb_band_us  = null;
  private RadioButton   rb_band_eu  = null;

  private RadioButton   rb_rate_8  = null;                              // Note used/visible right now
  private RadioButton   rb_rate_22 = null;
  private RadioButton   rb_rate_44 = null;
  private RadioButton   rb_rate_48 = null;

    // Checkboxes:
  private CheckBox      cb_speaker  = null;


    // Presets
  private int           m_presets_curr  = 0;
  private ImageButton[] m_preset_ib     = {null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null};   // 16 Preset Image Buttons
  private TextView   [] m_preset_tv     = {null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null};   // 16 Preset Text Views
  private String     [] m_preset_freq   = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};   //  Frequencies
  private String     [] m_preset_name   = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};   //  Names


  private int           pixel_width     = 480;
  private int           pixel_height    = 800;
  private float         pixel_density   = 1.5f;


    // Dial:                                    !!!! Dial power bounce problem, accidental 2+ hit
  private android.os.Handler delay_dial_handler  = null;
  private Runnable           delay_dial_runnable = null;

  private gui_dia       m_dial          = null;
  private long          last_rotate_time= 0;

  private double        freq_at_210 = 85200;
  private double        freq_percent_factor = 251.5;

  private int           last_dial_freq = -1;

    // Color:
  private int           lite_clr = Color.WHITE;
  private int           dark_clr = Color.GRAY;
  private int           blue_clr = Color.BLUE;


  private Dialog        intro_dialog = null;

  private String        last_rt = "";
  private int           last_int_audio_sessid = 0;


    // Code:

  public gui_gui (Context c, com_api the_com_api) {                     // Constructor
    com_uti.logd ("m_obinits: " + m_obinits++);

    m_context = c;
    m_gui_act = (Activity) c;
    m_com_api = the_com_api;
  }

    // Lifecycle API

  public boolean gap_state_set (String state) {
    boolean ret = false;
    if (state.equalsIgnoreCase ("start"))
      ret = gui_start ();
    else if (state.equalsIgnoreCase ("stop"))
      ret = gui_stop ();
    return (ret);
  }
  private boolean gui_stop () {
    //bcast_lstnr_stop ();
    return (true);
  }


  private boolean gui_start () {

    com_uti.strict_mode_set (false);                                      // !! Hack for s2d comms to allow network activity on UI thread

    DisplayMetrics dm = new DisplayMetrics ();
    m_gui_act.getWindowManager ().getDefaultDisplay ().getMetrics (dm);
    pixel_width  = dm.widthPixels;
    pixel_height = dm.heightPixels;
    pixel_density = m_context.getResources ().getDisplayMetrics ().density;
    com_uti.logd ("pixel_width: " + pixel_width + "  pixel_height: " + pixel_height + "  pixel_density: " + pixel_density);


    m_gui_act.requestWindowFeature (Window.FEATURE_NO_TITLE);            // No title to save screen space
    m_gui_act.setContentView (R.layout.gui_gui_layout);                   // Main Layout


/*  Programmatic harder than XML (?? Should do before setContentView !! ??

//    LinearLayout main_linear_layout =  (LinearLayout) m_gui_act.findViewById (R.id.main_hll);
    LinearLayout                            gui_pg1_layout          =  (LinearLayout) m_gui_act.findViewById (R.id.gui_pg1_layout);
    //FrameLayout                             new_frame_layout        = (FrameLayout) m_gui_act.findViewById (R.id.new_fl);//new FrameLayout (m_context);
    FrameLayout                             new_frame_layout        = new FrameLayout (m_context);
    FrameLayout.LayoutParams new_frame_layout_params = new android.widget.FrameLayout.LayoutParams ((int) (pixel_width / pixel_density), ViewGroup.LayoutParams.MATCH_PARENT);
    //com_uti.loge ("gui_pg1_layout: " + gui_pg1_layout + "  new_frame_layout_params: " + new_frame_layout_params);
    new_frame_layout.addView (gui_pg1_layout, new_frame_layout_params);
//  To:     new_frame_layout    FrameLayout  View       with new_frame_layout_params FrameLayout.LayoutParams
//  add:    gui_pg1_layout   LinearLayout View

    LinearLayout gui_pg2_layout =  (LinearLayout) m_gui_act.findViewById (R.id.gui_pg2_layout);
    FrameLayout                 old_frame_layout = (FrameLayout) m_gui_act.findViewById (R.id.old_fl);//new FrameLayout (m_context);
    FrameLayout.LayoutParams    old_frame_layout_params = new android.widget.FrameLayout.LayoutParams ((int) (pixel_width / pixel_density), ViewGroup.LayoutParams.MATCH_PARENT);
    old_frame_layout.addView (gui_pg2_layout, old_frame_layout_params);

//    main_linear_layout.addView (gui_pg1_layout);
//    main_linear_layout.addView (gui_pg2_layout);
*/

    LinearLayout.LayoutParams frame_layout_params = new android.widget.LinearLayout.LayoutParams ((int) (pixel_width), ViewGroup.LayoutParams.MATCH_PARENT);

    FrameLayout new_fl_view = (FrameLayout) m_gui_act.findViewById (R.id.new_fl);
    new_fl_view.setLayoutParams (frame_layout_params);

    FrameLayout old_fl_view = (FrameLayout) m_gui_act.findViewById (R.id.old_fl);
    old_fl_view.setLayoutParams (frame_layout_params);

    dial_init ();


    m_ani_button = AnimationUtils.loadAnimation (m_context, R.anim.ani_button);// Set button animation

    m_tv_rssi = (TextView)  m_gui_act.findViewById (R.id.tv_rssi);
    m_tv_state = (TextView)  m_gui_act.findViewById (R.id.tv_state);  // License State / Phase
    m_tv_most = (TextView)  m_gui_act.findViewById (R.id.tv_most);

    m_tv_band = (TextView) m_gui_act.findViewById (R.id.tv_band);

    m_tv_picl = (TextView) m_gui_act.findViewById (R.id.tv_picl);
    m_tv_ps   = (TextView) m_gui_act.findViewById (R.id.tv_ps);
    m_tv_ptyn = (TextView) m_gui_act.findViewById (R.id.tv_ptyn);
    m_tv_rt   = (TextView) m_gui_act.findViewById (R.id.tv_rt);

    m_iv_seekdn = (ImageView) m_gui_act.findViewById (R.id.iv_seekdn);
    m_iv_seekdn.setOnClickListener (click_lstnr);

    m_iv_seekup = (ImageView) m_gui_act.findViewById (R.id.iv_seekup);
    m_iv_seekup.setOnClickListener (click_lstnr);

    m_iv_prev = (ImageView) m_gui_act.findViewById (R.id.iv_prev);
    m_iv_prev.setOnClickListener (click_lstnr);
    m_iv_prev.setId (R.id.iv_prev);

    m_iv_next = (ImageView) m_gui_act.findViewById (R.id.iv_next);
    m_iv_next.setOnClickListener (click_lstnr);
    m_iv_next.setId (R.id.iv_next);

    m_tv_freq = (TextView) m_gui_act.findViewById (R.id.tv_freq);
    m_tv_freq.setOnClickListener (click_lstnr);

    m_iv_paupla = (ImageView) m_gui_act.findViewById (R.id.iv_paupla);
    m_iv_paupla.setOnClickListener (click_lstnr);
    m_iv_paupla.setId (R.id.iv_paupla);

    m_iv_stop = (ImageView) m_gui_act.findViewById (R.id.iv_stop);
    m_iv_stop.setOnClickListener (click_lstnr);
    m_iv_stop.setId (R.id.iv_stop);

    m_iv_pause = (ImageView) m_gui_act.findViewById (R.id.iv_pause);
    m_iv_pause.setOnClickListener (click_lstnr);
    m_iv_pause.setId (R.id.iv_pause);

    m_iv_mute = (ImageView) m_gui_act.findViewById (R.id.iv_mute);
    m_iv_mute.setOnClickListener (click_lstnr);
    m_iv_mute.setId (R.id.iv_mute);

    m_iv_record = (ImageView) m_gui_act.findViewById (R.id.iv_record);
    m_iv_record.setOnClickListener (click_lstnr);
    m_iv_record.setId (R.id.iv_record);

    m_iv_menu = (ImageView) m_gui_act.findViewById (R.id.iv_menu);
    m_iv_menu.setOnClickListener (click_lstnr);
    m_iv_menu.setId (R.id.iv_menu);


    rb_band_us = (RadioButton) m_gui_act.findViewById (R.id.rb_band_us);
    rb_band_eu = (RadioButton) m_gui_act.findViewById (R.id.rb_band_eu);

    rb_rate_8  = (RadioButton) m_gui_act.findViewById (R.id.rb_rate_8);
    rb_rate_22 = (RadioButton) m_gui_act.findViewById (R.id.rb_rate_22);
    rb_rate_44 = (RadioButton) m_gui_act.findViewById (R.id.rb_rate_44);
    rb_rate_48 = (RadioButton) m_gui_act.findViewById (R.id.rb_rate_48);

    cb_speaker = (CheckBox) m_gui_act.findViewById (R.id.cb_speaker);


    try {
      lite_clr = Color.parseColor ("#ffffffff");                        // lite like PS
      dark_clr = Color.parseColor ("#ffa3a3a3");                        // grey like RT
      blue_clr = Color.parseColor ("#ff32b5e5");                        // ICS Blue
    }
    catch (Throwable e) {
      e.printStackTrace ();
    };
    m_tv_rt.setTextColor (lite_clr);
    m_tv_ps.setTextColor (lite_clr);

    presets_setup (lite_clr);

    gui_pwr_update (false);

    long curr_time = com_uti.ms_get ();
    long radio_gui_first_time = com_uti.long_get (com_uti.prefs_get (m_context, "radio_gui_first_time", ""));
    if (radio_gui_first_time <= 0L) {
      radio_gui_first_time = curr_time;
      com_uti.prefs_set (m_context, "radio_gui_first_time", "" + curr_time);
    }

    int radio_gui_start_count = com_uti.prefs_get (m_context, "radio_gui_start_count", 0);
    radio_gui_start_count ++;
    com_uti.prefs_set (m_context, "radio_gui_start_count", radio_gui_start_count);


    m_rg_band = (RadioGroup) m_gui_act.findViewById (R.id.rg_band);

    if (radio_gui_start_count <= 1) {                                   // If first 1 runs...
      String cc = com_uti.country_get (m_context).toUpperCase ();
      if (cc.equals ("US") || cc.equals ("CA") || cc.equals ("MX")) {   // If USA, Canada or Mexico
        com_uti.logd ("Setting band US");
        tuner_band_set_non_volatile ("US");                             // Band = US
      }
      else {
        com_uti.logd ("Setting band EU");
        tuner_band_set_non_volatile ("EU");                             // Else Band = EU
      }
      m_gui_act.showDialog (DLG_INTRO);                                 // Show the intro dialog
    }
    else if (radio_gui_start_count <= 3) {                              // If first 3 runs...
      m_gui_act.showDialog (DLG_INTRO);                                 // Show the intro dialog
    }

    String band = com_uti.prefs_get (m_context, "tuner_band", "EU");
    tuner_band_set (band);
    if (m_com_api.tuner_band.equalsIgnoreCase ("US")) {
      rb_band_eu.setChecked (false);
      rb_band_us.setChecked (true);
    }

    m_com_api.key_set ("audio_state", "start");                         // Start audio service

    gui_pwr_update (true);                                              // !!!! Move later to Radio API callback

    visual_state_load_prefs ();

    audio_output_load_prefs ();

    audio_stereo_load_prefs ();

    tuner_stereo_load_prefs ();


    //else
      //svc_rcc.stop ();


    return (true);
  }


  private void dial_init () {

        // Dial Relative Layout:
    RelativeLayout freq_dial_relative_layout = (RelativeLayout) m_gui_act.findViewById (R.id.freq_dial);
    android.widget.RelativeLayout.LayoutParams lp_dial = new android.widget.RelativeLayout.LayoutParams (android.widget.RelativeLayout.LayoutParams.MATCH_PARENT, android.widget.RelativeLayout.LayoutParams.MATCH_PARENT);   // WRAP_CONTENT
    //int dial_size = (pixel_width * 3) / 4;
    int dial_size = (pixel_width * 7) / 8;
    m_dial = new gui_dia (m_context, R.drawable.freq_dial_needle, -1, dial_size, dial_size); // Get dial instance/RelativeLayout view
    lp_dial.addRule (RelativeLayout.CENTER_IN_PARENT);
    freq_dial_relative_layout.addView (m_dial, lp_dial);

        // Dial internal Power Relative Layout:
    android.widget.RelativeLayout.LayoutParams lp_power;
    lp_power = new android.widget.RelativeLayout.LayoutParams (android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT, android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT);
    lp_power.addRule (RelativeLayout.CENTER_IN_PARENT);

    m_iv_pwr = new ImageView (m_context);
    m_iv_pwr.setImageResource (R.drawable.dial_power_off);
    freq_dial_relative_layout.addView (m_iv_pwr, lp_power);

    m_dial.listener_set (new gui_dia.gui_dia_listener () {                      // Setup listener for state_chngd() and dial_chngd()

      public boolean prev_go () {
        if (! m_com_api.tuner_state.equalsIgnoreCase ("start")) {
          com_uti.logd ("via gui_dia abort tuner_state: " + m_com_api.tuner_state);
          return (false);                                               // Not Consumed
        }
        m_iv_prev.startAnimation (m_ani_button);
        m_com_api.key_set ("tuner_freq", "down");
        return (true);                                                  // Consumed
      }
      public boolean next_go () {
        if (! m_com_api.tuner_state.equalsIgnoreCase ("start")) {
          com_uti.logd ("via gui_dia abort tuner_state: " + m_com_api.tuner_state);
          return (false);                                               // Not Consumed
        }
        m_iv_next.startAnimation (m_ani_button);
        m_com_api.key_set ("tuner_freq", "up");
        return (true);                                                  // Consumed
      }
      public boolean state_chngd () {
        com_uti.logd ("via gui_dia m_com_api.audio_state: " + m_com_api.audio_state);
        if (m_com_api.audio_state.equalsIgnoreCase ("start"))
          m_com_api.key_set ("tuner_state", "stop");
        else
          m_com_api.key_set ("audio_state", "start");
        return (true);                                                  // Consumed
      }
      public boolean freq_go () {
        com_uti.logd ("via gui_dia");
        if (! m_com_api.tuner_state.equalsIgnoreCase ("start")) {
          com_uti.logd ("via gui_dia abort tuner_state: " + m_com_api.tuner_state);
          return (false);                                               // Not Consumed
        }
        if (last_dial_freq < 87500 || last_dial_freq > 108000)
          return (false);                                               // Not Consumed
        m_com_api.key_set ("tuner_freq", "" + last_dial_freq);
        return (true);                                                  // Consumed
      }
      private int last_dial_freq = 0;
      public boolean dial_chngd (double angle) {
        if (! m_com_api.tuner_state.equalsIgnoreCase ("start")) {
          com_uti.logd ("via gui_dia abort tuner_state: " + m_com_api.tuner_state);
          return (false);                                               // Not Consumed
        }
        long curr_time = com_uti.ms_get ();
        int freq = dial_freq_get (angle);
        com_uti.logd ("via gui_dia angle: " + angle + "  freq: " + freq);
        freq += 25;
        freq /= 50;
        freq *= 50;
        if (freq < 87500 || freq > 108000)
          return (false);                                               // Not Consumed
        freq = com_uti.tnru_freq_enforce (freq);
        com_uti.logd ("via gui_dia freq: " + freq + "  curr_time: " + curr_time + "  last_rotate_time: " + last_rotate_time);
        dial_freq_set (freq);   // !! Better to set fast !!
//        if (last_rotate_time >= 0 || last_rotate_time + 1000 > curr_time) {
//          com_uti.logd ("via gui_dia NOW freq set");
//          last_rotate_time = com_uti.ms_get ();
//          m_com_api.key_set ("tuner_freq", "" + freq);
          last_dial_freq = freq;
//        }
//        else
//          com_uti.logd ("via gui_dia DELAY freq set");

        if (delay_dial_handler != null) {
          if (delay_dial_runnable != null)
            delay_dial_handler.removeCallbacks (delay_dial_runnable);
        }
        else
        /*android.os.Handler*/ delay_dial_handler = new android.os.Handler ();

        delay_dial_runnable = new Runnable () {

        //delay_dial_handler.postDelayed (new Runnable () { 
          public void run() { 
            m_com_api.key_set ("tuner_freq", "" + last_dial_freq);
          } 
        //}, 200);    // 0.2 second delay
        };

        delay_dial_handler.postDelayed (delay_dial_runnable, 50);

        return (true);                                                  // Consumed

        //tv2.post (new Runnable() {                                    // !! ADD delayed runnable for freq
        //  public void run () {
        //    tv2.setText ("\n" + angle + "%\n");
        //  }
        //});
      }
    });


  }

/*  Original top  =   0 degrees =  97.0 MHz
    ""       right=  90 degrees = 104.6 MHz
    ""       left = 270 degrees =  89.2 MHz

90 deg = 7.8
90 deg = 7.6

85 - 109    -> 87.5 - 108, middle = 97.75

Rotate counter by 0.75 MHz = 8.766 degrees

*/


  private int dial_freq_get (double angle) {
    double percent = (angle + 150) / 3;
    return ((int) (freq_percent_factor * percent + freq_at_210));
  }


  private void dial_freq_set (int freq) {
    if (last_dial_freq == freq)                                         // Optimize
      return;
    if (m_dial == null)
      return;
    last_dial_freq = freq;
    double percent = (freq - freq_at_210) / freq_percent_factor;
    double angle = (percent * 3) - 150;
    m_dial.dial_angle_set (angle);
  }



    // Visualizer:


  private void presets_setup (int clr) {    // preset_

    // 16 Max Preset Buttons hardcoded
    m_preset_tv [0] = (TextView) m_gui_act.findViewById (R.id.tv_preset_0);
    m_preset_tv [1] = (TextView) m_gui_act.findViewById (R.id.tv_preset_1);
    m_preset_tv [2] = (TextView) m_gui_act.findViewById (R.id.tv_preset_2);
    m_preset_tv [3] = (TextView) m_gui_act.findViewById (R.id.tv_preset_3);
    m_preset_tv [4] = (TextView) m_gui_act.findViewById (R.id.tv_preset_4);
    m_preset_tv [5] = (TextView) m_gui_act.findViewById (R.id.tv_preset_5);
    m_preset_tv [6] = (TextView) m_gui_act.findViewById (R.id.tv_preset_6);
    m_preset_tv [7] = (TextView) m_gui_act.findViewById (R.id.tv_preset_7);
    m_preset_tv [8] = (TextView) m_gui_act.findViewById (R.id.tv_preset_8);
    m_preset_tv [9] = (TextView) m_gui_act.findViewById (R.id.tv_preset_9);
    m_preset_tv [10]= (TextView) m_gui_act.findViewById (R.id.tv_preset_10);
    m_preset_tv [11]= (TextView) m_gui_act.findViewById (R.id.tv_preset_11);
    m_preset_tv [12]= (TextView) m_gui_act.findViewById (R.id.tv_preset_12);
    m_preset_tv [13]= (TextView) m_gui_act.findViewById (R.id.tv_preset_13);
    m_preset_tv [14]= (TextView) m_gui_act.findViewById (R.id.tv_preset_14);
    m_preset_tv [15]= (TextView) m_gui_act.findViewById (R.id.tv_preset_15);

    m_preset_ib [0] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_0);
    m_preset_ib [1] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_1);
    m_preset_ib [2] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_2);
    m_preset_ib [3] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_3);
    m_preset_ib [4] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_4);
    m_preset_ib [5] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_5);
    m_preset_ib [6] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_6);
    m_preset_ib [7] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_7);
    m_preset_ib [8] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_8);
    m_preset_ib [9] = (ImageButton) m_gui_act.findViewById (R.id.ib_preset_9);
    m_preset_ib [10]= (ImageButton) m_gui_act.findViewById (R.id.ib_preset_10);
    m_preset_ib [11]= (ImageButton) m_gui_act.findViewById (R.id.ib_preset_11);
    m_preset_ib [12]= (ImageButton) m_gui_act.findViewById (R.id.ib_preset_12);
    m_preset_ib [13]= (ImageButton) m_gui_act.findViewById (R.id.ib_preset_13);
    m_preset_ib [14]= (ImageButton) m_gui_act.findViewById (R.id.ib_preset_14);
    m_preset_ib [15]= (ImageButton) m_gui_act.findViewById (R.id.ib_preset_15);
    for (int idx = 0; idx < com_api.max_presets; idx ++) {               // For all presets...
      m_preset_ib [idx].setOnClickListener (preset_go_lstnr);
      m_preset_ib [idx].setOnLongClickListener (preset_chng_lstnr);
      //m_preset_ib [idx].setVisibility (View.INVISIBLE);
    }


    for (int idx = 0; idx < com_api.max_presets; idx ++) {               // For all presets...
      String name = com_uti.prefs_get (m_context, "radio_name_prst_" + idx, "");
      String freq = com_uti.prefs_get (m_context, "radio_freq_prst_" + idx, "");
      if (freq != null && ! freq.equals ("")) {                         // If non empty frequency (if setting exists)
        m_presets_curr = idx + 1;                                       // Update number of presets
        m_preset_freq [idx] = freq;
        m_preset_name [idx] = name;

        if (name != null)
          m_preset_tv [idx].setText (name);
        else //if (freq != null && ! freq.equals (""))
          m_preset_tv [idx].setText ("" + ((double) com_uti.int_get (freq)) / 1000);
        m_preset_ib [idx].setImageResource (R.drawable.transparent);
        //m_preset_ib [idx].setEnabled (false);
      }
      else {
        m_preset_ib [idx].setImageResource (R.drawable.btn_preset);
        //m_preset_ib [idx].setEnabled (true);

      }
      m_preset_tv [idx].setTextColor (clr);

    }

    com_uti.logd ("m_presets_curr: " + m_presets_curr);
  }


  private void text_default () {
    m_tv_state.setText   ("");
    m_tv_most.setText   ("");
    m_tv_band.setText   ("");

    m_tv_rssi.setText   ("");
    m_tv_ps.setText     ("");
    m_tv_picl.setText   ("");
    m_tv_ptyn.setText   ("");
    m_tv_rt.setText     ("");
    m_tv_rt.setSelected (true);      // Need for marquis
    //m_tv_rt.setText     ("");
  }


  private void eq_start () {
    int int_sessid = com_uti.int_get (m_com_api.audio_sessid);
    com_uti.logd ("audio_sessid: " + m_com_api.audio_sessid + "  int_sessid: " + int_sessid);
    try {   // !! Not every phone has EQ installed !!
      Intent i = new Intent (AudioEffect.ACTION_DISPLAY_AUDIO_EFFECT_CONTROL_PANEL);
      i.putExtra (AudioEffect.EXTRA_AUDIO_SESSION, int_sessid);            //The EXTRA_CONTENT_TYPE extra will help the control panel application customize both the UI layout and the default audio effect settings if none are already stored for the calling application. 
      m_gui_act.startActivityForResult (i, 0);//-1);
    }
    catch (Throwable e) {
      com_uti.loge ("exception");
    }
  }


    // Dialog methods:

  private static final int DLG_INTRO    = 1;                            // First few times show this Intro
  private static final int DLG_FREQ_SET = 2;                            // Frequency set
  private static final int DLG_MENU     = 3;                            // Menu
  private static final int DLG_PRST     = 4;                            // Preset functions
  private static final int DLG_PRST_REN = 5;                            // Preset rename

  public Dialog gap_dialog_create (int id, Bundle args) {               // Create a dialog by calling specific *_dialog_create function    ; Triggered by showDialog (int id);
  //public DialogFragment gap_dialog_create (int id, Bundle args) {
    //com_uti.logd ("id: " + id + "  args: " + args);
    Dialog ret = null;
    //DialogFragment ret = null;
    switch (id) {
      case DLG_INTRO:
        ret = intro_dialog_create     (id);
        intro_dialog = ret;
        break;
      case DLG_MENU:
        ret = menu_dialog_create  (id);
        break;
      case DLG_FREQ_SET:
        ret = freq_set_dialog_create  (id);
        break;
      case DLG_PRST:
        ret = prst_dialog_create  (id);
        break;
      case DLG_PRST_REN:
        ret = prst_ren_dialog_create  (id);
        break;
    }
    //com_uti.logd ("dialog: " + ret);
    return (ret);
  }
/*
  private void app_install (String filename) {

    m_com_api.key_set ("tuner_state", "Stop");                      // Full power down/up

//    Intent intent = new Intent (Intent.ACTION_VIEW)
//        .setDataAndType (Uri.parse (Environment.getExternalStorageDirectory() + "/download/" + filename), "application/vnd.android.package-archive");

    Intent intent = new Intent (Intent.ACTION_VIEW);
    intent.setDataAndType (Uri.fromFile (new java.io.File (com_uti.getExternalStorageDirectory() + "/download/" + filename)), "application/vnd.android.package-archive");
    intent.setFlags (Intent.FLAG_ACTIVITY_NEW_TASK);

    m_gui_act.startActivity (intent);

    m_gui_act.finish ();
  }
*/
  private Dialog menu_dialog_create (final int id) {
    com_uti.logd ("id: " + id);

    AlertDialog.Builder dlg_bldr = new AlertDialog.Builder (m_context);

    dlg_bldr.setTitle ("SpiritF " + com_uti.app_version_get (m_context));

    boolean free = true;

    String menu_msg = "Select EQ (Equalizer) or Cancel.";
    if (free)
      menu_msg = "Select Go Pro or Cancel.";

    if (com_uti.device == com_uti.DEV_ONE || com_uti.device == com_uti.DEV_LG2 || com_uti.device == com_uti.DEV_XZ2) {
      if (free)
        menu_msg = "Select Go Pro, Install BT Shim, Remove BT Shim.";
      else
        menu_msg = "Select EQ (Equalizer), Install BT Shim, Remove BT Shim.";
    }

    dlg_bldr.setMessage (menu_msg);

    if (free) {
      dlg_bldr.setNegativeButton ("Go Pro", new DialogInterface.OnClickListener () {
        public void onClick (DialogInterface dialog, int whichButton) {
          purchase ("fm.a2d." + "s2");  // Avoid app name change
        }
      });
    }
    else {
      dlg_bldr.setNegativeButton ("EQ", new DialogInterface.OnClickListener () {
        public void onClick (DialogInterface dialog, int whichButton) {
          eq_start ();
        }
      });
    }

    if (com_uti.device == com_uti.DEV_ONE || com_uti.device == com_uti.DEV_LG2 || com_uti.device == com_uti.DEV_XZ2) {

      dlg_bldr.setNeutralButton ("IBTS", new DialogInterface.OnClickListener () {     //
      public void onClick (DialogInterface dialog, int whichButton) {

        if (! com_uti.shim_files_operational_get ())
          com_uti.shim_install ();
        else
          Toast.makeText (m_context, "Shim file already installed !!", Toast.LENGTH_LONG).show ();
      }
    });

    dlg_bldr.setPositiveButton ("RBTS", new DialogInterface.OnClickListener () {     //
      public void onClick (DialogInterface dialog, int whichButton) {
        com_uti.shim_remove ();
      }
    });

    }
    else {

    dlg_bldr.setPositiveButton ("Cancel", new DialogInterface.OnClickListener () {     //
      public void onClick (DialogInterface dialog, int whichButton) {
        //purchase ("fm.a2d.sf");
      }
    });

    }

    return (dlg_bldr.create ());
  }



  private Dialog intro_dialog_create (final int id) {
  //private DialogFragment intro_dialog_create (final int id) {
    com_uti.logd ("id: " + id);

    AlertDialog.Builder dlg_bldr = new AlertDialog.Builder (m_context);

    dlg_bldr.setTitle ("SpiritF " + com_uti.app_version_get (m_context));

    String intro_msg = "";
    if (! com_uti.file_get ("/system/bin/su") && ! com_uti.file_get ("/system/xbin/su"))
      intro_msg +=  "ERROR: NO SuperUser/SuperSU/Root  SpiritF REQUIRES Root.\n\n";
    else if (com_uti.device == com_uti.DEV_UNK)
      intro_msg +=  "ERROR: Unknown Device. SpiritF REQUIRES International GS1/GS2/GS3/Note/Note2, HTC One, LG G2, Xperia Z+/Qualcomm.\n\n";

    else if (com_uti.device == com_uti.DEV_LG2 || com_uti.device == com_uti.DEV_ONE || com_uti.device == com_uti.DEV_XZ2)
      intro_msg +=  "SpiritF Starting...\n\n" +
        "HTC One, LG G2 & Sony Z2+ can take 15 seconds and may REBOOT on install.\n\n";

    else
      intro_msg +=  "SpiritF Starting...\n\n";

    dlg_bldr.setMessage (intro_msg);

    return (dlg_bldr.create ());
  }

  void tuner_band_set (String band) {
    m_com_api.tuner_band = band;
    com_uti.tnru_band_set (band);                                            // To setup band values; different process than service
    m_com_api.key_set ("tuner_band", band);
  }

  void tuner_band_set_non_volatile (String band) {
    tuner_band_set (band);
    m_com_api.key_set ("tuner_band", band);
  }


  void purchase (String app_name) {                                     // Purchase app_name: fm.a2d.sf

    try {
      m_gui_act.startActivity (new Intent (Intent.ACTION_VIEW, Uri.parse ("market://details?id=" + app_name)));                  // Market only
      return;   // Done
    }
    catch (android.content.ActivityNotFoundException e) {
      com_uti.loge ("There is no Google Play app installed");
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }

    try {
      //m_gui_act.startActivity (new Intent (Intent.ACTION_VIEW, Uri.parse ("http://market.android.com/details?" + app_name))); // Choose browser or market (Browser only if no market)
      m_gui_act.startActivity (new Intent (Intent.ACTION_VIEW, Uri.parse ("https://play.google.com/store/apps/details?id=" + app_name))); // Choose browser or market (Browser only if no market)
    }
    catch (android.content.ActivityNotFoundException e) {
      com_uti.loge ("There is no browser or market app installed");
    }
    catch (Throwable e) {
      e.printStackTrace ();
    }
  }



    // Regional settings:
  private static final int        m_freq_lo   = 87500;
  private static final int        m_freq_hi   = 108000;

  private Dialog freq_set_dialog_create (final int id) {                   // Set new frequency
    com_uti.logd ("id: " + id);
    LayoutInflater factory = LayoutInflater.from (m_context);
    final View edit_text_view = factory.inflate (R.layout.edit_number, null);
    AlertDialog.Builder dlg_bldr = new AlertDialog.Builder (m_context);
    dlg_bldr.setTitle ("Set Frequency"/*R.string.dialog_freqset_title*/);
    dlg_bldr.setView (edit_text_view);
    dlg_bldr.setPositiveButton ("OK"/*R.string.alert_dialog_ok*/, new DialogInterface.OnClickListener () {

      public void onClick (DialogInterface dialog, int whichButton) {

        EditText edit_text = (EditText) edit_text_view.findViewById (R.id.edit_number);
        CharSequence newFreq = edit_text.getEditableText ();
        String nFreq = String.valueOf (newFreq);                        // Get entered text as String
        freq_set (nFreq);

        //m_gui_act.showDialog (DLG_FREQ_SET);
      }
    });
    dlg_bldr.setNegativeButton ("Cancel"/*R.string.alert_dialog_cancel*/, new DialogInterface.OnClickListener () {
      public void onClick (DialogInterface dialog, int whichButton) {
      }
    });                         
    return (dlg_bldr.create ());
  }

// _PRST
  private Dialog prst_ren_dialog_create (final int id) {                 // Rename preset
    com_uti.logd ("id: " + id);
    LayoutInflater factory = LayoutInflater.from (m_context);
    final View edit_text_view = factory.inflate (R.layout.edit_text, null);
    AlertDialog.Builder dlg_bldr = new AlertDialog.Builder (m_context);
    dlg_bldr.setTitle ("Preset Rename");//R.string.dialog_presetlist_rename_title);
    dlg_bldr.setView (edit_text_view);
    dlg_bldr.setPositiveButton ("OK"/*R.string.alert_dialog_ok*/, new DialogInterface.OnClickListener () {
      public void onClick (DialogInterface dialog, int whichButton) {
        EditText edit_text = (EditText) edit_text_view.findViewById (R.id.edit_text);
        CharSequence new_name = edit_text.getEditableText ();
        String name = String.valueOf (new_name);
        preset_rename (cur_preset_idx, name);
      }
    });
    dlg_bldr.setNegativeButton ("Cancel"/*R.string.alert_dialog_cancel*/, new DialogInterface.OnClickListener () {
      public void onClick (DialogInterface dialog, int whichButton) {
      }
    });
    return (dlg_bldr.create ());
  }


  private Dialog prst_dialog_create (final int id) {
    com_uti.logd ("id: " + id);
    AlertDialog.Builder dlg_bldr = new AlertDialog.Builder (m_context);
    dlg_bldr.setTitle ("m_preset");//_button_sttn.name_get ());                        // Title = Station Name !! Variable
    ArrayList <String> array_list = new ArrayList <String> ();
    array_list.add ("Tune");//getResources ().getString (R.string.preset_tune));
    array_list.add ("Replace");//getResources ().getString (R.string.preset_replace));
    array_list.add ("Rename");//getResources ().getString (R.string.preset_rename));
    array_list.add ("Delete");//getResources ().getString (R.string.preset_delete));

    dlg_bldr.setOnCancelListener (new DialogInterface.OnCancelListener () {
      public void onCancel (DialogInterface dialog) {
      }
    });
    String [] items = new String [array_list.size ()];
    array_list.toArray (items);

    dlg_bldr.setItems (items, new DialogInterface.OnClickListener () {
      public void onClick (DialogInterface dialog, int item) {

        switch (item) {
          case 0:                                                       // Tune to station
            com_uti.logd ("prst_dialog_create onClick Tune freq: " + m_preset_freq [cur_preset_idx]);
            com_uti.logd ("prst_dialog_create onClick Tune name: " + m_preset_name [cur_preset_idx]);
            freq_set ("" + m_preset_freq [cur_preset_idx]);             // Change to preset frequency
            break;
          case 1:                                                       // Replace preset with currently tuned station
            com_uti.logd ("prst_dialog_create onClick Replace freq: " + m_preset_freq [cur_preset_idx]);
            com_uti.logd ("prst_dialog_create onClick Replace name: " + m_preset_name [cur_preset_idx]);
            preset_set (cur_preset_idx);
            com_uti.logd ("prst_dialog_create onClick Replace freq: " + m_preset_freq [cur_preset_idx]);
            com_uti.logd ("prst_dialog_create onClick Replace name: " + m_preset_name [cur_preset_idx]);
            break;
          case 2:                                                       // Rename preset
            com_uti.logd ("prst_dialog_create onClick Rename freq: " + m_preset_freq [cur_preset_idx]);
            com_uti.logd ("prst_dialog_create onClick Rename name: " + m_preset_name [cur_preset_idx]);
            m_gui_act.showDialog (DLG_PRST_REN);
            com_uti.logd ("prst_dialog_create onClick Rename freq: " + m_preset_freq [cur_preset_idx]);
            com_uti.logd ("prst_dialog_create onClick Rename name: " + m_preset_name [cur_preset_idx]);
            break;
          case 3:                                                       // Delete preset   !! Deletes w/ no confirmation
            com_uti.logd ("prst_dialog_create onClick Delete freq: " + m_preset_freq [cur_preset_idx]);
            com_uti.logd ("prst_dialog_create onClick Delete name: " + m_preset_name [cur_preset_idx]);
            preset_delete (cur_preset_idx);
            com_uti.logd ("prst_dialog_create onClick Delete freq: " + m_preset_freq [cur_preset_idx]);
            com_uti.logd ("prst_dialog_create onClick Delete name: " + m_preset_name [cur_preset_idx]);
            break;
          default:                                                      // Should not happen
            break;
        }

    }
  });

    return (dlg_bldr.create ());
  }


  private void freq_set (String nFreq) {
    if (TextUtils.isEmpty (nFreq)) {                                    // If an empty string...
      com_uti.loge ("nFreq: " + nFreq);
      return;
    }
    Float ffreq = 0f;
    try {
      ffreq = Float.valueOf (nFreq);
    }
    catch (Throwable e) {
      com_uti.loge ("ffreq = Float.valueOf (nFreq); failed");
      //e.printStackTrace ();
    }
    int freq = (int) (ffreq * 1000);
    if (freq < 0.1) {
      return;
    }
    else if (freq > 199999 && freq < 300000) {                          // Codes 200 000 - 2xx xxx - 299 999 = get/set private Broadcom/Qualcomm control
      com_uti.logd ("get/set private Qualcomm/V4L control: " + freq);
      m_com_api.key_set ("tuner_extra_cmd", "" + freq);
      return;
    }
    else if (freq < 46001 && freq > 45999) {                            // Code 46 = Remove libbt-hci.so BT shim
      com_uti.shim_remove ();
      return;
    }
    else if (freq < 47001 && freq > 46999) {                            // Code 47 = disable monitoring
      //com_uti.prefs_set (m_context, "radio_report_period", 0);
      return;
    }
    else if (freq < 48001 && freq > 47999) {                            // Code 48 = Enable transmit
      com_uti.s2_tx_set (true);
      if (! com_uti.m_manufacturer.startsWith ("SONY"))
        Toast.makeText (m_context, "!!! TRANSMIT likely on SONY ONLY !!!", Toast.LENGTH_LONG).show ();
      return;
    }
    else if (freq < 49001 && freq > 48999) {                            // Code 49 = Disable transmit
      com_uti.s2_tx_set (false);
      return;
    }
    else if (freq < 7001 && freq > 6999) {                              // Code 7 = logs_email
      logs_email ();
      return;
    }
    else if (freq >= m_freq_lo * 10 && freq <= m_freq_hi * 10) {      // For 760 - 1080
      freq /= 10;
    }
    else if (freq >= m_freq_lo * 100 && freq <= m_freq_hi * 100) {    // For 7600 - 10800
      freq /= 100;
    }
    else if (freq >= m_freq_lo * 1000 && freq <= m_freq_hi * 1000) {  // For 76000 - 108000
      freq /= 1000;
    }
    if (freq >= m_freq_lo && freq <= m_freq_hi) {
      //need_freq_result = true;
      com_uti.logd ("Frequency changing to : " + freq + " KHz");
      m_com_api.key_set ("tuner_freq", "" + freq);
    }
    else {
      com_uti.loge ("Frequency invalid: " + ffreq);
    }
  }


    // :

  private void gui_pwr_update (boolean pwr) {                            // Enables/disables buttons based on power

    if (pwr) {
      if (m_iv_pwr != null)
        m_iv_pwr.setImageResource (R.drawable.dial_power_on);
    }
    else {
      if (m_iv_pwr != null)
        m_iv_pwr.setImageResource (R.drawable.dial_power_off);
      text_default ();                                                  // Set all displayable text fields to initial OFF defaults
    }

    m_iv_record.setEnabled  (pwr);
                                                                        // Power button is always enabled
    m_iv_seekup.setEnabled  (pwr);
    m_iv_seekdn.setEnabled  (pwr);

    m_tv_rt.setEnabled      (pwr);

    for (int idx = 0; idx < com_api.max_presets; idx ++)                      // For all presets...
      m_preset_ib [idx].setEnabled (pwr);

  }



    // 


    // !! Duplicated from svc_svc:
  private BluetoothAdapter      m_bt_adapter    = null;

  private boolean bta_get () {
    m_bt_adapter = BluetoothAdapter.getDefaultAdapter ();                // Just do this once, shouldn't change
    if (m_bt_adapter == null) {
      com_uti.loge ("BluetoothAdapter.getDefaultAdapter () returned null");
      return (false);
    }
    return (true);
  }

  private boolean bt_get () {                            // !! Should check with pid_get ("bluetoothd"), "brcm_patchram_plus", "btld", "hciattach" etc. for consistency w/ fm_hrdw
    boolean ret = false;                                                //      BUT: if isEnabled () doesn't work, m_bt_adapter.enable () and m_bt_adapter.disable () may not work either.
    if (m_bt_adapter == null)
      if (! bta_get ())
        return (false);
    ret = m_bt_adapter.isEnabled ();
    com_uti.logd ("bt_get isEnabled (): " + ret);
    return (ret);
  }


    // Radio API Callback:

  public void gap_radio_update (Intent intent) {
    //com_uti.logd ("");

    // Audio Session ID:

    int audio_sessid = com_uti.int_get (m_com_api.audio_sessid);
    if (audio_sessid != 0 && last_int_audio_sessid != audio_sessid) {                        // If audio session ID has changed...
      last_int_audio_sessid = audio_sessid;
      com_uti.logd ("m_com_api.audio_sessid: " + m_com_api.audio_sessid + "  audio_sessid: " + audio_sessid);
      if (audio_sessid == 0) {                                          // If no session, do nothing (or stop visual and EQ)
      }
      else {                                                            // Else if session...
      }
    }

    //com_uti.logd ("m_com_api.audio_state: " + m_com_api.audio_state);

    if (m_com_api.audio_state.equalsIgnoreCase ("Start")) {
      if (intro_dialog != null) {
        com_uti.logd ("intro_dialog.dismiss ()");
        intro_dialog.dismiss ();
        intro_dialog = null;

      }
    }

    // Buttons:

    // Mode Buttons at bottom:
    // Mute/Unmute:
    if (m_com_api.audio_state.equalsIgnoreCase ("starting")) {
      com_uti.logd ("Audio starting");
      m_iv_paupla.setImageResource (R.drawable.sel_pause);
    }
    else if (m_com_api.audio_state.equalsIgnoreCase ("start")) {
      m_iv_paupla.setImageResource (R.drawable.sel_pause);
    }
    else if (m_com_api.audio_state.equalsIgnoreCase ("pause")) {
      m_iv_paupla.setImageResource (R.drawable.btn_play);
    }
    else if (m_com_api.audio_state.equalsIgnoreCase ("stop")) {
      m_iv_paupla.setImageResource (R.drawable.btn_play);
    }
    else if (m_com_api.audio_state.equalsIgnoreCase ("stopping")) {
      m_iv_paupla.setImageResource (R.drawable.btn_play);
    }
    else {
      m_iv_paupla.setImageResource (R.drawable.btn_play);//sel_pause);
    }

    // Speaker/Headset:     NOT USED NOW
    if (m_com_api.audio_output.equalsIgnoreCase ("speaker")) {                                  // Else if speaker..., Pressing button goes to headset
      //if (m_iv_out != null)
      //  m_iv_out.setImageResource (android.R.drawable.stat_sys_headset);//ic_volume_bluetooth_ad2p);
//      cb_speaker.setChecked (true);
//com_uti.loge ("Speaker Mode");
    }
    else {                                                              // Pressing button goes to speaker
      //if (m_iv_out != null)
      //  m_iv_out.setImageResource (android.R.drawable.ic_lock_silent_mode_off);
//      cb_speaker.setChecked (false);
//com_uti.loge ("Headset Mode");
    }



    // Record/Stop:
    if (m_com_api.audio_record_state.equalsIgnoreCase ("start")) {
      m_iv_record.setImageResource (R.drawable.btn_record_press);
    }
    else {
      m_iv_record.setImageResource (R.drawable.btn_record);
    }
    // Power:
    if (m_com_api.tuner_state.equalsIgnoreCase ("start"))
      gui_pwr_update (true);
    else
      gui_pwr_update (false);

    //if (false) {
//    if (need_freq_result) {
    //if (radio_update_last_dial_freq

        // Freq Button:
      //int ifreq = com_uti.int_get (m_com_api.tuner_freq);
      //int ifreq = m_com_api.int_tuner_freq;

      int ifreq = (int) (com_uti.double_get (m_com_api.tuner_freq) * 1000);
      ifreq = com_uti.tnru_freq_fix (ifreq + 25);                       // Must fix due to floating point rounding need, else 106.1 = 106.099

//      if (last_dial_freq != ifreq) {
//        if (last_dial_freq_change_start_time + 500 > com_uti.ms_get ()) {
          String freq = null;
          if (ifreq >= 50000 && ifreq < 500000) {
            dial_freq_set (ifreq);
            freq = ("" + (double) ifreq / 1000);
          }
          if (freq != null) {
            m_tv_freq.setText (freq);
          }
          //need_freq_result = false;
          //last_dial_freq_change_start_time = -1;
//        }
//        else if (last_dial_freq_change_start_time < 0)
//          last_dial_freq_change_start_time = com_uti.ms_get ();
//      }

//    }

    m_tv_rssi.setText (m_com_api.tuner_rssi);

    m_tv_band.setText (m_com_api.tuner_band);
/*
    if (m_com_api.tuner_band.equalsIgnoreCase ("US")) {
      rb_band_us.setChecked (true);
      rb_band_eu.setChecked (false);
    }
    else {
      rb_band_us.setChecked (false);
      rb_band_eu.setChecked (true);
    }
*/

    if (m_com_api.tuner_most.equalsIgnoreCase ("Mono"))
      m_tv_most.setText ("");
    else if (m_com_api.tuner_most.equalsIgnoreCase ("Stereo"))
      m_tv_most.setText ("S");
    else
      m_tv_most.setText ("");

    m_tv_state.setText ("" + m_com_api.tuner_state + " " + m_com_api.audio_state);

    m_tv_picl.setText (m_com_api.tuner_rds_picl);

    m_tv_ps.setText (m_com_api.tuner_rds_ps);
    //m_tv_ps2.setText (m_com_api.tuner_rds_ps);

    //m_com_api.tuner_rds_ptyn = com_uti.tnru_rds_ptype_get (m_com_api.tuner_band, com_uti.int_get (m_com_api.tuner_rds_pt));
    m_tv_ptyn.setText (m_com_api.tuner_rds_ptyn);

    if (! last_rt.equalsIgnoreCase (m_com_api.tuner_rds_rt)) {
      //com_uti.loge ("rt changed: " + m_com_api.tuner_rds_rt);
      last_rt = m_com_api.tuner_rds_rt;
      m_tv_rt.setText (m_com_api.tuner_rds_rt);
      //m_tv_rt.setMarqueeRepeatLimit (-1);  // Forever
      m_tv_rt.setSelected (true);
    }

  }



    // UI buttons and other controls

  private void preset_delete (int idx) {
    com_uti.logd ("idx: " + idx);
    m_preset_tv [idx].setText ("");
    m_preset_freq [idx] = "";
    m_preset_name [idx] = "";
    m_preset_ib [idx].setImageResource (R.drawable.btn_preset);
    m_com_api.key_set ("radio_name_prst_" + idx, "delete", "radio_freq_prst_" + idx, "delete");   // !! Current implementation requires simultaneous
  }
  private void preset_rename (int idx, String name) {
    com_uti.logd ("idx: " + idx);
    m_preset_tv [idx].setText (name);
    m_preset_name [idx] = name;
    m_com_api.key_set ("radio_name_prst_" + idx, name, "radio_freq_prst_" + idx, m_preset_freq [idx]);   // !! Current implementation requires simultaneous
  }
  private void preset_set (int idx) {
    com_uti.logd ("idx: " + idx);
    String freq_text = m_com_api.tuner_freq;  //!!"" + com_uti.double_get (m_com_api.tuner_freq);
    if (m_com_api.tuner_band.equalsIgnoreCase ("US")) {
      if (! m_com_api.tuner_rds_picl.equals (""))
        freq_text = m_com_api.tuner_rds_picl;
    }
    else {
      if (! m_com_api.tuner_rds_ps.trim ().equals (""))
        freq_text = m_com_api.tuner_rds_ps;
    }
    m_preset_tv [idx].setText ("" + freq_text);
    m_preset_name [idx] = freq_text;
    m_preset_freq [idx] = m_com_api.tuner_freq;
    m_com_api.key_set ("radio_name_prst_" + idx, "" + freq_text, "radio_freq_prst_" + idx, m_com_api.tuner_freq);   // !! Current implementation requires simultaneous
    m_preset_ib [idx].setImageResource (R.drawable.transparent);  // R.drawable.btn_preset
  }

  private View.OnClickListener preset_go_lstnr = new                    // Tap: Tune to preset
        View.OnClickListener () {
    public void onClick (View v) {
      //v.startAnimation (m_ani_button);
      com_uti.logd ("view: " + v);

      for (int idx = 0; idx < com_api.max_presets; idx ++) {            // For all presets...
        if (v == m_preset_ib [idx]) {                                   // If this preset...
          com_uti.logd ("idx: " + idx);
          try {
            if (m_preset_freq [idx].equals (""))                        // If no preset yet...
              preset_set (idx);                                         // Set preset
            else
              freq_set ("" + m_preset_freq [idx]);                      // Else change to preset frequency
          }
          catch (Throwable e) {
            e.printStackTrace ();
          };
          return;
        }
      }

    }
  };
  private int cur_preset_idx = 0;
  private View.OnLongClickListener preset_chng_lstnr = new              // Tap and Hold: Show preset change options
        View.OnLongClickListener () {
    public boolean onLongClick (View v) {
      //v.startAnimation (m_ani_button);
      com_uti.logd ("view: " + v);

      for (int idx = 0; idx < com_api.max_presets; idx ++) {            // For all presets...
        if (v == m_preset_ib [idx]) {                                   // If this preset...
          cur_preset_idx = idx;
          com_uti.logd ("idx: " + idx);

            if (m_preset_freq [idx].equals (""))                        // If no preset yet...
              preset_set (idx);                                         // Set preset
            else {
              //preset_set (idx);                                       // Else Set/Change preset
              m_gui_act.showDialog (DLG_PRST);                          // Show preset options dialog
            }

          break;
        }
      }
      return (true);                                                    // Consume long click
    }
  };



  private void ani (View v) {
    //v.startAnimation (m_ani_button);
  }




  private View.OnClickListener click_lstnr = new View.OnClickListener () {
    public void onClick (View v) {

      com_uti.logd ("view: " + v);
      ani (v);                                                          // Animate button

      if (v == m_iv_mute) {
        AudioManager m_am = (AudioManager) m_context.getSystemService (Context.AUDIO_SERVICE);
        if (m_am != null) {
          int svol = m_am.getStreamVolume (audio_stream);
          m_am.setStreamVolume (audio_stream, svol, AudioManager.FLAG_SHOW_UI);// Display volume change
        }
      }

      else if (v == m_iv_paupla) {
        /*if (m_com_api.audio_state.equalsIgnoreCase ("start"))
          m_com_api.key_set ("audio_state", "pause");
        else if (m_com_api.audio_state.equalsIgnoreCase ("pause"))
          m_com_api.key_set ("audio_state", "start");
        else
          m_com_api.key_set ("audio_state", "start");*/                 // Full power up like widget

        m_com_api.key_set ("audio_state", "Toggle");
      }
      else if (v == m_iv_stop)
        m_com_api.key_set ("tuner_state", "Stop");                      // Full power down/up

      else if (v == m_iv_out)                                           // Speaker/headset  NOT USED NOW
        m_com_api.key_set ("audio_output", "toggle");

      else if (v == m_iv_record)
        m_com_api.key_set ("audio_record_state", "toggle");

      else if (v == m_tv_freq)                                           // Frequency direct entry
        m_gui_act.showDialog (DLG_FREQ_SET);

      else if (v == m_iv_menu) {
        m_gui_act.showDialog (DLG_MENU);
        //starting_gui_dlg_show (true);
      }

      else if (v == m_iv_seekdn) {                                    // Seek down
        //need_freq_result = true;
        m_com_api.key_set ("tuner_scan_state", "down");
      }

      else if (v == m_iv_seekup) {                                    // Seek up
        m_com_api.key_set ("tuner_scan_state", "up");
      }

      else if (v == m_iv_prev) {
        m_com_api.key_set ("tuner_freq", "down");
      }
      else if (v ==  m_iv_next) {
        m_com_api.key_set ("tuner_freq", "up");
      }

    }
  };


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
  private boolean starting_gui_dlg_active = false;
  private boolean stopping_gui_dlg_active = false;

  private void starting_gui_dlg_show (boolean start) {
    //start_dlg_frag.dismiss ();    // Dismiss previous

    // DialogFragment.show() will take care of adding the fragment in a transaction.  We also want to remove any currently showing dialog, so make our own transaction and take care of that here.
    FragmentTransaction ft = m_gui_act.getFragmentManager ().beginTransaction ();
    Fragment prev = m_gui_act.getFragmentManager ().findFragmentByTag ("start_stop_dialog");
    if (prev != null) {
        ft.remove (prev);
    }
    ft.addToBackStack (null);

    if (start) {
      start_dlg_frag = gui_dlg.init (R.drawable.img_icon_128, m_com_api.radio_phase, m_com_api.radio_error, null, null, m_context.getString (android.R.string.cancel));//"", m_context.getString (android.R.string.ok), "", null);//"");
      start_dlg_frag.setgui_dlg_lstnr (this);//m_gui_act);
      starting_gui_dlg_active = true;
      start_dlg_frag.show (m_gui_act.getFragmentManager (), "start_stop_dialog");
      start_dlg_frag.show (ft, "start_stop_dialog");
    }
    else
      starting_gui_dlg_active = false;
  }
*/
/*
    if (m_com_api.audio_output.equalsIgnoreCase ("speaker")) {                                  // Else if speaker..., Pressing button goes to headset
      //if (m_iv_out != null)
      //  m_iv_out.setImageResource (android.R.drawable.stat_sys_headset);//ic_volume_bluetooth_ad2p);
    }
    else {                                                              // Pressing button goes to speaker
      //if (m_iv_out != null)
      //  m_iv_out.setImageResource (android.R.drawable.ic_lock_silent_mode_off);
    }
*/
/*
  private void stopping_gui_dlg_show (boolean start) {
    // DialogFragment.show() will take care of adding the fragment in a transaction.  We also want to remove any currently showing dialog, so make our own transaction and take care of that here.
    FragmentTransaction ft = m_gui_act.getFragmentManager ().beginTransaction ();
    Fragment prev = m_gui_act.getFragmentManager ().findFragmentByTag ("start_stop_dialog");
    if (prev != null) {
        ft.remove (prev);
    }
    ft.addToBackStack (null);

    if (start) {
      stop_dlg_frag = gui_dlg.init (android.R.drawable.stat_sys_headset, "Stopping", null, null, null, null);//"", m_context.getString (android.R.string.ok), "", null);//"");
      stop_dlg_frag.setgui_dlg_lstnr (this);//m_gui_act);
      stopping_gui_dlg_active = true;
      //stop_dlg_frag.show (m_gui_act.getFragmentManager (), "start_stop_dialog");
      stop_dlg_frag.show (ft, "start_stop_dialog");
    }
    else
      stopping_gui_dlg_active = false;
  }
*/



  private String tuner_stereo_load_prefs () {
    String value = com_uti.prefs_get (m_context, "tuner_stereo", "");
    if (value.equalsIgnoreCase ("Mono"))
      ((CheckBox) m_gui_act.findViewById (R.id.cb_tuner_stereo)).setChecked (false);
    else
      ((CheckBox) m_gui_act.findViewById (R.id.cb_tuner_stereo)).setChecked (true);
    return (value);
  }

  private String audio_stereo_load_prefs () {
    String value = com_uti.prefs_get (m_context, "audio_stereo", "");
    if (value.equalsIgnoreCase ("Mono"))
      ((CheckBox) m_gui_act.findViewById (R.id.cb_audio_stereo)).setChecked (false);
    else
      ((CheckBox) m_gui_act.findViewById (R.id.cb_audio_stereo)).setChecked (true);
    return (value);
  }

  private String audio_output_load_prefs () {
    String value = com_uti.prefs_get (m_context, "audio_output", "");
    if (value.equalsIgnoreCase ("speaker"))
      cb_speaker.setChecked (true);
    else
      cb_speaker.setChecked (false);
    return (value);
  }
  private String audio_output_set_nonvolatile (String value) {  // Called only by speaker/headset checkbox change
    com_uti.logd ("value: " + value);
    m_com_api.key_set ("audio_output", value);
    return (value);                                                     // No error
  }


  private String visual_state_load_prefs () {
    String pref = com_uti.prefs_get (m_context, "visual_state", "");
    String ret = "";
    if (! pref.equals ("")) {
      ret = visual_state_set (pref);

      if (pref.equalsIgnoreCase ("Start"))
        ((CheckBox) m_gui_act.findViewById (R.id.cb_visu)).setChecked (true);
      else
        ((CheckBox) m_gui_act.findViewById (R.id.cb_visu)).setChecked (false);
    }
    return (ret);
  }
  private String visual_state_set_nonvolatile (String state) {
    String ret = visual_state_set (state);
    com_uti.prefs_set (m_context, "visual_state", state);
    return (ret);
  }
  private String visual_state_set (String state) {
    com_uti.logd ("state: " + state);
    if (state.equalsIgnoreCase ("Start")) {
      ((LinearLayout) m_gui_act.findViewById (R.id.vis)).setVisibility (View.VISIBLE);  //dial_init

      m_dial.setVisibility (View.INVISIBLE);
      m_iv_pwr.setVisibility (View.INVISIBLE);
      ((ImageView) m_gui_act.findViewById (R.id.frequency_bar)).setVisibility (View.INVISIBLE);

      int audio_sessid = com_uti.int_get (m_com_api.audio_sessid);
    }
    else {
      ((LinearLayout) m_gui_act.findViewById (R.id.vis)).setVisibility (View.INVISIBLE);//GONE);

      m_dial.setVisibility (View.VISIBLE);
      m_iv_pwr.setVisibility (View.VISIBLE);
      ((ImageView) m_gui_act.findViewById (R.id.frequency_bar)).setVisibility (View.VISIBLE);

    }
    return (state);                                                     // No error
  }


  private void cb_tuner_stereo (boolean checked) {
    com_uti.logd ("checked: " + checked);
    String val = "Stereo";
    if (! checked)
      val = "Mono";
    m_com_api.key_set ("tuner_stereo", val);
  }

  private void cb_audio_stereo (boolean checked) {
    com_uti.logd ("checked: " + checked);
    String val = "Stereo";
    if (! checked)
      val = "Mono";
    m_com_api.key_set ("audio_stereo", val);
  }

  private void cb_af (boolean checked) {
    com_uti.logd ("checked: " + checked);
    if (checked)
      m_com_api.key_set ("tuner_rds_af_state", "Start");
    else
      m_com_api.key_set ("tuner_rds_af_state", "Stop");
  }


  public void gap_gui_clicked (View view) {

    int id = view.getId ();
    com_uti.logd ("id: " + id + "  view: " + view);
    switch (id) {

      case R.id.cb_test:
        break;

      case R.id.cb_visu:
        if (((CheckBox) view).isChecked ())
          visual_state_set_nonvolatile ("Start");
        else
          visual_state_set_nonvolatile ("Stop");
        break;

      case R.id.cb_tuner_stereo:
        cb_tuner_stereo (((CheckBox) view).isChecked ());
        break;

      case R.id.cb_audio_stereo:
        cb_audio_stereo (((CheckBox) view).isChecked ());
        break;

      case R.id.cb_af:
        cb_af (((CheckBox) view).isChecked ());
        break;

      case R.id.cb_speaker:
        if (((CheckBox) view).isChecked ())
          audio_output_set_nonvolatile ("Speaker");
        else
          audio_output_set_nonvolatile ("Headset");
        break;

      case R.id.rb_band_eu:
        tuner_band_set_non_volatile ("EU");
        rb_log (view, (RadioButton) view, ((RadioButton) view).isChecked ());
        break;

      case R.id.rb_band_us:
        tuner_band_set_non_volatile ("US");
        rb_log (view, (RadioButton) view, ((RadioButton) view).isChecked ());
        break;
    }
  }

  private void rb_log (View view, RadioButton rbt, boolean checked) {
    int btn_id = m_rg_band.getCheckedRadioButtonId ();            // get selected radio button from radioGroup
    RadioButton rad_band_btn = (RadioButton) m_gui_act.findViewById (btn_id);            // find the radiobutton by returned id
    com_uti.logd ("view: " + view + "  rbt: " + rbt + "  checked: " + checked + "  rad btn text: " + "  btn_id: " + btn_id + "  rad_band_btn: " + rad_band_btn + 
                "  text: " + rad_band_btn.getText ());
  }


  private boolean logs_email () {
    String fname = "/sdcard/bugreport.txt";
    String cmd = "bugreport > " + fname;
    int ret = com_uti.sys_run (cmd, true);

    String subject = "SpiritF " + com_uti.app_version_get (m_context);
    boolean bret = file_email (subject, fname);
    return (bret);
  }

/*
    String dbgd = "";
    File m_cache_dir = getCacheDir ();                                  // This creates the "cache" directory if it doesn't exist
    dbgd = m_cache_dir.getAbsolutePath ();
    dbgd += "/";
    com_uti.logd ("logs_email debug dir: " + dbgd);
    sys_run ("chmod 777 " + dbgd, 1);


    // !! don't remove sprt_lcstart.txt
    // Was ~ 340 bytes:
    sys_run ("rm  " + dbgd + "sprt_getprop.txt " + dbgd + "sprt_logcat.txt " + dbgd + "sprt_dmesg.txt " + dbgd + "sprt_files.txt " + dbgd + "sprt_ps.txt " + dbgd + "sprt_bugreport.txt " + dbgd + "sprt_audio.txt " + dbgd + "sprt_prefs.txt ", 1);

        // !! Use /sdcard because /data/data/com.WHATEVER.fm/cache/ and /data/local/tmp didn't work well
    //sys_run ("/data/data/com.WHATEVER.fm/files/dologcat -v time > " + dbgd + "sprt_logcat.txt &", 1);    // Collect the logcat in the background

    String subject = "2013 ";
    if (fm_srvc.is_m6)
      subject += "AAAA-FM";
    else if (fm_srvc.is_ul)
      subject += "Unlocked";
    else if (fm_srvc.is_li)
      subject += "Light";
    else
      subject += "Free";

    subject += " v: " + vn_get ();

    if (m_srvc != null)
      //subject += "  " + m_srvc.misc_str_get (1) + "  SU: " + m_srvc.misc_str_get (6) + "  pmo: " + m_srvc.misc_str_get (7) + "  pbr: " + m_srvc.misc_str_get (8) + "  pd: " + m_srvc.misc_str_get (9) + "  pbo: " + m_srvc.misc_str_get (10) + "  pv: " + m_srvc.misc_str_get (11) + "  hw: " + m_srvc.misc_str_get (2) + "  pn: " + m_srvc.misc_str_get (3) + "  bp: " + m_srvc.misc_str_get (4) + "  mv: " + m_srvc.misc_str_get (5) + " ";
      subject += "  " + m_srvc.misc_str_get (1) + "  SU: " + m_srvc.misc_str_get (6) + "  pmo: " + m_srvc.misc_str_get (7) + "  pbr: " + m_srvc.misc_str_get (8) + "  pd: " + m_srvc.misc_str_get (9) + "  mv: " + m_srvc.misc_str_get (5) + " ";
        //              "Fm:SLS Su:0 Bt:0 Ht:0 Mo:SLS"       "SU: 1"
        //              fm_hrdw: fm_info                    itoa (su_type)
        // OEM          "FM: bc"    ti, mo, sa, qc, none
    else
      subject += "  m_srvc = null, No Debug Info";

    if (license_identity != null)
      //loge ("zx lid: " + license_identity);
      subject += license_identity;

    subject += " END";

    sys_run ("echo " + subject + " > " + dbgd + "sprt_getprop.txt", 1);

    sys_run ("getprop >> " + dbgd + "sprt_getprop.txt", 1);
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_getprop.txt", 1);

    if (file_get ("/dev/msm_snd")) {
      sys_run ("killall -9 android.process.media ; killall -9 mediaserver ; " + getFilesDir ().getAbsolutePath () + "/ssd 1 0 2>&1 >  " + dbgd + "sprt_audio.txt", 1);
      sys_run ("kill -9 `pidof android.process.media` ; kill -9 `pidof mediaserver` ; " + getFilesDir ().getAbsolutePath () + "/ssd 1 0 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
      m_srvc.misc_set_get (1, "android.process.media", 1);              // Killall brutal
      m_srvc.misc_set_get (1, "mediaserver", 1);                        // Killall brutal
      sys_run (" " + getFilesDir ().getAbsolutePath () + "/ssd 1 0 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
    }
    else {
      sys_run ("" + getFilesDir ().getAbsolutePath () + "/ssd 1 0 2>&1 >  " + dbgd + "sprt_audio.txt", 1);
    }
    sys_run (getFilesDir ().getAbsolutePath () + "/ssd 2 0 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
    sys_run (getFilesDir ().getAbsolutePath () + "/ssd 3 0 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
    sys_run (getFilesDir ().getAbsolutePath () + "/ssd 4 0 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
    sys_run (getFilesDir ().getAbsolutePath () + "/ssd 4 0 0 0 /dev/snd/controlC1 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
    sys_run (getFilesDir ().getAbsolutePath () + "/ssd 4 0 0 0 /dev/snd/controlC2 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
    sys_run (getFilesDir ().getAbsolutePath () + "/ssd 4 0 0 0 /dev/snd/controlC3 2>&1 >> " + dbgd + "sprt_audio.txt", 1);
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_audio.txt", 1);

    String full_prefs_file = full_prefs_file_get ();
    if (full_prefs_file != null)
      sys_run ("cat " + full_prefs_file + " > " + dbgd + "sprt_prefs.txt", 1);  // !!!!!!!! No cp on some ROMs like P500h 2.2.1 stock so use cat !!!!!!
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_prefs.txt", 1);


    //m_srvc.misc_set_get (1, "dologcat", 0);                           // Killall logcat non-brutal.
    //ms_sleep (1000);                                                  // Wait 1 second

    sys_run ("logcat -d -v time 2>&1 > " + dbgd + "sprt_logcat.txt", 1);// + " 2> " + dbgd + "sprt_err.txt", 1);    // Collect the logcat (?? Some devices this doesn't work ???) With SU can we read /dev/log/main ??
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_logcat.txt", 1);



// !!!! Shell limit seems to be around 256 bytes

    sys_run ("dmesg 2>&1 > " + dbgd + "sprt_dmesg.txt", 1);             // Collect kernel messages
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_dmesg.txt", 1);

    sys_run ("ls -l -a    / " + dbgd + " 2>&1 > " + dbgd + "sprt_files.txt", 1);

    sys_run ("ls -l -a -R " + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt/ " + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/spirit_cfg/ 2>&1 >> " + dbgd + "sprt_files.txt", 1);
    sys_run ("ls -l -a -R /system/ /sbin/ /vendor/ /dev/ /lib/ 2>&1 >> " + dbgd + "sprt_files.txt", 1);
// ~ 220 bytes:
    //sys_run ("ls -l -a -R /sys/bus /sys/dev /sys/kernel /sys/firmware /sys/module /sys/class/misc /sys/class/rfkill/ /sys/class/htc_accessory/fm/ /sys/class/switch/h2w/ /sys/devices/virtual/misc/ 2>&1 >> " +
    sys_run ("ls -l -a -R /proc/asound/ /sys/ 2>&1 >> " +
        dbgd + "sprt_files.txt", 1);
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_files.txt", 1);


    sys_run ("ps > " + dbgd + "sprt_ps.txt", 1);                        // Record processes
    sys_run ("lsmod >> " + dbgd + "sprt_ps.txt", 1);                    // Record modules
    sys_run ("modinfo /lib/modules/* /system/lib/modules/* >> " + dbgd + "sprt_ps.txt", 1);                    // Record module info
    sys_run ("uname -a >> " + dbgd + "sprt_ps.txt", 1);                    // Record kernel etc info
    sys_run ("cat " + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/Download/spiritfm_license.gif | grep -v license_identity >> " + dbgd + "sprt_ps.txt", 1); // Record license
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_ps.txt", 1);

    if (m_srvc.misc_str_get (6).equals ("1"))
      sys_run ("bugreport 2>&1 > " + dbgd + "sprt_bugreport.txt", 1); // Android Bug Report tool
    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_bugreport.txt", 1);

    sys_run ("echo  ======================================================================================================================== >> " + dbgd + "sprt_lcstart.txt", 1);

//    sys_run ("rm  " + dbgd + "sprt_debug.txt", 1);

//sys_run ("mv " + m_cache_dir + "/sprt_log0.txt " + m_cache_dir + "/sprt_log1.txt", 0);    // Now in onCreate

    sys_run ("cat " + dbgd + "sprt_getprop.txt " + dbgd + "sprt_lcstart.txt " + dbgd + "/sprt_log0.txt " + dbgd + "sprt_log1.txt "
      + dbgd + "sprt_logcat.txt " + dbgd + "sprt_dmesg.txt "
      + dbgd + "sprt_files.txt " + dbgd + "sprt_ps.txt " + dbgd + "sprt_bugreport.txt " + dbgd + "sprt_audio.txt " + dbgd + "sprt_prefs.txt "
      + " 2>&1 > " + dbgd + "sprt_debug.txt", 1);

    sys_run ("cat " + dbgd + "sprt_getprop.txt " + dbgd + "sprt_lcstart.txt " + dbgd + "/sprt_log0.txt " + dbgd + "sprt_log1.txt "
      + dbgd + "sprt_logcat.txt " + dbgd + "sprt_dmesg.txt "
      + dbgd + "sprt_files.txt " + dbgd + "sprt_ps.txt " + dbgd + "sprt_bugreport.txt " + dbgd + "sprt_audio.txt " + dbgd + "sprt_prefs.txt "
      + " 2>&1 > " + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt_debug.txt", 1);

    //sys_run ("cat " + dbgd + "sprt_getprop.txt " + dbgd + "sprt_lcstart.txt " + dbgd + "sprt_logcat.txt " + dbgd + "sprt_dmesg.txt " + dbgd + "sprt_files.txt " + dbgd + "sprt_ps.txt " + dbgd + "sprt_bugreport.txt " + dbgd + "sprt_audio.txt " + dbgd + "sprt_prefs.txt "+ " 2>&1 > " +
    //    fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt_dbg.txt", 1);
    // Another copy so we can always delete the debug file, even across re-installs w/ different UIDs !!
    // Not needed owner: ----rwxr-x system   sdcard_rw   455309 2012-02-07 13:42 sprt_debug.txt

// Was ~ 340 bytes:
    //sys_run ("rm  " + dbgd + "sprt_getprop.txt " + dbgd + "sprt_lcstart.txt " + dbgd + "sprt_logcat.txt " + dbgd + "sprt_dmesg.txt " + dbgd + "sprt_files.txt " + dbgd + "sprt_ps.txt " + dbgd + "sprt_bugreport.txt " + dbgd + "sprt_audio.txt " + dbgd + "sprt_prefs.txt ", 1);

    sys_run ("chmod 666 " + dbgd + "sprt_debug.txt", 1);                // Open permissions for email client

    //sys_run ("cat " + dbgd + "sprt_debug.txt > /sdcard/sprt_debug.txt", 1);
    //sys_run ("cat " + dbgd + "sprt_debug.txt > " + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt_debug.txt", 1);

    sys_run ("chmod 666 " + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt_debug.txt", 1);                // Open permissions for email client

    if (! file_get ("" + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt_debug.txt") ) {
      //dlg_dismiss (DLG_WAIT);
      Toast.makeText (apln_context, "Attachment file " + fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt_debug.txt not found.", Toast.LENGTH_LONG).show ();
      return (false);    
    }


    //dlg_dismiss (DLG_WAIT);

    boolean bret = file_email (subject, fm_srvc.spirit_getExternalStorageDirectory ().toString () + "/sprt_debug.txt");
    //sys_run ("/data/data/com.WHATEVER.fm/files/dologcat -v time 2>&1 > " + dbgd + "sprt_logcat.txt &", 1);   // Restart logcat as daemon
    }
    return (bret);
  }
*/

  private boolean file_email (String subject, String filename) {
                    // See http://stackoverflow.com/questions/2264622/android-multiple-email-attachment-using-intent-question
    Intent i = new Intent (Intent.ACTION_SEND);
    i.setType ("message/rfc822");
    //i.setType ("text/plain");
    i.putExtra (Intent.EXTRA_EMAIL  , new String []{"mikereidis@gmail.com"});

    i.putExtra (Intent.EXTRA_SUBJECT, subject);

    //i.putExtra (Intent.EXTRA_TEXT   , "Please write write problem, device/model and ROM/version. Please ensure " + filename + " file is actually attached or send manually. Thanks ! Mike.");

    ////i.putExtra (Intent.EXTRA_STREAM, Uri.fromFile (new File ("" + dbgd + "sprt_debug.txt")));
    //i.putExtra (Intent.EXTRA_STREAM, Uri.parse ("file://" + dbgd + "sprt_debug.txt"));  // File -> attachment

    //i.putExtra (Intent.EXTRA_STREAM, Uri.parse ("file:///sdcard/sprt_debug.txt"));  // File -> attachment
    i.putExtra (Intent.EXTRA_STREAM, Uri.parse ("file://" + filename));  // File -> attachment

    try {
      m_gui_act.startActivity (Intent.createChooser (i, "Send email..."));
    }
    catch (android.content.ActivityNotFoundException e) {
      Toast.makeText (m_context, "No email. Manually send " + filename, Toast.LENGTH_LONG).show ();
    }
    //dlg_dismiss (DLG_WAIT);

    return (true);
  }


    // Was in gui_dlg.java :
/*

// Had: public class gui_gui implements gui_gap, gui_dlg.gui_dlg_lstnr {

package fm.a2d.sf;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;


public class gui_dlg extends DialogFragment implements DialogInterface.OnClickListener {

  private gui_dlg_lstnr listener;

  public static gui_dlg init (int icon, String title, String message, String positive, String neutral, String negative) {
    gui_dlg frag = new gui_dlg ();
    Bundle args = new Bundle ();
    args.putInt ("icon", icon);
    args.putString ("title", title);
    args.putString ("message", message);
    args.putString ("positive", positive);
    args.putString ("neutral", neutral);
    args.putString ("negative", negative);
    frag.setArguments (args);
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
    int     icon    = getArguments ().getInt    ("icon");
    String title    = getArguments ().getString ("title");
    String message  = getArguments ().getString ("message");
    String positive = getArguments ().getString ("positive");
    String neutral  = getArguments ().getString ("neutral");
    String negative = getArguments ().getString ("negative");

    return new AlertDialog.Builder (getActivity ())
        .setIcon (icon)
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
}
