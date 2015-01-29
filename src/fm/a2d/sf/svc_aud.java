
    // Audio Sub-service

package fm.a2d.sf;

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
import android.os.Environment;
import android.os.PowerManager;

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

  private boolean       old_htc     = false;

  private int           m_hw_size   = 4096;
  private int           at_min_size = 32768;//5120 * 16;//65536;
  private int           chan_out    = AudioFormat.CHANNEL_OUT_STEREO;

  private int           int_audio_sessid = 0;

  private boolean       pcm_write_thread_active = false;
  private boolean       pcm_read_thread_active  = false;

  private AudioTrack    m_audiotrack            = null;
  private Thread        pcm_write_thread        = null;
  private Thread        pcm_read_thread         = null;

  private boolean       m_hdst_plgd = false;
  private BroadcastReceiver m_hdst_lstnr = null;


    // Up to 32 buffers:
  private static final int  aud_buf_num = 32;   // 4, 8 skips too often

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


  private int                   writes_processed = 0;
  private int                   reads_processed  = 0;
  private int                   buf_errs = 0;
  private int                   max_bufs = 0;

  private int                   min_pcm_write_buffers = 1;//2;          // Runs if at least 1 buffers are ready...

  private int                   m_channels = 2;
  private int                   m_samplerate  = 44100;                  // Default = 8000 (Max w/ AMR)

  private int                   write_stats_seconds     = 60;//10; // Over 11184 will overflow int in calcs of "stats_frames = (2 * write_stats_seconds * m_samplerate * m_channels) / len;"
  private int                   read_stats_seconds      = 60;//00;//10; // Over 11184 will overflow int in calcs of "stats_frames = (2 *  read_stats_seconds * m_samplerate * m_channels) / len;"

  private boolean               pcm_write_thread_waiting= false;

  //private int                   pcm_priority            = -19;//-20;//-19;                                 // For both read and write

  private int                   pcm_size_max            = 65536;//4096;//8192;//Not tunable yet !!!     65536; // 64Kbytes = 16K stereo samples    // @ 44.1K = 0.37 seconds each buffer = ~ 12 second delay (& mem alloc errors) for 32
  private int                   pcm_size_min            = 320;

  private int                   max_sample = -1000000;
  private int                   min_sample =  1000000;

    // used for "new AudioTrack"        in audio_start()
    // used for requestAudioFocus       in focus_set()  (via audio_start()/audio_stop()
    // used for getStream(Max)Volume    in svc_svc:radio_status_send()  for unused volume reporting
    // used for setVolumeControlStream  in gui_act:onCreate()
  private int                   audio_stream = AudioManager.STREAM_MUSIC;

  private int                   in_spkr_call_vol_poll_tmr_hndlr         = 0;
  private int                   spkr_call_vol_poll_tmr_hndlr_re_enters  = 0;
  private Timer                 spkr_call_vol_poll_tmr                  = null;
  private int                   spkr_call_vol_poll_tmr_hndlr_times      = 0;

  private int max_call_vol = 35;
  private int cur_call_vol = 25;

  private int max_musi_vol = 65;
  private int cur_musi_vol = 55;

  private int []  cur_vols  = new int [8];// {-1, -1, -1, -1, -1, -1, -1, -1};

  private AudioRecord   m_audiorecorder = null;
  private boolean       m_audiorecord_reading = false;

  private boolean aud_mic = true;
  private int m_aud_src = 0;


    // Code:

  private void audio_transmit_pause () {
    audio_transmit_stop ();
  }
  private void audio_transmit_stop () {
dai_set (false);

//    String cmd1 = "/data/data/fm.a2d.st/lib/libssd.so 4 1 \"SLIMBUS_0_RX Audio Mixer MultiMedia1\" 1";    // Wired headset audio on
//    sys_run (cmd1, true);//false);

//    com_uti.alsa_bool_set ("SLIMBUS_0_RX Audio Mixer MultiMedia1", 1);    // Wired headset audio on
//    com_uti.ssd_commit ();

    //AudioSystem.setDeviceConnectionState (AudioSystem.DEVICE_OUT_WIRED_HEADSET, AudioSystem.DEVICE_STATE_AVAILABLE, "");     // Headset available
  }

  private void audio_transmit_start () {
dai_set (true);


  //AudioManager  m_AM        = null;
  //m_AM = (AudioManager) this.getSystemService (Context.AUDIO_SERVICE);
  if (m_AM != null) {
    int max_musi_vol = m_AM.getStreamMaxVolume  (AudioManager.STREAM_MUSIC);
    //int cur_musi_vol = m_AM.getStreamVolume     (AudioManager.STREAM_MUSIC);

    m_AM.setStreamVolume (AudioManager.STREAM_MUSIC, max_musi_vol, AudioManager.FLAG_SHOW_UI);// Display volume change
  }

//      AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_FM_TX, AudioSystem.DEVICE_STATE_AVAILABLE, "");
/*
      String cmd1 = "/data/data/fm.a2d.st/lib/libssd.so 4 1 \"SLIMBUS_0_RX Audio Mixer MultiMedia1\" 0";    // Wired headset audio off

      //AudioSystem.setDeviceConnectionState (AudioSystem.DEVICE_OUT_WIRED_HEADSET, AudioSystem.DEVICE_STATE_UNAVAILABLE, "");     // Headset unavailable

      String cmd2 = "/data/data/fm.a2d.st/lib/libssd.so 4 1 \"INTERNAL_FM_RX Audio Mixer MultiMedia1\" 1";
      String cmd3 = "/data/data/fm.a2d.st/lib/libssd.so 4 1 \"INTERNAL_FM_RX Audio Mixer MultiMedia4\" 1";
      String cmd4 = "/data/data/fm.a2d.st/lib/libssd.so 4 1 \"INTERNAL_FM_RX Audio Mixer MultiMedia5\" 1";
      String [] cmds = {cmd1, cmd2, cmd3, cmd4};
      //String [] cmds = {cmd2, cmd3, cmd4};
      //String [] cmds = {cmd1, cmd2};
      sys_run (cmds, true);//false);
*/
/* !!!! Why does this block on MOG, OXL and ONE ???
      String cmd1 = "/data/data/fm.a2d.sf/files/ssd 4 1 \"SLIMBUS_0_RX Audio Mixer MultiMedia1\" 0";    // Wired headset audio off
      String cmd2 = "/data/data/fm.a2d.sf/files/ssd 4 1 \"INTERNAL_FM_RX Audio Mixer MultiMedia1\" 1";
      //String cmd3 = "/data/data/fm.a2d.sf/files/ssd 4 1 \"INTERNAL_FM_RX Audio Mixer MultiMedia4\" 1";
      //String cmd4 = "/data/data/fm.a2d.sf/files/ssd 4 1 \"INTERNAL_FM_RX Audio Mixer MultiMedia5\" 1";
      //String [] cmds = {cmd1, cmd2, cmd3, cmd4};
      String [] cmds = {cmd1, cmd2};
      com_uti.sys_run (cmds, true);//false);
*/

/*
      com_uti.alsa_bool_set ("SLIMBUS_0_RX Audio Mixer MultiMedia1", 0);    // Wired headset audio off
      com_uti.ssd_commit ();
      com_uti.alsa_bool_set ("INTERNAL_FM_RX Audio Mixer MultiMedia1", 1);
      com_uti.ssd_commit ();
      com_uti.alsa_bool_set ("INTERNAL_FM_RX Audio Mixer MultiMedia4", 1);
      com_uti.ssd_commit ();
      com_uti.alsa_bool_set ("INTERNAL_FM_RX Audio Mixer MultiMedia5", 1);
      com_uti.ssd_commit ();
*/
      //if (m_svc_acb != null)
      //  m_svc_acb.cb_audio_state_chngd ("start");

  }

  private boolean s2_tx = false;

  public svc_aud (Context c, svc_acb cb_aud, com_api svc_com_api) {                              // Constructor

    com_uti.logd ("m_obinits: " + m_obinits++);

    m_svc_acb = cb_aud;
    m_context = c;
    m_com_api = svc_com_api;
    com_uti.logd ("");

    s2_tx = com_uti.s2_tx_get ();
    if (s2_tx)
      com_uti.logd ("s2_tx");

    old_htc = false;
    if (com_uti.m_device.startsWith ("EVITA") || com_uti.m_device.startsWith ("VILLE") || com_uti.m_device.startsWith ("JEWEL")) {// || com_uti.m_device.startsWith ("M7C")) {
      old_htc = true;
    }

    m_AM = (AudioManager) c.getSystemService (Context.AUDIO_SERVICE);


/* Make buffer size a multiple of AudioManager.getProperty (PROPERTY_OUTPUT_FRAMES_PER_BUFFER). Otherwise your callback will occasionally get two calls per timeslice rather than one.
    Unless your CPU usage is really light, this will probably end up glitching.

Use the sample rate provided by AudioManager.getProperty (PROPERTY_OUTPUT_SAMPLE_RATE). Otherwise your buffers take a detour through the system resampler.

API level 17 / 4.2+
 */
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
    com_uti.logd ("m_samplerate 1: " + m_samplerate);
    if (m_samplerate >= 8000 && m_samplerate <= 192000)
      m_samplerate = m_samplerate;
    else
      m_samplerate = 44100;
    com_uti.logd ("m_samplerate 2: " + m_samplerate);
//at_min_size =

    if (com_uti.device == com_uti.DEV_GEN)
      m_samplerate = 48000;
    else if (com_uti.device == com_uti.DEV_QCV && old_htc)                       // Still no good w/ 48K x 3840
      m_samplerate = 48000;//24000;//48000;//22050;//48000;//24000;//22050;//24000;//48000;//22050;//8000;//22050;
    else if (com_uti.device == com_uti.DEV_QCV)
      m_samplerate = 48000;//Feb28 back to 44.1     48000;//22050;//Feb10!!!!  48000;                                         // !!!! Different than prior default of 44100
    else if (com_uti.device == com_uti.DEV_ONE)
      m_samplerate = 48000;//24000;//48000;//22050;//48000;//22050;//44100;//8000;
    else if (com_uti.device == com_uti.DEV_XZ2)
      m_samplerate = 48000;
    else if (com_uti.device == com_uti.DEV_LG2 && com_uti.android_version >= 21) {
      m_samplerate = 48000;
    }
    else if (com_uti.device == com_uti.DEV_LG2) {
      m_samplerate = 44100;//48000;//22050;//44100;//Mar03  48000;//Mar02  44100;//Feb28 back to 44.1     48000;                                         // !!!! Different than prior default of 44100
      if (com_uti.lg2_stock_get ())
        m_samplerate = 44100;//22050;//44100;//48000;//22050;//44100;
    }
    else if (com_uti.device == com_uti.DEV_GS1)
      m_samplerate = 44100;
    else if (com_uti.device == com_uti.DEV_GS2)
      m_samplerate = 44100;
    else if (com_uti.device == com_uti.DEV_GS3)
      m_samplerate = 44100;


    if (android.os.Build.MANUFACTURER.toUpperCase (Locale.getDefault ()).equals ("SONY")
            && (com_uti.device != com_uti.DEV_XZ2 || com_uti.android_version < 21)  // Add because FXP on Z2 needs 48k
            && (com_uti.device != com_uti.DEV_QCV || com_uti.android_version < 21)  // Add because FXP on Z1 needs 48k too
    )
      m_samplerate = 44100;

    com_uti.logd ("m_samplerate 3: " + m_samplerate);

//!!DISABLE!!    m_samplerate = 44100;   // !! Fixed now !!
//!!DISABLE!!    com_uti.logd ("m_samplerate 4 forced all to: " + m_samplerate);

    m_hw_size = 3840;//320;//3840;//4096;        If wrong, 3840 works better on Qualcomm at least
    try {
      m_hw_size = 2 * m_channels * Integer.parseInt (hw_size_str);
    }
    catch (Throwable e) {
      com_uti.loge ("Rate Throwable e: " + e);
      e.printStackTrace ();
    }
    com_uti.logd ("m_hw_size 1: " + m_hw_size);
    if (m_hw_size >= 64 && m_hw_size <= pcm_size_max)
      m_hw_size = m_hw_size;
    else
      m_hw_size = 3840;//320;//3840;//4096;
    com_uti.logd ("m_hw_size 2: " + m_hw_size);

    if (com_uti.device == com_uti.DEV_GEN)
      m_hw_size = 16384;
    else if (com_uti.device == com_uti.DEV_QCV && old_htc)   // HTC OneXL misreports as 2048, but atminsize shows same as MotoG
      m_hw_size = 320;//1920;//3840;//320;//3840;//1920;//320; // Still no good w/ 48K x 3840
    else if (com_uti.device == com_uti.DEV_QCV && com_uti.m_manufacturer.equals ("SONY"))
      m_hw_size = 5120;//320;
    else if (com_uti.device == com_uti.DEV_QCV)
      m_hw_size = 5120;//3840;//320;//3840;                                    // ?? Actual buffersize = 960 bytes ?
    else if (com_uti.device == com_uti.DEV_ONE)         // ?? HTC J One Try 8160 ?? (25.5 * 320)
      m_hw_size = 320;//3840 / 1;//1920;//960;//2304;//3072;
    else if (com_uti.device == com_uti.DEV_XZ2)
      m_hw_size = 5120;//320
    else if (com_uti.device == com_uti.DEV_LG2) {
      if (com_uti.lg2_stock_get ())
        m_hw_size = 16384;//32768;//4096;//1024;
      else
        m_hw_size = 3840;//320;//3840;
    }
    else if (com_uti.device == com_uti.DEV_GS1)
      m_hw_size = 3520;
    else if (com_uti.device == com_uti.DEV_GS2)
      m_hw_size = 4096;
    else if (com_uti.device == com_uti.DEV_GS3)
      m_hw_size = 4224;
// Sony C6603: 1024

    com_uti.logd ("m_hw_size 3: " + m_hw_size);

    if (com_uti.device == com_uti.DEV_QCV && old_htc)
      m_hw_size = 5120 * 2;
    else if (com_uti.device == com_uti.DEV_ONE)
      m_hw_size = 5120 * 2;
    else
      m_hw_size = AudioRecord.getMinBufferSize (44100, AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT);
    com_uti.logd ("m_hw_size 4: " + m_hw_size);

    if (m_hw_size > pcm_size_max)
      m_hw_size = pcm_size_max;
    if (m_hw_size < pcm_size_min)
      m_hw_size = pcm_size_min;

 }

    // Command handlers:

    //android.media.audiofx.Equalizer m_equalizer = null;

    // Open audio effect control session (before playback ?):           For EQ and Visualisations ?
  private void audio_session_start (int sessid) {
    com_uti.logd ("sessid: " + sessid);
    //int priority = 0;//     !! Any value over 0 kills CM11 DSP Manager     1;//2^7;//2^15;//2147483647;
    //m_equalizer = new android.media.audiofx.Equalizer (priority, sessid);
  }
    // Close audio effect control session (before audio object dies ?)
  private void audio_session_stop (int sessid) {
    com_uti.logd ("sessid: " + sessid);
  }

  public String audio_sessid_get () {                                  // Handle audio session changes
    if (s2_tx) {
      com_uti.logd ("s2_tx");
      //return;
      //return (0);
      return ("0");
    }
    int new_int_audio_sessid = 0;
    if (m_audiotrack != null)                                           // If we have an audiotrack active...
      new_int_audio_sessid = m_audiotrack.getAudioSessionId ();

    //com_uti.logd ("new_int_audio_sessid: " + new_int_audio_sessid + "  int_audio_sessid: " + int_audio_sessid);

    if (new_int_audio_sessid > 0) {                                     // If valid session ID...
      if (int_audio_sessid != new_int_audio_sessid) {                   // If new session ID...
        com_uti.logd ("new_int_audio_sessid: " + new_int_audio_sessid + "  int_audio_sessid: " + int_audio_sessid);
        int_audio_sessid = new_int_audio_sessid;
        audio_session_start (new_int_audio_sessid);
      }
    }
    else {                                                              // Else if no session ID / audiotrack active...
      if (int_audio_sessid > 0) {                                       // If we previously had an active session...
        com_uti.logd ("new_int_audio_sessid: " + new_int_audio_sessid + "  int_audio_sessid: " + int_audio_sessid);
        audio_session_stop (int_audio_sessid);                          // Stop it
        int_audio_sessid = 0;
      }
    }
    m_com_api.audio_sessid = "" + int_audio_sessid;
    return (m_com_api.audio_sessid);
  }



    // Player and overall audio state control: (public's now via m_com_api)

  public String audio_state_set (String desired_state) {                // Called only by svc_svc:audio_state_set() & svc_svc:audio_start()
    if (s2_tx)
      com_uti.logd ("s2_tx");

    com_uti.logd ("desired_state: " + desired_state + "  current audio_state: " + m_com_api.audio_state);
    if (desired_state.equalsIgnoreCase ("toggle")) {                    // TOGGLE:
      if (m_com_api.audio_state.equalsIgnoreCase ("start"))
        desired_state = "pause";
      else
        desired_state = "start";
    }

    if (desired_state.equalsIgnoreCase ("start")) {                     // START:
      if (s2_tx) {
        audio_transmit_start ();
        m_com_api.audio_state = desired_state;
        //if (m_svc_acb != null)
        //  m_svc_acb.cb_audio_state_chngd (desired_state);
      }
      else
        audio_start ();
    }
    else if (desired_state.equalsIgnoreCase ("stop")) {                 // STOP:
      if (s2_tx) {
        audio_transmit_stop ();
        m_com_api.audio_state = desired_state;
        //if (m_svc_acb != null)
        //  m_svc_acb.cb_audio_state_chngd (desired_state);
      }
      else
        audio_stop ();
    }
    else if (desired_state.equalsIgnoreCase ("pause")) {                // PAUSE:
      if (s2_tx) {
        audio_transmit_pause ();
        m_com_api.audio_state = desired_state;
        //if (m_svc_acb != null)
        //  m_svc_acb.cb_audio_state_chngd (desired_state);
      }
      else
        audio_pause ();
    }
    return (m_com_api.audio_state);
  }

  private void at_min_size_set () {

    at_min_size = AudioTrack.getMinBufferSize (m_samplerate, chan_out, AudioFormat.ENCODING_PCM_16BIT );
    com_uti.logd ("at_min_size 1: " + at_min_size);
            //QCV at 44.1: 22576
    at_min_size = 32768;//5120 * 16;//65536;//                                        // 80 KBytes about 0.5 seconds of stereo audio data at 44K
            // QCV: 11288, 12288 (12 * 2^10) = 6144 samples = 128 milliseconds, 24576


      if (com_uti.device == com_uti.DEV_QCV)
        at_min_size = 30720;
      else if (com_uti.device == com_uti.DEV_ONE)       // 22050
        at_min_size = 30720;//32000;//768;//24576;//30720;//38400; // ??
      else if (com_uti.device == com_uti.DEV_XZ2)
        at_min_size = 30720;
      else if (com_uti.device == com_uti.DEV_LG2) {
        if (com_uti.lg2_stock_get ())
          at_min_size = 32768;
        else
          at_min_size = 30720;
      }
      else if (com_uti.device == com_uti.DEV_GS1)
        at_min_size = 12672;
      else if (com_uti.device == com_uti.DEV_GS2)
        at_min_size = 16384;
      else if (com_uti.device == com_uti.DEV_GS3)
        at_min_size = 25344;

      com_uti.logd ("at_min_size 2: " + at_min_size);
  }

  private void audio_start () {
    com_uti.logd ("audio_state: " + m_com_api.audio_state + "  m_audiotrack: " + m_audiotrack + "  device: " + com_uti.device);

    if (m_com_api.audio_state.equalsIgnoreCase ("start"))               // If already playing...
      return;

    headset_plgd_lstnr_start ();                                        // Register for headset plugged/unplugged events

    focus_set (true);                                                   // Get audio focus

    pcm_audio_start (true);                                             // Start input and output

    m_com_api.audio_state = "start";
    if (m_svc_acb != null)
      m_svc_acb.cb_audio_state (m_com_api.audio_state);
  }

    // Called externally for user requested pause
    // Called internally for audio_stop and transient focus loss
  private void audio_pause () {

    com_uti.logd ("audio_state: " + m_com_api.audio_state);

    if (! m_com_api.audio_state.equalsIgnoreCase ("start") && ! m_com_api.audio_state.equalsIgnoreCase ("pausing") && ! m_com_api.audio_state.equalsIgnoreCase ("stopping"))
      return;   // !! What about Starting ??

    pcm_audio_pause (true);

    audio_output_off ();    // AFTER

    m_com_api.audio_state = "pause";
    if (m_svc_acb != null)
      m_svc_acb.cb_audio_state (m_com_api.audio_state);                 // CAN_DUCK
  }

    // Called externally for tuner stop and service onDestroy
    // Called internally for former GS1 audio_pause and for onError of MediaPlayer       May re-add for digital/analog toggle
  private void audio_stop () {                                           // Stop audio

    com_uti.logd ("audio_state: " + m_com_api.audio_state);

    if (! m_com_api.audio_state.equalsIgnoreCase ("start") && ! m_com_api.audio_state.equalsIgnoreCase ("pause") && ! m_com_api.audio_state.equalsIgnoreCase ("stopping"))
      return;   // !! What about Starting ??

    headset_plgd_lstnr_stop ();                                         // Unregister for headset plugged/unplugged events

    audio_pause ();

    if (m_audiotrack != null)
      m_audiotrack.release ();
    m_audiotrack = null;

    m_com_api.audio_state = "stop";
    if (m_svc_acb != null)
      m_svc_acb.cb_audio_state (m_com_api.audio_state);
    focus_set (false);
    //stopSelf ();                                                    // service is no longer necessary. Will be started again if needed.
  }


