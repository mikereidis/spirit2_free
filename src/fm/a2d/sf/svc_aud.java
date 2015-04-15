
    // Audio Sub-service

package fm.a2d.sf;

import android.media.AudioAttributes;
import android.media.AudioAttributes.Builder;
import android.media.AudioFormat;
//import android.media.AudioFormat.Builder;

import android.app.Service;
import android.media.MediaRecorder;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.Context;
import android.content.IntentFilter;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.os.Build.VERSION_CODES;
import android.os.Environment;
import android.os.PowerManager;
import android.provider.Settings;
import android.provider.Settings.System;

import java.lang.reflect.Method;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.RandomAccessFile;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Locale;


public class svc_aud implements svc_aap, AudioManager.OnAudioFocusChangeListener {

  private static int    m_obinits = 1;

  private AudioManager  m_AM        = null;
  private Context       m_context   = null;
  private svc_acb       m_svc_acb   = null;
  private com_api       m_com_api   = null;

  private int           audiotrack_sessid_int = 0;

  private boolean       thread_pcm_write_active = false;
  private boolean       thread_pcm_read_active  = false;

  private AudioRecord   m_audiorecord = null;
  private Thread        thread_pcm_read         = null;

  private AudioTrack    m_audiotrack            = null;
  private Thread        thread_pcm_write        = null;

  private boolean       m_hdst_plgd = false;
  private BroadcastReceiver m_hdst_lstnr = null;

  private int audiorecord_sessid_int = 0;

  private double audio_digital_amp = 1;                                 // Current Digital Amplification 

    // Up to 32 buffers:
  private static final int  aud_buf_num = 32;   // (For old ALSA w/ smaller buffers 4, 8 skips too often)

  private
  byte []   aud_buf_001, aud_buf_002, aud_buf_003, aud_buf_004, aud_buf_005, aud_buf_006, aud_buf_007, aud_buf_008, aud_buf_009, aud_buf_010,
            aud_buf_011, aud_buf_012, aud_buf_013, aud_buf_014, aud_buf_015, aud_buf_016, aud_buf_017, aud_buf_018, aud_buf_019, aud_buf_020,
            aud_buf_021, aud_buf_022, aud_buf_023, aud_buf_024, aud_buf_025, aud_buf_026, aud_buf_027, aud_buf_028, aud_buf_029, aud_buf_030,
            aud_buf_031, aud_buf_032;
  private
  byte [][] aud_buf_data = new byte [][] {        
            aud_buf_001, aud_buf_002, aud_buf_003, aud_buf_004, aud_buf_005, aud_buf_006, aud_buf_007, aud_buf_008, aud_buf_009, aud_buf_010,
            aud_buf_011, aud_buf_012, aud_buf_013, aud_buf_014, aud_buf_015, aud_buf_016, aud_buf_017, aud_buf_018, aud_buf_019, aud_buf_020,
            aud_buf_021, aud_buf_022, aud_buf_023, aud_buf_024, aud_buf_025, aud_buf_026, aud_buf_027, aud_buf_028, aud_buf_029, aud_buf_030,
            aud_buf_031, aud_buf_032};

  private int [] aud_buf_len = new int [] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  private int aud_buf_tail = 0;    // Tail is next index for pcm recorder to write to.   If head = tail, there is no info.   Increment on pcm read  (write to  buffer).
  private int aud_buf_head = 0;    // Head is next index for pcm player   to read  from.                                     Increment on pcm write (read from buffer).

//bufs  0   1   2   3
//tail  0   1   2   3
//head  0   0   0   0
//tail  1   2   3   0
//head  1   1   1   1
//tail  2   3   0   1
//head  2   2   2   2
//tail  3   0   1   2
//head  3   3   3   3


  private long                  writes_processed = 0;
  private long                  reads_processed  = 0;
  private int                   buf_errs = 0;
  private int                   max_bufs = 0;

  private int                   min_pcm_write_bufs  = 1;//2;            // Runs if at least 1 buffers are ready... (minimum 2 were needed for ALSA due to timing issues)

  private int                   m_channels          = 2;
  private int                   m_samplerate        = 44100;            // Default that all devices can handle =  44.1Khz CD samplerate
  private int                   m_audio_bufsize     = 32768;            // 186 ms at 44.1K stereo 16 bit        //5120 * 16;//65536;

  //private int                 m_alsa_bufsize      = 4096;             // Not used anymore; was used for direct ALSA PCM only
  //private int                   m_alsa_bufsize_max            = 65536;//4096;//8192;//Not tunable yet !!!     65536; // 64Kbytes = 16K stereo samples    // @ 44.1K = 0.37 seconds each buffer = ~ 12 second delay (& mem alloc errors) for 32
  //private int                   m_alsa_bufsize_min            = 320;

  private long                  write_stats_seconds     = 60;           // Every 60 seconds
  private long                  read_stats_seconds      = 60;           // Every 60 seconds

  private boolean               thread_pcm_write_waiting= false;

  //private int                   pcm_priority            = -19;//-20;//-19;                                 // For both read & write


  private int                   max_sample = -1000000;
  private int                   min_sample =  1000000;

  public static final int       audio_stream    = AudioManager.STREAM_MUSIC;
  private int                   cur_stream_vol  = -1;
  private int                   max_stream_vol  = 15;

  private int                   m_aud_src = 0;

    // Code:

  public svc_aud (Context c, svc_acb cb_aud, com_api svc_com_api) {                              // Constructor

    com_uti.logd ("m_obinits: " + m_obinits++);

    m_svc_acb = cb_aud;
    m_context = c;
    m_com_api = svc_com_api;
    com_uti.logd ("");

    m_AM = (AudioManager) c.getSystemService (Context.AUDIO_SERVICE);

    max_stream_vol = m_AM.getStreamMaxVolume  (audio_stream);           // Get maximum Android volume. Only need to update if stream changes
    if (max_stream_vol > 32 || max_stream_vol < 4)                      // Use sensible/usable default 15 value if out of expected range
      max_stream_vol = 15;

    cur_stream_vol = m_AM.getStreamVolume     (audio_stream);           // Get current Android volume
    if (cur_stream_vol > 32 || cur_stream_vol < 0)                      // Use sensible/usable default 7 value if out of expected range
      cur_stream_vol = 7;

    //audio_parameters_set ();

  }

