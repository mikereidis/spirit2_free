
    // General utility functions

package fm.a2d.sf;

import android.bluetooth.BluetoothAdapter;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.StrictMode;
import android.os.Looper;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.content.SharedPreferences;
import android.content.Context;
import android.content.pm.PackageInfo;

import java.net.SocketTimeoutException;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Locale;
import java.util.TimeZone;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileReader;
import java.io.OutputStream;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import java.io.FileInputStream;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;


public final class com_uti  {

    // Stats:
  private static int            m_obinits = 0;
  private static int            string_daemon_cmd_num       = 0;
  private static int            daemon_audio_data_get_num   = 0;

    // Basic:
  public  static final String   api_result_id   = "fm.a2d.sf.result.get";
  public  static final String   api_action_id   = "fm.a2d.sf.action.set";
  public  static final int      android_version = android.os.Build.VERSION.SDK_INT;
  private static final int      DEF_BUF         = 512;

    // s2d daemon communications:
  private static int            do_daemon_cmd_sem  = 0;                // Only 1 client at a time
  private static InetAddress    loop                = null;             // Make global to avoid constant re-initialization; reuse
  private static boolean        loop_set            = false;

    // Tuner utilities:
  public static int band_freq_lo =  87500;
  public static int band_freq_hi = 108000;
  public static int band_freq_inc =   100;
  public static int band_freq_odd =     0;

    // Notification ID should be unique on system
  public static final int s2_notif_id = 1963;                           // No relevance except to uniquely identify notification. Spirit1 uses 2112

    // Device support:

  public com_uti () {                                                    // Default constructor
    //com_uti.logd ("m_obinits: " + m_obinits++);   // !! Can't log from internal code, class is not set up yet !!
    final String tag = tag_prefix_get () + "comuti";
    m_obinits ++;
    Log.d (tag, "m_obinits: " + m_obinits);


    Thread.setDefaultUncaughtExceptionHandler (new Thread.UncaughtExceptionHandler () {
      public void uncaughtException (Thread aThread, Throwable aThrowable) {
        //com_uti.loge ("!!!!!!!! Uncaught exception: " + aThrowable);
        Log.e (tag, "!!!!!!!! Uncaught exception: " + aThrowable);
      }
    });
    //com_uti.loge ("done");
    Log.e (tag, "done");
  }


    // Logging External:
  public  static final boolean ena_log_pcm_stat         = true;
  public  static final boolean ena_log_audio_routing_get= true;

    // Logging Internal to Utilities:
  private static       boolean ena_log_daemon_cmd        = false;
  private static       boolean ena_log_daemon_cmd_sem    = false;

    // Android Logging Levels:
  private static final boolean ena_log_verbo = false;
  private static final boolean ena_log_debug = true;
  private static final boolean ena_log_warni = true;
  private static final boolean ena_log_error = true;

  private static String tag_prefix = "";
  private static final int max_log_char = 7;//8;

  private static String tag_prefix_get () {
    try {
      if (tag_prefix != null && ! tag_prefix.equals (""))
        return (tag_prefix);
      String pkg = "fm.a2d.sf";
      tag_prefix = pkg.substring (7);
      if (tag_prefix.equals (""))
        tag_prefix = "s!";
    }
    catch (Throwable e) {
      //com_uti.loge ("Throwable e: " + e);
      Log.e ("s2comuti", "Throwable e: " + e);
      e.printStackTrace ();
      tag_prefix = "E!";
    }
    return (tag_prefix);
  }

  private static void log (int level, String text) {

    final StackTraceElement   stack_trace_el = new Exception ().getStackTrace () [2];
    //final StackTraceElement   stack_trace_el2 = new Exception ().getStackTrace () [3];
    String tag = stack_trace_el.getFileName ().substring (0, max_log_char);

    int idx = tag.indexOf (".");
    if (idx > 0 && idx < max_log_char)
      tag = tag.substring (0, idx);
    int index = 3;
    String tag2 = tag.substring (0, index) + tag.substring (index + 1);
    String method = stack_trace_el.getMethodName ();
    //String method2 = stack_trace_el2.getMethodName ();

    String full_tag = tag_prefix_get () + tag2;
    String full_txt = method + ": " + text;

    if (level == Log.ERROR)
      Log.e (full_tag, full_txt);
    else if (level == Log.WARN)
      Log.w (full_tag, full_txt);
    else if (level == Log.DEBUG)
      Log.d (full_tag, full_txt);
    else if (level == Log.VERBOSE)
      Log.v (full_tag, full_txt);
  }

  public static void logv (String text) {
    if (ena_log_verbo)
      log (Log.VERBOSE, text);
  }
  public static void logd (String text) {
    if (ena_log_debug)
      log (Log.DEBUG, text);
  }
  public static void logw (String text) {
    if (ena_log_warni)
      log (Log.WARN, text);
  }
  public static void loge (String text) {
    if (ena_log_error)
      log (Log.ERROR, text);
  }


    //

  public static String country_get (Context context) {

    String cc = "DE";//"CA";   // Canada
        // getSubscriberId() function Returns the unique subscriber ID, for example, the IMSI for a GSM phone.
        //http://developer.samsung.com/android/technical-docs/How-to-retrieve-the-Device-Unique-ID-from-android-device
    TelephonyManager mngr = (TelephonyManager) context.getSystemService (Context.TELEPHONY_SERVICE);

    //String number = ""
    //if (mngr.getLine1Number () != null)
    //  number = mngr.getLine1Number ();

    String nci = mngr.getNetworkCountryIso ();
    String sci = mngr.getSimCountryIso ();
    if (nci != null && ! nci.equals (""))
      cc = nci;
    else if (sci != null && ! sci.equals (""))
      cc = sci;

    com_uti.logd ("cc: " + cc);
    return (cc);
  }

  public static boolean main_thread_get (String source) {
    boolean ret = (Looper.myLooper () == Looper.getMainLooper ());
    if (ret)
      com_uti.logd ("YES MAIN THREAD source: " + source);
    //else
    //  com_uti.logd ("Not main thread source: " + source);
    return (ret);
  }

  //public static boolean strict_mode = false;
  //StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
  //StrictMode.setThreadPolicy(policy);

  public static void strict_mode_set (boolean strict_mode) {
    if (! strict_mode) {
      StrictMode.setThreadPolicy (new StrictMode.ThreadPolicy.Builder ()
      .permitAll ()
      .build () );

      return;
    }

    StrictMode.setThreadPolicy (new StrictMode.ThreadPolicy.Builder ()
      .detectDiskReads ()
      .detectDiskWrites ()
      .detectNetwork ()
      .detectAll ()                                                     // For all detectable problems
      .penaltyLog ()
      .build () );

    StrictMode.setVmPolicy (new StrictMode.VmPolicy.Builder ()
      .detectLeakedSqlLiteObjects ()
      .detectLeakedClosableObjects ()
      .penaltyLog ()
      //.penaltyDeath ()
      //.penaltyDialog ()   ????
      .build () );
  }


    // The API for Integer.valueOf(String) says the String is interpreted exactly as if it were given to Integer.parseInt(String).
    // However, valueOf(String) returns a new Integer() object whereas parseInt(String) returns a primitive int. 
 
  public static boolean boolean_get (String str) {
    try {
      return (Boolean.parseBoolean (str));
    }
    catch (Exception e) {
    }
    return (false);
  }
  public static long long_get (String val) {
    return (long_get (val, 0));
  }
  public static long long_get (String val, long def) {
    //return ((long) double_get (val, def));                            // !!!! This gives nasty rounding errors !!
    try {
      if (val == null)
        return (def);
      if (val.equals (""))
        return (def);
      long lval = Long.parseLong (val);
      return (lval);
    }
    catch (Exception e) {
      //e.printStackTrace ();
      loge ("Exception e: " + e);
    };
    return (def);
  }
  public static int int_get (String val) {
    return (int_get (val, 0));                                          // Get integer, default = 0
  }
  public static int int_get (String val, int def) {
    //return ((int) double_get (val, def));                             // !!!! This gives nasty rounding errors !!
    try {
      if (val == null)
        return (def);
      if (val.equals (""))
        return (def);
      //int ival = Integer.parseInt (val);
      int ival = (int) Double.parseDouble (val);
      return (ival);
    }
    catch (Exception e) {
      //e.printStackTrace ();
      loge ("Exception e: " + e);
    };
    return (def);
  }
  public static double double_get (String val) {
    return (double_get (val, 0));
  }
  public static double double_get (String val, double def) {
    try {
      if (val == null)
        return (def);
      if (val.equals (""))
        return (def);
      double dval = Float.parseFloat (val);
      return (dval);
    }
    catch (Exception e) {
      //e.printStackTrace ();
      loge ("Exception e: " + e);
    };
    return (def);
  }

    // Time:

  public static long quiet_ms_sleep (long ms) {
    //main_thread_get ("quiet_ms_sleep ms: " + ms);
    try {
      Thread.sleep (ms);                                                // Wait ms milliseconds
      return (ms);
    }
    catch (InterruptedException e) {
      return (0);
    }
  }
  public static long ms_sleep (long ms) {
    main_thread_get ("ms_sleep ms: " + ms);

//    if (ms > 10 && (ms % 101 != 0) && (ms % 11 != 0))
      com_uti.logw ("ms: " + ms);                                       // Warning

    try {
      Thread.sleep (ms);                                                // Wait ms milliseconds
      return (ms);
    }
    catch (InterruptedException e) {
      //Thread.currentThread().interrupt();
      e.printStackTrace ();
      loge ("Exception e: " + e);
      return (0);
    }
  }

  public static long tmr_ms_get () {        // Current timestamp of the most precise timer available on the local system, in nanoseconds. Equivalent to Linux's CLOCK_MONOTONIC. 
    long ms = System.nanoTime () / 1000000; // Should only be used to measure a duration by comparing it against another timestamp on the same device.
                                            // Values returned by this method do not have a defined correspondence to wall clock times; the zero value is typically whenever the device last booted
    //com_uti.logd ("ms: " + ms);           // Changing system time will not affect results.
    return (ms);
  }

  public static long utc_ms_get () {        // Current time in milliseconds since January 1, 1970 00:00:00.0 UTC.
    long ms = System.currentTimeMillis ();  // Always returns UTC times, regardless of the system's time zone. This is often called "Unix time" or "epoch time".
    //com_uti.logd ("ms: " + ms);           // This method shouldn't be used for measuring timeouts or other elapsed time measurements, as changing the system time can affect the results.
    return (ms);
  }

  public static String utc_timestamp_get () {
    Date date = Calendar.getInstance ().getTime ();
    SimpleDateFormat sdf = new SimpleDateFormat ("yyyy-MM-dd_HH-mm-ss.SSSZ", Locale.US);
    sdf.setTimeZone (TimeZone.getTimeZone ("UTC"));
    String date_str = sdf.format (date);
    return (date_str);
  }


    // Network:
/*
  private static android.net.ConnectivityManager connectivity_srvc = null;     // Connectivity Service

  public static boolean log_nag = false;
  public static boolean network_access_get (Context context, String network_access, String url) {

    //com_uti.logd ("network_access: " + network_access + "  url: " + url);

    if (connectivity_srvc == null)
      connectivity_srvc = (android.net.ConnectivityManager) context.getSystemService (Context.CONNECTIVITY_SERVICE);
    if (connectivity_srvc == null) {
      com_uti.loge ("connectivity_srvc == null");
      return (false);
    }

    //boolean bg_data_allowed = connectivity_srvc.getBackgroundDataSetting ();    // Deprecated API level 14. ICS+ bg data depends on several factors, always return true.
    //if (! bg_data_allowed && ! network_access.equals ("cell")) {
    //  com_uti.loge ("bg_data_allowed: " + bg_data_allowed + "  network_access: " + network_access);
    //  return (-3);
    }

    boolean connected_wifi = false;
    boolean connected_cell = false;
    NetworkInfo [] all_net_info = connectivity_srvc.getAllNetworkInfo ();
    for (NetworkInfo ni : all_net_info) {
      if (ni.getTypeName ().equals ("WIFI")) {
        if (ni.isConnected ())
          connected_wifi = true;
        if (log_nag)
          com_uti.logd ("connected_wifi: " + connected_wifi);
      }
      if (ni.getTypeName ().equals ("MOBILE")) {
        if (ni.isConnected ())
          connected_cell = true;
        if (log_nag)
          com_uti.logd ("connected_cell: " + connected_cell);
      }
    }
//getType() provides a finer level of granularity (I believe) and might return TYPE_MOBILE, TYPE_MOBILE_DUN, TYPE_MOBILE_HIPRI and so on.
//I'm not interested in the specifics of what type of 'mobile' connection is available and getTypeName() will simply return 'MOBILE' for all of them. –


//getActiveNetworkInfo() returns a NetworkInfo instance representing the first connected network interface it can find or null if none if the interfaces are connected.
//Checking if this returns null should be enough to tell if an internet connection is available.

//.isAvailable() && networkInfo.isConnected();
//getActiveNetworkInfo() can return null if there is no active network e.g. Airplane Mode 

    NetworkInfo net_info = connectivity_srvc.getActiveNetworkInfo ();   // ICS+ bg data unavailable: getActiveNetworkInfo() will appear disconnected.
    if (net_info == null) {
      //com_uti.loge ("net_info == null  network_access: " + network_access);
      return (false);
    }
    boolean is_connected = net_info.isConnected ();     // ! .isConnectedOrConnecting()
    if ( ! is_connected && ! network_access.equals ("cell")) {   // If not "connected" AND if cell activity disallowed (assume we can bypass ?)
      //com_uti.loge ("is_connected: " + is_connected + "  network_access: " + network_access);
      return (false);
    }
    return (true);
  }
*/

  public static boolean file_delete (final String filename) {
    main_thread_get ("file_delete filename: " + filename);
    java.io.File f = null;
    boolean ret = false;
    try {
      f = new File (filename);
      f.delete ();
      ret = true;
    }
    catch (Throwable e) {
      com_uti.logd ("Throwable e: " + e);
      e.printStackTrace();
    }
    com_uti.logd ("ret: " + ret);
    return (ret);
  }

  public static boolean file_create (final String filename) {
    main_thread_get ("file_create filename: " + filename);
    java.io.File f = null;
    boolean ret = false;
    try {
      f = new File (filename);
      ret = f.createNewFile ();
      com_uti.logd ("ret: " + ret);
    }
    catch (Throwable e) {
      com_uti.logd ("Throwable e: " + e);
      e.printStackTrace();
    }
    return (ret);
  }

  public static void perms_all (final String filename) {
    main_thread_get ("perms_all filename: " + filename);
    try {
      final File file = new File (filename);
      boolean readable = file.setReadable    (true, false);  // r--r--r--
      boolean writable = file.setWritable    (true, false);  // -r--w--w-
      boolean execable = file.setExecutable  (true, false);  // --x--x--x
      if (readable && writable && execable)
        logd ("filename readable && writable && execable");
      else
        loge ("filename readable: " + readable + "  writable: " + writable + "  execable: " + execable);
    }
    catch (Throwable e) {
      //e.printStackTrace ();
      loge ("Throwable e: " + e);
    }
  }