/*
  private void sink_state_handler (Intent intent) {
            if (m_pwr_state){

              int state = intent.getIntExtra ("android.bluetooth.a2dp.extra.SINK_STATE", BluetoothA2dp.STATE_DISCONNECTED);

              if (state == BluetoothA2dp.STATE_CONNECTED || state == BluetoothA2dp.STATE_PLAYING) {
                if (! m_over_a2dp) {
                  mute_set (true);                                         // Restart audio
                  mute_set (false);
                }
              }
              else {
                if (m_over_a2dp) {
                  mute_set (true);                                         // Restart audio
                  mute_set (false);
                }
              }
            }
  }
*/

    // Headset listener:

/*  Receiver Lifecycle

A BroadcastReceiver object is only valid for the duration of the call to onReceive(Context, Intent).
Once your code returns from this function, the system considers the object to be finished and no longer active.

This has important repercussions to what you can do in an onReceive(Context, Intent) implementation: anything that requires asynchronous operation is not available,
because you will need to return from the function to handle the asynchronous operation, but at that point the BroadcastReceiver is no longer active and thus
the system is free to kill its process before the asynchronous operation completes.

In particular, you may not show a dialog or bind to a service from within a BroadcastReceiver. For the former, you should instead use the NotificationManager API.



For the latter, you can use Context.startService() to send a command to the service.  */


    // Listen for ACTION_HEADSET_PLUG notifications. (Plugged in/out)

  private void headset_plgd_lstnr_stop () {
    com_uti.logd ("m_hdst_lstnr: " + m_hdst_lstnr);
    try {
      if (m_hdst_lstnr != null)
        m_context.unregisterReceiver (m_hdst_lstnr);
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      //e.printStackTrace ();
    }
  }
  private void headset_plgd_lstnr_start () {                             // Headset plug events
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

/* ACTION_HEADSET_PLUG is a sticky broadcast. Call:

    Intent last_broadcast_hdst_plug = registerReceiver (null, new IntentFilter (ACTION_HEADSET_PLUG), null, null);
    
To get the last-broadcast Intent for that action, which will tell you what the headset state is. 

My code now looks as follows:

Intent intent = registerReceiver (myHandler,new IntentFilter(Intent.ACTION_HEADSET_PLUG));
if (intent != null)
  myHandler.onReceive (this, intent);
*/

  private void headset_plgd_handler (Intent intent) {
    com_uti.logd ("headset_plgd_handler Intent received: " + intent + "  state: " + intent.getIntExtra ("state", -555) + "  name: " + intent.getStringExtra ("name"));

    //headset_plgd_lstnr_start onReceive ACTION_HEADSET_PLUG Intent received: Intent { act=android.intent.action.HEADSET_PLUG flg= 0x40000000 (has extras) }  state: 1  name: h2w
    //headset_plgd_lstnr_start onReceive ACTION_HEADSET_PLUG Intent received: Intent { act=android.intent.action.HEADSET_PLUG flg= 0x40000000 (has extras) }  state: 0  name: h2w
    // ?? state: 8 means unplugged ?
    //headset_plgd_handler Intent received: Intent { act=android.intent.action.HEADSET_PLUG flg=0x40000010 (has extras) }  state: 0  name: Headset

    int state = intent.getIntExtra ("state", -555);
    if (state == -555) {
      com_uti.loge ("headset_plgd_handler no state");
    }

    if (state != 0) {
      m_hdst_plgd = true;
      return;
    }

    m_hdst_plgd = false;

        // If audio started & headset mic selected & want FM and not mic...
    if (m_com_api.audio_state.equalsIgnoreCase ("start") && m_aud_src <= 1 && ! com_uti.file_get ("/sdcard/spirit/aud_mic"))
      dai_set (true);                                                   // Re-establish FM instead of microphone
  }

  private int pcm_write_start () {
    m_channels = 2;
    if (m_com_api.audio_stereo.equalsIgnoreCase ("Mono")) {
      //chan_out = AudioFormat.CHANNEL_OUT_MONO;        // !!!!!!!! Why disabled !!!! ????
      //m_channels = 1;
    }
    at_min_size_set ();
    com_uti.logd ("samp_rate: " + m_samplerate + "  chan_out: " + chan_out + "  at_min_size: " + at_min_size + "  m_audiotrack: " + m_audiotrack);
    try {
      m_audiotrack = new AudioTrack (audio_stream, m_samplerate, chan_out, AudioFormat.ENCODING_PCM_16BIT, at_min_size, AudioTrack.MODE_STREAM);
      if (m_audiotrack == null)
        return (-1);
      m_audiotrack.play ();                                           // java.lang.IllegalStateException: play() called on uninitialized AudioTrack.
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
    }

    if (pcm_write_thread_active)
      return (-1);

    pcm_write_thread = new Thread (pcm_write_runnable, "pcm_write");
    com_uti.logd ("pcm_write_thread: " + pcm_write_thread);
    if (pcm_write_thread == null) {
      com_uti.loge ("pcm_write_thread == null");
      return (-1);
    }
    pcm_write_thread_active = true;

    try {                                                       // !!!! Thread may already be started
      if (pcm_write_thread.getState () == java.lang.Thread.State.NEW || pcm_write_thread.getState () == java.lang.Thread.State.TERMINATED) {
        //com_uti.logd ("thread priority: " + pcm_write_thread.getPriority ());   // Get 5
        pcm_write_thread.start ();
      }
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
      return (-1);
    }
    return (0);
  }
  private int pcm_write_stop () {
    pcm_write_thread_active = false;
    if (pcm_write_thread != null)
      pcm_write_thread.interrupt ();
    return (0);
  }

    // Focus:
  private void focus_set (boolean focus_request) {
    com_uti.logd ("focus_request: " + focus_request);
    int ret = 0;
    if (focus_request)                                                  // If focus desired...
      ret = m_AM.requestAudioFocus (this, audio_stream, AudioManager.AUDIOFOCUS_GAIN);
      //ret = m_AM.requestAudioFocus (this, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
    else                                                                // If focus return...
      ret = m_AM.abandonAudioFocus (this);
    if (ret != AudioManager.AUDIOFOCUS_REQUEST_GRANTED)
      com_uti.loge ("ret: " + ret);
  }
  public void onAudioFocusChange (int focusChange) {
    if (s2_tx) {
      com_uti.logd ("s2_tx");
      return;
      //return (0);
      //return ("0");
    }
    com_uti.logd ("focusChange: " + focusChange + "  audio_state: " + m_com_api.audio_state);
    switch (focusChange) {
      case AudioManager.AUDIOFOCUS_GAIN:                                // Gain
        com_uti.logd ("focusChange: GAIN");
        audio_start ();                                                 // Start/Restart audio
        break;
      case AudioManager.AUDIOFOCUS_LOSS:                                // Permanent loss
        //com_uti.logd ("focusChange: LOSS");
        audio_stop ();                                                  // Stop audio       Loss of/stopping audio could/should stop tuner (& Tuner API ?)
        com_uti.logd ("DONE focusChange: LOSS");
        break;
      case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:                      // Transient loss
        //com_uti.logd ("focusChange: ...LOSS_TRANSIENT");
        audio_pause ();                                                 // Pause audio
        com_uti.logd ("DONE focusChange: ...LOSS_TRANSIENT");
        break;
      case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:             // Transient loss we can ignore if we want
        //com_uti.logd ("focusChange: LOSS_TRANSIENT_CAN_DUCK");
        audio_pause ();                                                 // Pause audio
        com_uti.logd ("DONE focusChange: LOSS_TRANSIENT_CAN_DUCK");
        break;
       default:
    }
  }

    // PCM:

  public String audio_record_state_set (String state) {
    if (s2_tx) {
      com_uti.logd ("s2_tx");
      return ("Stop");
      //return (0);
      //return ("0");
    }
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

/* !!
private final int getAndIncrement(int modulo) {
    for (;;) {
        int current = atomicInteger.get();
        int next = (current + 1) % modulo;
        if (atomicInteger.compareAndSet(current, next))
            return current;
    }
}
*/

    // pcm_read -> pcm_write
  private final Runnable pcm_write_runnable = new Runnable () {
    public void run () {
      com_uti.logd ("pcm_write_runnable run()");

      //native_priority_set (pcm_priority);

                                                                        // Setup temp vars before loop to minimize garbage collection
      byte [] aud_buf;
      int bufs = 0;
      int len = 0;
      int len_written = 0;
      long ms_start = 0;
      long ms_time  = 0;

      try {

        while (pcm_write_thread_active) {                               // While PCM Write Thread should be active...
          bufs = aud_buf_tail - aud_buf_head;
          //com_uti.loge ("pcm_write_runnable run() bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);

          if (bufs < 0)
            bufs += aud_buf_num;                                        // Fix underflow
          if (bufs < min_pcm_write_buffers) {                           // If minimum number of buffers is not ready... (Currently at least 2)
            try {
              pcm_write_thread_waiting = true;
              //Thread.sleep (1);                                       // Wait ms milliseconds
              //Thread.sleep (2000);                                    // Wait ms milliseconds   More efficient
              //Thread.sleep (3);                                         // Wait ms milliseconds    3 matches Spirit1
              Thread.sleep (100);       // 100 ms compromise ?
              pcm_write_thread_waiting = false;
            }
            catch (InterruptedException e) {
              pcm_write_thread_waiting = false;
              //Thread.currentThread().interrupt();
              //e.printStackTrace ();
            }
            continue;                                                   // Restart loop
          }
                                                                        // Here when at least 2 buffers are ready to write, so we write the 1st at the head...
          //com_uti.loge ("pcm_write_runnable run() ready to write bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);
          len = aud_buf_len [aud_buf_head];                             // Length of head buffer in bytes
          aud_buf = aud_buf_data [aud_buf_head];                        // Pointer to head buffer
          if (aud_buf == null) {
            com_uti.loge ("pcm_write_runnable run() len: " + len + "  len_written: " + len_written + "  aud_buf: " + aud_buf);
            continue;
          }
          //com_uti.loge ("pcm_write_runnable run() ready to write bufs: " + bufs + "  len: " + len + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);
          ms_start = com_uti.ms_get ();
          len_written = m_audiotrack.write (aud_buf, 0, len);  // Write head buffer to audiotrack  All parameters in bytes (but could be all in shorts)
          ms_time = com_uti.ms_get () - ms_start;
          if (ms_time >= 300)   // GS1 shows over 140 ms
            com_uti.loge ("pcm_write_runnable run() m_audiotrack.write too long ms_time: " + ms_time + "  len: " + len + "  len_written: " + len_written + "  aud_buf: " + aud_buf);

          if (len_written != len)   // !! Note2: run: pcm_write_runnable run() ms_time: 0  len: 23040  len_written: 8448  aud_buf: [B@3147d103
            com_uti.loge ("pcm_write_runnable run() ms_time: " + ms_time + "  len: " + len + "  len_written: " + len_written + "  aud_buf: " + aud_buf);


          if (com_uti.ena_debug_log && writes_processed % ((2 * write_stats_seconds * m_samplerate * m_channels) / len) == 0)   // Every stats_seconds
            pcm_stat_logs ("Write", m_channels, len, aud_buf);
          writes_processed ++;                                          // Update pointers etc
          aud_buf_head ++;
          if (aud_buf_head < 0 || aud_buf_head > aud_buf_num - 1)
            aud_buf_head &= aud_buf_num - 1;
          continue;                                                     // Restart loop

        }   // while (pcm_write_thread_active) {
                                                                        // Here when thread is finished...
        com_uti.logd ("pcm_write_runnable run() done writes_processed: " + writes_processed);
      }
      catch (Throwable e) {
        com_uti.loge ("pcm_write_runnable run() throwable: " + e);
        e.printStackTrace ();
      }                                                                 // Fall through to terminate if exception
      return;
    }
  };



    // Native API:
/*
  static {
    System.loadLibrary ("jut");
  }

    // PCM other:
  private native int native_priority_set    (int priority);

  private native int native_prop_get        (int prop);
*/


  private android.os.Handler dai_delay_handler  = new android.os.Handler ();    // Need at init else: java.lang.RuntimeException: Can't create handler inside thread that has not called Looper.prepare()
  private Runnable           dai_delay_runnable = null;
  private int dai_delay = 0;

  private String dai_do (final boolean enable) {
    com_uti.logd ("enable: " + enable);
    String ret = "";
    if (enable)
      ret = com_uti.daemon_set ("radio_dai_state", "Start");       // m_plg_api.digital_input_on ();
    else
      ret = com_uti.daemon_set ("radio_dai_state", "Stop");        // m_plg_api.digital_input_off ();
    return (ret);
  }

  private String dai_set (final boolean enable) {
    String ret = "";
    com_uti.logd ("enable: " + enable);

    if (! enable)                                                       // If disable
      dai_delay = 0;                                                    // Do immediately
    else if (com_uti.device == com_uti.DEV_GS2)
      dai_delay = 500;//200;
    else if (com_uti.device == com_uti.DEV_GS3)
      dai_delay = 200;
    else
      dai_delay = 200;//0;//1000;

dai_delay = 0;  // !! No delay ??

    if (dai_delay <= 0)
      dai_do (enable);
    else if (dai_delay_handler != null) {
      dai_delay_runnable = new Runnable () {
        public void run() { 
          dai_do (enable);
        } 
      };
      dai_delay_handler.postDelayed (dai_delay_runnable, dai_delay);
    }
    else {
      com_uti.loge ("!!!!!!!!!!!!!!");
      dai_do (enable);
    }

    return (ret);
  }


  private int pcm_read_start () {
    com_uti.logd ("pcm_read_start pcm_read_thread_active: " + pcm_read_thread_active);
    if (pcm_read_thread_active)
      return (-1);

    audiorecorder_read_start ();

    pcm_read_thread_active = true;
    pcm_read_thread = new Thread (pcm_read_runnable, "pcm_read");
    pcm_read_thread.start ();

    return (0);
  }
  private int pcm_read_stop () {
    com_uti.logd ("pcm_read_stop pcm_read_thread_active: " + pcm_read_thread_active);
    if (! pcm_read_thread_active)
      return (-1);

    pcm_read_thread_active = false;
    pcm_read_thread.interrupt ();

    audiorecorder_read_stop ();

        // If mic selected & not FM & want FM and not mic...
    if (m_aud_src <= 8 && ! com_uti.file_get ("/sdcard/spirit/aud_mic"))
      dai_set (false);                                                  // De-establish FM instead of microphone

    return (0);
  }

  private final Runnable pcm_read_runnable = new Runnable () {          // Read/Input thread
    public void run () {
      com_uti.logd ("pcm_read_runnable run()");

      //native_priority_set (pcm_priority);
                                                                        // Setup temp vars before loop to minimize garbage collection
      byte [] aud_buf;
      int bufs = 0;
      int len = 0;
      //int len_written = 0;
      int ctr = 0;

      try {
        buf_errs = 0;                                                   // Init stats, pointers, etc
        aud_buf_tail = aud_buf_head = 0;                                // Drop all buffers
        com_uti.logd ("pcm_read_runnable run() m_samplerate: " +  m_samplerate + "  m_channels: " + m_channels);

        while (pcm_read_thread_active) {                                // While PCM Read Thread should be active...

          bufs = aud_buf_tail - aud_buf_head;
          if (bufs < 0)                                                 // If underflowed...
            bufs += aud_buf_num;                                        // Wrap
          //logd ("bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);

          if (bufs > max_bufs)                                          // If new maximum buffers in progress...
            max_bufs = bufs;                                            // Save new max

          if (bufs >= (aud_buf_num * 3) / 4) {                          // If 75% or more or buffers still in progress (If write thread is getting backed up)
            if (pcm_write_thread != null && pcm_write_thread_waiting)
              pcm_write_thread.interrupt ();                            // Wake up pcm_write_thread sooner than usual
            com_uti.ms_sleep (300);                                     // Sleep to let write thread process.   0.1s/0.3s = 20/60KBytes @ 48k stereo  (2.5/8 8k buffers)
          }
          if (bufs >= aud_buf_num - 1) {                                // If room to write another buffer (max = aud_buf_num - 1 to prevent wrap-around)
            com_uti.loge ("Out of aud_buf");
            buf_errs ++;
            aud_buf_tail = aud_buf_head = 0;                            // Drop all buffers
          }

          if (aud_buf_data [aud_buf_tail] == null)                      // If audio buffer not yet allocated...
            aud_buf_data [aud_buf_tail] = new byte [pcm_size_max];      // Allocate memory to pcm_size_max. Could use m_hw_size but that prevents live tuning unless re-allocate while running.

          aud_buf = aud_buf_data [aud_buf_tail];

          len = -555;                                                   // Default = error if no m_audiorecorder (shouldn't happen except at shutdown)
          if (m_audiorecorder != null)
            len = m_audiorecorder.read (aud_buf, 0, at_min_size);//m_hw_size);

          if (len <= 0) {
            com_uti.loge ("get error: " + len + "  tail index: " + aud_buf_tail);
            com_uti.ms_sleep (1010);                                    // Wait for errors to clear
          }
          else {
            if (com_uti.device == com_uti.DEV_QCV) {                    // Detect all 0's in audio to kickstart Xperia Z audio (by doing any FM chip function); Why does Z do this ?? !!!!
              for (ctr = 0; ctr < len; ctr ++) {
                if (aud_buf [ctr] != 0)
                  break;
              }
              audio_blank = false;
              if (ctr >= len)
                audio_blank = true;
            }

            if (need_aud_mod)
              aud_mod (len, aud_buf);

            if (com_uti.ena_debug_log && reads_processed % ((2 * read_stats_seconds * m_samplerate * m_channels) / len) == 0)   // Every stats_seconds
              pcm_stat_logs ("Read ", m_channels, len, aud_buf);

            if (aud_buf_tail < 0 || aud_buf_tail > aud_buf_num - 1)     // Protect from ArrayIndexOutOfBoundsException
              aud_buf_tail &= aud_buf_num - 1;

            aud_buf_len [aud_buf_tail] = len;                           // On shutdown: java.lang.ArrayIndexOutOfBoundsException: length=32; index=32

            aud_buf_tail ++;
            if (aud_buf_tail < 0 || aud_buf_tail > aud_buf_num - 1)
              aud_buf_tail &= aud_buf_num - 1;
            reads_processed ++;

            bufs = aud_buf_tail - aud_buf_head;
            //com_uti.loge ("pcm_read_runnable run() bufs: " + bufs + "  tail: " + aud_buf_tail + "  head: " + aud_buf_head);
            if (bufs < 0)
              bufs += aud_buf_num;                                      // Fix underflow
            if (bufs >= min_pcm_write_buffers)                          // If minimum number of buffers is ready... (Currently at least 2)
              if (pcm_write_thread != null && pcm_write_thread_waiting)
                pcm_write_thread.interrupt ();                          // Wake up pcm_write_thread sooner than usual
          }
        }

      }
      catch (Throwable e) {
        com_uti.loge ("pcm_read_runnable run() throwable: " + e);
        e.printStackTrace ();
      }                                                                 // Fall through to terminate if exception

      com_uti.logd ("pcm_read_runnable run() reads_processed: " + reads_processed + "  writes_processed: " + writes_processed + "  buf_errs: " + buf_errs + "  max_bufs: " + max_bufs);

      return;
    }
  };


  private boolean audio_blank = false;
  public  boolean audio_blank_get () {
    return (audio_blank);
  }
  public  boolean audio_blank_set (boolean blank) {
    audio_blank = blank;
    return (audio_blank);
  }

/* C code:
  void aud_mod (int len, signed char * buf) {
    int i = 0;
    signed short * sbuf = (signed short *) buf;
    for (i = 0; i < len / 2; i++) {
      //signed short short_sample = * ((signed short *) & buf [i * 2]);
      //int sample = short_sample;
      sbuf [i] *= 4;//3;
    }
  }
*/

  private boolean need_aud_mod = false;
  void aud_mod (int len, byte [] buf) {   // Modify read audio, such as amplification
    if (com_uti.device == com_uti.DEV_GS1) {
        // Amplify by 4
    }
    else if (com_uti.device == com_uti.DEV_GS3) {
/*
      int ctr = 0;
      for (ctr = 0; ctr < len / 4; ctr ++) {    // Swap channels
        byte temp1 = buf [0];
        byte temp2 = buf [1];
        buf [0] = buf [2];
        buf [1] = buf [3];
        buf [2] = temp1;
        buf [3] = temp2;
      }
*/
    }
  }

  public String audio_stereo_set (String new_audio_stereo) {
    if (s2_tx) {
      com_uti.logd ("s2_tx");
      //return;
      //return (0);
      return (new_audio_stereo);
    }
    //if (m_com_api.audio_stereo.equalsIgnoreCase (new_audio_stereo))   // Done if no change
    //  return (-1);
    m_com_api.audio_stereo = new_audio_stereo;                          // Set new audio output
    com_uti.logd ("Set new audio_stereo: " + m_com_api.audio_stereo);
    return (m_com_api.audio_stereo);
  }


    // ? Use stereo_set() for speaker mode ?

//pcm_read_runnable
//pcm_write_runnable
//min_pcm
  private void pcm_audio_start (boolean include_read) {                 // Start input and output   Called only by audio_output_set() (for restart) or audio_start ()
    pcm_write_start ();
    if (include_read)
      pcm_read_start ();

        // If mic selected & not FM & want FM and not mic...
    if (m_aud_src <= 8 && ! com_uti.file_get ("/sdcard/spirit/aud_mic"))
      dai_set (true);                                                   // Establish FM instead of microphone
  }
  private void pcm_audio_pause (boolean include_read) {
    if (m_audiotrack != null)
      m_audiotrack.pause ();
    if (include_read)
      pcm_read_stop ();
    pcm_write_stop ();
    com_uti.logd ("reads_processed: " + reads_processed + " writes_processed: " + writes_processed);
  }
  private void pcm_audio_stop (boolean include_read) {
    pcm_audio_pause (include_read);
    if (m_audiotrack != null)
      m_audiotrack.release ();
    m_audiotrack = null;
    com_uti.logd ("reads_processed: " + reads_processed + " writes_processed: " + writes_processed);
  }

  private void audio_output_off () {                                                    // Called only from audio_pause, after pcm read and write stopped
    if (s2_tx) {                                                        // Do nothing if transmit mode...
      return;
    }
    com_uti.logd ("current api audio_state: " + m_com_api.audio_state + "  current api audio_output: " + m_com_api.audio_output);
    if (m_com_api.audio_output.equalsIgnoreCase ("speaker") && m_hdst_plgd)             // If Speaker switch off and headset plugged in...
      setDeviceConnectionState (DEVICE_OUT_WIRED_HEADSET, DEVICE_STATE_AVAILABLE, "");  // Headset available
    else
      audio_routing_get ();
  }

    // CAN_DUCK
    // Called by svc_svc:onStartCommand() (change from UI/Widget) & svc_svc:audio_state_set() (at start from prefs)
  public String audio_output_set (String new_audio_output) {
    if (s2_tx) {                                                        // Do nothing if transmit mode...
      com_uti.logd ("s2_tx");
      return (new_audio_output);
    }

    com_uti.logd ("current api audio_state: " + m_com_api.audio_state + "  current api audio_output: " + m_com_api.audio_output + "  new_audio_output: " + new_audio_output);
    if (new_audio_output.equalsIgnoreCase ("toggle")) {                 // If toggle...
      if (m_com_api.audio_output.equalsIgnoreCase ("speaker"))          // If Speaker...
        new_audio_output = "Headset";                                   // Switch to Headset...
      else
        new_audio_output = "Speaker";                                   // Or switch to Speaker...
    }

    boolean need_restart = false;
    if (m_com_api.audio_state.equalsIgnoreCase ("start") && (com_uti.device == com_uti.DEV_GS1 || com_uti.device == com_uti.DEV_GS2 || com_uti.device == com_uti.DEV_GS3))
      //pcm_audio_stop (true);                                            // If audio started and device needs restart... (GS3 only needs for OmniROM, but make universal)
      need_restart = true;

    if (new_audio_output.equalsIgnoreCase ("speaker")) {                                        // If -> Speaker...
      if (need_restart)
        pcm_audio_stop (true);
      setDeviceConnectionState (DEVICE_OUT_WIRED_HEADSET, DEVICE_STATE_UNAVAILABLE, "");        // Headset unavailable
      if (com_uti.device == com_uti.DEV_GS1)
        setDeviceConnectionState (DEVICE_OUT_WIRED_HEADPHONE, DEVICE_STATE_UNAVAILABLE, "");    // "Headphone" also unavailable ?
    }
    else {                                                                                      // If -> Headset...
      if (m_hdst_plgd && m_com_api.audio_output.equalsIgnoreCase ("speaker")) {                 // If headset plugged in and last_out was speaker...
        if (need_restart)
          pcm_audio_stop (true);
        setDeviceConnectionState (DEVICE_OUT_WIRED_HEADSET, DEVICE_STATE_AVAILABLE, "");        // Headset available
        if (com_uti.device == com_uti.DEV_GS1)
          setDeviceConnectionState (DEVICE_OUT_WIRED_HEADPHONE, DEVICE_STATE_AVAILABLE, "");    // "Headphone" also available ?
      }
      else {
        need_restart = false;
        audio_routing_get ();
      }
    }

    m_com_api.audio_output = new_audio_output;                          // Set new audio output

    if (need_restart)
      pcm_audio_start (true);                                           // If audio started and device needs restart... (GS3 only needs for OmniROM, but make universal)

        // If audio started & headset mic selected & want FM and not mic...
    else if (m_com_api.audio_state.equalsIgnoreCase ("start") && m_aud_src <= 8 && ! com_uti.file_get ("/sdcard/spirit/aud_mic"))
      dai_set (true);                                                   // Re-establish FM instead of microphone


    com_uti.logd ("Done new audio_output: " + m_com_api.audio_output);
    return (m_com_api.audio_output);
  }

// http://osxr.org/android/source/hardware/libhardware_legacy/include/hardware_legacy/AudioSystemLegacy.h
  //enum audio_devices {         // output devices
  private static final int DEVICE_OUT_EARPIECE          = 0x1;
  private static final int DEVICE_OUT_SPEAKER           = 0x2;
  private static final int DEVICE_OUT_WIRED_HEADSET     = 0x4;
  private static final int DEVICE_OUT_WIRED_HEADPHONE   = 0x8;

  private static final int DEVICE_STATE_UNAVAILABLE   = 0;
  private static final int DEVICE_STATE_AVAILABLE     = 1;

  private int setDeviceConnectionState (final int device, final int state, final String address) {   // Called by audio_output_off () & audio_output_set ()
    int ret = -3;
    com_uti.logd ("device: " + device + "  state: " + state + "  address: '" + address + "'");
    audio_routing_get ();
    try {
      Class<?> audioSystem = Class.forName ("android.media.AudioSystem");        // Use reflection
      Method setDeviceConnectionState = audioSystem.getMethod ("setDeviceConnectionState", int.class, int.class, String.class);
      ret = (Integer) setDeviceConnectionState.invoke (audioSystem, device, state, address);
      com_uti.logd ("ret: " + ret);
    }
    catch (Exception e) {
      com_uti.loge ("exception: " + e);
    }
    audio_routing_get ();
    return (ret);
  }


  public int audio_routing_get () {
    int ctr = 0, bits = 0, ret = 0, ret_bits = 0;
    String bin_out = "";
    for (ctr = 31; ctr >= 0; ctr --) {
      bits = 1 << ctr;
      ret = getDeviceConnectionState (bits, "");
      //com_uti.logd ("getDeviceConnectionState for " + bits + "  is: " + ret);
      bin_out += ret;
      if (ctr % 4 == 0)                                                 // Every 4 bits...
        bin_out += " ";                                                 // Add seperator space
      if (ret == 1)
        ret_bits |= bits;
    }
    com_uti.logd ("getDeviceConnectionState: " + bin_out + "  ret_bits: " + ret_bits);
    return (ret_bits);
  }

  private int getDeviceConnectionState (final int device, final String address) {   // Reflector
    int ret = -5;
    try {
      Class<?> audioSystem = Class.forName ("android.media.AudioSystem");
      Method getDeviceConnectionState = audioSystem.getMethod ("getDeviceConnectionState", int.class, String.class);
      ret = (Integer) getDeviceConnectionState.invoke (audioSystem, device, address);
//      com_uti.logd ("getDeviceConnectionState ret: " + ret); 
    }
    catch (Throwable e) {
      com_uti.loge ("throwable: " + e);
      //e.printStackTrace ();
    }
    return (ret);
  }


    // Start:

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
*/



  private boolean audiorecorder_read_stop () {
    m_audiorecord_reading = false;
    if (m_audiorecorder == null)
      return (false);
    m_audiorecorder.stop ();
    m_audiorecorder.release ();
    m_audiorecorder = null;
    return (true);
  }

  private boolean audiorecorder_read_start () {
                                                                        // Capture mono data at 16kHz
    int channelConfiguration = AudioFormat.CHANNEL_IN_STEREO;//CHANNEL_CONFIGURATION_STEREO;//MONO;
    if (m_channels == 1)
      channelConfiguration = AudioFormat.CHANNEL_IN_MONO;//CHANNEL_CONFIGURATION_MONO;

    int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;

    try {
      m_audiorecorder = audio_record_get ();

      if (m_audiorecorder != null) {
        m_audiorecorder.startRecording ();    //java.lang.IllegalStateException: startRecording() called on an uninitialized AudioRecord.
        com_uti.logd ("getChannelConfiguration: " + m_audiorecorder.getChannelConfiguration () + "   getChannelCount: " +  m_audiorecorder.getChannelCount ());

        m_audiorecord_reading = true;
      }
      else {
        com_uti.loge ("m_audiorecorder == null !!");
        m_audiorecord_reading = false;
      }
    }
    catch (Exception e) {//FileNotFoundException e) {
      e.printStackTrace();
      return (false);
    }

    return (true);
  }


  private AudioRecord audio_record_get () {

    int[] m_all_rate = new int []   {44100/*, 48000, 22050, 11025, 8000*/};

    m_all_rate [0] = m_samplerate;

      // first entry "11" is replaced by aud_src if set, or 5 CAMCORDER
    int [] m_mic_srcs = new int []   {11,  1, 0, 5, 10, 9, 6, 7};   // Microphone sources       MediaRecorder.AudioSource.DEFAULT ++
    int [] m_dir_srcs = new int []   {11, 10, 9, 5, 1,  0, 5, 6, 7}; // Direct sources           MediaRecorder.AudioSource.DEFAULT ++        !! Stock Xperia Z worked w/ 9/10, CM11 + Lollipop use 5
    int [] m_srcs = null;

    int default_src = MediaRecorder.AudioSource.MIC;//MediaRecorder.AudioSource.CAMCORDER;  // 5
    if (com_uti.device == com_uti.DEV_LG2)
      default_src = MediaRecorder.AudioSource.MIC;  // 1
    else if (com_uti.device == com_uti.DEV_QCV && (com_uti.m_device.startsWith ("M8") || com_uti.m_device.startsWith ("HTC_M8")))
      default_src = MediaRecorder.AudioSource.CAMCORDER;  // 5      This avoids a 10 second delay resuming paused state when wired headset not plugged in.
    else if (com_uti.device == com_uti.DEV_QCV)
      default_src = MediaRecorder.AudioSource.CAMCORDER;    // MotoG and Xperia Z lose audio (input 0's) after an hour or 2. Will thix fix ??

    boolean use_fmr = false;
    m_srcs = m_mic_srcs;
    if (com_uti.m_manufacturer.startsWith ("SONY") ||                   // Doesn't work with Xperia Z w/ AOSP, which ends up selecting 5/Camcorder because 9 and 10 don't work.
        com_uti.motorola_stock_get ()              ||   // com_uti.m_manufacturer.startsWith ("MOTO") ||
//?? Disable to see ??       (com_uti.m_manufacturer.startsWith ("LG") && com_uti.device == com_uti.DEV_QCV) ||      // LG G3
       (com_uti.m_device.startsWith ("SERRANO"))         // GS4 Mini
            ) { 
      use_fmr = true;
      m_srcs = m_dir_srcs;
    }

    int src = 0;
    for (int cnt_src : m_srcs) {                                        // For all sources...
      src = cnt_src;
      if (src == 11) {                                                  // If special first entry...
        //com_uti.loge ("99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 ");
        String filename = "/sdcard/spirit/aud_src";
        if (com_uti.file_get (filename)) {                              // If aud_src file
          byte [] content = com_uti.file_read_16k (filename);
          String cont = com_uti.ba_to_str (content);
          src = com_uti.int_get (cont);
          com_uti.loge ("cont: " + cont + "  src: " + src);
        }
        else {                                                          // Else if NO aud_src file
          if (use_fmr)                                                  // If direct FM
            continue;                                                   // Go to next iteration...
          else
            src = default_src;                                          // Else try default source
        }
      }

      if (src < -1 || src > 12)
        src = default_src;

      for (int rate : m_all_rate) {
      for (short audioFormat : new short[]{AudioFormat.ENCODING_PCM_16BIT}) {
        for (short channelConfig : new short[]{AudioFormat.CHANNEL_IN_STEREO}) {   // AUDIO_CHANNEL_IN_FRONT_BACK ?
          try {
            int bufferSize = AudioRecord.getMinBufferSize (rate, channelConfig, audioFormat);
            com_uti.logd ("src: " + src + "  rate: " + rate + "  audioFormat: " + audioFormat + "  channelConfig: " + channelConfig + "  bufferSize: " + bufferSize);
            //if (bufferSize = AudioRecord.ERROR_BAD_VALUE)
            //continue; / break;
            AudioRecord recorder = new AudioRecord (src, rate, channelConfig, audioFormat, at_min_size);//m_hw_size);//bufferSize);    // at_min_size
            if (recorder.getState() == AudioRecord.STATE_INITIALIZED) { // If works, then done
              m_aud_src = src;
              return (recorder);
            }
          }
          catch (Exception e) {
            com_uti.logd ("Exception: " + e );  // "java.lang.IllegalArgumentException: Invalid audio source."
          }
        }
      }
      }
    }
    return (null);
  }

}