  private void audio_parameters_set () {

    audio_digital_amp_set ();

    m_channels = 2;
    String audio_stereo = com_uti.prefs_get (m_context, "audio_stereo", "Stereo");
    if (audio_stereo.equals ("Mono"))
      m_channels = 1;
    com_uti.logd ("m_channels final: " + m_channels);


    /* Make buffer size a multiple of AudioManager.getProperty (PROPERTY_OUTPUT_FRAMES_PER_BUFFER). Otherwise your callback will occasionally get two calls per timeslice rather than one.
        Unless your CPU usage is really light, this will probably end up glitching.

    Use the sample rate provided by AudioManager.getProperty (PROPERTY_OUTPUT_SAMPLE_RATE). Otherwise your buffers take a detour through the system resampler.

    API level 17 / 4.2+ */

    m_samplerate = 44100;   // Default

    if (com_uti.android_version >= 17) {
      String hw_rate_str = "";
      String hw_size_str = "";
      try {
        hw_rate_str = m_AM.getProperty (AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        hw_size_str = m_AM.getProperty (AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
      }
      catch (Throwable e) {
        com_uti.loge ("AudioManager.getProperty Throwable e: " + e);
        e.printStackTrace ();
      }
      try {
        m_samplerate = Integer.parseInt (hw_rate_str);
      }
      catch (Throwable e) {
        com_uti.loge ("Rate Throwable e: " + e);
        e.printStackTrace ();
      }
    }
    com_uti.logd ("m_samplerate 1: " + m_samplerate);

    if (m_samplerate >= 8000 && m_samplerate <= 192000)
      m_samplerate = m_samplerate;
    else
      m_samplerate = 44100;
    com_uti.logd ("m_samplerate 2: " + m_samplerate);

    if (m_com_api.chass_plug_aud.equals ("CUS"))
      m_samplerate = 48000;
    else if (m_com_api.chass_plug_aud.equals ("QCV"))
      m_samplerate = 48000;
    else if (m_com_api.chass_plug_aud.equals ("OM7"))
      m_samplerate = 48000;
    else if (m_com_api.chass_plug_aud.equals ("XZ2"))
      m_samplerate = 48000;
    else if (m_com_api.chass_plug_aud.equals ("LG2") && com_uti.android_version >= VERSION_CODES.LOLLIPOP)
      m_samplerate = 48000;
    else if (m_com_api.chass_plug_aud.equals ("LG2"))
      m_samplerate = 44100;
    else if (m_com_api.chass_plug_aud.equals ("GS1"))
      m_samplerate = 44100;
    else if (m_com_api.chass_plug_aud.equals ("GS2"))
      m_samplerate = 44100;
    else if (m_com_api.chass_plug_aud.equals ("GS3"))
      m_samplerate = 44100;

    if (com_uti.sony_get () && com_uti.android_version < VERSION_CODES.LOLLIPOP)
      m_samplerate = 44100;
    com_uti.logd ("m_samplerate 3: " + m_samplerate);

    m_samplerate = com_uti.prefs_get (m_context, "audio_samplerate", m_samplerate);
    com_uti.logd ("m_samplerate final: " + m_samplerate);

    m_audio_bufsize = AudioTrack.getMinBufferSize (m_samplerate, chan_out_get (m_channels), AudioFormat.ENCODING_PCM_16BIT);
    com_uti.logd ("m_audio_bufsize 1: " + m_audio_bufsize);
            //QCV at 44.1: 22576

    m_audio_bufsize = 32768;//5120 * 16;//65536;//                                        // 80 KBytes about 0.5 seconds of stereo audio data at 44K        Default
            // QCV: 11288, 12288 (12 * 2^10) = 6144 samples = 128 milliseconds, 24576

    if (m_com_api.chass_plug_aud.equals ("QCV"))
      m_audio_bufsize = 30720;
    else if (m_com_api.chass_plug_aud.equals ("OM7"))       // 22050
      m_audio_bufsize = 30720;//32000;//768;//24576;//30720;//38400; // ??
    else if (m_com_api.chass_plug_aud.equals ("XZ2"))
      m_audio_bufsize = 30720;
    else if (m_com_api.chass_plug_aud.equals ("LG2")) {
      /*if (com_uti.stock_lg2_get ())
        m_audio_bufsize = 32768;
      else*/
        m_audio_bufsize = 30720;
    }
    else if (m_com_api.chass_plug_aud.equals ("GS1"))
      m_audio_bufsize = 12672;
    else if (m_com_api.chass_plug_aud.equals ("GS2"))
      m_audio_bufsize = 16384;
    else if (m_com_api.chass_plug_aud.equals ("GS3"))
      m_audio_bufsize = 25344;

    com_uti.logd ("m_audio_bufsize 2: " + m_audio_bufsize);

    m_audio_bufsize = com_uti.prefs_get (m_context, "m_audio_bufsize", m_audio_bufsize);
    com_uti.logd ("m_audio_bufsize final: " + m_audio_bufsize);

    for (int ctr = 0; ctr < aud_buf_num; ctr ++) {
      aud_buf_data [ctr] = new byte [m_audio_bufsize];                  // Re-Allocate audio buffer
    }

  }

  private int chan_out_get (int channels) {
    if (channels == 1)
      return (AudioFormat.CHANNEL_OUT_MONO);
    else
      return (AudioFormat.CHANNEL_OUT_STEREO);
  }
  private int chan_in_get (int channels) {
    if (channels == 1)
      return (AudioFormat.CHANNEL_IN_MONO);
    else
      return (AudioFormat.CHANNEL_IN_STEREO);
  }

/*
    m_alsa_bufsize = 3840;
    try {
      m_alsa_bufsize = 2 * m_channels * Integer.parseInt (hw_size_str);
    }
    catch (Throwable e) {
      com_uti.loge ("Rate Throwable e: " + e);
      e.printStackTrace ();
    }
    com_uti.logd ("m_alsa_bufsize 1: " + m_alsa_bufsize);
    if (m_alsa_bufsize >= 64 && m_alsa_bufsize <= m_alsa_bufsize_max)
      m_alsa_bufsize = m_alsa_bufsize;
    else
      m_alsa_bufsize = 3840;
    com_uti.logd ("m_alsa_bufsize 2: " + m_alsa_bufsize);

    if (m_com_api.chass_plug_aud.equals ("QCV") && com_uti.old_qcv_get ())
      m_alsa_bufsize = 5120 * 2;
    else if (m_com_api.chass_plug_aud.equals ("OM7"))
      m_alsa_bufsize = 5120 * 2;
    else
      m_alsa_bufsize = AudioRecord.getMinBufferSize (m_samplerate, chan_in_get (m_channels), AudioFormat.ENCODING_PCM_16BIT);
    com_uti.logd ("m_alsa_bufsize 3: " + m_alsa_bufsize);

    if (m_alsa_bufsize > m_alsa_bufsize_max)
      m_alsa_bufsize = m_alsa_bufsize_max;
    if (m_alsa_bufsize < m_alsa_bufsize_min)
      m_alsa_bufsize = m_alsa_bufsize_min;
    com_uti.logd ("m_alsa_bufsize 4: " + m_alsa_bufsize);
*/


    // Command handlers:

    //android.media.audiofx.Equalizer m_equalizer = null;

    // Open audio effect control session (before playback ?):           For EQ & Visualisations ?
  private void audio_session_start (int sessid) {
    com_uti.logd ("sessid: " + sessid);
    //int priority = 0;//     !! Any value over 0 kills CM11 DSP Manager     1;//2^7;//2^15;//2147483647;
    //m_equalizer = new android.media.audiofx.Equalizer (priority, sessid);
  }
    // Close audio effect control session (before audio object dies ?)
  private void audio_session_stop (int sessid) {
    com_uti.logd ("sessid: " + sessid);
  }

  public String audio_sessid_get () {                                   // Handle audio session changes: Called by svc_svc:service_update_send() (could be svc_tnr:service_update_send())
    if (com_uti.s2_tx_apk ()) {
      return ("0");
    }
    int new_audiotrack_sessid_int = 0;
    if (m_audiotrack != null)                                           // If we have an audiotrack active...
      new_audiotrack_sessid_int = m_audiotrack.getAudioSessionId ();

    //com_uti.logd ("new_audiotrack_sessid_int: " + new_audiotrack_sessid_int + "  audiotrack_sessid_int: " + audiotrack_sessid_int);

    if (new_audiotrack_sessid_int > 0) {                                     // If valid session ID...
      if (audiotrack_sessid_int != new_audiotrack_sessid_int) {                   // If new session ID...
        com_uti.logd ("new_audiotrack_sessid_int: " + new_audiotrack_sessid_int + "  audiotrack_sessid_int: " + audiotrack_sessid_int);
        audio_session_stop (audiotrack_sessid_int);                          // Stop use of old session ID
        audiotrack_sessid_int = new_audiotrack_sessid_int;                        // Set new session ID
        audio_session_start (new_audiotrack_sessid_int);                     // Start use of new session ID
      }
    }
    else {                                                              // Else if no session ID (if no audiotrack active)...
      if (audiotrack_sessid_int > 0) {                                       // If we previously had an active session...
        com_uti.logd ("new_audiotrack_sessid_int: " + new_audiotrack_sessid_int + "  audiotrack_sessid_int: " + audiotrack_sessid_int);
        audio_session_stop (audiotrack_sessid_int);                          // Stop use of old session ID
        audiotrack_sessid_int = 0;                                           // Set session ID to 0 (No audio session currently active)
      }
    }
    m_com_api.audio_sessid = "" + audiotrack_sessid_int;
    return (m_com_api.audio_sessid);                                    // Return session ID
  }


    // FM Transmit audio state:

  private int transmit_save_vol = -1;

  private int transmit_audio_pause () {                                 // Pause FM transmit audio: Pause is same as stop at this time
    m_com_api.audio_state = "Pausing";
    transmit_audio_stop ();
    return (0);
  }

  private int transmit_audio_stop () {                                  // Stop FM transmit audio
    m_com_api.audio_state = "Stopping";
    com_uti.daemon_set ("audio_state", "Stop");                         // Send to daemon
    if (transmit_save_vol >= 0 && transmit_save_vol <= max_stream_vol) {
      com_uti.logd ("Restoring stream volume to transmit_save_vol: " + transmit_save_vol);
      m_AM.setStreamVolume (audio_stream, transmit_save_vol, 0);          // Restore volume without displaying volume change on screen
    }
    return (0);
  }

  private int transmit_audio_start () {                                 // Start FM transmit audio
    m_com_api.audio_state = "Starting";
    com_uti.daemon_set ("audio_state", "Start");                        // Send to daemon
    if (m_AM != null) {
      cur_stream_vol = m_AM.getStreamVolume (audio_stream);
      transmit_save_vol = cur_stream_vol;
      if (cur_stream_vol != max_stream_vol) {
        com_uti.logd ("Setting stream volume to maximise transmitter modulation to max_stream_vol: " + max_stream_vol);
        m_AM.setStreamVolume (audio_stream, max_stream_vol, 0);         // Maximize volume without displaying volume change on screen
      }
    }
    return (0);
  }


    // FM Receive audio state:

  private int receive_audio_pause () {                                  // Pause FM receive audio
    m_com_api.audio_state = "Pausing";

    mode_audio_pause ();                                                // Pause Audio at hardware level based on mode.
    audio_output_set ("Headset", false);                                // Set Audio Output to headset (if plugged and was set to speaker) with no restart
    return (0);
  }

  private int receive_audio_stop () {                                   // Stop FM receive audio
    m_com_api.audio_state = "Stopping";
    volume_observer_unregister ();                                      // Unregister for volume change events
    headset_plgd_lstnr_stop ();                                         // Unregister for headset plugged/unplugged events
    focus_set (false);

    mode_audio_stop ();                                                 // Stop Audio at hardware level based on mode.
    audio_output_set ("Headset", false);                                // Set Audio Output to headset (if plugged and was set to speaker) with no restart
    return (0);
  }

  private int receive_audio_start () {                                  // Start FM receive audio
    m_com_api.audio_state = "Starting";
    audio_parameters_set ();                                            // Setup parameters, like stereo, samplerate,...
    volume_observer_register ();                                        // Register for volume change events
    headset_plgd_lstnr_start ();                                        // Register for headset plugged/unplugged events
    focus_set (true);                                                   // Get audio focus

    String audio_output = com_uti.prefs_get (m_context, "audio_output", "Headset");
    audio_output_set (audio_output, false);                             // Set Audio Output from prefs with no restart

    mode_audio_start ();                                                // Start Audio at hardware level based on mode.
    return (0);
  }

    // Player and overall audio state control:
  private int service_timeout_audio_state = 6;

  public String audio_state_set (String desired_state) {                // Called only by onAudioFocusChange(Start/Stop/Pause),
                                                                            //       svc_svc:audio_state_set    (Start/Stop/Pause) (via onStartCommand()),
                                                                            //       svc_svc:tuner_state_set    (Stop),
                                                                            //       svc_svc:cb_tuner_state     (Start)
    String last_audio_state = m_com_api.audio_state;
    com_uti.logd ("desired_state: " + desired_state + "  last_audio_state: " + last_audio_state);
    boolean success = true;                                             // Default = success

    if (desired_state.equals ("Start")) {                               // START:
      m_com_api.service_update_send (null, "Starting Audio", "" + service_timeout_audio_state);      // Send Phase Update

      if (m_com_api.tuner_mode.equals ("Transmit"))                     // If Transmit... !! Don't check state, see audio re-kicking requirement in svc_rcc:rds_update_do() !!!
        transmit_audio_start ();
      else if (m_com_api.audio_state.equals ("Stop") || m_com_api.audio_state.equals ("Pause"))
        receive_audio_start ();                                         // If receive and Audio State = Stop or Pause (If Start or Starting then do nothing)
      else {
        //com_uti.logw ("desired_state: " + desired_state + "  m_com_api.audio_state: " + m_com_api.audio_state);
        return (m_com_api.audio_state);
      }
    }
    else if (desired_state.equals ("Stop")) {                           // STOP:
      m_com_api.service_update_send (null, "Stopping Audio", "" + service_timeout_audio_state);      // Send Phase Update
      if (m_com_api.tuner_mode.equals ("Transmit"))                     // If Transmit...
        transmit_audio_stop ();
      else if (m_com_api.audio_state.equals ("Start") || m_com_api.audio_state.equals ("Pause"))
        receive_audio_stop ();                                          // If receive and Audio State = Start or Pause (If Stop or Starting then do nothing)
      else {
        //com_uti.logw ("desired_state: " + desired_state + "  m_com_api.audio_state: " + m_com_api.audio_state);
        return (m_com_api.audio_state);
      }
    }
    else if (desired_state.equals ("Pause")) {                          // PAUSE:
      m_com_api.service_update_send (null, "Pausing Audio", "" + service_timeout_audio_state);       // Send Phase Update
      if (m_com_api.tuner_mode.equals ("Transmit"))                     // If Transmit...
        transmit_audio_pause ();
      else if (m_com_api.audio_state.equals ("Start"))
        receive_audio_pause ();                                         // If receive and Audio State = Start (If Pause or Stop or Starting then do nothing)
      else {
        //com_uti.logw ("desired_state: " + desired_state + "  m_com_api.audio_state: " + m_com_api.audio_state);
        return (m_com_api.audio_state);
      }
    }
    else {
      com_uti.loge ("Unexpected desired_state: " + desired_state);
      return (m_com_api.audio_state);
    }

    m_com_api.service_update_send (null, "Success " + m_com_api.chass_phase, "0");

    m_com_api.audio_state = desired_state;
    if (m_svc_acb != null)
      m_svc_acb.cb_audio_state (desired_state);                         // Callback to indicate success

    return (m_com_api.audio_state);
  }


/* private void sink_state_handler (Intent intent) {
    if (m_pwr_state) {
      int state = intent.getIntExtra ("android.bluetooth.a2dp.extra.SINK_STATE", BluetoothA2dp.STATE_DISCONNECTED);
      if (state == BluetoothA2dp.STATE_CONNECTED || state == BluetoothA2dp.STATE_PLAYING) {
        if (! m_over_a2dp) {
          mute__set (true);                                             // Restart audio
          mute__set (false);
        }
      }
      else {
        if (m_over_a2dp) {
          mute__set (true);                                             // Restart audio
          mute__set (false);
        }
      }
    }
  }*/

    // Receiver Lifecycle:

    // A BroadcastReceiver object is only valid for the duration of the call to onReceive(Context, Intent).
    // Once your code returns from this function, the system considers the object to be finished and no longer active.

    // This has important repercussions to what you can do in an onReceive(Context, Intent) implementation: anything that requires asynchronous operation is not available,
    // because you will need to return from the function to handle the asynchronous operation, but at that point the BroadcastReceiver is no longer active & thus
    // the system is free to kill its process before the asynchronous operation completes.

    // In particular, you may not show a dialog or bind to a service from within a BroadcastReceiver. For the former, you should instead use the NotificationManager API.
    // For the latter, you can use Context.startService() to send a command to the service. 

    // Headset listener: Listen for ACTION_HEADSET_PLUG notifications. (Plugged in/out)

  private void headset_plgd_lstnr_stop () {
    com_uti.logd ("m_hdst_lstnr: " + m_hdst_lstnr);
    try {
      if (m_hdst_lstnr != null)
        m_context.unregisterReceiver (m_hdst_lstnr);
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);                                 // From Audio Pause to Audio Stop: java.lang.IllegalArgumentException: Receiver not registered: fm.a2d.sf.svc_aud$1@484941
      //e.printStackTrace ();
    }
  }
  private void headset_plgd_lstnr_start () {                            // Headset plug events
    if (m_hdst_lstnr == null) {
      m_hdst_lstnr = new BroadcastReceiver () {
        @Override
        public void onReceive (Context context, Intent intent) {
          String action = intent.getAction ();
          com_uti.logd ("headset_plgd_lstnr_start onReceive intent action: " + action);

          if (action.equals (Intent.ACTION_HEADSET_PLUG))
            headset_plgd_handler (intent);

          //else if (action.equals ("android.bluetooth.a2dp.action.SINK_STATE_CHANGED"))
          //  sink_state_handler (intent);
          //else if (action.equals ("HDMI_CONNECTED"))
          //  hdmi_handler (intent);
        }
      };

      IntentFilter iFilter = new IntentFilter ();

      iFilter.addAction (Intent.ACTION_HEADSET_PLUG);

      //iFilter.addAction (android.intent.action.MODE_CHANGED);
      //iFilter.addAction ("HDMI_CONNECTED");
      //ACTION_MEDIA_BUTTON
      //iFilter.addAction (android.bluetooth.intent.action.HEADSET_STATE_CHANGED);
      //iFilter.addAction (android.bluetooth.intent.HEADSET_STATE);
      //iFilter.addAction (android.bluetooth.intent.HEADSET_STATE_CHANGED);
      //iFilter.addAction (android.bluetooth.intent.action.MODE_CHANGED);
      //iFilter.addAction ("android.bluetooth.a2dp.action.SINK_STATE_CHANGED");

      iFilter.addCategory (Intent.CATEGORY_DEFAULT);

      Intent last_broadcast_hdst_plug = m_context.registerReceiver (m_hdst_lstnr, iFilter);

      com_uti.logd ("headset_plgd_lstnr_start onReceive intent last_broadcast_hdst_plug: " + last_broadcast_hdst_plug);

      //if (last_broadcast_hdst_plug != null)                           // Don't need extra
      //  m_hdst_lstnr.onReceive (this, last_broadcast_hdst_plug);
    }
  }

    // ACTION_HEADSET_PLUG is a sticky broadcast:   Get the last-broadcast Intent for that action, which will tell you what the headset state is:
    //  Intent last_broadcast_hdst_plug = registerReceiver (null, new IntentFilter (ACTION_HEADSET_PLUG), null, null);

  private void headset_plgd_handler (Intent intent) {
    com_uti.logd ("headset_plgd_handler Intent received: " + intent + "  state: " + intent.getIntExtra ("state", -555) + "  name: " + intent.getStringExtra ("name"));

    // onReceive ACTION_HEADSET_PLUG Intent received: Intent { act=android.intent.action.HEADSET_PLUG flg= 0x40000000 (has extras) }  state: 1  name: h2w
    // onReceive ACTION_HEADSET_PLUG Intent received: Intent { act=android.intent.action.HEADSET_PLUG flg= 0x40000000 (has extras) }  state: 0  name: h2w
    //                               Intent received: Intent { act=android.intent.action.HEADSET_PLUG flg= 0x40000010 (has extras) }  state: 0  name: Headset
    // HTC state 8 = unplugged

    int state = intent.getIntExtra ("state", -555);
    if (state == -555) {
      com_uti.loge ("headset_plgd_handler no state");
    }

    if (state != 0) {
      m_hdst_plgd = true;
      return;
    }

    m_hdst_plgd = false;

  }

    // Focus:
  private void focus_set (boolean focus_request) {
    com_uti.logd ("focus_request: " + focus_request);
    int ret = 0;
    //restart_audio_on_focus_regain = false;
    if (focus_request)                                                  // If focus desired...
      ret = m_AM.requestAudioFocus (this, audio_stream, AudioManager.AUDIOFOCUS_GAIN);
      //ret = m_AM.requestAudioFocus (this, audio_stream, AudioManager.AUDIOFOCUS_GAIN);
    else {                                                              // If focus return...
      restart_audio_on_focus_regain = false;
      ret = m_AM.abandonAudioFocus (this);
    }
    if (ret != AudioManager.AUDIOFOCUS_REQUEST_GRANTED)
      com_uti.loge ("ret: " + ret);
  }

  private boolean restart_audio_on_focus_regain = false;
  public void onAudioFocusChange (int focusChange) {
    if (m_com_api.tuner_mode.equals ("Transmit")) {
      com_uti.loge ("TX !!!!!!!!!");
      return;
    }
    com_uti.logd ("focusChange: " + focusChange + "  audio_state: " + m_com_api.audio_state + "  restart_audio_on_focus_regain: " + restart_audio_on_focus_regain);
    switch (focusChange) {
      case AudioManager.AUDIOFOCUS_GAIN:                                // Gain
        if (restart_audio_on_focus_regain && m_com_api.audio_state.equals ("Pause")) {
          com_uti.logd ("focusChange: GAIN. Restarting...");
          restart_audio_on_focus_regain = false;
          audio_state_set ("Start");                                      // Start/Restart audio
        }
        else
          com_uti.logd ("focusChange: GAIN. NOT Restarting...");
        break;
      case AudioManager.AUDIOFOCUS_LOSS:                                // Permanent loss
        //com_uti.logd ("focusChange: LOSS");
        restart_audio_on_focus_regain = false;
        if (m_com_api.audio_state.equals ("Pause") || m_com_api.audio_state.equals ("Start") || m_com_api.audio_state.equals ("Starting") || m_com_api.audio_state.equals ("Stopping")) {
          audio_state_set ("Stop");                                       // Stop audio       Loss of/stopping audio could/should stop tuner (& Tuner API ?)
          com_uti.logd ("DONE focusChange: LOSS");
        }
        else
          com_uti.logd ("focusChange: LOSS. NOT Stopping...");
        break;
      case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:                      // Transient loss
        //com_uti.logd ("focusChange: ...LOSS_TRANSIENT");
        if (m_com_api.audio_state.equals ("Start") || m_com_api.audio_state.equals ("Starting") || m_com_api.audio_state.equals ("Stopping")) {
          restart_audio_on_focus_regain = true;
          audio_state_set ("Pause");                                      // Pause audio
          com_uti.logd ("DONE focusChange: ...LOSS_TRANSIENT");
        }
        else
          com_uti.logd ("focusChange: LOSS_TRANSIENT. NOT Pausing...");
        break;
      case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:             // Transient loss we can ignore if we want
        //com_uti.logd ("focusChange: LOSS_TRANSIENT_CAN_DUCK");
        if (m_com_api.audio_state.equals ("Start") || m_com_api.audio_state.equals ("Starting") || m_com_api.audio_state.equals ("Stopping")) {
          restart_audio_on_focus_regain = true;
          audio_state_set ("Pause");                                      // Pause audio
          com_uti.logd ("DONE focusChange: LOSS_TRANSIENT_CAN_DUCK");
        }
        else
          com_uti.logd ("focusChange: LOSS_TRANSIENT_CAN_DUCK. NOT Pausing...");
        break;
       default:
    }
  }

    // PCM:

  public String audio_record_state_set (String new_record_state) {
    com_uti.logd ("new_record_state: " + new_record_state);
    if (com_uti.s2_tx_apk ())
      return ("Stop");
    String str = "";
    return (m_com_api.audio_record_state);
  }

  private void pcm_stat_log (String prefix, int increment, int offset, int len, byte [] buf) {
    int max = -32768, min = 32769, avg = 0, max_avg = 0;
    int i = 0;
    for (i = 0; i < len / 2; i += increment) {
      ///*signed*/ short short_sample = *((/*signed*/ short *) & buf [i + 0 + offset]);
      int sample = 0;
      int lo_byte = buf [i * 2 + offset * 2 + 0];
      int hi_byte = buf [i * 2 + offset * 2 + 1];
      sample = hi_byte * 256 + lo_byte;
      if (lo_byte < 0)
        sample += 256;
      
      if (sample > max)
        max = sample;
      if (sample < min)
        min = sample;
      avg += sample;
    }
    int samples = len / (2 * increment);
    //avg = (max + min) / 2;      // Doesn't work well due to spikes
    avg /= samples;             // This smooths average to about -700 on GS3.
    //if ( (max - avg) > max_avg)   // Avg means we don't have to test min
      max_avg = max - avg; 
    com_uti.logd (prefix + "  offset: " + offset + " reads_processed: " + reads_processed + " writes_processed: " + writes_processed + "  len: " + len + "  samples: " + samples + "  max: " + max + "  min: " + min + "  avg: " + avg + "  max_avg: " + max_avg);
  }
  private void pcm_stat_logs (String prefix, int channels, int len, byte [] buf) {         // Length in bytes
    if (channels == 1) {
      pcm_stat_log (prefix, 1, 0, len, buf);                                // Treat as mono
    }
    else {
      pcm_stat_log (prefix, 2, 0, len, buf);                                // Left is first
      pcm_stat_log (prefix, 2, 1, len, buf);                                // Right is second
    }
  }


    // pcm_read -> pcm_write
  private final Runnable run_pcm_write = new Runnable () {
    public void run () {
      com_uti.logd ("run_pcm_write");
      //native_priority_set (pcm_priority);
                                                                        // Setup temp vars before loop to minimize garbage collection   (!! but aud_mod() has this issue)
      byte [] aud_buf;
      int bufs = 0;
      int len = 0;
      int len_written = 0;
      int new_len = 0;
      long curr_ms_start = 0;
      long curr_ms_time  = 0;
      long total_ms_start = 0;
      long total_ms_time  = 0;

      try {

        while (thread_pcm_write_active) {                               // While PCM Write Thread should be active...

// GET read audio buffer to write
          bufs = aud_buf_tail - aud_buf_head;
          //com_uti.loge ("run_pcm_write bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);

          if (bufs < 0)
            bufs += aud_buf_num;                                        // Fix underflow
          if (bufs < min_pcm_write_bufs) {                              // If minimum number of buffers is not ready... (Currently at least 2)
            try {
              thread_pcm_write_waiting = true;
              //Thread.sleep (1);                                       // Wait ms milliseconds
              //Thread.sleep (2000);                                    // Wait ms milliseconds   More efficient
              //Thread.sleep (3);                                       // Wait ms milliseconds    3 matches Spirit1
              Thread.sleep (100);       // 100 ms compromise ?
              thread_pcm_write_waiting = false;
            }
            catch (InterruptedException e) {
              thread_pcm_write_waiting = false;
              //Thread.currentThread().interrupt();
              //e.printStackTrace ();
            }
            continue;                                                   // Restart loop
          }
                                                                        // Here when at least 2 buffers are ready to write, so we write the 1st at the head...
          //com_uti.loge ("run_pcm_write ready to write bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);
          len = aud_buf_len [aud_buf_head];                             // Length of head buffer in bytes
          aud_buf = aud_buf_data [aud_buf_head];                        // Pointer to head buffer
          if (aud_buf == null) {
            com_uti.loge ("run_pcm_write len: " + len + "  len_written: " + len_written + "  aud_buf: " + aud_buf);
            continue;
          }
          //com_uti.loge ("run_pcm_write ready to write bufs: " + bufs + "  len: " + len + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);

// RECORD audio buffer before aud_mod() for audio output device write

// MODIFY / amplify audio for audio output device write
          aud_mod (len, aud_buf);                                       // Modify / amplify audio for audio output device write

// WRITE audio to audio output device write
          total_ms_start = curr_ms_start = com_uti.tmr_ms_get ();       // Start timeout timers
          total_ms_time  = curr_ms_time  = -1;                          // 

          len_written = 0;
          while (thread_pcm_write_active && len_written < len && total_ms_time < 3000) {    // While reading active and not written all and less than 3 seconds total has elapsed...
            if (total_ms_time >= 0) {                                   // If already looped
              com_uti.logd ("run_pcm_write len_written < len  total_ms_time: " + total_ms_time + "  curr_ms_time: " + curr_ms_time + "  len: " + len + "  new_len: " + new_len + "  len_written: " + len_written + "  aud_buf: " + aud_buf);
              com_uti.quiet_ms_sleep (30);                              // Wait for about 6 KBytes worth of audio to be de-buffered
            }
            if (! thread_pcm_write_active)
              break;
            curr_ms_start = com_uti.tmr_ms_get ();                      // Start current write timer
            new_len = m_audiotrack.write (aud_buf, len_written, len - len_written);  // Write head buffer to audiotrack  All parameters in bytes (but could be all in shorts)
            if (new_len > 0)
              len_written += new_len;                                   // If we wrote, update total length written
            total_ms_time = com_uti.tmr_ms_get () - total_ms_start;     // Calculate time taken for total write
            curr_ms_time  = com_uti.tmr_ms_get () - curr_ms_start;      // Calculate time taken for current write
            if (curr_ms_time >= 700)                                    // If current write has taken too long...   GS1 sees 350 ms sometimes   ; Saw 431 on OM7    ; 564 on OXL
              com_uti.loge ("run_pcm_write m_audiotrack.write too long total_ms_time: " + total_ms_time + "  curr_ms_time: " + curr_ms_time + "  len: " + len + "  new_len: " + new_len + "  len_written: " + len_written + "  aud_buf: " + aud_buf);
          }

          if (len != 0 && com_uti.ena_log_pcm_stat && writes_processed % ((2 * write_stats_seconds * m_samplerate * m_channels) / len) == 0)   // Every stats_seconds
            pcm_stat_logs ("Write", m_channels, len, aud_buf);
          writes_processed ++;                                          // Log and update stats

          aud_buf_head ++;                                              // Update aud_buf_head pointer
          if (aud_buf_head < 0 || aud_buf_head > aud_buf_num - 1)
            aud_buf_head &= aud_buf_num - 1;
          continue;                                                     // Restart loop

        }   // while (thread_pcm_write_active && len_written < len && ms_time < 3000) {

                                                                        // Here when thread is finished...
        com_uti.logd ("run_pcm_write done writes_processed: " + writes_processed);

          audio_record_state_set ("Stop");                              // Ensure we finish any recording in progress
      }
      catch (Throwable e) {
        com_uti.loge ("run_pcm_write throwable: " + e);
        e.printStackTrace ();
      }                                                                 // Fall through to terminate if exception
      return;
    }
  };


  private boolean is_analog_audio_mode () {
    return (m_com_api.audio_mode.equals ("Analog"));
  }
  private boolean is_digital_audio_mode () {
    return (m_com_api.audio_mode.equals ("Digital"));
  }
  private boolean is_testmic_audio_mode () {
    return (m_com_api.audio_mode.equals ("TestMic"));
  }


  private long time_of_last_blank = 0;
  private int  num_audio_low = 0;

  private final Runnable run_pcm_read = new Runnable () {               // Read/Input thread
    public void run () {
      com_uti.logd ("run_pcm_read");

      //native_priority_set (pcm_priority);

      long time_since_last_blank = 0;
      long time_current = 0;
                                                                        // Setup temp vars before loop to minimize garbage collection
      byte [] aud_buf;
      int bufs = 0;
      int len = 0;
      //int len_written = 0;
      int ctr = 0;

      try {
        buf_errs = 0;                                                   // Init stats, pointers, etc
        aud_buf_tail = aud_buf_head = 0;                                // Drop all buffers
        com_uti.logd ("run_pcm_read m_samplerate: " +  m_samplerate + "  m_channels: " + m_channels);

        while (thread_pcm_read_active) {                                // While PCM Read Thread should be active...

          bufs = aud_buf_tail - aud_buf_head;
          if (bufs < 0)                                                 // If underflowed...
            bufs += aud_buf_num;                                        // Wrap
          //logd ("bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);

          if (bufs > max_bufs)                                          // If new maximum buffers in progress...
            max_bufs = bufs;                                            // Save new max

          if (bufs >= (aud_buf_num * 3) / 4) {                          // If 75% or more or buffers still in progress (If write thread is getting backed up)
            //if (thread_pcm_write != null && thread_pcm_write_waiting)
            //  thread_pcm_write.interrupt ();                            // Wake up thread_pcm_write sooner than usual
            //com_uti.ms_sleep (300);                                     // Sleep to let write thread process.   0.1s/0.3s = 20/60KBytes @ 48k stereo  (2.5/8 8k buffers)

            com_uti.loge ("Low on aud_buf");
            com_uti.ms_sleep (105);//400);//105);

            aud_buf_tail = aud_buf_head = 0;                            // Need to Drop all buffers or error will continue...
            continue;
          }
          if (bufs >= aud_buf_num - 1) {                                // If NOT room to write another buffer (max = aud_buf_num - 1 to prevent wrap-around)
            com_uti.loge ("Out of aud_buf");
            buf_errs ++;
            aud_buf_tail = aud_buf_head = 0;                            // Drop all buffers
          }

          //if (aud_buf_data [aud_buf_tail] == null)                      // If audio buffer not yet allocated...
          //  aud_buf_data [aud_buf_tail] = new byte [m_alsa_bufsize_max];      // Allocate memory to m_alsa_bufsize_max. Could use m_alsa_bufsize but that prevents live tuning unless re-allocate while running.

          aud_buf = aud_buf_data [aud_buf_tail];

          len = -555;                                                   // Default = error if no m_audiorecord (shouldn't happen except at shutdown)
          if (m_audiorecord != null)
            len = m_audiorecord.read (aud_buf, 0, m_audio_bufsize);//m_alsa_bufsize);

// m_audiorecord = ???

          if (len <= 0) {                                               // If no audio data...
            if (len == android.media.AudioRecord.ERROR_INVALID_OPERATION ) {  // -3
              com_uti.logd ("get expected interruption error due to shutdown: " + len + "  tail index: " + aud_buf_tail);       // 
              break;
            }
// -2: ERROR_BAD_VALUE 
            com_uti.loge ("get error: " + len + "  tail index: " + aud_buf_tail);       // 
            com_uti.ms_sleep (101);//0);                                    // Wait for errors to clear
/*
02-04 03:40:55.369 D/s2svcaud(21385): pcm_read_stop: pcm_read_stop thread_pcm_read_active: true
02-04 03:40:55.369 E/s2svcaud(21385): run: get error: -3  tail index: 5
02-04 03:40:55.374 E/s2comuti(21385): ms_sleep: ms: 101
02-04 03:40:55.374 E/s2comuti(21385): ms_sleep: Exception e: java.lang.InterruptedException
02-04 03:40:55.374 D/s2svcaud(21385): run: run_pcm_read reads_processed: 2149  writes_processed: 2149  buf_errs: 0  max_bufs: 3
*/
          }
          else {                                                        // Else if we have audio data...
            if (com_uti.ena_log_pcm_stat && reads_processed % ((2 * read_stats_seconds * m_samplerate * m_channels) / len) == 0)   // Every stats_seconds
              pcm_stat_logs ("Read ", m_channels, len, aud_buf);

            if (len > 1000 && m_com_api.chass_plug_aud.equals ("QCV")) {// Detect all 0's in audio to kickstart QCV audio (by doing any FM chip function)
              for (ctr = 0; ctr < len; ctr ++) {
                if (aud_buf [ctr] != 0)                                 // If data
                  break;
              }
              if (ctr < len) {                                          // If data...
                //short [] sa = com_uti.ba_to_sa (aud_buf);
                //for (int idx = 0; idx < len / 2; idx ++)
                //  if (sa [idx] > 255
                for (ctr = 0; ctr < len; ctr += 2) {
                  if (aud_buf [ctr + 1] != 0 && aud_buf [ctr + 1] != 0xff && aud_buf [ctr + 1] != -1) { // If significant data
                    num_audio_low = 0;
                    break;
                  }
                }
                if (ctr >= len) {                                       // If no significant data...
                  num_audio_low ++;
                  if (num_audio_low >= 4) {                             // If 4 consecutive...
                    com_uti.logw ("power hack unmute YES 4 consecutive low");
                  }
                  else {
                    ctr = 0;                                            // Set ctr to 0 to avoid triggering fix
                    if (num_audio_low >= 3)
                      com_uti.logw ("power hack unmute num_audio_low: " + num_audio_low);
                    else if (num_audio_low >= 2)
                      com_uti.logd ("power hack unmute num_audio_low: " + num_audio_low);
                    else
                      com_uti.logd ("power hack unmute num_audio_low: " + num_audio_low);

                    num_audio_low = 0;
                  }
                }
              }
              else {                                                    // Else if no data...
                num_audio_low = 0;
              }


              if (ctr >= len) {                                         // If no data or 4 consecutive little data...
                time_current = com_uti.tmr_ms_get ();
                time_since_last_blank = time_current - time_of_last_blank;
                if (time_since_last_blank > 10000) {
                  com_uti.logw ("power hack unmute time_of_last_blank: " + time_of_last_blank + "  time_current: " + time_current);
                  time_of_last_blank = time_current;
                  com_uti.daemon_set ("tuner_mute", "Unmute");
                }
              }
            }

            //aud_mod (len, aud_buf);                                   // Modify / amplify audio

            if (aud_buf_tail < 0 || aud_buf_tail > aud_buf_num - 1)     // Protect from ArrayIndexOutOfBoundsException
              aud_buf_tail &= aud_buf_num - 1;

            aud_buf_len [aud_buf_tail] = len;                           // On shutdown: java.lang.ArrayIndexOutOfBoundsException: length=32; index=32

            aud_buf_tail ++;
            if (aud_buf_tail < 0 || aud_buf_tail > aud_buf_num - 1)
              aud_buf_tail &= aud_buf_num - 1;
            reads_processed ++;

            bufs = aud_buf_tail - aud_buf_head;
            //com_uti.loge ("run_pcm_read bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);
            if (bufs < 0)
              bufs += aud_buf_num;                                      // Fix underflow
            if (bufs >= min_pcm_write_bufs)                          // If minimum number of buffers is ready... (Currently at least 2)
              if (thread_pcm_write != null && thread_pcm_write_waiting)
                thread_pcm_write.interrupt ();                          // Wake up thread_pcm_write sooner than usual
          }
        }

      }
      catch (Throwable e) {
        com_uti.loge ("run_pcm_read throwable: " + e);
        e.printStackTrace ();
      }                                                                 // Fall through to terminate if exception

      com_uti.logd ("run_pcm_read reads_processed: " + reads_processed + "  writes_processed: " + writes_processed + "  buf_errs: " + buf_errs + "  max_bufs: " + max_bufs);

      return;
    }
  };


    // Audio modify / amplify:

    /* C code:
  void aud_mod (int len, signed char * buf) {
    int i = 0;
    signed short * sbuf = (signed short *) buf;
    for (i = 0; i < len / 2; i++) {
      //signed short short_sample = * ((signed short *) & buf [i * 2]);
      //int sample = short_sample;
      sbuf [i] *= 4;//3;
    }
  }*/

  private void aud_mod (int len, byte [] buf) {                         // Modify audio, such as amplification

    if (audio_digital_amp == 1)
      return;

    short [] sa = com_uti.ba_to_sa (buf);
    for (int idx = 0; idx < len / 2; idx ++)
      sa [idx] *= audio_digital_amp;

    byte [] ba = com_uti.sa_to_ba (sa);
    //buf = java.util.Arrays.copyOf (ba, ba.length);                    // System.arraycopy ??
    java.lang.System.arraycopy (ba, 0, buf, 0, len);
  }

  public String audio_stereo_set (String new_audio_stereo) {            // Must restart audio for stereo change to take effect
    //com_uti.logd ("s2_tx: " + s2_tx + "  new_audio_stereo: " + new_audio_stereo);
    //if (s2_tx)
    //  return (new_audio_stereo);
    m_com_api.audio_stereo = new_audio_stereo;                          // Set new audio stereo
    com_uti.logd ("Set new audio_stereo: " + m_com_api.audio_stereo);
    return (m_com_api.audio_stereo);
  }

  public String audio_mode_set (String new_audio_mode) {                // Must restart audio for mode change to take effect
    com_uti.logd ("new_audio_mode: " + new_audio_mode + "  m_com_api.audio_mode: " + m_com_api.audio_mode + "  m_com_api.audio_state: " + m_com_api.audio_state);
    if (com_uti.s2_tx_apk ())
      return (new_audio_mode);

    if (! m_com_api.audio_state.equals ("Start")) {           // If Audio is not started we do not need to do anything except store mode for next audio start
      m_com_api.audio_mode = new_audio_mode;                            // Set new audio mode
      return (m_com_api.audio_mode);                                    // Done for now
    }
                                                                        // Else if Audio is started we need to stop and restart PCM...
    mode_audio_stop ();                                                 // Stop Audio at hardware level based on mode.

    com_uti.logd ("Before set new_audio_mode: " + new_audio_mode + "  m_com_api.audio_mode: " + m_com_api.audio_mode + "  m_com_api.audio_state: " + m_com_api.audio_state);
    m_com_api.audio_mode = new_audio_mode;                              // Set new audio mode
    com_uti.logd ("After  set new_audio_mode: " + new_audio_mode + "  m_com_api.audio_mode: " + m_com_api.audio_mode + "  m_com_api.audio_state: " + m_com_api.audio_state);

    mode_audio_start ();                                                // Restart Audio at hardware level based on mode.

    com_uti.logd ("Done   set new_audio_mode: " + new_audio_mode + "  m_com_api.audio_mode: " + m_com_api.audio_mode + "  m_com_api.audio_state: " + m_com_api.audio_state);
    return (m_com_api.audio_mode);
  }


    // ? Use stereo_set() for speaker mode ? HTC One has stereo speakers ?

  private boolean enable_pcmwrite       = true;
  private boolean enable_pcmread        = true;
  private boolean enable_daemonaudio    = true;
  private boolean enable_daemonvolume   = true;

  private boolean enable_stockmode      = true;

  private int fm_device = 0x100000;

                                                                        // Start audio at hardware level based on mode.
  private void mode_audio_start () {                                    //     Called only by audio_start() (for audio start) & audio_output_set() (for audio restart)
    com_uti.logd ("m_audiotrack: " + m_audiotrack + "  m_com_api.audio_state: " + m_com_api.audio_state);

    if (com_uti.file_get ("/sdcard/spirit/pcm_stats")) {
      write_stats_seconds = 6;
       read_stats_seconds = 6;
    }
    else {
      write_stats_seconds = 60;
       read_stats_seconds = 60;
    }

    enable_pcmwrite     = ! com_uti.file_get ("/sdcard/spirit/no_pcmwrite");
    enable_pcmread      = ! com_uti.file_get ("/sdcard/spirit/no_pcmread");
    enable_daemonaudio  = ! com_uti.file_get ("/sdcard/spirit/no_daemonaudio");
    enable_daemonvolume = ! com_uti.file_get ("/sdcard/spirit/no_daemonvolume");

    enable_stockmode    =   com_uti.file_get ("/sdcard/spirit/enable_stockmode");

    if (enable_stockmode) {
      com_uti.logd ("enable_stockmode");
      enable_pcmwrite       = false;
      enable_pcmread        = false;
      enable_daemonaudio    = false;
      enable_daemonvolume   = false;

      com_uti.setDeviceConnectionState (fm_device, com_uti.DEVICE_STATE_AVAILABLE, "");   // Works on MOG only     com_uti.DEVICE_OUT_FM
    }


        // START pcm_write:
    if (enable_pcmwrite) {
      if (thread_pcm_write_active)
        com_uti.loge ("thread_pcm_write_active");
      com_uti.logd ("m_samplerate: " + m_samplerate + "  m_channels: " + m_channels + "  m_audio_bufsize: " + m_audio_bufsize + "  m_audiotrack: " + m_audiotrack + "  m_audiorecord: " + m_audiorecord);
      try {
        m_audiotrack = new AudioTrack (audio_stream, m_samplerate, chan_out_get (m_channels), AudioFormat.ENCODING_PCM_16BIT, m_audio_bufsize, AudioTrack.MODE_STREAM);
        if (m_audiotrack == null)
          com_uti.loge ("m_audiotrack == null");
        else {
          m_audiotrack.play ();                                         // Start output

          thread_pcm_write = new Thread (run_pcm_write, "pcm_write");
          com_uti.logd ("thread_pcm_write: " + thread_pcm_write);
          if (thread_pcm_write == null)
            com_uti.loge ("thread_pcm_write == null");
          else {
            thread_pcm_write_active = true;
            java.lang.Thread.State thread_state = thread_pcm_write.getState ();
            if (thread_state == java.lang.Thread.State.NEW || thread_state == java.lang.Thread.State.TERMINATED) {
              //com_uti.logd ("thread priority: " + thread_pcm_write.getPriority ());   // Get 5
              thread_pcm_write.start ();
            }
            else
              com_uti.loge ("thread_pcm_write thread_state: " + thread_state);
          }
        }
      }
      catch (Throwable e) {
        com_uti.loge ("Throwable: " + e);
        e.printStackTrace ();
      }
    }


        // START pcm_read:
    if (enable_pcmread) {
      if (thread_pcm_read_active)
        com_uti.loge ("thread_pcm_read_active");
      com_uti.logd ("m_samplerate: " + m_samplerate + "  m_channels: " + m_channels + "  m_audio_bufsize: " + m_audio_bufsize + "  m_audiotrack: " + m_audiotrack + "  m_audiorecord: " + m_audiorecord);
      try {
        m_audiorecord = audio_record_get ();
        if (m_audiorecord == null) {
          com_uti.loge ("m_audiorecord == null");
        }
        else {
          m_audiorecord.startRecording ();                            // Start input
          com_uti.logd ("getChannelConfiguration: " + m_audiorecord.getChannelConfiguration () + "   getChannelCount: " +  m_audiorecord.getChannelCount ());
          audiorecord_sessid_int = m_audiorecord.getAudioSessionId ();
          com_uti.logd ("audiorecord_sessid_int: " + audiorecord_sessid_int);
          audiorecord_info_log ();

          thread_pcm_read = new Thread (run_pcm_read, "pcm_read");
          com_uti.logd ("thread_pcm_read: " + thread_pcm_read);
          if (thread_pcm_read == null)
            com_uti.loge ("thread_pcm_read == null");
          else {
            thread_pcm_read_active = true;

            java.lang.Thread.State thread_state = thread_pcm_read.getState ();
            if (thread_state == java.lang.Thread.State.NEW || thread_state == java.lang.Thread.State.TERMINATED) {
              //com_uti.logd ("thread priority: " + thread_pcm_read.getPriority ());   // Get 5
              thread_pcm_read.start ();
            }
            else
              com_uti.loge ("thread_pcm_read thread_state: " + thread_state);
          }
        }
      }
      catch (Exception e) {
        com_uti.loge ("Throwable: " + e);
        e.printStackTrace ();
      }
    }


        // SETUP audio_mode in daemon and start audio from daemon:
        // If analog mode or if digital mode with a pseudo-source (non-direct) and not microphone test mode...
    if (enable_daemonaudio) {
      if (is_analog_audio_mode () || (m_aud_src <= 8 && is_digital_audio_mode ())) {

        if (is_analog_audio_mode ())                                    // If Analog, set daemon to analog...
          com_uti.daemon_set ("audio_mode", "Analog");
        else                                                            // Else if Digital, set daemon to digital...
          com_uti.daemon_set ("audio_mode", "Digital");

        if (is_digital_audio_mode () && com_uti.android_version >= 21 && m_com_api.chass_plug_aud.equals ("OM7") && com_uti.file_get ("/system/framework/htcirlibs.jar")) // If HTC One M7 GPE       (Stock Android 5 too ????)
          com_uti.quiet_ms_sleep (2000);                                // !! Else get microphone 1500 ms not enough sometimes

        com_uti.daemon_set ("audio_state", "Start");                    // Analog: Enable audio directly to output. Digital: Switch from microphone to FM
      }
    }

        // SETUP Volume and digital amp:
    if (enable_daemonvolume)
      volume_set ();                                                    // Set chip tuner volume as needed for analog or digital mode
  }

/*-26 23:20:12.815 D/FMService( 1732): In startFM
02-26 23:20:12.815 I/MediaFocusControl(  688):  AudioFocus  requestAudioFocus() from android.media.AudioManager@26b4e035com.caf.fmradio.FMRadioService$19@3551ccca
02-26 23:20:12.825 D/FMService( 1732): FM registering for registerMediaButtonEventReceiver
02-26 23:20:12.825 D/FMService( 1732): FMRadio: Requesting to start FM

02-26 23:20:12.825 D/FMService( 1732): Audio source set it as headset
02-26 23:20:12.825 D/audio_hw_primary(  300): adev_set_parameters: enter: connect=1048576
02-26 23:20:12.825 E/audio_a2dp_hw(  300): adev_set_parameters: ERROR: set param called even when stream out is null    */


  private void mode_audio_pause () {                                    // Pause audio at hardware level based on mode.   Called only by audio_pause() and mode_audio_stop()
    com_uti.logd ("m_audiotrack: " + m_audiotrack + "  m_audiorecord: " + "  m_com_api.audio_state: " + m_com_api.audio_state);

        // STOP pcm_read:
    com_uti.logd ("thread_pcm_read: " + thread_pcm_read + "  thread_pcm_read_active: " + thread_pcm_read_active);
    if (thread_pcm_read_active) {
      thread_pcm_read_active = false;
      if (thread_pcm_read != null)
        thread_pcm_read.interrupt ();

      if (m_audiorecord != null) {
        m_audiorecord.stop ();                                        // No pause for audioRecord, only stop
        //m_audiorecord.release ();
        //m_audiorecord = null;
      }
    }

        // STOP pcm_write:
    com_uti.logd ("thread_pcm_write: " + thread_pcm_write + "  thread_pcm_write_active: " + thread_pcm_write_active);
    if (thread_pcm_write_active) {
      thread_pcm_write_active = false;
      if (thread_pcm_write != null)
        thread_pcm_write.interrupt ();
    }
    if (m_audiotrack != null)
      m_audiotrack.pause ();                                            // Pause Audiotrack


    if (enable_stockmode)                                               // For stock mode:
       com_uti.setDeviceConnectionState (fm_device, com_uti.DEVICE_STATE_UNAVAILABLE, "");

    boolean muted = false;
    if (enable_daemonaudio) {
        // If analog mode or...
        // If digital mode with a pseudo-source and not in the special microphone test mode...
      if (is_analog_audio_mode () || (m_aud_src <= 8 && is_digital_audio_mode ())) {
        com_uti.daemon_set ("audio_state", "Stop");                     // Analog: Disable audio directly to output. Digital: Switch from FM to microphone (or at least turn FM path off, if needed)
        muted = true;
      }
    }
    if (! muted)
      com_uti.daemon_set ("tuner_mute", "Mute");  // !!!! Ensure tuner is muted to avoid phone call interference !!!! Unmuted when audio restarts



    //if (enable_daemonvolume)
    //  com_uti.logd ("Could/should restore volume here: " + volume_restore ());  // Restore system and chip tuner volume as needed

    com_uti.logd ("reads_processed: " + reads_processed + " writes_processed: " + writes_processed);
  }

  private void mode_audio_stop () {                                     // Stop audio at hardware level based on mode.    Called only by audio_stop() & audio_output_set() (for restart)
    com_uti.logd ("m_audiotrack: " + m_audiotrack + "  m_com_api.audio_state: " + m_com_api.audio_state);

    mode_audio_pause ();                                                // First, pause the audio

    if (m_audiotrack != null) {
      m_audiotrack.stop ();
      m_audiotrack.release ();                                          // Release AudioTrack resources
      m_audiotrack = null;
    }

    if (m_audiorecord != null) {                                        // Release AudioRecord resources
      //m_audiorecord.stop ();                                          // DISABLE: already stopped
      m_audiorecord.release ();
      m_audiorecord = null;
    }

  }


  private String audio_output_get () {
    String ret = "";
    if (m_AM.isBluetoothScoOn ())
      ret += " BluetoothSco";
    if (m_AM.isBluetoothA2dpOn ())
      ret += " BluetoothA2dp";
    if (m_AM.isSpeakerphoneOn ())
      ret += " Speaker";
    if (m_AM.isWiredHeadsetOn ())
      ret += " Headset";
    //com_uti.logd ("ret: " + ret);
    if (ret.equals (""))
      ret = " (none)";
    return (ret);
  }

/* FMRadioService.java:
   private void startFM(){
...

       if (!isSpeakerEnabled() && !mA2dpDeviceSupportInHal &&  (true == mA2dpDeviceState.isDeviceAvailable()) &&
           !isAnalogModeEnabled()
            && (true == startA2dpPlayback())) {
            mOverA2DP=true;
            Log.d(LOGTAG, "Audio source set it as A2DP");
            AudioSystem.setForceUse(AudioSystem.FOR_MEDIA, AudioSystem.FORCE_BT_A2DP);
       } else {
           Log.d(LOGTAG, "FMRadio: Requesting to start FM");
           //reason for resending the Speaker option is we are sending
           //ACTION_FM=1 to AudioManager, the previous state of Speaker we set
           //need not be retained by the Audio Manager.
           if (isSpeakerEnabled()) {
               mSpeakerPhoneOn = true;
               Log.d(LOGTAG, "Audio source set it as speaker");
               AudioSystem.setForceUse(AudioSystem.FOR_MEDIA, AudioSystem.FORCE_SPEAKER);
           } else {
               Log.d(LOGTAG, "Audio source set it as headset");
               AudioSystem.setForceUse(AudioSystem.FOR_MEDIA, AudioSystem.FORCE_NONE);
           }
           AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_FM,
                               AudioSystem.DEVICE_STATE_AVAILABLE, "");

       }
       sendRecordServiceIntent(RECORD_START);
       mPlaybackInProgress = true;
   }

   private void stopFM(){
       Log.d(LOGTAG, "In stopFM");
       if (mOverA2DP==true){
           mOverA2DP=false;
           stopA2dpPlayback();
       }else{
           Log.d(LOGTAG, "FMRadio: Requesting to stop FM");
           AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_FM,
                                     AudioSystem.DEVICE_STATE_UNAVAILABLE, "");




*/
    // CAN_DUCK ?                                                   // Called by:
                                                                        // prefsval  & no restart:         audio_start()
                                                                        // "Headset" & no restart:         audio_pause()    (for audio_state=pause or stop), after pcm read & write stopped
                                                                        // newval    &    restart: svc_svc:onStartCommand() (change from UI/Widget)

  public String audio_output_set (String new_audio_output, boolean restart_param) {

    com_uti.logd ("m_hdst_plgd: " + m_hdst_plgd + "  restart_param: " + restart_param + "  api audio_state: " + m_com_api.audio_state + "  api audio_output: " + m_com_api.audio_output + "  new_audio_output: " + new_audio_output
              + "  audio_output_get: " + audio_output_get ());

    if (com_uti.s2_tx_apk ())                                           // Do nothing if transmit APK...
      return (m_com_api.audio_output = new_audio_output);

    boolean lg2_restart = restart_param;                                // If param restart, then LG2 restart...
    if (! m_com_api.chass_plug_aud.equals ("LG2") || new_audio_output.equals ("Headset"))
      lg2_restart = false;                                              // Unless not LG2 or switching to headset

    boolean restart_enable = restart_param;
    if (! m_com_api.chass_plug_aud.equals ("GS1") && ! m_com_api.chass_plug_aud.equals ("GS2") && ! lg2_restart)
      restart_enable = false;                                            // Only GS1 and GS2 (and LG2 going to speaker) need restart. GS3 may need restart on OmniROM only ?

    if (restart_enable)
      mode_audio_stop ();                                               // Stop Audio at hardware level based on mode.

    if (lg2_restart)                                                    // Without lg2_restart, speaker audio is really messed up   See below at function end; doesn't matter where sleep happens
      com_uti.ms_sleep (3000);                                          // 2500 too small

    if (new_audio_output.equals ("Speaker")) {                          // If speaker is desired...
      if (m_hdst_plgd) {                                                // If headset is plugged (if unplugged, audio is already going to speaker)
        //if (m_com_api.audio_output.equals ("Headset"))                // DISABLED: So we don't check for change, in case we are re-starting audio after an output switch to speaker that was delayed

                                                                        // Headset unavailable: Fool Android that headset unplugged (Android doesn't like apps controlling audio routing,
                                                                        //  and FM reception is better with wired headset plugged in as antenna, even in speaker mode.)
                                                                        // This works better than setForceUse() ??
        com_uti.setDeviceConnectionState (com_uti.DEVICE_OUT_WIRED_HEADSET, com_uti.DEVICE_STATE_UNAVAILABLE, "");

        //if (m_com_api.chass_plug_aud.equals ("GS1"))                  // DISABLED: Don't need anymore ?   WAS: On GS1 "Headphone" also unavailable
        //  com_uti.setDeviceConnectionState (com_uti.DEVICE_OUT_WIRED_HEADPHONE, com_uti.DEVICE_STATE_UNAVAILABLE, "");
      }
      else
        com_uti.logd ("Speaker switch do nothing: m_hdst_plgd == false");
    }

    else if (new_audio_output.equals ("Headset")) {                     // Else if headset is desired...
      //if (m_hdst_plgd)                                                // DISABLED: setDeviceConnectionState() makes this unusable. WAS: If headset is plugged (if unplugged, audio is already going to speaker and can't be switched to unplugged headset)
        if (restart_param) {                                            // WAS DISABLED ?: but should only need to switch to headset when we are restarting due to a UI/Widget output change request
          if (m_com_api.audio_output.equals ("Speaker")) {              // Only need to switch if coming from speaker  

                                                                        // Headset available
            com_uti.setDeviceConnectionState (com_uti.DEVICE_OUT_WIRED_HEADSET, com_uti.DEVICE_STATE_AVAILABLE, "");

            //if (m_com_api.chass_plug_aud.equals ("GS1"))              // DISABLED: Don't need anymore ?   WAS: On GS1 "Headphone" also available
            //  com_uti.setDeviceConnectionState (com_uti.DEVICE_OUT_WIRED_HEADPHONE, com_uti.DEVICE_STATE_AVAILABLE, "");
          }
          else
            com_uti.logd ("Headset switch do nothing: m_com_api.audio_output != Speaker");
        }
        else
          com_uti.logd ("Headset switch do nothing: restart_param == false");
    }

    //if (lg2_restart)                                                  // Without lg2_restart, speaker audio is really messed up   See above at function start; doesn't matter where sleep happens
    //  com_uti.ms_sleep (3000);

    if (restart_enable) {
      mode_audio_start ();                                              // Start audio at hardware level based on mode.
      com_uti.daemon_set ("tuner_mute", "Unmute");                      // !!!! Ensure tuner is unmuted to allow GS2 speaker mode change without needing Pause/Play
    }

    com_uti.output_audio_routing_get ();                                // Log audio routing

    return (m_com_api.audio_output = new_audio_output);                 // Done w/ new output
  }


    // Audio Recorder:

  private void audiorecord_info_log () {
      android.media.audiofx.AutomaticGainControl agc = android.media.audiofx.AutomaticGainControl.create    (audiorecord_sessid_int);
      if (agc != null)
        com_uti.logd ("agc.isAvailable(): " + agc.isAvailable ());
      if (agc != null && agc.isAvailable ()) {
        com_uti.logd ("agc.getEnabled(): "  + agc.getEnabled ());
        com_uti.logd ("agc.hasControl(): "  + agc.hasControl ());
        //com_uti.logd ("agc.setEnabled(!): " + agc.setEnabled (! agc.getEnabled ()));
        //com_uti.logd ("agc.isAvailable(): " + agc.isAvailable ());
        //com_uti.logd ("agc.setEnabled(0): " + agc.setEnabled (false));
        //com_uti.logd ("agc.isAvailable(): " + agc.isAvailable ());
      }
      else
        com_uti.logd ("agc.isAvailable(): false");
      android.media.audiofx.NoiseSuppressor nsu = android.media.audiofx.NoiseSuppressor.create              (audiorecord_sessid_int);
      if (nsu != null)
        com_uti.logd ("nsu.isAvailable(): " + nsu.isAvailable ());
      if (nsu != null && nsu.isAvailable ()) {
        com_uti.logd ("nsu.getEnabled(): "  + nsu.getEnabled ());
        com_uti.logd ("nsu.hasControl(): "  + nsu.hasControl ());
        //com_uti.logd ("nsu.setEnabled(!): " + nsu.setEnabled (! nsu.getEnabled ()));
        //com_uti.logd ("nsu.isAvailable(): " + nsu.isAvailable ());
        //com_uti.logd ("nsu.setEnabled(0): " + nsu.setEnabled (false));
        //com_uti.logd ("nsu.isAvailable(): " + nsu.isAvailable ());
      }
      else
        com_uti.logd ("nsu.isAvailable(): false");
      android.media.audiofx.AcousticEchoCanceler aec = android.media.audiofx.AcousticEchoCanceler.create    (audiorecord_sessid_int);
      if (aec != null)
        com_uti.logd ("aec.isAvailable(): " + aec.isAvailable ());
      if (aec != null && aec.isAvailable ()) {
        com_uti.logd ("aec.getEnabled(): "  + aec.getEnabled ());
        com_uti.logd ("aec.hasControl(): "  + aec.hasControl ());
        //com_uti.logd ("aec.setEnabled(!): " + aec.setEnabled (! aec.getEnabled ()));
        //com_uti.logd ("aec.isAvailable(): " + aec.isAvailable ());
        //com_uti.logd ("aec.setEnabled(0): " + aec.setEnabled (false));
        //com_uti.logd ("aec.isAvailable(): " + aec.isAvailable ());
      }
      else
        com_uti.logd ("aec.isAvailable(): false");
  }


/* android.media.MediaRecorder.AudioSource :
            Value   API Level
DEFAULT             0       1
MIC                 1       1
VOICE_UPLINK        2       4   (Tx)
VOICE_DOWNLINK      3       4   (Rx)
VOICE_CALL          4       4   (uplink + downlink (! if supported !))
CAMCORDER           5       7
VOICE_RECOGNITION   6       7   (Microphone audio source tuned for voice recognition if available, behaves like DEFAULT otherwise. )
VOICE_COMMUNICATION 7       11  (Microphone audio source tuned for voice communications such as VoIP. It will for instance take advantage of echo cancellation or automatic gain control if available. It otherwise behaves like DEFAULT if no voice processing is applied.)

REMOTE
*/


  private AudioRecord audio_record_get () {

      // first entry "16" is replaced by aud_src if set, or 5 CAMCORDER
    int [] m_mic_srcs = new int []   {16,  1, 0, 5, 11, 10, 9, 6, 7};       // Microphone sources       MediaRecorder.AudioSource.DEFAULT ++

    boolean support_direct_sources = false;     // Disable for now; messes up switching analog and digital
    int [] m_dir_srcs = new int []   {16, 11, 10, 9, 5, 1,  0, 5, 6, 7};    // Direct sources           MediaRecorder.AudioSource.DEFAULT ++        !! Stock Xperia Z worked w/ 9/10, CM11 + Lollipop use 5

    int [] m_srcs = null;

    int default_src = MediaRecorder.AudioSource.MIC;//MediaRecorder.AudioSource.CAMCORDER;  // 5

    if (m_com_api.chass_plug_aud.equals ("LG2"))
      default_src = MediaRecorder.AudioSource.MIC;                      // 1

    else if (m_com_api.chass_plug_aud.equals ("QCV") && com_uti.om8_get ())
      default_src = MediaRecorder.AudioSource.CAMCORDER;                // 5      This avoids a 10 second delay resuming paused state when wired headset not plugged in.

    else if (m_com_api.chass_plug_aud.equals ("QCV"))
      default_src = MediaRecorder.AudioSource.CAMCORDER;                // MotoG & Xperia Z lose audio (input 0's) after an hour or 2. Does this work better on some ROMs ?


    else if (m_com_api.chass_plug_aud.equals ("XZ2") && com_uti.android_version >= VERSION_CODES.LOLLIPOP)
      default_src = MediaRecorder.AudioSource.CAMCORDER;            // Camcorder for audio fix.

    m_srcs = m_mic_srcs;

                                                                        // Doesn't work with Xperia Z w/ AOSP, which ends up selecting 5/Camcorder because 9 & 10 don't work.
    if (com_uti.sony_get () || com_uti.stock_mot_get () || com_uti.gs4_mini_get ()) {
      if (support_direct_sources)
        m_srcs = m_dir_srcs;
    }

    if (com_uti.file_get ("/sdcard/spirit/dir_src")) {

      //com_uti.setDeviceConnectionState (/*com_uti.DEVICE_OUT_FM*/0x100000, com_uti.DEVICE_STATE_AVAILABLE, "");

      com_uti.loge ("Force direct source");
      m_srcs = m_dir_srcs;
    }

    int src = 0;
    for (int cnt_src : m_srcs) {                                        // For all sources...
      src = cnt_src;
      if (src == 16) {                                                  // If special first entry...
        String audio_pseudo_source = com_uti.prefs_get (m_context, "audio_pseudo_source", "");
        if (audio_pseudo_source != null && ! audio_pseudo_source.equals ("")) {
          String arr [] = audio_pseudo_source.split (" ", 2);
          String src_num  = arr [0];
          String src_desc = arr [1];
          if (src_num != null)
            src = com_uti.int_get (src_num, src);
          com_uti.logd ("From audio_pseudo_source src: " + src);
        }
      }

      if (src < -1 || (src > 15 && src != 1999) ) {
        src = default_src;
        com_uti.logd ("From default_src final src to try: " + src);
      }
      else
        com_uti.logd ("Have final src to try: " + src);
      try {
        AudioRecord recorder = new AudioRecord (src, m_samplerate, chan_in_get (m_channels), AudioFormat.ENCODING_PCM_16BIT, m_audio_bufsize);
        int rec_state = recorder.getState ();
        com_uti.logd ("rec_state: " + rec_state);
        if (rec_state == AudioRecord.STATE_INITIALIZED) {               // If works, then done, otherwise on to the next to try (Some devices throw exception, some return 0 = Not Initialize)
          com_uti.logd ("Success with src: " + src);
          m_aud_src = src;
          return (recorder);
        }
      }
      catch (Exception e) {
        com_uti.logd ("Exception: " + e );  // "java.lang.IllegalArgumentException: Invalid audio source."
      }
    }
    return (null);
  }


  public void setInternalCapturePreset (int preset) {
    if ((preset == 1999)//MediaRecorder.AudioSource.HOTWORD)
                    || (preset == MediaRecorder.AudioSource.REMOTE_SUBMIX)) {
//      mSource = preset;
    } else {
//      setCapturePreset(preset);
    }
    return;// this;
  }

/*
    AudioFormat.Builder afb = new AudioFormat.Builder ();
    afb.setSampleRate (m_samplerate);
    afb.setChannelMask (chan_in_get (m_channels));
    afb.setEncoding (AudioFormat.ENCODING_PCM_16BIT);
    AudioFormat af = afb.build ();

    AudioAttributes.Builder aab = new AudioAttributes.Builder ();
    aab.setUsage (AudioAttributes.USAGE_MEDIA);
    aab.setContentType (AudioAttributes.CONTENT_TYPE_MUSIC);
    aab.setLegacyStreamType (AudioManager.STREAM_MUSIC);

//aab.mSource = 12; // !!!! Reflect w/ set access !!

    AudioAttributes aa = aab.build ();

    AudioRecord ar = new AudioRecord (m_aud_src, m_samplerate, chan_in_get (m_channels), AudioFormat.ENCODING_PCM_16BIT, m_audio_bufsize);

//    ar = new AudioRecord (aa, af, m_audio_bufsize, 1);
    
  }
*/

/*
  public AudioRecord ar_new (AudioAttributes attributes, AudioFormat format, int bufferSizeInBytes, int sessionId) throws IllegalArgumentException {
    AudioRecord ret = new AudioRecord (attributes, format, bufferSizeInBytes, sessionId);
  }
*/
/*
   public AudioRecord (int audioSource, int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes)
    throws IllegalArgumentException {
        this((new AudioAttributes.Builder())
                    .setInternalCapturePreset(audioSource)
                    .build(),
                (new AudioFormat.Builder())
                    .setChannelMask(getChannelMaskFromLegacyConfig(channelConfig, true))  //allow legacy configurations
                    .setEncoding(audioFormat)
                    .setSampleRate(sampleRateInHz)
                    .build(),
                bufferSizeInBytes,
                AudioManager.AUDIO_SESSION_ID_GENERATE);
    }
*/

  private void volume_observer_register () {
    com_uti.logd ("");
 	android.content.ContentResolver cr = m_context.getContentResolver();

 	android.net.Uri content_uri = null;
    content_uri = android.provider.Settings.System.CONTENT_URI;
    //content_uri = Settings.System.getUriFor (Settings.System.VOLUME_SETTINGS [AudioManager.STREAM_M‌​USIC]);

    boolean notify_for_descendents = true;//false;   // Only the URI we want

    cr.registerContentObserver (content_uri, notify_for_descendents, volume_observer);
  }

  private void volume_observer_unregister () {
    com_uti.logd ("");
    m_context.getContentResolver().unregisterContentObserver (volume_observer);
  }

  private int volume_set () {                                           // Always update cur_stream_vol
                                                                        // If analog mode,  set tuner volume as needed
                                                                        // If digital mode, set tuner volume to maximum
    cur_stream_vol = m_AM.getStreamVolume (audio_stream);
    com_uti.logd ("cur_stream_vol: " + cur_stream_vol + "  enable_stockmode: " + enable_stockmode);

    if (enable_stockmode) {
      return (0);
    }

    int tuner_vol = 65535;
    if (is_analog_audio_mode ()) {
      audio_digital_amp = 0;                                            // Ensure no digital audio is output (except blank 0 filled buffers) (record still works)
      tuner_vol = (cur_stream_vol * 65535) / max_stream_vol;            // tuner_vol range = 0 - 65535
      if (tuner_vol < 0 || tuner_vol > 65535)
        tuner_vol = 32767;
    }
    else {                                                              // If digital mode
      audio_digital_amp = 1;                                            // Factor 1 = normal default
      m_AM.setStreamVolume (audio_stream, cur_stream_vol, 0);           // Restore volume without displaying volume change on screen (have HAL do it after audio plugin has changed volumes)
    }

    //if (! m_com_api.chass_plug_aud.equals ("QCV"))
      com_uti.logd ("tuner_vol: " + tuner_vol);
      com_uti.daemon_set ("tuner_vol", "" + tuner_vol);                   // Set in daemon
    //}

    return (0);
  }

  public String audio_digital_amp_set () {                              // Called from audio_parameters_set() (for audio start) and svc_svc:onStartCommand() (for settings change)
    if (is_analog_audio_mode ()) {                                      // If analog mode...
      audio_digital_amp = 0;
      return ("" + audio_digital_amp);
    }

    double default_audio_digital_amp = 1;
    if (m_com_api.chass_plug_aud.equals ("GS1"))
      default_audio_digital_amp = 3;//2;//4;    // Amplify by 1 = Don't amplify. 4 is good for GS1 but takes up too much CPU on poor old single core GT-I9000; at least with the horribly inefficient Java code at present

    audio_digital_amp = com_uti.prefs_get (m_context, "audio_digital_amp", default_audio_digital_amp);
    com_uti.logd ("audio_digital_amp: " + audio_digital_amp);
    return ("" + audio_digital_amp);
  }

  private android.database.ContentObserver volume_observer = new android.database.ContentObserver (new android.os.Handler()) {
///*
    @Override
    public boolean deliverSelfNotifications() {
      com_uti.logd ("");
      return super.deliverSelfNotifications();
    }
//*/

    @Override
    public void onChange (boolean selfChange) {                         // Faster before super ? !

      if (m_com_api.audio_state.equals ("Start"))
        volume_set ();                                                  // Set chip tuner volume as needed for analog mode
      else
        super.onChange (selfChange);

      com_uti.logd ("selfChange: " + selfChange);
    }

  };

}