  public static String res_file_create (Context context, int id, String filename, boolean exe) {     // When daemon running !!!! java.io.FileNotFoundException: /data/data/com.WHATEVER.fm/files/sprtd (Text file busy)
    main_thread_get ("res_file_create filename: " + filename);

    if (context == null)
      return ("");

    String full_filename = context.getFilesDir () + "/" + filename;
    try {
      InputStream ins = context.getResources().openRawResource (id);          // Open raw resource file as input
      int size = ins.available ();                                      // Get input file size (actually bytes that can be read without indefinite wait)

      if (size > 0 && file_size_get (full_filename) == size) {          // If file already exists and size is unchanged... (assumes size will change on update !!)
        logd ("file exists size unchanged");                            // !! Have to deal with updates !! Could check filesize, assuming filesize always changes.
                                                                        // Could use indicator file w/ version in file name... SSD running problem for update ??
                                                                            // Hypothetically, permissions may not be set for ssd due to sh failure

//!! Disable to re-write all non-EXE w/ same permissions and all EXE w/ permissions 755 !!!!        return (full_filename);                                         // Done

      }

      byte [] buffer = new byte [size];                                 // Allocate a buffer
      ins.read (buffer);                                                // Read entire file into buffer. (Largest file is s.wav = 480,044 bytes)
      ins.close ();                                                     // Close input file

      FileOutputStream fos = context.openFileOutput (filename, Context.MODE_PRIVATE); // | MODE_WORLD_WRITEABLE      // NullPointerException here unless permissions 755
                                                                        // Create/open output file for writing
      fos.write (buffer);                                               // Copy input to output file
      fos.close ();                                                     // Close output file

      //com_uti.sys_run ("chmod 755 " + full_filename + " 1>/dev/null 2>/dev/null" , false);              // Set execute permission; otherwise rw-rw----
      perms_all (full_filename);

    }
    catch (Exception e) {
      //e.printStackTrace ();
      loge ("Exception e: " + e);
      return (null);
    }

    return (full_filename);
  }

/*
  public static int cached_sys_run (String cmd) {                              // Additive single string w/ commit version

    if (sys_cmd.equals (""))
      sys_cmd += (cmd);
    else
      sys_cmd += (";" + cmd);

    sys_commit (false);  // Commit every command now, due to GS3/Note problems
    return (0);
  }
  public static int sys_commit (boolean su) {
    String [] cmds = {sys_cmd};
    com_uti.sys_run (cmds, su);
    sys_cmd = "";
    return (0);
  }
*/

/*
  private static void sh_run (String cmd) {//, String params) {
    logd ("cmd: " + cmd);
    Process process = null;
    try {
      process = new ProcessBuilder ()
        .command ("/system/xbin/su", "WAS_chmod 667 /dev/fmradio")//cmd)
        .redirectErrorStream (true)
        .start ();
      logd ("process: " + process);
      InputStream in = process.getInputStream ();
      OutputStream out = process.getOutputStream ();
      logd ("in: " + in + "  out: " + out);
      logd ("stream_read: " + stream_read (in));
    }
    catch (Exception e) {//IOException e) {
      //return;
    }    
    finally {
      logd ("before process.destroy ()");
      process.destroy ();   // Block of code that is always executed when the try block is exited, no matter how the try block is exited.
    }
    logd ("done");
  }

  private static String stream_read (InputStream in) {
    ByteArrayOutputStream bo = new ByteArrayOutputStream ();
    try {

      //bo = new ByteArrayOutputStream ();
      int i = in.read ();
      while (i != -1) {
        bo.write (i);
        i = in.read ();
      }
      return bo.toString ();


    //StringBuilder sb = new StringBuilder();  
    //BufferedReader r = new BufferedReader(new InputStreamReader(is),1000);  
    //for (String line = r.readLine(); line != null; line =r.readLine()){  
    //    sb.append(line);  
    //}  
    //is.close();  
    //return sb.toString();

    }
    catch (Exception e) {//IOException e) {
      loge ("Exception: " + e);
      //return "";// + in;
      return bo.toString ();
    }    
  }

*/

//    <acdb_ids>
//        <device name="SND_DEVICE_IN_CAMCORDER_MIC" acdb_id="0" />

  //FileOutputStream fos = null;
  private static boolean fixed_once = false;
  private static void line_fix (Context context, String line) {
    com_uti.logd ("line: " + line);
    try {
      bw.write (line);
      bw.newLine ();
      if (! fixed_once && line.contains ("acdb_ids")) {
        fixed_once = true;
        String add_line = "        <device name=\"SND_DEVICE_IN_CAMCORDER_MIC\" acdb_id=\"34\" />";
        com_uti.logd ("add_line: " + add_line);
        bw.write (add_line);
        bw.newLine ();
      }
    }
    catch (Throwable e) {
      com_uti.loge ("exception: " + e);
      e.printStackTrace ();
      return;
    }
  }


  private static void create_file_fix (Context context) {
    com_uti.logd ("");
    try {
      bw.write ("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
      bw.write ("<audio_platform_info>\n");
      bw.write ("    <acdb_ids>\n");
      bw.write ("        <device name=\"SND_DEVICE_IN_CAMCORDER_MIC\" acdb_id=\"34\" />\n");
      bw.write ("    </acdb_ids>\n");
      bw.write ("</audio_platform_info>\n");
      com_uti.logd ("done");
    }
    catch (Throwable e) {
      com_uti.loge ("exception: " + e);
      e.printStackTrace ();
      return;
    }
  }

  public static boolean platform_file_entirely_ours () {
    return (com_uti.file_size_get (com_uti.platform_file) == 187);      // Hard coded !!!!!!!!!!!!!!
  }

  public static void acdbfix_remove (Context context) {
    com_uti.logd ("");
    String cmd = "";
    cmd += ("mount -o remount,rw /system ; ");
    if (! com_uti.file_get (platform_orig)) {
      com_uti.loge ("Original file does not exist platform_orig: " + platform_orig);
      if (com_uti.platform_file_entirely_ours ()) {
        //boolean bret = com_uti.file_delete (platform_file);
        //com_uti.loge ("!!!!!!!!!! Our file remove bret: : " + bret);
        cmd += ("rm " + platform_file + " ; ");
        cmd += ("mount -o remount,ro /system ; ");
        //cmd += ("killall -hup mediaserver ; ");
        cmd += ("killall mediaserver ; ");
        com_uti.sys_run (cmd, true);
        com_uti.logd ("Done commands");
      }
      return;
    }
    cmd += ("mv " + platform_orig + " " + platform_file + " ; ");
    cmd += ("chmod 644 " + platform_file + " ; ");
    cmd += ("mount -o remount,ro /system ; ");
    //cmd += ("killall -hup mediaserver ; ");
    cmd += ("killall mediaserver ; ");
    com_uti.sys_run (cmd, true);
    com_uti.logd ("Done commands");
  }

  private static String platform_copy = "/data/data/fm.a2d.sf/files/audio_platform_info.xml";
  private static String platform_file = "/system/etc/audio_platform_info.xml";
  public  static String platform_orig = "/system/etc/audio_platform_info.xml.orig";
  private static java.io.BufferedWriter bw = null;
  public static void acdbfix_install (Context context) {
    com_uti.logd ("");
    String line = null;
    try {
      //fos = context.openFileOutput (platform_copy, Context.MODE_WORLD_WRITEABLE | Context.MODE_WORLD_READABLE);     // NullPointerException here unless permissions 755
                                                                        // Create/open output file for writing & reading
      bw = new java.io.BufferedWriter (new java.io.FileWriter (platform_copy), 8192);//512);  // Should never need more than 512 bytes per line

      if (! com_uti.file_get (platform_file)) {                         // If there is no audio platform info file...
        create_file_fix (context);                                      // Create it
        bw.close ();                                                    // Close output file
        acdbfix_copy (context);                                         // Copy audio platform info and addon.d script to /system

        com_uti.logd ("done");
        return;
      }

      fixed_once = false;
      java.io.BufferedReader br = new java.io.BufferedReader (new java.io.FileReader (platform_file), 8192);//512);  // Should never need more than 512 bytes per line
      while (true) {
        line = br.readLine ();
        if (line == null) {
          bw.close ();                                                  // Close output file
          acdbfix_copy (context);                                       // Copy audio platform info and addon.d script to /system

          com_uti.logd ("done");
          return;
        }
        else {
          //com_uti.logd ("line: " + line);
          line_fix (context, line);
        }
      }
    }
    catch (Throwable e) {
      com_uti.loge ("exception: " + e);
      e.printStackTrace ();
    }
  }

  private static void acdbfix_copy (Context context) {
    com_uti.logd ("");
    String cmd = "";
    cmd += ("mount -o remount,rw /system ; ");
    if (com_uti.file_get (platform_file) && ! com_uti.file_get (platform_orig)) {   // !! Only copy to orig if orig does not already exist !
      cmd += ("mv " + platform_file + " " + platform_orig + " ; ");
    }
    cmd += ("cp " + platform_copy + " " + platform_file + " ; ");
    cmd += ("chmod 644 " + platform_file + " ; ");

    cmd += ("cp /data/data/fm.a2d.sf/files/99-spirit.sh /system/addon.d/99-spirit.sh ; ");
    cmd += ("chmod 755 /system/addon.d/99-spirit.sh ; ");

    cmd += ("mount -o remount,ro /system ; ");
    //cmd += ("killall -hup mediaserver ; ");
    cmd += ("killall mediaserver ; ");
    com_uti.sys_run (cmd, true);
    com_uti.logd ("Done commands");
  }

  public static boolean su_installed_get () {
    boolean ret = false;
    if (com_uti.file_get ("/system/bin/su"))
      ret = true;
    else if (com_uti.file_get ("/system/xbin/su"))
      ret = true;
    com_uti.logd ("ret: " + ret);
    return (ret);
  }

  public static int sys_run (String cmd, boolean su) {
    main_thread_get ("sys_run cmd: " + cmd);

    String [] cmds = {("")};
    cmds [0] = cmd;
    return (arr_sys_run (cmds, su));
  }

  private static int arr_sys_run (String [] cmds, boolean su) {         // !! Crash if any output to stderr !!
    //logd ("sys_run: " + cmds);

    try {
      Process p;
      if (su)
        p = Runtime.getRuntime ().exec ("su");
      else
        p = Runtime.getRuntime ().exec ("sh");
      DataOutputStream os = new DataOutputStream (p.getOutputStream ());
      for (String line : cmds) {
        logd ("su: " + su + "  line: " + line);
        os.writeBytes (line + "\n");
      }           
      os.writeBytes ("exit\n");  
      os.flush ();

      int exit_val = p.waitFor ();                                      // This could hang forever ?
      if (exit_val != 0)
        com_uti.logw ("cmds [0]: " + cmds [0] + "  exit_val: " + exit_val);
      else
        com_uti.logd ("cmds [0]: " + cmds [0] + "  exit_val: " + exit_val);

      //os.flush ();
      return (exit_val);
    }
    catch (Exception e) {
      //e.printStackTrace ();
      loge ("Exception e: " + e);
    };
    return (-1);
  }

/*
  public static void rec_sys_run (String [] cmds, boolean su) {             // Array of commands version
    com_uti.logd ("su: " + su + "  cmds: " + cmds);
    main_thread_get ("rec_sys_run");                                                 // Should not run on main thread, but currently does. Does it matter for service in different process ?
    try {
      Process p;
      if (su)
        p = Runtime.getRuntime ().exec ("/system/xbin/su");
        //p = Runtime.getRuntime ().exec ("sh -c /system/xbin/su");
      else
        p = Runtime.getRuntime ().exec ("sh");
      DataOutputStream os = new DataOutputStream (p.getOutputStream ());
      for (String line : cmds) {
        com_uti.logd ("su: " + su + "  line: " + line);
        os.writeBytes (line + "\n");
//        os.writeBytes (line + "\r");
      }           

      //byte ctrl_d = 0x04;
//      os.writeByte (ctrl_d);

      os.writeBytes ("exit\n");
//      //os.writeBytes ("exit\r");

//      os.flush ();
      int exit_val = p.waitFor ();
      if (exit_val != 0)
        com_uti.loge ("exit_val: " + exit_val);
      else
        com_uti.logd ("exit_val: " + exit_val);

      os.flush ();
    }
    catch (Exception e) {
      //e.printStackTrace ();
      loge ("Exception e: " + e);
    };
  }
*/
/* There are four overloaded versions of the exec() command:
    public Process exec(String command);
    public Process exec(String [] cmdArray);
    public Process exec(String command, String [] envp);
    public Process exec(String [] cmdArray, String [] envp);

For each of these methods, a command -- and possibly a set of arguments -- is passed to an operating-system-specific function call.
This subsequently creates an operating-system-specific process (a running program) with a reference to a Process class returned to the Java VM.
The Process class is an abstract class, because a specific subclass of Process exists for each operating system.

You can pass three possible input parameters into these methods:
    A single string that represents both the program to execute and any arguments to that program
    An array of strings that separate the program from its arguments
    An array of environment variables

Pass in the environment variables in the form name=value. If you use the version of exec() with a single string for both the program and its arguments,
note that the string is parsed using white space as the delimiter via the StringTokenizer class.

The first pitfall relating to Runtime.exec() is the IllegalThreadStateException. The prevalent first test of an API is to code its most obvious methods.
For example, to execute a process that is external to the Java VM, we use the exec() method. To see the value that the external process returns,
we use the exitValue() method on the Process class. In our first example, we will attempt to execute the Java compiler (javac.exe): 

        try {            
          Runtime rt = Runtime.getRuntime ();
          Process proc = rt.exec ("javac");
          int exitVal = proc.exitValue ();
          logd ("Process exitValue: " + exitVal);
        }
        catch (Throwable t) {
          t.printStackTrace();
        }

If an external process has not yet completed, the exitValue() method will throw an IllegalThreadStateException

            Runtime rt = Runtime.getRuntime ();
            Process proc = rt.exec ("javac");
            int exitVal = proc.waitFor ();


Why Runtime.exec() hangs:  Because some native platforms only provide limited buffer size for standard input and output streams,
failure to promptly write the input stream or read the output stream of the subprocess may cause the subprocess to block, and even deadlock.

*/


  public static long file_size_get (String filename) {
    main_thread_get ("file_size_get filename: " + filename);
    File ppFile = null;
    long ret = -1;
    try {
      ppFile = new File (filename);
      if (ppFile.exists ())
        ret = ppFile.length ();
    }
    catch (Exception e) {
      //e.printStackTrace ();
    } 
    logd ("ret: " + ret + "  \'" + filename + "\'");
    return (ret);
  }

  public static boolean file_get (String filename, boolean log) {
    //main_thread_get ("file_get filename: " + filename);
    File ppFile = null;
    boolean exists = false;    
    try {
      ppFile = new File (filename);
      if (ppFile.exists ())
        exists = true;
    }
    catch (Exception e) {
      //e.printStackTrace ();
      loge ("Exception: " + e );
      exists = false;                                                   // Exception means no file or no permission for file
    } 
    if (log)
      logd ("exists: " + exists + "  \'" + filename + "\'");
    return (exists);
  }
  public static boolean quiet_file_get (String filename) {
    return (file_get (filename, false));
  }
  public static boolean file_get (String filename) {
    return (file_get (filename, true));
  }

  public static boolean access_get (String filename, boolean read, boolean write, boolean execute) {
    main_thread_get ("access_get filename: " + filename);
    File ppFile = null;
    boolean exists = false, access = false;
    try {
      ppFile = new File (filename);
      if (ppFile.exists ()) {
        exists = true;
        access = true;
        if (read & ! ppFile.canRead ())
          access = false;
        if (write & ! ppFile.canWrite ())
          access = false;
        if (execute & ! ppFile.canExecute ())
          access = false;
      }
    }
    catch (Exception e) {
      loge ("exception: " + e + "  filename: " + filename);
      e.printStackTrace ();
      exists = false;                                                   // Exception = no file (always ?)
    } 
    logd ("filename: \'" + filename + "\'  exists: " + exists + "  access: " + access);
    return (exists & access);
  }


    // Devices:

/* Unused at this time:
  public static boolean stock_sam_get () {
    //return (file_get ("/system/framework/twframework.jar"));// && file_get ("/system/app/FmRadio.apk") && file_get ("/system/lib/libfmradio_jni.so"));
    return (file_get ("/system/framework/twframework.jar") && file_get ("/system/lib/libfmradio_jni.so"));  // Oct 28, REM-ICS JB 3.0.1 has twframework.jar
  }

  public static boolean stock_htc_get () {
    return (om7_get () && (file_get ("/system/framework/com.htc.fusion.fx.jar") || file_get ("/system/framework/com.htc.lockscreen.fusion.jar"))    // Special case for HTC One
        || file_get ("/system/framework/com.htc.framework.jar") || file_get ("/system/framework/framework-htc-res.apk")
        || file_get ("/system/framework/HTCDev.jar") || file_get ("/system/framework/HTCExtension.jar"));
  }

  public static boolean gpe_om7_get () {
    return (htc_get () && m7_get () && ! stock_htc_get () && file_get ("/system/framework/htcirlibs.jar"));
  }

  public static boolean old_qcv_get () {
    return (com_uti.sys_prop_device.startsWith ("EVITA") || com_uti.sys_prop_device.startsWith ("VILLE") || com_uti.sys_prop_device.startsWith ("JEWEL"));// || com_uti.sys_prop_device.startsWith ("M7C"));
  }
*/

    // Manuf:
  public static boolean sony_get () {
    return (com_uti.sys_prop_manuf.startsWith ("SONY"));
  }
  public static boolean htc_get () {
    return (file_get ("/sys/class/htc_accessory/fm/flag") || sys_prop_manuf.startsWith ("HTC"));// || stock_htc_get ());
  }

    // Stock Manuf:
  public static boolean stock_mot_get () {
    return (file_get ("/system/framework/com.motorola.blur.library.app.service.jar") || file_get ("/system/framework/com.motorola.frameworks.core.addon.jar"));
  }
  public static boolean stock_lg2_get () {
    return (com_uti.file_get ("/system/framework/com.lge.systemservice.core.jar") || com_uti.file_get ("/system/framework/com.lge.core.jar") || com_uti.file_get ("/system/framework/com.lge.frameworks.jar"));
  }

    // Device:
  public static boolean om8_get () {
    return (com_uti.sys_prop_device.startsWith ("M8") || com_uti.sys_prop_device.startsWith ("HTC_M8") || sys_prop_device.startsWith ("M7C"));   // M7C is HTC One dual sim 802w/802d/802t);
  }
  public static boolean om7_get () {                                    // For One M7 and One Mini, which has same FM
    return (sys_prop_device.startsWith ("M7") || (sys_prop_device.startsWith ("M4") && htc_get ()) || sys_prop_device.startsWith ("HTC_M4"));
  }
  private static boolean gs3_no2_get () {
    if (sys_prop_device.startsWith ("T03G")        ||                          // Galaxy Note2 3G
        sys_prop_device.startsWith ("GT-N71")      ||
        sys_prop_device.startsWith ("GALAXYN71")   ||

        sys_prop_device.startsWith ("M0")          ||                          // Galaxy S3
        sys_prop_device.startsWith ("GALAXYS3")    ||
        sys_prop_device.startsWith ("I93")         ||                          // OmniROM
        sys_prop_device.startsWith ("GT-I93")    )  {
      return (true);
    }
    return (false);
  }
  public static boolean mog_get () {
    return (sys_prop_device.startsWith ("FALCON") || sys_prop_device.startsWith ("PEREGRINE") || sys_prop_device.startsWith ("TITAN") || sys_prop_device.startsWith ("XT102") || sys_prop_device.startsWith ("XT103") || sys_prop_device.startsWith ("XT104") || sys_prop_device.startsWith ("XT106"));
  }
  public static boolean gs4_mini_get () {
    return (com_uti.sys_prop_device.startsWith ("SERRANO"));
  }

/* OMNIROM:

GT-N7100    T03G
GT-I9300    i9300
GT-N7000    n7000
GT-I9100    i9100

HTCONE      m7ul

GT-I9000    GT-I9000        Mackay. Unofficial ?

Evo 4G LTE  jewel
*/

  private static String  sys_prop_device    = "";
  private static String  sys_prop_board     = "";
  private static String  sys_prop_manuf     = "";

  private static String chass_plug_aud = "UNK";
  private static String chass_plug_tnr = "UNK";

  public static String chass_plug_aud_get (Context context) {

    com_uti.chass_plug_aud = "UNK";
                                                                        // Can't get these from getprop !!
    String arch = System.getProperty ("os.arch");                       // armv7l           armv7l
    if (arch != null)
      com_uti.logd ("os.arch: " + arch);
    else
      com_uti.loge ("!!!! os.arch");

    String name = System.getProperty ("os.name");                       // Linux            Linux
    if (name != null)
      com_uti.logd ("os.name: " + name);
    else
      com_uti.loge ("!!!! os.name");

    String vers = System.getProperty ("os.version");                    // 3.0.31-CM-ge296ffc   3.0.60-g8a65ee9
    if (vers != null)
      com_uti.logd ("os.vers: " + vers);
    else
      com_uti.loge ("!!!! os.version");

    com_uti.logd ("ro.product.board:        " + android.os.Build.BOARD);                                // smdk4x12       aries
    com_uti.logd ("ro.product.brand:        " + android.os.Build.BRAND);                                // samsung        samsung          SEMC
    com_uti.logd ("ro.product.device:       " + android.os.Build.DEVICE);                               // m0             GT-I9000
    com_uti.logd ("ro.hardware:             " + android.os.Build.HARDWARE);                             // smdk4x12       aries            st-ericsson
    com_uti.logd ("ro.build.id:             " + android.os.Build.ID);                                   // LRX22G         JOP40D
    com_uti.logd ("ro.product.manufacturer: " + android.os.Build.MANUFACTURER);                         // samsung        samsung          Sony
    com_uti.logd ("ro.product.model:        " + android.os.Build.MODEL);                                // GT-I9300       GT-I9000
    com_uti.logd ("ro.product.name:         " + android.os.Build.PRODUCT);                              // m0xx           GT-I9000

    if (sys_prop_board.equals (""))
      sys_prop_board = android.os.Build.BOARD.toUpperCase (Locale.getDefault ());

    if (sys_prop_device.equals (""))
      sys_prop_device = android.os.Build.DEVICE.toUpperCase (Locale.getDefault ());

    if (sys_prop_manuf.equals (""))
      sys_prop_manuf = android.os.Build.MANUFACTURER.toUpperCase (Locale.getDefault ());


    if (false)
      com_uti.loge ("Impossible !");

    else if (gs3_no2_get ())
      com_uti.chass_plug_aud = "GS3";

    //else if (com_uti.sony_get () && file_get ("/system/lib/libbt-fmrds.so"))      // DISABLED: Was causing many QCV Sony's to be mis-identified  // ? Z2/Z3 need to be more specific ?
    //  com_uti.chass_plug_aud = "XZ2";

    else if (sys_prop_device.startsWith ("Z3") || sys_prop_device.startsWith ("SGP5") || sys_prop_device.startsWith ("SOT") || sys_prop_device.startsWith ("SO-05") || sys_prop_device.startsWith ("D65") || sys_prop_device.startsWith ("SO-03") || sys_prop_device.startsWith ("CASTOR") || sys_prop_device.startsWith ("SIRIUS") || 
        sys_prop_device.startsWith ("D66") || sys_prop_device.startsWith ("D58") || sys_prop_device.startsWith ("LEO"))
      com_uti.chass_plug_aud = "XZ2";

//C65, C66, C69
                                // NECCASIO G'zOne Commando 4G LTE– C811
    else if (sys_prop_device.startsWith ("C811") || sys_prop_device.startsWith ("EVITA") || sys_prop_device.startsWith ("VILLE") || sys_prop_device.startsWith ("JEWEL")//) // || sys_prop_device.startsWith ("SCORPION")) //_MINI_U"))
          || sys_prop_device.startsWith ("C2") || sys_prop_device.startsWith ("C21") || sys_prop_device.startsWith ("C19") || sys_prop_device.startsWith ("C6") || sys_prop_device.startsWith ("SGP3") || sys_prop_device.startsWith ("LT29")
          || sys_prop_device.startsWith ("LT30") || sys_prop_device.startsWith ("D55") ||
                //  || sys_prop_device.startsWith ("C65") || sys_prop_device.startsWith ("C66") || sys_prop_device.startsWith ("C69")    // Japan Xperia Z1  SO-01f & SOL23 (Australia also SOL23)
             sys_prop_device.startsWith ("YUGA") || sys_prop_device.startsWith ("ODIN") || sys_prop_device.startsWith ("TSUBASA") || sys_prop_device.startsWith ("HAYABUSA") || sys_prop_device.startsWith ("MINT") || sys_prop_device.startsWith ("POLLUX") || sys_prop_device.startsWith ("HONAMI") ||
             sys_prop_device.startsWith ("GEE")   )    // LG Optimus G/G Pro
      com_uti.chass_plug_aud = "QCV";

    else if (sys_prop_device.startsWith ("GT-I9000") || sys_prop_device.startsWith ("GT-I9010") || sys_prop_device.equals ("GALAXYS") || sys_prop_device.startsWith ("GALAXYSM") || sys_prop_device.startsWith ("SC-02B") || sys_prop_device.startsWith ("YP"))
      com_uti.chass_plug_aud = "GS1";

    else if (sys_prop_device.startsWith ("GT-I91") || sys_prop_device.startsWith ("I91") || sys_prop_device.startsWith ("N70") || sys_prop_device.startsWith ("GT-N70") || sys_prop_device.equals ("GALAXYS2") || sys_prop_device.startsWith ("SC-02C") || sys_prop_device.startsWith ("GALAXYN"))
      com_uti.chass_plug_aud = "GS2";

    else if (com_uti.om8_get ())                                        // HTC One M8 2014
      com_uti.chass_plug_aud = "QCV";

    else if (om7_get ())
      com_uti.chass_plug_aud = "OM7";

    else if (mog_get ())
      com_uti.chass_plug_aud = "QCV";

    else if (sys_prop_board.startsWith ("GALBI") || sys_prop_device.startsWith ("G2") || sys_prop_device.startsWith ("LS980") || sys_prop_device.startsWith ("D80") || sys_prop_device.startsWith ("ZEE"))  // "zee" RayGlobe Flex - ls980     Non-Sprint US & VZN VS980 = No FM
      com_uti.chass_plug_aud = "LG2";

    else
      com_uti.chass_plug_aud = "UNK";


    if (com_uti.chass_plug_aud.equals ("UNK")) {
      if (com_uti.file_get ("/dev/radio0") && com_uti.file_get ("/sys/devices/platform/APPS_FM.6")) {   // Qualcomm is always V4L and has this FM /sys directory
        com_uti.chass_plug_aud = "QCV";
      }
      else if (com_uti.file_get ("/dev/radio0") || com_uti.file_get ("/dev/fmradio")) {                 // Samsung GalaxyS class devices always have one of these driver names for Samsung Silicon Labs driver
        if (com_uti.file_get ("/sys/kernel/debug/asoc/smdkc110/wm8994-samsung-codec.4-001a/codec_reg"))
          com_uti.chass_plug_aud = "GS1";
        else if (com_uti.file_get ("/sys/kernel/debug/asoc/U1-YMU823") || com_uti.file_get ("/sys/devices/platform/soc-audio/MC1N2 AIF1") || com_uti.file_get ("/sys/kernel/debug/asoc/U1-YMU823/mc1n2.6-003a"))
          com_uti.chass_plug_aud = "GS2";
        else if (com_uti.file_get ("/sys/kernel/debug/asoc/T0_WM1811/wm8994-codec/codec_reg") || com_uti.file_get ("/sys/kernel/debug/asoc/Midas_WM1811/wm8994-codec/codec_reg") || com_uti.file_get ("/sys/devices/platform/soc-audio/WM8994 AIF1/codec_reg"))
          com_uti.chass_plug_aud = "GS3";
      }
      else {                                                                          // Only remaining alternative is Broadcom
        if (com_uti.file_get ("/dev/ttyHS99") && com_uti.file_get ("/sys/class/g2_rgb_led"))
          com_uti.chass_plug_aud = "LG2";
        else if (com_uti.file_get ("/dev/ttyHS0") && (com_uti.file_get ("/sys/devices/platform/m7_rfkill") || com_uti.file_get ("/sys/devices/platform/mipi_m7.0") || com_uti.file_get ("/sys/module/board_m7_audio")))
          com_uti.chass_plug_aud = "OM7";
      }
      com_uti.loge ("UNK fix -> chass_plug_aud: " + com_uti.chass_plug_aud);
    }
    com_uti.logd ("Auto-Detected chass_plug_aud: " + com_uti.chass_plug_aud);
    //if (com_uti.chass_plug_aud.equals ("UNK"))    DISABLED: because gui_gui needs to know if device unknown.
    //  com_uti.chass_plug_aud = "CUS";

    if (s2_tx_apk () && ! com_uti.chass_plug_aud.equals ("QCV")) {
      com_uti.loge ("Transmit forcing QCV, otherwise chass_plug_aud: " + com_uti.chass_plug_aud);
      com_uti.chass_plug_aud = "QCV";
    }

    com_uti.chass_plug_aud = com_uti.prefs_get (context, "chass_plug_aud", com_uti.chass_plug_aud);
    com_uti.logd ("Final chass_plug_aud: " + com_uti.chass_plug_aud);

    return (com_uti.chass_plug_aud);
  }

  public static String chass_plug_tnr_get (Context context) {

    if (com_uti.chass_plug_aud.equals ("UNK"))                          // If audio (essentially device) not set yet, set it
      chass_plug_aud_get (context);

    com_uti.chass_plug_tnr = "UNK";

    if (com_uti.chass_plug_aud.equals ("UNK"))
      com_uti.chass_plug_tnr = "UNK";
    else if (com_uti.chass_plug_aud.equals ("CUS"))
      com_uti.chass_plug_tnr = "CUS";
    else if (com_uti.chass_plug_aud.equals ("GS3"))
      com_uti.chass_plug_tnr = "SSL";
    else if (com_uti.chass_plug_aud.equals ("GS2"))
      com_uti.chass_plug_tnr = "SSL";
    else if (com_uti.chass_plug_aud.equals ("GS1"))
      com_uti.chass_plug_tnr = "SSL";
    else if (com_uti.chass_plug_aud.equals ("QCV"))
      com_uti.chass_plug_tnr = "QCV";
    else if (com_uti.chass_plug_aud.equals ("OM7"))
      com_uti.chass_plug_tnr = "BCH";
    else if (com_uti.chass_plug_aud.equals ("LG2"))
      com_uti.chass_plug_tnr = "BCH";
    else if (com_uti.chass_plug_aud.equals ("XZ2"))
      com_uti.chass_plug_tnr = "BCH";
    else
      com_uti.chass_plug_tnr = "UNK";

    com_uti.logd ("Auto-Detected chass_plug_tnr: " + com_uti.chass_plug_tnr);
    //if (com_uti.chass_plug_tnr.equals ("UNK"))
    //  com_uti.chass_plug_tnr = "CUS";

    if (s2_tx_apk () && ! com_uti.chass_plug_tnr.equals ("QCV")) {
      com_uti.loge ("Transmit forcing QCV, otherwise chass_plug_tnr: " + com_uti.chass_plug_tnr);
      com_uti.chass_plug_tnr = "QCV";
    }

    com_uti.chass_plug_tnr = com_uti.prefs_get (context, "chass_plug_tnr", com_uti.chass_plug_tnr);
    com_uti.logd ("Final chass_plug_tnr: " + com_uti.chass_plug_tnr);

    return (com_uti.chass_plug_tnr);
  }

  public static String app_version_get (Context act) {                                             // Get versionName (from AndroidManifest.xml)
    String version = "";
    PackageInfo package_info;
    try {
      package_info = act.getPackageManager ().getPackageInfo (act.getPackageName (), 0);
      version = package_info.versionName;
    }
    catch (Exception e) {//NameNotFoundException e) {
      //e.printStackTrace ();
    }
    return (version);
  }



    // Preferences:

  public  static final String   prefs_file = "s2_prefs";
                                                                        // prefs_mode: Match previous releases to avoid loss of settings
  private static final int      prefs_mode = Context.MODE_PRIVATE | Context.MODE_MULTI_PROCESS;

  public static SharedPreferences sp_get (Context context) {
    SharedPreferences m_sp = context.getSharedPreferences (prefs_file, prefs_mode);
    return (m_sp);
  }

  public static String def_set (Context context, String key) {
    String res = com_uti.prefs_get (context, key, "");
    if (res.equals (""))
      com_uti.prefs_set (context, key, "");
    //com_uti.logd ("context: " + context + "  key: " + key + "  res: " + res);
    return (res);
  }

    // Prefs Get:
  public static int prefs_get (Context context, String key, int def_int) {
    String def = "" + def_int;
    String str = prefs_get (context, key, def);
    int getd = int_get (str);
    com_uti.logd ("key: " + key + "  def: " + def + "  str: " + str + "  getd: " + getd);
    return (getd);
  }

  public static String prefs_get (Context context, String key, String def) {
    String res = def;
    try {
      SharedPreferences m_sp = sp_get (context);
      if (m_sp != null) {
        res = m_sp.getString (key, def);                                // java.lang.ClassCastException if wrong type
        if (res.equals (""))
          res = def;
      }
    }
    catch (Exception e) {
    }
    com_uti.logd ("key: " + key + "  def: " + def + "  res: " + res);
    return (res);
  }

  public static double prefs_get (Context context, String key, double def) {
    double resd = def;
    String res = "" + def;
    try {
      SharedPreferences m_sp = sp_get (context);
      if (m_sp != null)
        res = m_sp.getString (key, Double.toString (def));
    }
    catch (Exception e) {
    }
    try {
      resd = Double.parseDouble (res);
    }
    catch (Exception e) {
      resd = def;
    }
    return (resd);
  } 

/*
  public static long prefs_get (Context context, String key, long def) {
    long res = def;
    try {
      SharedPreferences m_sp = sp_get (context);
      if (m_sp != null)
        res = m_sp.getLong (key, def);
    }
    catch (Exception e) {
    }
    return (res);
  }
  public static boolean prefs_get (Context context, String key, boolean def) {
    boolean res = def;
    try {
      SharedPreferences m_sp = sp_get (context);
      if (m_sp != null)
        res = m_sp.getBoolean (key, def);
    }
    catch (Exception e) {
    }
    return (res);
  }
*/
    // Prefs Set:
  public static void prefs_set (Context context, String key, String val) {
    prefs_set (context, prefs_file, key, val);
  }
  public static void prefs_set (Context context, String key, int val_int) {
    String val = "" + val_int;
    prefs_set (context, key, val);
    return;
  }
  public static void prefs_set (Context context, String prefs_file, String key, String val) {
    com_uti.logd ("String: " + key + " = " + val);
    try {
      SharedPreferences.Editor ed = null;
      SharedPreferences m_sp = sp_get (context);
      if (m_sp != null)
        ed = m_sp.edit ();
      if (ed != null) {
        ed.putString (key, val);
        ed.commit ();
      }
    }
    catch (Exception e) {
    }
  }
/*
  public static void prefs_set (Context context, String key, long val) {
    prefs_set (context, prefs_file, key, val);
  }
  public static void prefs_set (Context context, String prefs_file, String key, long val) {
    com_uti.logd ("long: " + key + " = " + val);
    try {
      SharedPreferences.Editor ed = null;
      SharedPreferences m_sp = sp_get (context);
      if (m_sp != null)
        ed = m_sp.edit ();
      if (ed != null) {
        ed.putLong (key, val);
        ed.commit ();
     }
    }
    catch (Exception e) {
    }
  }
  public static void prefs_set (Context context, String key, boolean val) {
    prefs_set (context, prefs_file, key, val);
  }
  public static void prefs_set (Context context, String prefs_file, String key, boolean val) {
    com_uti.logd ("boolean: " + key + " = " + val);
    try {
      SharedPreferences.Editor ed = null;
      SharedPreferences m_sp = sp_get (context);
      if (m_sp != null)
        SharedPreferences.Editor ed = m_sp.edit ();
      if (ed != null) {
        ed.putBoolean (key, val);
        ed.commit ();
      }
    }
    catch (Exception e) {
    }
  }
*/
  
    // Hidden Audiosystem stuff:
/*
    public enum audio_mode {
        MODE_INVALID            (-2),
        MODE_CURRENT            (-1),
        MODE_NORMAL             (0),
        MODE_RINGTONE           (1),
        MODE_IN_CALL            (2),
        MODE_IN_COMMUNICATION   (3),
        NUM_MODES  // not a valid entry, denotes end-of-list
    };

    public enum audio_in_acoustics {
        AGC_ENABLE    (0x0001),
        AGC_DISABLE   (0),
        NS_ENABLE     (0x0002),
        NS_DISABLE    (0),
        TX_IIR_ENABLE (0x0004),
        TX_DISABLE    (0)
    };

    // device connection states used for setDeviceConnectionState()
    public enum device_connection_state {
        DEVICE_STATE_UNAVAILABLE    (0),
        DEVICE_STATE_AVAILABLE      (1),
        NUsys_prop_device_STATES
    };
*/

  public static final int DEVICE_STATE_UNAVAILABLE  = 0;
  public static final int DEVICE_STATE_AVAILABLE    = 1;

   // usage for setForceUse, must match AudioSystem::force_use
  public static final int FOR_COMMUNICATION = 0;
  public static final int FOR_MEDIA = 1;
  public static final int FOR_RECORD = 2;
  public static final int FOR_DOCK = 3;
  public static final int FOR_SYSTEM = 4;
  public static final int FOR_HDMI_SYSTEM_AUDIO = 5;

    // device categories config for setForceUse, must match AudioSystem::forced_config
  public static final int FORCE_NONE = 0;
  public static final int FORCE_SPEAKER = 1;
  public static final int FORCE_HEADPHONES = 2;
  public static final int FORCE_BT_SCO = 3;
  public static final int FORCE_BT_A2DP = 4;
  public static final int FORCE_WIRED_ACCESSORY = 5;
  public static final int FORCE_BT_CAR_DOCK = 6;
  public static final int FORCE_BT_DESK_DOCK = 7;
  public static final int FORCE_ANALOG_DOCK = 8;
  public static final int FORCE_DIGITAL_DOCK = 9;
  public static final int FORCE_NO_BT_A2DP = 10;
  public static final int FORCE_SYSTEM_ENFORCED = 11;
  public static final int FORCE_HDMI_SYSTEM_AUDIO_ENFORCED = 12;

  public static int setForceUse (final int usage, final int config) {
    int ret = -9;
    com_uti.logd ("usage: " + usage + "  config: " + config);

    com_uti.logd ("getForceUse 1: " + getForceUse (usage));

    try {
      //Method setForceUse = AudioManager.class.getMethod ("setForceUse", int.class, int.class);
      Class<?> audioSystem = Class.forName ("android.media.AudioSystem");        // Use reflection
      Method setForceUse = audioSystem.getMethod ("setForceUse", int.class, int.class);
      ret = (Integer) setForceUse.invoke (audioSystem, usage, config);
      com_uti.logd ("ret: " + ret);
    }
    catch (Exception e) {
      com_uti.loge ("exception: " + e);
    }

    com_uti.logd ("getForceUse 2: " + getForceUse (usage));

    return (ret);
  }

  public static int getForceUse (final int usage) {
    int ret = -9;
    com_uti.logd ("usage: " + usage);
    try {
      //Method getForceUse = AudioManager.class.getMethod ("getForceUse", int.class, int.class);
      Class<?> audioSystem = Class.forName ("android.media.AudioSystem");        // Use reflection
      Method getForceUse = audioSystem.getMethod ("getForceUse", int.class);
      ret = (Integer) getForceUse.invoke (audioSystem, usage);
      com_uti.logd ("ret: " + ret);
    }
    catch (Exception e) {
      com_uti.loge ("exception: " + e);
    }
    return (ret);
  }

/* OLD !!:
    public enum audio_devices {
        // output devices
        DEVICE_OUT_EARPIECE             0x1,
        DEVICE_OUT_SPEAKER = 0x2,
        DEVICE_OUT_WIRED_HEADSET = 0x4,
        DEVICE_OUT_WIRED_HEADPHONE = 0x8,
        DEVICE_OUT_BLUETOOTH_SCO = 0x10,
        DEVICE_OUT_BLUETOOTH_SCO_HEADSET = 0x20,
        DEVICE_OUT_BLUETOOTH_SCO_CARKIT = 0x40,
        DEVICE_OUT_BLUETOOTH_A2DP               0x80,
        DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES    0x100,
        DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER = 0x200,
        DEVICE_OUT_AUX_DIGITAL = 0x400,
//#ifdef HAVE_FM_RADIO
        DEVICE_OUT_FM = 0x800,
        DEVICE_OUT_FM_SPEAKER = 0x1000,
        DEVICE_OUT_FM_ALL = (DEVICE_OUT_FM | DEVICE_OUT_FM_SPEAKER),
/*#elif defined(OMAP_ENHANCEMENT)
        DEVICE_OUT_FM_TRANSMIT = 0x800,
        DEVICE_OUT_LOW_POWER = 0x1000,
#endif*/

/*!!!!
        DEVICE_OUT_HDMI = 0x2000,
        DEVICE_OUT_DEFAULT = 0x8000,
        DEVICE_OUT_ALL = (DEVICE_OUT_EARPIECE | DEVICE_OUT_SPEAKER | DEVICE_OUT_WIRED_HEADSET |
//#ifdef HAVE_FM_RADIO
                DEVICE_OUT_WIRED_HEADPHONE | DEVICE_OUT_FM | DEVICE_OUT_FM_SPEAKER | DEVICE_OUT_BLUETOOTH_SCO | DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
/*#else
                DEVICE_OUT_WIRED_HEADPHONE | DEVICE_OUT_BLUETOOTH_SCO | DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
#endif*/

/*!!!!
                DEVICE_OUT_BLUETOOTH_SCO_CARKIT | DEVICE_OUT_BLUETOOTH_A2DP | DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
/*#if defined(OMAP_ENHANCEMENT) && !defined(HAVE_FM_RADIO)
                DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER | DEVICE_OUT_AUX_DIGITAL | DEVICE_OUT_LOW_POWER |
                DEVICE_OUT_FM_TRANSMIT | DEVICE_OUT_DEFAULT),
#else*/

/*!!!!

                DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER | DEVICE_OUT_AUX_DIGITAL | DEVICE_OUT_HDMI | DEVICE_OUT_DEFAULT),
//#endif
        DEVICE_OUT_ALL_A2DP = (DEVICE_OUT_BLUETOOTH_A2DP | DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER),

        // input devices
        DEVICE_IN_COMMUNICATION             0x10000,
        DEVICE_IN_AMBIENT                   0x20000,
        DEVICE_IN_BUILTIN_MIC               0x40000,
        DEVICE_IN_BLUETOOTH_SCO_HEADSET     0x80000,
        DEVICE_IN_WIRED_HEADSET             0x100000,
        DEVICE_IN_AUX_DIGITAL               0x200000,
        DEVICE_IN_VOICE_CALL                0x400000,
        DEVICE_IN_BACK_MIC                  0x800000,
//#ifdef HAVE_FM_RADIO
        DEVICE_IN_FM_RX                     0x1000000,
        DEVICE_IN_FM_RX_A2DP                0x2000000,
/*#endif
#ifdef OMAP_ENHANCEMENT
        DEVICE_IN_FM_ANALOG = 0x1000000,
#endif*/

/*!!!!

        DEVICE_IN_DEFAULT = 0x80000000,

        DEVICE_IN_ALL = (DEVICE_IN_COMMUNICATION | DEVICE_IN_AMBIENT | DEVICE_IN_BUILTIN_MIC |
                DEVICE_IN_BLUETOOTH_SCO_HEADSET | DEVICE_IN_WIRED_HEADSET | DEVICE_IN_AUX_DIGITAL |
//#ifdef HAVE_FM_RADIO
                DEVICE_IN_VOICE_CALL | DEVICE_IN_BACK_MIC | DEVICE_IN_FM_RX | DEVICE_IN_FM_RX_A2DP | DEVICE_IN_DEFAULT)
/*#elif OMAP_ENHANCEMENT
                DEVICE_IN_VOICE_CALL | DEVICE_IN_BACK_MIC  | DEVICE_IN_FM_ANALOG | DEVICE_IN_DEFAULT)
#else
                DEVICE_IN_VOICE_CALL | DEVICE_IN_BACK_MIC | DEVICE_IN_DEFAULT)
#endif*/

//!!!!    };


// AudioSystem.java

    //
    // audio device definitions: must be kept in sync with values in system/core/audio.h
    //

  public static final int DEVICE_NONE = 0x0;
    // reserved bits
  public static final int DEVICE_BIT_IN = 0x80000000;
  public static final int DEVICE_BIT_DEFAULT = 0x40000000;
    // output devices, be sure to update AudioManager.java also
  public static final int DEVICE_OUT_EARPIECE = 0x1;
  public static final int DEVICE_OUT_SPEAKER = 0x2;
  public static final int DEVICE_OUT_WIRED_HEADSET = 0x4;
  public static final int DEVICE_OUT_WIRED_HEADPHONE = 0x8;
  public static final int DEVICE_OUT_BLUETOOTH_SCO = 0x10;
  public static final int DEVICE_OUT_BLUETOOTH_SCO_HEADSET = 0x20;
  public static final int DEVICE_OUT_BLUETOOTH_SCO_CARKIT = 0x40;
  public static final int DEVICE_OUT_BLUETOOTH_A2DP = 0x80;
  public static final int DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES = 0x100;
  public static final int DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER = 0x200;
  public static final int DEVICE_OUT_AUX_DIGITAL = 0x400;
  public static final int DEVICE_OUT_HDMI = DEVICE_OUT_AUX_DIGITAL;
  public static final int DEVICE_OUT_ANLG_DOCK_HEADSET = 0x800;
  public static final int DEVICE_OUT_DGTL_DOCK_HEADSET = 0x1000;
  public static final int DEVICE_OUT_USB_ACCESSORY = 0x2000;
  public static final int DEVICE_OUT_USB_DEVICE = 0x4000;
  public static final int DEVICE_OUT_REMOTE_SUBMIX = 0x8000;
  public static final int DEVICE_OUT_TELEPHONY_TX = 0x10000;
  public static final int DEVICE_OUT_LINE = 0x20000;
  public static final int DEVICE_OUT_HDMI_ARC = 0x40000;
  public static final int DEVICE_OUT_SPDIF = 0x80000;
  public static final int DEVICE_OUT_FM = 0x100000;
  public static final int DEVICE_OUT_AUX_LINE = 0x200000;
  public static final int DEVICE_OUT_FM_TX = 0x1000000;
  public static final int DEVICE_OUT_PROXY = 0x2000000;

  public static final int DEVICE_OUT_DEFAULT = DEVICE_BIT_DEFAULT;

  public static final int DEVICE_OUT_ALL = (DEVICE_OUT_EARPIECE |
                                              DEVICE_OUT_SPEAKER |
                                              DEVICE_OUT_WIRED_HEADSET |
                                              DEVICE_OUT_WIRED_HEADPHONE |
                                              DEVICE_OUT_BLUETOOTH_SCO |
                                              DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                                              DEVICE_OUT_BLUETOOTH_SCO_CARKIT |
                                              DEVICE_OUT_BLUETOOTH_A2DP |
                                              DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                                              DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER |
                                              DEVICE_OUT_HDMI |
                                              DEVICE_OUT_ANLG_DOCK_HEADSET |
                                              DEVICE_OUT_DGTL_DOCK_HEADSET |
                                              DEVICE_OUT_USB_ACCESSORY |
                                              DEVICE_OUT_USB_DEVICE |
                                              DEVICE_OUT_REMOTE_SUBMIX |
                                              DEVICE_OUT_TELEPHONY_TX |
                                              DEVICE_OUT_LINE |
                                              DEVICE_OUT_HDMI_ARC |
                                              DEVICE_OUT_SPDIF |
                                              DEVICE_OUT_FM |
                                              DEVICE_OUT_AUX_LINE |
                                              DEVICE_OUT_FM_TX |
                                              DEVICE_OUT_PROXY |
                                              DEVICE_OUT_DEFAULT);
  public static final int DEVICE_OUT_ALL_A2DP = (DEVICE_OUT_BLUETOOTH_A2DP |
                                                   DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES |
                                                   DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER);
  public static final int DEVICE_OUT_ALL_SCO = (DEVICE_OUT_BLUETOOTH_SCO |
                                                  DEVICE_OUT_BLUETOOTH_SCO_HEADSET |
                                                  DEVICE_OUT_BLUETOOTH_SCO_CARKIT);
  public static final int DEVICE_OUT_ALL_USB = (DEVICE_OUT_USB_ACCESSORY |
                                                  DEVICE_OUT_USB_DEVICE);
  public static final int DEVICE_OUT_ALL_HDMI_SYSTEM_AUDIO = (DEVICE_OUT_AUX_LINE |
                                                                DEVICE_OUT_HDMI_ARC |
                                                                DEVICE_OUT_SPDIF);
  public static final int DEVICE_ALL_HDMI_SYSTEM_AUDIO_AND_SPEAKER =
            (DEVICE_OUT_ALL_HDMI_SYSTEM_AUDIO |
             DEVICE_OUT_SPEAKER);

    // input devices
  public static final int DEVICE_IN_COMMUNICATION = DEVICE_BIT_IN | 0x1;
  public static final int DEVICE_IN_AMBIENT = DEVICE_BIT_IN | 0x2;
  public static final int DEVICE_IN_BUILTIN_MIC = DEVICE_BIT_IN | 0x4;
  public static final int DEVICE_IN_BLUETOOTH_SCO_HEADSET = DEVICE_BIT_IN | 0x8;
  public static final int DEVICE_IN_WIRED_HEADSET = DEVICE_BIT_IN | 0x10;
  public static final int DEVICE_IN_AUX_DIGITAL = DEVICE_BIT_IN | 0x20;
  public static final int DEVICE_IN_HDMI = DEVICE_IN_AUX_DIGITAL;
  public static final int DEVICE_IN_VOICE_CALL = DEVICE_BIT_IN | 0x40;
  public static final int DEVICE_IN_TELEPHONY_RX = DEVICE_IN_VOICE_CALL;
  public static final int DEVICE_IN_BACK_MIC = DEVICE_BIT_IN | 0x80;
  public static final int DEVICE_IN_REMOTE_SUBMIX = DEVICE_BIT_IN | 0x100;
  public static final int DEVICE_IN_ANLG_DOCK_HEADSET = DEVICE_BIT_IN | 0x200;
  public static final int DEVICE_IN_DGTL_DOCK_HEADSET = DEVICE_BIT_IN | 0x400;
  public static final int DEVICE_IN_USB_ACCESSORY = DEVICE_BIT_IN | 0x800;
  public static final int DEVICE_IN_USB_DEVICE = DEVICE_BIT_IN | 0x1000;
  public static final int DEVICE_IN_FM_TUNER = DEVICE_BIT_IN | 0x2000;
  public static final int DEVICE_IN_TV_TUNER = DEVICE_BIT_IN | 0x4000;
  public static final int DEVICE_IN_LINE = DEVICE_BIT_IN | 0x8000;
  public static final int DEVICE_IN_SPDIF = DEVICE_BIT_IN | 0x10000;
  public static final int DEVICE_IN_BLUETOOTH_A2DP = DEVICE_BIT_IN | 0x20000;
  public static final int DEVICE_IN_LOOPBACK = DEVICE_BIT_IN | 0x40000;
  public static final int DEVICE_IN_PROXY = DEVICE_BIT_IN | 0x100000;
  public static final int DEVICE_IN_FM_RX = DEVICE_BIT_IN | 0x200000;
  public static final int DEVICE_IN_FM_RX_A2DP = DEVICE_BIT_IN | 0x400000;
  public static final int DEVICE_IN_DEFAULT = DEVICE_BIT_IN | DEVICE_BIT_DEFAULT;

  public static final int DEVICE_IN_ALL = (DEVICE_IN_COMMUNICATION |
                                             DEVICE_IN_AMBIENT |
                                             DEVICE_IN_BUILTIN_MIC |
                                             DEVICE_IN_BLUETOOTH_SCO_HEADSET |
                                             DEVICE_IN_WIRED_HEADSET |
                                             DEVICE_IN_HDMI |
                                             DEVICE_IN_TELEPHONY_RX |
                                             DEVICE_IN_BACK_MIC |
                                             DEVICE_IN_REMOTE_SUBMIX |
                                             DEVICE_IN_ANLG_DOCK_HEADSET |
                                             DEVICE_IN_DGTL_DOCK_HEADSET |
                                             DEVICE_IN_USB_ACCESSORY |
                                             DEVICE_IN_USB_DEVICE |
                                             DEVICE_IN_FM_TUNER |
                                             DEVICE_IN_TV_TUNER |
                                             DEVICE_IN_LINE |
                                             DEVICE_IN_SPDIF |
                                             DEVICE_IN_BLUETOOTH_A2DP |
                                             DEVICE_IN_LOOPBACK |
                                             DEVICE_IN_PROXY |
                                             DEVICE_IN_FM_RX |
                                             DEVICE_IN_FM_RX_A2DP |
                                             DEVICE_IN_DEFAULT);
  public static final int DEVICE_IN_ALL_SCO = DEVICE_IN_BLUETOOTH_SCO_HEADSET;
  public static final int DEVICE_IN_ALL_USB = (DEVICE_IN_USB_ACCESSORY |
                                                 DEVICE_IN_USB_DEVICE);


  public static int setDeviceConnectionState (final int device, final int state, final String address) {   // Called by audio_output_off () & audio_output_set ()
    int ret = -3;
    com_uti.logd ("device: " + device + "  state: " + state + "  address: '" + address + "'");
    //output_audio_routing_get ();
    //input_audio_routing_get ();
    try {
      Class<?> audioSystem = Class.forName ("android.media.AudioSystem");        // Use reflection
      Method setDeviceConnectionState = audioSystem.getMethod ("setDeviceConnectionState", int.class, int.class, String.class);
      ret = (Integer) setDeviceConnectionState.invoke (audioSystem, device, state, address);
      com_uti.logd ("ret: " + ret);
    }
    catch (Exception e) {
      com_uti.loge ("exception: " + e);
    }
    //output_audio_routing_get ();
    //input_audio_routing_get ();
    return (ret);
  }

  public static int getDeviceConnectionState (final int device, final String address) {   // Reflector
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

  private static final int max_out_bit = 25;//31;       b31 -> b25
  public static int output_audio_routing_get () {
    if (! com_uti.ena_log_audio_routing_get)
      return (0);    

    int ctr = 0, bits = 0, ret = 0, ret_bits = 0;
    String bin_out = "";

    for (ctr = 31; ctr > max_out_bit; ctr --) {                         // Replace high bits with "-"
      bits = 1 << ctr;
      bin_out += "-";
      if (ctr % 4 == 0)                                                 // Every 4 bits...
        bin_out += " ";                                                 // Add seperator space
    }

    for (ctr = max_out_bit; ctr >= 0; ctr --) {
      bits = 1 << ctr;
      ret = 0;
      if (ctr == 23)
        bin_out += "-";
      else
        bin_out += ret = com_uti.getDeviceConnectionState (bits, "");
      if (ctr % 4 == 0)                                                 // Every 4 bits...
        bin_out += " ";                                                 // Add seperator space
      if (ret == 1)
        ret_bits |= bits;
    }
    com_uti.logd ("output getDeviceConnectionState: " + bin_out + "  ret_bits hex: " + com_uti.hex_get (ret_bits) + "    decimal: " + ret_bits);
    return (ret_bits);
  }
/*MOG:0000 0000 0000 0000 0000 0000 0000 0111
b?: speaker
b?: handset
b2: wired headset
...
b19:    #define AUDIO_DEVICE_OUT_FM     0x80000     Audio output to FM Transmit ?       AUDIO_DEVICE_OUT_SPDIF =  0x80000,
b20:    #define AUDIO_DEVICE_OUT_FM_TX 0x100000     Normal FM receive output            AUDIO_DEVICE_OUT_FM    = 0x100000,
b21:    DEVICE_OUT_AUX_LINE = 0x200000;
b22:
-
b24:    DEVICE_OUT_FM_TX = 0x1000000;
b25:    DEVICE_OUT_PROXY = 0x2000000;
-
b30:    AUDIO_DEVICE_BIT_DEFAULT = 0x40000000,
*/

  private static final int max_in_bit = 22;//31;       b31 -> b23
  public static int input_audio_routing_get () {
    if (! com_uti.ena_log_audio_routing_get)
      return (0);

    int ctr = 0, bits = 0, ret = 0, ret_bits = 0;
    String bin_out = "";

    for (ctr = 31; ctr > max_in_bit; ctr --) {                          // Replace high bits with "-"
      bits = 1 << ctr;
      bin_out += "-";
      if (ctr % 4 == 0)                                                 // Every 4 bits...
        bin_out += " ";                                                 // Add seperator space
    }

    for (ctr = max_in_bit; ctr >= 0; ctr --) {
      bits = 1 << ctr;
      ret = 0;
      if (ctr == 19)
        bin_out += "-";
      else
        bin_out += ret = com_uti.getDeviceConnectionState (bits | 0x80000000, "");
      if (ctr % 4 == 0)                                                 // Every 4 bits...
        bin_out += " ";                                                 // Add seperator space
      if (ret == 1)
        ret_bits |= bits;
    }
    com_uti.logd ("  input getDeviceConnectionState: " + bin_out + "  ret_bits hex: " + com_uti.hex_get (ret_bits) + "    decimal: " + ret_bits);
    return (ret_bits);
  }
/*MOG:0000 0000 0110 0000       0000 0001 1001 0100
 b0:    0   AUDIO_DEVICE_IN_COMMUNICATION
 b1:    0   AUDIO_DEVICE_IN_AMBIENT
 b2:    1   AUDIO_DEVICE_IN_BUILTIN_MIC
 b3:    0
 b4:    1   AUDIO_DEVICE_IN_WIRED_HEADSET
 b5:    0
 b6:    0
 b7:    1
 b8:    1
...
b13:    0   AUDIO_DEVICE_IN_FM_TUNER            DEVICE_IN_FM_TUNER = DEVICE_BIT_IN | 0x2000;
b14:    0   AUDIO_DEVICE_IN_TV_TUNER            DEVICE_IN_TV_TUNER = DEVICE_BIT_IN | 0x4000;
b15:    0   #define AUDIO_DEVICE_IN_FM_RX (AUDIO_DEVICE_BIT_IN | 0x8000)            AUDIO_DEVICE_IN_LINE 
b16:    0   #define AUDIO_DEVICE_IN_FM_RX_A2DP (AUDIO_DEVICE_BIT_IN | 0x10000)      AUDIO_DEVICE_IN_SPDIF
b17:
b18:
b19:
-
b21:    1   DEVICE_IN_FM_RX      = DEVICE_BIT_IN | 0x200000;
b22:    1   DEVICE_IN_FM_RX_A2DP = DEVICE_BIT_IN | 0x400000;
-
b30:    AUDIO_DEVICE_BIT_DEFAULT = 0x40000000,
b31:    AUDIO_DEVICE_BIT_IN      = 0x80000000,
*/


    //

  public static byte [] file_read_16k (String filename) {
    main_thread_get ("file_read_16k filename: " + filename);
    byte [] content = new byte [0];
    int bufSize = 16384;
    byte [] content1 = new byte [bufSize];
    try {
      FileInputStream in = new FileInputStream (filename);
      int n = in.read (content1, 0, bufSize);
      in.close ();
      content = new byte [n];
      for (int ctr = 0; ctr < n; ctr ++)
        content [ctr] = content1 [ctr];
    }
    catch (Exception e) {
      com_uti.logd ("Exception: " + e);
      //e.printStackTrace ();
    }
    return (content);
  }

    // Conversions:
/*
  private static byte [] sa_to_ba_hard_way (short [] sa, int size) {           // Byte array to short array; size is in shorts
    byte [] ba = new byte [size * 2];
    for (int i = 0; i < size; i ++) {
      byte lb = (byte) sa [i];
      byte hb = (byte) (sa [i] >> 8);
      ba [(i * 2) + 0] = lb;
      ba [(i * 2) + 1] = hb;
    }
    return (ba);
  }

  private static short [] ba_to_sa_hard_way (byte [] ba, int size) {           // Short array to byte array; size is in shorts
    short [] sa = new short [size];
    for (int i = 0; i < size; i ++) {
      //short s = (short) (ba [(i * 2)] + (ba [(i * 2) + 1] << 8));
      short s = (short) (ba [(i * 2)] << 8);
      sa [i] = s;
    }
    return (sa);
  }
*/

  //short val=(short)( ((hi&0xFF)<<8) | (lo&0xFF) );

  public static short [] ba_to_sa (byte [] ba) {                        // Byte array to short array
    short [] sa = new short [ba.length / 2];
    ByteBuffer.wrap (ba).order (ByteOrder.LITTLE_ENDIAN).asShortBuffer ().get (sa); // to turn bytes to shorts as either big endian or little endian. 
    return (sa);
  }

  public static byte [] sa_to_ba (short [] sa) {                        // Short array to byte array
    byte [] ba = new byte [sa.length * 2];
    ByteBuffer.wrap (ba).order (ByteOrder.LITTLE_ENDIAN).asShortBuffer ().put (sa);
    return (ba);
  }

  public static byte [] str_to_ba (String s) {                          // String to byte array
    //s += "�";     // RDS test ?
    char [] buffer = s.toCharArray ();
    byte [] content = new byte [buffer.length];
    for (int i = 0; i < content.length; i ++) {
      content [i] = (byte) buffer [i];
      //if (content [i] == -3) {            // ??
      //  loge ("s: " + s);//content [i]);
      //  content [i] = '~';
      //}
    }
    return (content);
  }
  /*private static byte [] str_to_ba (String str) {                                     // String to byte array
    byte [] ba = new byte [str.length ()];
    //for (int i = 0; i < str.length (); i++ ) {
    //  ba [i] = Byte.parseByte (str [i]);
    //}
    try {
      ba = str.getBytes ("UTF-8");//16");
    }
    catch (Exception e) {
      loge ("exception");
      //e.printStackTrace ();
    }
    return (ba);
  }*/

  public static String ba_to_str (byte [] b) {                  // Byte array to string
    String s = "";
    try {
      s = new String (b, "UTF-8");
    }
    catch (Exception e) {
      //e.printStackTrace ();
    }
    return (s);
  }




  public static byte [] hexstr_to_ba (String s) {
    int len = s.length ();
    byte [] data = new byte [len / 2];
    for (int i = 0; i < len; i += 2) {
      data [i / 2] = (byte) ((Character.digit (s.charAt (i), 16) << 4) + Character.digit(s.charAt (i + 1), 16));
    }
    return (data);
  }
/*
  public static byte [] hexstr_to_baScooby (String s) {    // For slightly better crypto
    int len = s.length ();
    byte [] data = new byte [len / 2];
    for (int i = 0; i < len; i += 2) {
      data[i / 2] = (byte) ((Character.digit(s.charAt (i), 16) << 4) + Character.digit (s.charAt (i + 1), 16));
      data[i / 2] += 128;   // Add "Scooby Doo" bit 7 flip for each byte
    }
    return (data);
  }
*/

  public static String str_to_hexstr (String s) {
    byte [] ba = str_to_ba (s);
    return (ba_to_hexstr (ba));
  }

  public static String ba_to_hexstr (byte [] ba) {
  String hex = "";

  for (int ctr = 0; ctr < ba.length; ctr ++) {
    hex += hex_get (ba [ctr]);
    //hex += "" + hex_get ((byte) (ba [ctr] >> 4));
  }

  return (hex.toString());
  }


  public static String hex_get (byte b) {
    byte c1 = (byte)((b & 0x00F0) >> 4);
    byte c2 = (byte)((b & 0x000F) >> 0);

    byte [] buffer = new byte [2];

    if (c1 < 10)
      buffer[0] = (byte)(c1 + '0');
    else
      buffer[0] = (byte)(c1 + 'A' - 10);
    if (c2 < 10)
      buffer[1] = (byte)(c2 + '0');
    else
      buffer[1] = (byte)(c2 + 'A' - 10);

    String str = new String (buffer);

    return (str);
  }

  public static String hex_get (short s) {
    byte byte_lo = (byte) (s >> 0 & 0xFF);
    byte byte_hi = (byte) (s >> 8 & 0xFF);
    String res = hex_get (byte_hi) + hex_get (byte_lo);
    return (res);
  }

  public static String hex_get (int i) {
    byte byte_0 = (byte) (i >> 0 & 0xFF);
    byte byte_1 = (byte) (i >> 8 & 0xFF);
    byte byte_2 = (byte) (i >>16 & 0xFF);
    byte byte_3 = (byte) (i >>24 & 0xFF);
    String res = hex_get (byte_3) + hex_get (byte_2) + hex_get (byte_1) + hex_get (byte_0);
    return (res);
  }



  public static File getExternalStorageDirectory () {
    File ret;
    //String path = Environment.getExternalStorageDirectory ().getPath ();
    //String path = Environment.getExternalStorageDirectory ().toString ();
    //ret = Environment.getExternalStorageDirectory ();   // Stopped working for some 4.2+ due to multi-user and 0
    ret = new File ("/sdcard/");
    return (ret);
  }

/*
  private static String temp_dir_get () {
    String tdir = "";

    //boolean have_writable_sd = false;
//    try {
      String es_state = Environment.getExternalStorageState ();
      if (Environment.MEDIA_MOUNTED.equals (es_state)) {
//        have_writable_sd=true;
        com_uti.logd ("SDCard / ExternalStorage available and writeable");

        //API 8 or greater:
        //File m_ext_dir_f= getExternalFilesDir ();
        //String m_ext_dir_s= m_ext_dir_f.getAbsolutePath ();
        //com_uti.logd ("API >=8 ext dir: " +m_ext_dir_s);
      
        //API 7 or lesser:
        File m_ext_dir_f2 = Environment.getExternalStorageDirectory ();
        String m_ext_dir_s= m_ext_dir_f2.getAbsolutePath ();
        com_uti.logd ("API <=7 ext dir: " +m_ext_dir_s);

        //tdir = m_ext_dir_s + "/spirit_cfg/";
        tdir = m_ext_dir_s + "/";                                  // Default temp dir is the sdcard root, if the sdcard is present and writeable
      }
      else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals (es_state)) {
        com_uti.logd ("SDCard / ExternalStorage available but read-only");
      }
      else {
        com_uti.logd ("SDCard / ExternalStorage NOT available");
      }


//      File m_cache_dir = getCacheDir ();                                  // This creates the "cache" directory if it doesn't exist
//      String m_ap= m_cache_dir.getAbsolutePath ();
//      com_uti.logd ("Cache dir: " +m_ap);
//      if (have_writable_sd==false) {                                    // If can't write to SDCard, use internal cache storage
//        //tdir: /sdcard/ OR /data/data/com.WHATEVER.fm/cache/
//        //tdir = m_ap + "/spirit_cfg/";
//        tdir = m_ap + "/";
//      }

//    }
//    catch (IOException e) {
//      //e.printStackTrace ();
//    }
    com_uti.logd ("tdir: " + tdir);

    return (tdir);
  }
*/




    // Datagram command API:

    // Native API:

  static {
    System.loadLibrary ("jut");
  }

    // PCM other:
  private        native int native_priority_set    (int priority);
  private static native int native_daemon_cmd      (int cmd_len, byte [] cmd_buf, int res_len, byte [] res_buf, int net_port, int rx_tmo);


  public static String extras_daemon_set (String key, Bundle extras) {
    String res = "";
    String val = extras.getString (key, "");
    if (! val.equals ("")) {
      res = com_uti.daemon_set (key, val);
      com_uti.logd ("Set key: " + key + "  to val: " + val + "  with res: " + res);
    }
    return (res);
  }

  private static DatagramSocket ds = null;
  private static DatagramPacket dps = null;
  private static int last_tmo = -1;

  private static int java_daemon_cmd (int cmd_len, byte [] cmd_buf, int res_len, byte [] res_buf, int net_port, int rx_tmo) {
    int len = 0;
    String cmd = (com_uti.ba_to_str (cmd_buf)).substring (0, cmd_len);
    try {
      DatagramPacket dps = null;

      if (ds == null /*|| rx_tmo < 100 */|| rx_tmo > 5000) {
        ds = new DatagramSocket (0);//net_port);
        last_tmo = -2;  // Force new timeout setting
      }
      if (last_tmo != rx_tmo) {
        ds.setSoTimeout (rx_tmo);
        last_tmo = rx_tmo;
      }


      if (! loop_set) {
        loop_set = true;
        loop = InetAddress.getByName ("127.0.0.1");
      }

      dps = new DatagramPacket (cmd_buf, cmd_len, loop, net_port);     // Send
      //com_uti.logd ("Before send() cmd: " + cmd + "  res_len: " + res_len);
      ds.send (dps);
      //com_uti.logd ("After  send() cmd: " + cmd);

      ds.receive (dps);                     // java.net.PortUnreachableException        Caused by: libcore.io.ErrnoException: recvfrom failed: ECONNREFUSED (Connection refused)
                                            // java.net.SocketTimeoutException
      //com_uti.logd ("After  receive() cmd: " + cmd);

      byte [] rcv_buf = dps.getData ();
      //com_uti.logd ("After  getData() cmd: " + cmd);

      len = dps.getLength ();

      if (ena_log_daemon_cmd) {
        com_uti.logd ("After  getLength() cmd: " + cmd + "  len: " + len);
        //com_uti.logd ("hexstr res: " + (com_uti.ba_to_hexstr (rcv_buf)).substring (0, len * 2));
        com_uti.logd ("   str res: " + (com_uti.ba_to_str (rcv_buf)).substring (0, len));
      }

      if (len < 0) {    // 0 is valid length, eg empty RT
        com_uti.loge ("cmd: " + cmd + "  len: " + len);
      }
      else {
        int ctr = 0;
        for (ctr = 0; ctr < len; ctr ++)
          res_buf [ctr] = rcv_buf [ctr];
      }

    }
    catch (SocketTimeoutException e) {
      com_uti.loge ("java.net.SocketTimeoutException rx_tmo: " + rx_tmo + "  cmd: " + cmd);
    }
    catch (Throwable e) {
      com_uti.loge ("Exception: " + e + "  rx_tmo: " + rx_tmo + "  cmd: " + cmd);
      e.printStackTrace ();
    };
    return (len);
  }

  private static       String  last_cmd             = "";
  private static final boolean use_java_daemon_cmd  = false;            // false = Use the better JNI implementation


    // Send byte array to dameon and wait for byte array response:

  private static int do_daemon_cmd (int cmd_len, byte [] cmd_buf, int res_len, byte [] res_buf, int rx_tmo) {
    //main_thread_get ("do_daemon_cmd cmd: " + cmd);

    String cmd = (com_uti.ba_to_str (cmd_buf)).substring (0, cmd_len);  // Get command as string for logging purposes

    long start_sem_ms = com_uti.tmr_ms_get ();                          // Start semaphore timer

    if (ena_log_daemon_cmd_sem)
      com_uti.logd ("Before sem++ do_daemon_cmd_sem: " + do_daemon_cmd_sem + "  res_len: " + res_len + "  rx_tmo: " + rx_tmo + "  cmd: \"" + cmd + "\"");

    do_daemon_cmd_sem ++;
    while (do_daemon_cmd_sem != 1) {                                // Get semaphore
      do_daemon_cmd_sem --;
      if (cmd.equals ("tuner_bulk")) {
        com_uti.logd ("Aborting tuner_bulk for do_daemon_cmd_sem: " + do_daemon_cmd_sem + "  res_len: " + res_len + "  rx_tmo: " + rx_tmo + "  cmd: \"" + cmd + "\"  last_cmd: \"" + last_cmd + "\"");
        return (-999);
      }
      com_uti.logw ("Waiting for do_daemon_cmd_sem: " + do_daemon_cmd_sem + "  res_len: " + res_len + "  rx_tmo: " + rx_tmo + "  cmd: \"" + cmd + "\"  last_cmd: \"" + last_cmd + "\"");
      com_uti.quiet_ms_sleep (101);
      do_daemon_cmd_sem ++;
    }

    last_cmd = cmd;                                                     // Update last command to our current command, now that semaphore is obtained

    long sem_ms = com_uti.tmr_ms_get () - start_sem_ms;                 // Log semaphore wait time
    if (sem_ms > 500)
      com_uti.loge ("Semaphore wait time sem_ms: " + sem_ms);
    else if (sem_ms > 100)
      com_uti.logw ("Semaphore wait time sem_ms: " + sem_ms);
    else if (ena_log_daemon_cmd_sem)
      com_uti.logd ("Semaphore wait time sem_ms: " + sem_ms);           // 6-7 ms is common, 11 ms less common

    int len = 0;
    long start_daemon_ms = com_uti.tmr_ms_get ();                       // Start command timer

    if (use_java_daemon_cmd)                                            // Send command to daemon and wait for response, up to rx_tmo milliseconds
      len = java_daemon_cmd (cmd_len, cmd_buf, res_len, res_buf, s2d_port, rx_tmo);
    else
      len = native_daemon_cmd (cmd_len, cmd_buf, res_len, res_buf, s2d_port, rx_tmo);

    long daemon_ms = com_uti.tmr_ms_get () - start_daemon_ms;           // Log command wait time
    if (daemon_ms > 300 && daemon_ms > (rx_tmo * 15) / 16)
      com_uti.loge ("Daemon transaction time daemon_ms: " + daemon_ms);
    else if (ena_log_daemon_cmd)
      com_uti.logd ("Daemon transaction time daemon_ms: " + daemon_ms); // 4-8 ms is common ; Broadcom is often 40+

    do_daemon_cmd_sem --;                                               // Release semaphore
    return (len);
  }


    // Send string to dameon and wait for string response:

  private static String string_daemon_cmd (String cmd, int rx_tmo) {
    int cmd_len = cmd.length ();
    if (ena_log_daemon_cmd)
      com_uti.logd ("cmd_len: " + cmd_len + "  cmd: \"" + cmd + "\"");

    byte [] cmd_buf = com_uti.str_to_ba (cmd);                          // Create command in byte array form from cmd string (Byte array has same length as string)

    int res_len = DEF_BUF;
    byte [] res_buf = new byte [res_len];                               // Allocate byte array for response (DEF_BUF = 512)

                                                                        // Send command to daemon and wait for response, up to rx_tmo milliseconds
    res_len = do_daemon_cmd (cmd_len, cmd_buf, res_len, res_buf, rx_tmo);

    String res = "";                                                    // Avoid showing 999 for RT when result is zero length string "" (actually 1 byte long for zero)      "999";

    if (res_len > 0 && res_len <= DEF_BUF) {                            // If valid response size...
      res = com_uti.ba_to_str (res_buf);                                // Result is string created from byte array response
      res = res.substring (0, res_len);                                 // Remove extra data
      if (ena_log_daemon_cmd)
        com_uti.logd ("res_len: " + res_len + "  res: \"" + res + "\"");
    }
    else if (res_len == 0) {                                            // If empty response ??
      com_uti.loge ("res_len: " + res_len + "  cmd: " + cmd);
      res = "";                                                         // Result is empty string
    }
    else if (/*res_len == -999 &&*/ cmd.equals ("tuner_bulk"))          // Else if tuner_bulk error
      com_uti.logd ("res_len: " + res_len + "  cmd: " + cmd);           // Log debug
    else
      com_uti.loge ("res_len: " + res_len + "  cmd: " + cmd);           // Else if other error, log error

    return (res);                                                       // Return response
  }


    // Get and set:

    // Get for "tuner_state" simply passes the key string "tuner_state" with the "get" implied by no space characters.
    // Since all key strings have no spaces this works well.

    // Set "tuner_rds_ps" to "AB D FG " passes the key and value as "tuner_rds_ps AB D FG ".
    // Value is one string, with optional embedded spaces. Second parameters not explicitly supported, but can be emulated with parsing.

  public static int num_daemon_get = 0;
  public static int num_daemon_set = 0;

  public static String daemon_get (String key) {
    num_daemon_get ++;

    int rx_tmo = 400;//800;//500;//300;                                 // Default timeout
    if (key.equals ("tuner_bulk"))
      rx_tmo = 100;                                                     // Low timeout for "disposable" commands like get tuner_bulk

    if (ena_log_daemon_cmd)
      com_uti.logd ("before string_daemon_cmd key: " + key + "  rx_tmo: " + rx_tmo + "  num_daemon_get: " + num_daemon_get);

    long start_ms = com_uti.tmr_ms_get ();                              // Reset start_ms to current time for first try and after each timeout
    String res = string_daemon_cmd (key, rx_tmo);                       // Send get command to daemon
    long cmd_time_ms = com_uti.tmr_ms_get () - start_ms;                // Calculate time taken for command

    if (ena_log_daemon_cmd)
      com_uti.logd ("after  string_daemon_cmd key: " + key + "  res: " + res + "  cmd_time_ms: " + cmd_time_ms);

    return (res);
  }

  public static String daemon_set (String key, String val) {
    num_daemon_set ++;

    long cmd_time_ms = 0;
    long start_ms = 0;
    String res = "";
    int timeouts = 0;
    int rx_tmo = 800;// 500; // 300;                                    // Default timeout
    if (key.startsWith ("test_99s_tmo"))
      rx_tmo = 99000;
    else if (key.equals ("tuner_seek_state"))
      rx_tmo = 20000;
    else if (key.equals ("tuner_api_state"))
      rx_tmo = 10000;
    else if (key.equals ("tuner_state"))
      rx_tmo =  6000;
    else if (key.equals ("audio_mode"))
      rx_tmo = 10000;
    else if (key.equals ("audio_state"))
      rx_tmo =  6000;

    if (ena_log_daemon_cmd)
      com_uti.logd ("before string_daemon_cmd key: " + key + "  val: " + val + "  rx_tmo: " + rx_tmo + "  num_daemon_set: " + num_daemon_set);

    int max_timeouts = 3;
    if (rx_tmo > 0)//2000)
      max_timeouts = 1;
    while (timeouts < max_timeouts) {
      start_ms = com_uti.tmr_ms_get ();                                 // Reset start_ms to current time for first try and after each timeout
      res = string_daemon_cmd (key + " " + val, rx_tmo);                // Send set command to daemon
      cmd_time_ms = com_uti.tmr_ms_get () - start_ms;                   // Calculate time taken for command

      if (cmd_time_ms <= (rx_tmo * 15) / 16)
        break;
      timeouts ++;
      com_uti.loge ("timeouts: " + timeouts + "  cmd_time_ms: " + cmd_time_ms);
    }

    com_uti.logd ("after  string_daemon_cmd key: " + key + "  val: " + val + "  res: " + res + "  cmd_time_ms: " + cmd_time_ms + "  timeouts: " + timeouts);

    return (res);
  }


/*
  private static void old_thread_priority_set () {

    int tid = -5;
    int priority = 3333;
    try {
      tid = android.os.Process.myTid ();
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
    }
    try {
      priority = android.os.Process.getThreadPriority (tid);
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
    }
    com_uti.logd ("tid: " + tid + "  priority: " + priority);

if (false) {
    try {
      android.os.Process.setThreadPriority (android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);//ANDROID_URGENT_AUDIO);
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
    }
    try {
      priority = android.os.Process.getThreadPriority (tid);
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
    }
    com_uti.logd ("priority: " + priority);


    try {
      android.os.Process.setThreadPriority (-20);//android.os.Process.);//ANDROID_PRIORITY_HIGHEST);
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
    }
    try {
      priority = android.os.Process.getThreadPriority (tid);
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
    }
    com_uti.logd ("priority: " + priority);
}
  }
*/

/*
    try {                                                       // !!!! Thread may already be started
      if (pcm_write_thread.getState () == java.lang.Thread.State.NEW || pcm_write_thread.getState () == java.lang.Thread.State.TERMINATED) {
        com_uti.logd ("thread priority: " + pcm_write_thread.getPriority ());   // Get 5
        //pcm_write_thread.setPriority (Thread.MAX_PRIORITY - 1);   // ?? android.os.Process.THREAD_PRIORITY_AUDIO = -16        THREAD_PRIORITY_URGENT_AUDIO = -19
        //com_uti.logd ("thread priority: " + pcm_write_thread.getPriority ());   // Get 9
        pcm_write_thread.start ();
      }
    }
    catch (Throwable e) {
      com_uti.loge ("Throwable: " + e);
      e.printStackTrace ();
      return (-1);
    }
*/
        // The maximum priority value allowed for a thread. This corresponds to (but does not have the same value as) android.os.Process.THREAD_PRIORITY_URGENT_DISPLAY.
        // Constant Value: 10
/*https://gitorious.org/freebroid/dalvik/source/962f896e1eeb159a6a2ac7a560708939cbb15575:vm/Thread.c
static const int kNiceValues[10] = {
ANDROID_PRIORITY_LOWEST,                // 1 (MIN_PRIORITY)
ANDROID_PRIORITY_BACKGROUND + 6,
ANDROID_PRIORITY_BACKGROUND + 3,
ANDROID_PRIORITY_BACKGROUND,
ANDROID_PRIORITY_NORMAL,                // 5 (NORM_PRIORITY)
ANDROID_PRIORITY_NORMAL - 2,
ANDROID_PRIORITY_NORMAL - 4,
ANDROID_PRIORITY_URGENT_DISPLAY + 3,
ANDROID_PRIORITY_URGENT_DISPLAY + 2,
ANDROID_PRIORITY_URGENT_DISPLAY         // 10 (MAX_PRIORITY)
};*/

/*> setThreadPriority
http://www.netmite.com/android/mydroid/frameworks/base/include/utils/threads.h
    ANDROID_PRIORITY_LOWEST         =  19,
    ANDROID_PRIORITY_BACKGROUND     =  10,      /* use for background tasks 
    ANDROID_PRIORITY_NORMAL         =   0,      /* most threads run at normal priority
    ANDROID_PRIORITY_FOREGROUND     =  -2,      /* threads currently running a UI that the user is interacting with
    ANDROID_PRIORITY_DISPLAY        =  -4,      /* the main UI thread has a slightly more favorable priority
    ANDROID_PRIORITY_URGENT_DISPLAY =  -8,      /* ui service treads might want to run at a urgent display (uncommon)
    ANDROID_PRIORITY_AUDIO          = -16,      /* all normal audio threads
    ANDROID_PRIORITY_URGENT_AUDIO   = -19,      /* service audio threads (uncommon)
    ANDROID_PRIORITY_HIGHEST        = -20,      /* should never be used in practice. regular process might not be allowed to use this level

    ANDROID_PRIORITY_DEFAULT        = ANDROID_PRIORITY_NORMAL,
    ANDROID_PRIORITY_MORE_FAVORABLE = -1,
    ANDROID_PRIORITY_LESS_FAVORABLE = +1,
*/



    // Tuner FM utility functions:

        // Match #defines in utils.c
  private static final int  NET_PORT_S2D =  2102;
  private static final int  NET_PORT_HCI =  2112;

  private static int s2d_port = NET_PORT_S2D;

  public static boolean s2_tx_apk () {
    String tx_name = "fm.a2d.s";
    tx_name += "t";                                                     // Protect name against build sed change
    if (tx_name.equals ("fm.a2d.sf")) {                                 // If equals string which gets modified to "fm.a2d.st" for transmit APK
      s2d_port = NET_PORT_S2D;// + 30;                                  // Transmit APK s2d port
      return (true);
    }
    else {
      s2d_port = NET_PORT_S2D;                                          // Receive APK s2d port (Should SpiritF Free/Open use a different port ?)
      return (false);
    }
  }

  public static int tnru_band_set (String band) {
    com_uti.logd ("band: " + band);
    com_uti.band_freq_lo =  87500;                                      // Actually  87900 for US
    com_uti.band_freq_hi = 108000;                                      // Actually 107900 for US
    com_uti.band_freq_inc = 100;
    com_uti.band_freq_odd = 0;

    if (band.equals ("EU")) {
    }
    else if (band.equals ("US")) {
      com_uti.band_freq_inc = 200;
      com_uti.band_freq_odd = 1;
    }
    else if (band.equals ("UU")) {
      if (com_uti.chass_plug_tnr.equals ("BCH")) {
        com_uti.band_freq_lo =  76000;//65000;
        com_uti.band_freq_hi = 90000;
      }
      else {
        com_uti.band_freq_lo =  76000;
      }
    }

    com_uti.logd ("lo: " + com_uti.band_freq_lo + "  hi: " + com_uti.band_freq_hi + "  inc: " + com_uti.band_freq_inc + "  odd: " + band_freq_odd);
    return (0);
  }

  private static int band_freq_updn_get (int tuner_freq_int, boolean up) {
    int new_freq;
    if (up)
      new_freq = com_uti.tnru_freq_enforce (tuner_freq_int + com_uti.band_freq_inc);
    else
      new_freq = com_uti.tnru_freq_enforce (tuner_freq_int - com_uti.band_freq_inc);
    return (new_freq);
  }

  private static int mhz_get_freq_inc = 50; //5;
  public static String tnru_mhz_get (int freq) {
    freq += mhz_get_freq_inc / 2;                                       // Round up...
    freq = freq / mhz_get_freq_inc;                                     // Nearest 50 KHz or mhz_freq_inc
    freq *= mhz_get_freq_inc;                                           // Back to KHz scale
    double dfreq = (double) freq;
    dfreq = java.lang.Math.rint (dfreq);
    dfreq /= 1000;
    return ("" + dfreq);
  }

  public static int tnru_khz_get (String freq) {
    double dfreq = com_uti.double_get (freq);
    if (dfreq >= 50000 && dfreq <= 499999)                          // If 50,000 - 499,999  KHz...
      dfreq *= 1;                                                   // -> Khz
    else if (dfreq >= 5000 && dfreq <= 49999)                       // If 5,000 - 49,999    x 0.01 MHz...
      dfreq *= 10;                                                  // -> Khz
    else if (dfreq >= 500 && dfreq <= 4999)                         // If 500 - 4,999       x 0.10 MHz...
      dfreq *= 100;                                                 // -> Khz
    else if (dfreq >= 50 && dfreq <= 499)                           // If 50 - 499          MHz...
      dfreq *= 1000;                                                // -> Khz
    return ((int) dfreq);
  }

  public static int tnru_band_new_freq_get (String freq, int tuner_freq_int) {
    int ifreq = 106900;
    if (freq.equals ("Down"))
      ifreq = com_uti.band_freq_updn_get (tuner_freq_int, false);
    else if (freq.equals ("Up"))
      ifreq = com_uti.band_freq_updn_get (tuner_freq_int, true);
    else
      ifreq = com_uti.tnru_khz_get (freq);
    return (ifreq);
  }


  public static int tnru_freq_fix (int freq) {
    return (tnru_freq_fix_inc (freq, com_uti.band_freq_inc));           // mhz_get_freq_inc
  }

  public static int tnru_freq_fix_inc (int freq, int inc) {

    // w/ Odd:  107900-108099 -> 107900     = Add 100, Divide by 200, then multiply by 200, then subtract 100
    // w/ Even: 108000-108199 -> 108000     = Divide by 200, then multiply by 200  (freq_inc)
    // w/ Odd:   87500- 87699 ->  87500     = Add 100, Divide by 200, then multiply by 200, then subtract 100
    // w/ Even:  87600- 87799 ->  87600     = Divide by 200, then multiply by 200  (freq_inc)

    //com_uti.logd ("lo: " + com_uti.band_freq_lo + "  hi: " + com_uti.band_freq_hi + "  inc: " + com_uti.band_freq_inc + "  odd: " + com_uti.band_freq_odd);

    if (com_uti.band_freq_odd != 0) {
      freq += inc / 2;    // 87700
      freq /= inc;
      freq *= inc;        // 87600
      freq -= inc / 2;    // 87500
      //com_uti.logd ("US ODD freq: "  + freq);
    }
    else {
      freq /= inc;
      freq *= inc;
      //com_uti.logd ("EU ALL freq: "  + freq);
    }
    return (freq);
  }

  public static int tnru_freq_enforce (int freq) {
    if (freq < com_uti.band_freq_lo)
      freq = com_uti.band_freq_hi;
    if (freq > com_uti.band_freq_hi)
      freq = com_uti.band_freq_lo;
    freq = com_uti.tnru_freq_fix (freq);
    return (freq);
  }


  public static String tnru_rds_picl_get (String band, int pi) {             // Get North American callsign string for Program ID
    String ret = "";

    if (band.equals ("US"))                                   // If North America...
      ret = na_pi_parse (pi);                                              // Parse/convert PI to callsign
    else if (pi != 0)
      ret = com_uti.hex_get ((short) pi);                                 // Return hex PI

    com_uti.logd ("band: " + band + "  pi: " + pi + "  ret: " + ret);
    return (ret);
  }

  // Convert Program ID to North American call-sign string, if possible.
  // Example: 0x54A6 -> KZZY
  private static String na_pi_parse (int pi) {
    String call_sign = "";                                              // CALL LETTERS THAT MAP TO PI CODES = _ _ 0 0.

    if ( (pi >> 8) == 0xAF) {                                           // If PI = 0xAFrs
      pi = ((pi & 0xFF) << 8);                                          //    PI = 0xrs00
    }
    // Run the second exception. NOTE: For 9 special cases 1000,2000,..,9000 a double mapping occurs utilizing exceptions 1 and 2:
    //     1000->A100->AFA1;2000->A200->AFA2; ... ;
    //     8000->A800->AFA8;9000->A900->AFA9
    if ( (pi >> 12) == 0xA) {                                           // If PI = 0xAqrs
      pi = ((pi & 0xF00) << 4) + (pi & 0xFF);                           //    PI = 0xq0rs
    }
    if ( (pi >= 0x1000) && (pi <= 0x994E) ) { //&& (pi != 0x163e) && (pi != 0x15d6) ) {   // If PI = 0x1000 (4096) - 0x944E (37966)  !!! (Exceptions) and not 0x163e (88.5) or 0x15d6 (89.9)
      String cs_start_char;
      // KAAA - KZZZ
      if ( (pi >= 0x1000) && (pi <= 0x54A7) ) {
        pi -= 0x1000;
        cs_start_char = "K";
      }
      else { /* WAAA - WZZZ*/
        pi -= 0x54A8;
        cs_start_char = "W";
      }
      int CharDiv = pi / 26;                                            // 26^3 = 17576 = 0x44a8
      int CharPos = pi - (CharDiv * 26);
      char c3 = (char)('A'+CharPos);

      pi = CharDiv;
      CharDiv = pi / 26;
      CharPos = pi - (CharDiv * 26);
      char c2 = (char)('A'+CharPos);

      pi = CharDiv;
      CharDiv = pi / 26;
      CharPos = pi - (CharDiv * 26);
      char c1 = (char) ('A' + CharPos);
      call_sign = cs_start_char + c1 + c2 + c3;
    }
    else if ( (pi >= 0x9950) && (pi <= 0x9EFF)) {   // 3-LETTER-ONLY CALL LETTERS
      call_sign = letter3_call_sign_get (pi);
    }
    else {//NATIONALLY-LINKED RADIO STATIONS CARRYING DIFFERENT CALL LETTERS
      call_sign = other_call_sign_get (pi);
    }
    return (call_sign);
  }

  private static String other_call_sign_get (int pi) {
    //String cs = "";
    int pi_nat_reg = pi & 0xf0ff;                                       // Isolate National/Regional

    if (pi == 0 || pi == -1)                                            // If invalid PI...
      return ("");                                                      // Done w/ empty string

    if (pi_nat_reg == 0xb001)                                           // If NPR 0xb*01...
      return ("NPR");

//    if (fm_apln.canada_get () == 0) {                                   // If not a Canadian applicable band...
//      return (null);                                                    // Done w/ no callsign
//    }

    //logd ("region / other_call_sign_get testing canada/ottawa pi");
    
    if (pi_nat_reg == 0xb002)                                           // If CBC Radio 1 (English)...
      return ("CBC R1");
    else if (pi_nat_reg == 0xb004)
      return ("CBC R1a");
    else if (pi_nat_reg == 0xb003)                                      // If CBC Radio 2 (French)...
      return ("CBC R2");
    else if (pi_nat_reg == 0xb005)
      return ("CBC R2a");

    //else if (pi== 0xb102)
    //  return ("CBC R1a");                                             // Ott 91.5
    //else if (pi== 0xB103)
    //  return ("CBC R2a");                                             // Ott 103.3 CBOQ

    if (pi >= 0xC000 && pi <= 0xCFFF) {
      return (to_canadian_call (pi));
    }
    //else
    //  cs = null;
    //return (cs);
    return (null);
  }


/*
// 97.9 CHIN    = 51755
// 96.5 Capitale= 49948

//AF: number: 3   freq: 92100
//AltFreq group: e3 2e 43 61 


// !!! Exception for 88.5       0x163e = KCJM
// !!! Exception for 89.9       0x15d6 = KCFM

// These first 2 do nothing now; over-ride with US callsigns
    if (pi== 0x163e)
      cs = "88.5 CILV";//"Ott 88.5 CILV";
//89.1 nothing but group 0B with spaces.
    else if (pi== 0x15d6)
      cs = "89.9 HOT";//"Ott 89.9 HOT";

    else if (pi== 0xb404)                                            // Ottawa Canadian numbers from  0xb20c - 0xcE0d  / 0xdc09
      cs="90.7 PREMIERE";//"Ott 90.7 PREMIERE";
//CBC R1 above 91.5
//93.1 nothing.
    else if (pi== 0xccb6)
      cs="93.9 BOB";//"Ott 93.9 BOB";
//94.5 nothing.
    else if (pi== 0xc7cd)
      cs="94.9 RDETENTE";//"Ott 94.9 RDETENTE";
//95.7 unscannable ??? ??? !!!!
//97.1...distant
//99.1 nothing.
    else if (pi== 0xca44)
      cs="100.3 Majic100";//"Ott 100.3 Majic100";
    else if (pi== 0xb205)
      cs="102.5 ESPACE M";//"Ott 102.5 ESPACE M";
//CBC R2 above 103.3 nothing ?
    else if (pi== 0xce0d)
      cs="104.1 NRJ";//"Ott 104.1 NRJ";
    else if (pi== 0xc87d)
      cs="105.3 KISS CISS";//"Ott 105.3 KISS CISS";
    else if (pi== 0xc448)
      cs="106.1 CHEZ";//"Ott 106.1 CHEZ";
    else if (pi== 0xdc09)
      cs="106.9 BEAR CKQB";//"Ott 106.9 BEAR CKQB";
    else
      cs = null;
    return (cs);
  }
*/

// https://www.cgran.org/browser/projects/simple_fm_rcv/trunk/handle_rds.c

// Canadian station normally use call letters from the CFAA-CFZZ and CHAA-CKZZ blocks. IE. CFxx, CHxx, CIxx, CJxx, CKxx = 5 * 26 * 27 = 3510  (0xC000 - 0xCDB5)
/*
C000 - CFFF assigned to Canada. Allows AF switching, but no regionalization. PI codes C0xx, and Cx00 are excluded from use.
Same for Mexico except uses 0xFxxx

B_01 - B_FF, D_01 - D_FF, E_01 - E_FF
assigned for national networks in US, Canada, and Mexico. Regionalization allowed. NRSC to provide assignments for all three countries.
It should be noted that operation in this region is the same as it is for all RDS PI codes.
*/
  private static String to_canadian_call (int pi) {
    if (pi < 0xC000 || pi > 0xCFFF)
      return ("cpi: " + pi);
    pi -= 0xC000;                                                       // pi = 0x0000 - 0x0FFF
    String retbuf = "C";
    int G, H;
    char i, j, k, tmp;
    for (i = 0; i < 5; i ++) {                                          // i = 0 - 4
      for (j = 0; j < 26; j ++) {                                       // j = 0 - 25
        for (k = 0; k < 27; k ++) {                                     // k = 0 - 26

          G = i * 702 + j * 27 + k + 257;                               // Canada PI formula for Rev 2. Rev 1 used 676 instead of 702
          H = (G - 257) / 255;  // -252-3=-2, 4-256=-1, 512-766=1, 767-1024=2, 1025-1280=3, 1277-  ???????????
//     H =          G/255 - 257/255
// G + H = pi = G +  G/255 - 257/255
//            = 256*G/255 - 257/255
//            = (256 * i * 702 + 256 * j * 27 + 256 * k + 256 * 257) - 257)/255

// 255 * H = G - 257
// G = 255 * H + 257




          if (pi == G + H) {                                            // If PI match

            if (i > 0)                                                  // If not first block, IE not CFxx...
              i ++;                                                     // Skip CGxx
            tmp = (char) i;
            tmp += 'F';
            retbuf += tmp;                                              // F, H, I, J, K

            tmp = (char) j;
            tmp += 'A';
            retbuf += tmp;                                              // A - Z

            if (k == 0) 
              k -= 32;                                                  // 0 = " "
            tmp = (char) k;
            tmp += '@';
            retbuf += tmp;

            return (retbuf);
          }

        }
      }
    }
    //return ("" + pi);
    return ("ci: " + pi);
  }


  private static String letter3_call_sign_get (int pi) {  // 72 values out of 106 possible (slightly sparse): 0x9950 - 0x99b9
    switch (pi) {
      case 0x99A5:  return "KBW";
      case 0x9992:  return "KOY";
      case 0x9978:  return "WHO";
      case 0x99A6:  return "KCY";
      case 0x9993:  return "KPQ";
      case 0x999C:  return "WHP";
      case 0x9990:  return "KDB";
      case 0x9964:  return "KQV";
      case 0x999D:  return "WIL";
      case 0x99A7:  return "KDF";
      case 0x9994:  return "KSD";
      case 0x997A:  return "WIP";
      case 0x9950:  return "KEX";
      case 0x9965:  return "KSL";
      case 0x99B3:  return "WIS";
      case 0x9951:  return "KFH";
      case 0x9966:  return "KUJ";
      case 0x997B:  return "WJR";
      case 0x9952:  return "KFI";
      case 0x9995:  return "KUT";
      case 0x99B4:  return "WJW";
      case 0x9953:  return "KGA";
      case 0x9967:  return "KVI";
      case 0x99B5:  return "WJZ";
      case 0x9991:  return "KGB";
      case 0x9968:  return "KWG";
      case 0x997C:  return "WKY";
      case 0x9954:  return "KGO";
      case 0x9996:  return "KXL";
      case 0x997D:  return "WLS";
      case 0x9955:  return "KGU";
      case 0x9997:  return "KXO";
      case 0x997E:  return "WLW";
      case 0x9956:  return "KGW";
      case 0x996B:  return "KYW";
      case 0x999E:  return "WMC";
      case 0x9957:  return "KGY";
      case 0x9999:  return "WBT";
      case 0x999F:  return "WMT";
      case 0x99AA:  return "KHQ";
      case 0x996D:  return "WBZ";
      case 0x9981:  return "WOC";
      case 0x9958:  return "KID";
      case 0x996E:  return "WDZ";
      case 0x99A0:  return "WOI";
      case 0x9959:  return "KIT";
      case 0x996F:  return "WEW";
      case 0x9983:  return "WOL";
      case 0x995A:  return "KJR";
      case 0x999A:  return "WGH";
      case 0x9984:  return "WOR";
      case 0x995B:  return "KLO";
      case 0x9971:  return "WGL";
      case 0x99A1:  return "WOW";
      case 0x995C:  return "KLZ";
      case 0x9972:  return "WGN";
      case 0x99B9:  return "WRC";
      case 0x995D:  return "KMA";
      case 0x9973:  return "WGR";
      case 0x99A2:  return "WRR";
      case 0x995E:  return "KMJ";
      case 0x999B:  return "WGY";
      case 0x99A3:  return "WSB";
      case 0x995F:  return "KNX";
      case 0x9975:  return "WHA";
      case 0x99A4:  return "WSM";
      case 0x9960:  return "KOA";
      case 0x9976:  return "WHB";
      case 0x9988:  return "WWJ";
      case 0x99AB:  return "KOB";
      case 0x9977:  return "WHK";
      case 0x9989:  return "WWL";
    }
    return "";
  }


  public static String tnru_rds_ptype_get (String band, int pt) {            // !! English only !!
    String ret = "";
    if (band.equals ("US"))                                           // If outside North America...
      ret = tuner_rbds_pt_str_get (pt);
    else
      ret = tuner_rds_pt_str_get (pt);
    //com_uti.loge ("band: " + band + "  pt: " + pt + "  ret: " + ret);
    return (ret);
  }
  private static String tuner_rbds_pt_str_get (int pt) {                // Get String for RBDS Program type Code (North America)
    switch (pt) {
      case 0:   return "";
      case 1:   return "News";
      case 2:   return "Info";//rmation";
      case 3:   return "Sports";
      case 4:   return "Talk";
      case 5:   return "Rock";
      case 6:   return "Class Rock";//"Classic Rock";
      case 7:   return "Adult Hits";
      case 8:   return "Soft Rock";
      case 9:   return "Top 40";
      case 10:  return "Country";
      case 11:  return "Oldies";
      case 12:  return "Soft";
      case 13:  return "Nostalgia";
      case 14:  return "Jazz";
      case 15:  return "Classical";
      case 16:  return "R & B";//"Rhythm and Blues";
      case 17:  return "Soft R & B";//"Soft Rhythm and Blues";
      case 18:  return "Foreign";// Language";
      case 19:  return "";//"Religious Music";
      case 20:  return "";//"Religious Talk";
      case 21:  return "Personality";
      case 22:  return "Public";
      case 23:  return "College";

//gap   24  !! Saw 24 in Montreal. Jazz ?
//""
//""
//""
//""    28
      case 24:  return "Jazz ?";//"Jazz Music";
      case 25:  return "Country";// Music";
      case 26:  return "National";// Music";
      case 27:  return "Oldies";// Music";
      case 28:  return "Folk";// Music";

      case 29:  return "Weather";
      case 30:  return "Emerg Test";//"Emergency Test";
      case 31:  return "Emergency";
    }
    if (pt == -1)
      com_uti.logd ("Unknown RBDS Program Type: " + pt);
    else if (pt == -2)
      com_uti.logd ("Unknown RBDS Program Type: " + pt);
    else
      com_uti.loge ("Unknown RBDS Program Type: " + pt);
    //return ("Pt: " + pt);
    return ("");
  }
  private static String tuner_rds_pt_str_get (int pt) {   // Get the Text String for the RDS Program type Code (Non-North America)
    switch (pt) {
      case 0:   return "";
      case 1:   return "News";
      case 2:   return "Cur Affairs";//"Current Affairs";
      case 3:   return "Information";
      case 4:   return "Sport";
      case 5:   return "Education";
      case 6:   return "Drama";
      case 7:   return "Culture";
      case 8:   return "Science";
      case 9:   return "Varied";
      case 10:  return "Pop";// Music";
      case 11:  return "Rock";// Music";
      case 12:  return "Easy Listen";//ing Music";
      case 13:  return "Light Class";//ical";
      case 14:  return "SeriousClass";//"Serious classical";
      case 15:  return "Other";// Music";
      case 16:  return "Weather";
      case 17:  return "Finance";
      case 18:  return "Children";//'s programs";
      case 19:  return "Soc Affairs";//"Social Affairs";
      case 20:  return "Religion";
      case 21:  return "Phone In";
      case 22:  return "Travel";
      case 23:  return "Leisure";
      case 24:  return "Jazz";// Music";
      case 25:  return "Country";// Music";
      case 26:  return "National";// Music";
      case 27:  return "Oldies";// Music";
      case 28:  return "Folk";// Music";
      case 29:  return "Documentary";
      case 30:  return "Emerg Test";//"Emergency Test";
      case 31:  return "Emergency";
    }
    if (pt == -1)
      com_uti.logd ("Unknown RDS Program Type: " + pt);
    else if (pt == -2)
      com_uti.logd ("Unknown RDS Program Type: " + pt);
    else
      com_uti.loge ("Unknown RDS Program Type: " + pt);
    //return ("Pt: " + pt);
    return ("");
   }


    // Shim:

  public static boolean shim_files_operational_get () {                                  // If our lib and have old lib to call  (If just large but no old, assume original)
    if (com_uti.file_size_get ("/system/lib/libbt-hci.so") > 60000 && com_uti.file_size_get ("/system/lib/libbt-hcio.so") > 10000 && com_uti.file_size_get ("/system/lib/libbt-hcio.so") < 60000)
      return (true);
    if (com_uti.file_size_get ("/system/vendor/lib/libbt-vendor.so") > 60000 && com_uti.file_size_get ("/system/vendor/lib/libbt-vendoro.so") > 10000 && com_uti.file_size_get ("/system/vendor/lib/libbt-vendoro.so") < 60000)
      return (true);
    return (false);
  }
  public static boolean shim_files_possible_get () {
    if (com_uti.file_get ("/system/lib/libbt-hci.so"))
      return (true);
    if (com_uti.file_get ("/system/vendor/lib/libbt-vendor.so"))// && ! chass_plug_aud.equals ("LG2"))   // Disable libbt-vendor for LG2 due to Audio enable issue !!!!
      return (true);
    return (false);
  }

/* OLD: 4 states:
    BT Off                                                            Use UART  (or BT on and install/use shim if possible)
    BT On & (Shim Not Installed or Shim Old)    Install Shim, BT Off, Use UART      First run before reboot or first boot after ROM update with no addon.d fix
    BT On &  Shim     Installed & NOT Active                  BT Off, Use UART      Need reboot & BT to be active
    BT On &  Shim     Installed &     Active                          Use SHIM  */

  public static int shim_install () {                                   // XZ2:    Can't remount /system as ro
    main_thread_get ("shim_install");
    int ret = 0;
    boolean restart_bt = false;

    String cmd = "";
    cmd += ("mount -o remount,rw /system ; ");
    if (com_uti.file_get ("/system/lib/libbt-hci.so")) {                // Favor old style
      cmd += ("mv /system/lib/libbt-hci.so  /system/lib/libbt-hcio.so ; ");
      cmd += ("cp /data/data/fm.a2d.sf/lib/libbt-hci.so /system/lib/libbt-hci.so ; ");
      cmd += ("chmod 644 /system/lib/libbt-hci.so ; ");
    }
    else if (com_uti.file_get ("/system/vendor/lib/libbt-vendor.so")) {
      cmd += ("mv /system/vendor/lib/libbt-vendor.so  /system/vendor/lib/libbt-vendoro.so ; ");
      cmd += ("cp /data/data/fm.a2d.sf/lib/libbt-vendor.so /system/vendor/lib/libbt-vendor.so ; ");
      cmd += ("chmod 644 /system/vendor/lib/libbt-vendor.so ; ");
    }
    cmd += ("cp /data/data/fm.a2d.sf/files/99-spirit.sh /system/addon.d/99-spirit.sh ; ");
    cmd += ("chmod 755 /system/addon.d/99-spirit.sh ; ");
    cmd += ("mount -o remount,ro /system ; ");
    com_uti.sys_run (cmd, true);
    com_uti.logd ("Done Bluedroid SU commands");

    if (shim_files_operational_get ())
      com_uti.logd ("Installed SHIM OK");
    else {
      com_uti.loge ("Install SHIM ERROR");
      ret = -1;
    }

com_uti.sys_run ("killall com.android.bluetooth", true);

    return (ret);
  }

//Doesn't help:
    //cmd += ("kill `pidof com.android.bluetooth` ; ");                // Kill bluetooth process and it will restart

    //cmd += ("pm clear com.android.bluetooth ; ");                       // Stop bluetooth process; can run even if BT is "off"
    //com_uti.sys_run (cmd, true);
    //com_uti.ms_sleep (1000);                                              // Extra 1 second delay to ensure


  public static int shim_remove () {
    main_thread_get ("shim_remove");
    int ret = 0;

    if (! shim_files_operational_get ()) {
      com_uti.loge ("Shim file not installed !!");
//      return (-1);
    }

    String cmd = "";
    cmd += ("mount -o remount,rw /system ; ");

    if (com_uti.file_get ("/system/lib/libbt-hcio.so"))  //shim_files_operational_get ())
      cmd += ("mv /system/lib/libbt-hcio.so  /system/lib/libbt-hci.so ; ");
    else if (! com_uti.file_get ("/system/lib/libbt-hci.so"))
      com_uti.logd ("No /system/lib/libbt-hci.so installed");
    else
      com_uti.loge ("No original hci shim file installed !!");

    if (com_uti.file_get ("/system/vendor/lib/libbt-vendoro.so"))  //shim_files_operational_get ())
      cmd += ("mv /system/vendor/lib/libbt-vendoro.so  /system/vendor/lib/libbt-vendor.so ; ");
    else if (! com_uti.file_get ("/system/vendor/lib/libbt-vendor.so"))
      com_uti.logd ("No /system/vendor/lib/libbt-vendor.so installed");
    else
      com_uti.loge ("No original vendor shim file installed !!");

    if (com_uti.file_get ("/system/addon.d/99-spirit.sh"))
      cmd += ("rm /system/addon.d/99-spirit.sh ; ");
    cmd += ("mount -o remount,ro /system ; ");
    com_uti.sys_run (cmd, true);
    com_uti.logd ("Done");

    if (! shim_files_operational_get ()) {
      com_uti.logd ("Removed SHIM OK");
    }
    else {
      com_uti.loge ("Remove SHIM ERROR");
      ret = -1;
    }

//com_uti.sys_run ("killall com.android.bluetooth", true);

/*
    //com_uti.loge ("WARM RESTART !!");
    com_uti.sys_run ("kill `pidof system_server`", true);
*/
    return (ret);
  }

/*
private final int getAndIncrement(int modulo) {
    for (;;) {
        int current = atomicInteger.get();
        int next = (current + 1) % modulo;
        if (atomicInteger.compareAndSet(current, next))
            return current;
    }
}
*/

  private static BluetoothAdapter      m_bt_adapter    = null;

  private static boolean bta_get () {
    m_bt_adapter = BluetoothAdapter.getDefaultAdapter ();                // Just do this once, shouldn't change
    if (m_bt_adapter == null) {
      com_uti.loge ("BluetoothAdapter.getDefaultAdapter () returned null");
      return (false);
    }
    return (true);
  }

  public static boolean bt_get () {                                     // Old note: Should check with pid_get ("bluetoothd"), "brcm_patchram_plus", "btld", "hciattach" etc. for consistency w/ fm_hrdw
    boolean ret = false;                                                //      BUT: if isEnabled () doesn't work, m_bt_adapter.enable () and m_bt_adapter.disable () may not work either.
    if (m_bt_adapter == null)
      if (! bta_get ())
        return (ret);
    ret = m_bt_adapter.isEnabled ();
    com_uti.logd ("bt_get isEnabled (): " + ret);
    return (ret);
  }

// bttest is_enabled
// bttest enable
// bttest disable
// Alternate/libbluedroid uses rfkill and ctl.stop/ctl.start bluetoothd

  public static int bt_set ( boolean bt_pwr, boolean wait ) {
    main_thread_get ("bt_set");
    if (m_bt_adapter == null)
      if (! bta_get ())
        return (-1);

    boolean bt = bt_get ();                                             // bt = current BT state
    if (bt_pwr && bt) {                                                 // If want BT and have BT...
      com_uti.logd ("bt_set BT already on");
      return (0);
    }
    if (! bt_pwr && ! bt) {                                             // If not want BT and not have BT...
      com_uti.logd ("bt_set BT already off");
      return (0);
    }
    if (bt_pwr) {                                                       // If actionable request for BT on
      com_uti.logd ("bt_set BT turning on");

      try {
        m_bt_adapter.enable ();                                         // Start enable BT
      }
      catch (Throwable e) {
        com_uti.loge ("bt_set m_bt_adapter.disable () Exception");
      }

      if (! wait)                                                       // If no wait
        return (0);                                                     // Done w/ no error

      bt_wait (true);                                                   // Else wait until BT is on or times out
      bt = bt_get ();
      if (bt) {
        com_uti.logd ("bt_set BT on success");
        return (0);
      }
      com_uti.loge ("bt_set BT on error");
      return (-1);
    }
    else {                                                              // Else if actionable request for BT off
      com_uti.logd ("bt_set BT turning off");
      try {
        m_bt_adapter.disable ();                                        // Start disable BT
      }
      catch (Throwable e) {
        com_uti.loge ("bt_set m_bt_adapter.disable () Exception");
      }

      if (! wait)                                                       // If no wait
        return (0);                                                     // Done w/ no error

      bt_wait (false);                                                  // Wait until BT is off or times out
      bt = bt_get ();
      if (! bt) {
        com_uti.logd ("bt_set BT off success");
        return (0);
      }
      com_uti.loge ("bt_set BT off error");
      return (-1);
    }
  }

  private static void bt_wait (boolean wait_on) {
    com_uti.logd ("Start wait_on: " + wait_on);
    int ctr = 0;
    boolean done = false;

    while (! done && ctr ++ < 100 ) {                                   // While not done and 10 seconds has not elapsed...
      if (wait_on)                                                      // If waiting for BT on...
        done = bt_get ();                                               // Done if BT on
      else                                                              // Else if waiting for BT off...
        done = ! bt_get ();                                             // Done if BT off
      if (! done)
        com_uti.quiet_ms_sleep (100);                                   // Wait 0.1 second
    }
    com_uti.logd ("wait_on: " + wait_on + "  done: " + done);
    return;
  }

  public static void rfkill_bt_wait (boolean wait_on) {
    com_uti.logd ("Start wait_on: " + wait_on);
    int ctr = 0;
    boolean done = false;

    while (! done && ctr ++ < 100 ) {                                   // While not done and 10 seconds has not elapsed...
      if (wait_on)                                                      // If waiting for BT on...
        done = bt_get ();                                               // Done if BT on
      else                                                              // Else if waiting for BT off...
        done = ! bt_get ();                                             // Done if BT off
      if (! done)
        com_uti.quiet_ms_sleep (100);                                   // Wait 0.1 second
    }
    com_uti.logd ("wait_on: " + wait_on + "  done: " + done);
    return;
  }


  private static boolean rfkill_bt_get () {
    String state = rfkill_state_get ();
    if (state.equals ("1"))
      return (true);
    else
      return (false);
  }

  private static String rfkill_state_get () {
    String state = "0";
    try {
      state = (new BufferedReader (new FileReader ("/sys/class/rfkill/rfkill0/state"))).readLine ();
      com_uti.logd ("Read rfkill0/state: " + state);
    }
    catch (Throwable e) {
      e.printStackTrace ();
      com_uti.loge ("Read rfkill0/state Exception");
    }
    return (state);
  }
/*
  private int rfkill_state_set (int state) {
    com_uti.logd ("state: " + state);
    if (state != 0) {                                                   // If turning on...
      //rfkill_state_set_on = true;
    }
    else {                                                              // Else if turning off...
      if (! rfkill_state_set_on)                                        // If was not previously off...
        return (0);                                                     // Done
    }
    rfkill_state_get ();                                                // Display rfkill state
    String [] cmds = {("")};
    cmds [0] = ("echo " + state + " > /sys/class/rfkill/rfkill0/state");
    com_uti.sys_WAS_run (cmds, true);                                         // Set rfkill state WITH SU/Root
    rfkill_state_get ();                                                // Display rfkill state
    if (state != 0)                                                     // If turning on...
      rfkill_state_set_on = true;
    else
      rfkill_state_set_on = false;
    return (0);
  }
*/

    // Install app:
/*private void app_install (String filename) {
    m_com_api.key_set ("tuner_state", "Stop");                      // Full power down/up
    Intent intent = new Intent (Intent.ACTION_VIEW);
    intent.setDataAndType (Uri.fromFile (new java.io.File (com_uti.getExternalStorageDirectory() + "/download/" + filename)), "application/vnd.android.package-archive");
    intent.setFlags (Intent.FLAG_ACTIVITY_NEW_TASK);
    m_gui_act.startActivity (intent);
    m_gui_act.finish ();
  }*/

}

/*private String getProperty (String propertyName) {
    String ret = System.getProperty (propertyName);         // Need to be system user for some properties ?
    if (ret != null)
      return (ret);

    String cmd = "getprop " + propertyName + " | grep 1";      // !! Only works with binary 0/1 results !!!
    int iret = com_uti.WAS_sys_run (cmd, true);
    if (iret == 0)
      ret = "1";
    else
      ret = "0";
    return (ret);
  }*/


