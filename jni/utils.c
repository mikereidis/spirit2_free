
    // Utilities: Used by many

#ifndef UTILS_INCLUDED

  #define UTILS_INCLUDED

  #define   S2D_POLL_MS     100


  #define   NET_PORT_S2D    2102
  #define   NET_PORT_HCI    2112

    // For *_sget() to indicate get(), not set()
  #define   GET     -1
  #define   GETP    NULL

  #ifndef DEF_BUF
    #define DEF_BUF 512                                                 // Raised from 256 so we can add headers to 255-256 byte buffers
  #endif

  #include "man_ver.h"

  #include <android/log.h>
  #include <dirent.h>                                                   // For opendir (), readdir (), closedir (), DIR, struct dirent.

  int gen_server_loop_func (unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max);
  int gen_server_poll_func (int poll_ms);

  #define  logv(...)  s2_log(ANDROID_LOG_VERBOSE,LOGTAG,__VA_ARGS__)
  #define  logd(...)  s2_log(ANDROID_LOG_DEBUG,LOGTAG,__VA_ARGS__)
  #define  logw(...)  s2_log(ANDROID_LOG_WARN,LOGTAG,__VA_ARGS__)
  #define  loge(...)  s2_log(ANDROID_LOG_ERROR,LOGTAG,__VA_ARGS__)


  int ena_log_verbo = 0;
  int ena_log_debug = 1;
  int ena_log_warni = 0;
  int ena_log_error = 1;

  int ena_log_verbose_tshoot= 0;

  int ena_log_alsa_error    = 1;    // ALSA support Error
  int ena_log_alsa_verbo    = 0;//1;    // ALSA support Verbose
  int ena_log_hex_dump      = 0;    // Hex dump for bt-hci and many others
  int ena_log_tnr_extra     = 0;    // Tuner verbose
  int ena_log_tnr_evt       = 0;    // Tuner events
  int ena_log_s2d_cmd       = 0;    // S2d commands
  int ena_log_ven_extra     = 0;    // Shim

  int ena_log_af_com_err    = 0;    // AF common errors
  int ena_log_af_ok         = 0;    // AF OK                !! AF Needs some work !!

  int ena_log_rds_com_err   = 1;    // RDS common errors

  int ena_log_rds_ok_iris   = 0;    // Qualcomm RDS OK

  int ena_log_rds_rt        = 1;    // New RT
  int ena_log_rds_pspt      = 1;    // New PS, PT
  int ena_log_rds_extra     = 0;    // Time/Date, Traffic, Group info
  int ena_log_bch_rds_stats = 0;    // Broadcom RDS stats
  int ena_log_bch_rds_err   = 0;    // Broadcom RDS errors

  int ena_log_bch_reg       = 0;    // Broadcom Registers
  int ena_log_bch_hci       = 0;    // Broadcom HCI


    // int __android_log_print(int prio, const char *tag, const char *fmt, ...);
    // int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap);


  int s2_log (int prio, const char * tag, const char * fmt, ...) {

    if (! ena_log_verbo && prio == ANDROID_LOG_VERBOSE)
      return -1;
    if (! ena_log_debug && prio == ANDROID_LOG_DEBUG)
      return -1;
    if (! ena_log_warni && prio == ANDROID_LOG_WARN)
      return -1;
    if (! ena_log_error && prio == ANDROID_LOG_ERROR)
      return -1;

    va_list ap;
    va_start (ap, fmt); 

    __android_log_vprint(prio, tag, fmt, ap);
    return (0);
  }


    // !!!! Eliminate duplicates, such as ms_get, hcd_file_find, ...

  #define MAX_ITOA_SIZE 32      // Int 2^32 need max 10 characters, 2^64 need 21
  char * itoa (int val, char * ret, int radix) {
    if (radix == 10)
      snprintf (ret, MAX_ITOA_SIZE - 1, "%d", val);
    else if (radix == 16)
      snprintf (ret, MAX_ITOA_SIZE - 1, "%x", val);
    else
      loge ("radix != 10 && != 16: %d", radix);
    return (ret);
  }
/*
  int noblock_set (int fd) {
    //#define IOCTL_METH
    #ifdef  IOCTL_METH
    int nbio = 1;
    int ret = ioctl (fd, FIONBIO, & nbio);
    if (ret == -1)
      loge ("noblock_set ioctl errno: %d", errno);
    else
      logd ("noblock_set ioctl ret: %d", ret);
    #else
    int flags = fcntl (fd, F_GETFL);
    if (flags == -1) {
      loge ("noblock_set fcntl get errno: %d", errno);
      flags = 0;
    }
    else
      logd ("noblock_set fcntl get flags: %d  nonblock flags: %d", flags, flags & O_NONBLOCK);
    int ret = fcntl (fd, F_SETFL, flags | O_NONBLOCK);
    if (ret == -1)
      loge ("noblock_set fcntl set errno: %d", errno);
    else
      logd ("noblock_set fcntl set ret: %d", ret);
    flags = fcntl (fd, F_GETFL);
    if (flags == -1)
      loge ("noblock_set fcntl result get errno: %d", errno);
    else
      logd ("noblock_set fcntl result get flags: %d  nonblock flags: %d", flags, flags & O_NONBLOCK);
    #endif
    return (0);
  }

*/

/*
  void alt_usleep (uint32_t us) {
    struct timespec delay;
    int err;
    //if (us == 0)
    //  return;
    delay.tv_sec = us / 1000000;
    delay.tv_nsec = 1000 * 1000 * (us % 1000000);
        // usleep can't be used because it uses SIGALRM
    do {
      err = nanosleep (& delay, & delay);
    } while (err < 0 && errno == EINTR);
  }
*/

  long quiet_ms_sleep (long ms) {
    usleep (ms * 1000);
    return (ms);
  }

  long ms_sleep (long ms) {

//    if (ms > 10 && (ms % 101 != 0) && (ms % 11 != 0))
      loge ("ms_sleep ms: %d", ms);

    usleep (ms * 1000);
    return (ms);
  }


    // In utils.c so that aud_all and tnr_qcv can access the same function:
/*
  int utils_qcv_ana_vol_set (int vol) {
    //alsa_long_set ("HPHL Volume", vol / 3276); // Range 0 - 20        // !! Doesn't work so must use Internal FM RX Volume
    //alsa_long_set ("HPHR Volume", vol / 3276); // Range 0 - 20
    int alsa_vol = vol / 128;//64;//32;//16;//8;
    logd ("Setting Internal FM RX Volume to alsa_vol: %d", alsa_vol);
    alsa_long_set ("Internal FM RX Volume", alsa_vol);
    return (vol);
  }
*/


char user_dev [DEF_BUF] = "";

char * user_char_dev_get (char * dir_or_dev, int user) {
  DIR  * dp;
  struct dirent * dirp;
  struct stat sb;
  int ret = 0;
  logd ("user_char_dev_get: %s  %d", dir_or_dev, user);

  ret = stat (dir_or_dev, & sb);                                        // Get file/dir status.
  if (ret == -1) {
    loge ("user_char_dev_get: dir_or_dev stat errno: %d", errno);
    return (NULL);
  }

  if (S_ISCHR (sb.st_mode)) {                                           // If this is a character device...
    if (sb.st_uid == user) {                                            // If user match...
      //strlcpy (user_dev, dir_or_dev, sizeof (user_dev));
      //return (user_dev);                                              // Device found
      return (dir_or_dev);
    }
    return (NULL);
  }

  if ((dp = opendir (dir_or_dev)) == NULL) {                            // Open the directory. If error...
    loge ("user_char_dev_get: can't open dir_or_dev: %s  errno: %d", dir_or_dev, errno);
    return (NULL);                                                      // Done w/ no result
  }
  //logd ("user_char_dev_get opened directory %s", dir_or_dev);

  while ((dirp = readdir (dp)) != NULL) {                               // For all files/dirs in this directory... (Could terminate with errno set !!)
    //logd ("user_char_dev_get: readdir returned file/dir %s", dirp->d_name);

    char filename [DEF_BUF] = {0};
    strlcpy (filename, dir_or_dev, sizeof (filename));
    strlcat (filename, "/", sizeof (filename));
    strlcat (filename, dirp->d_name, sizeof (filename));                // Set fully qualified filename

    ret = stat (filename, & sb);                                        // Get file/dir status.
    if (ret == -1) {
      loge ("user_char_dev_get: file stat errno: %d", errno);
      continue;                                                         // Ignore/Next if can't get status
    }

    if (S_ISCHR (sb.st_mode)) {                                         // If this is a character device...
      //logd ("user_char_dev_get: dir %d", sb.st_mode);
      if (sb.st_uid == user) {                                          // If user match...
        closedir (dp);                                                  // Close the directory.
        strlcpy (user_dev, filename, sizeof (user_dev));
        return (user_dev);                                              // Device found
      }
    }
  }
  closedir (dp);                                                        // Close the directory.
  return (NULL);
}


  int version_sdk;
  char version_sdk_prop_buf       [DEF_BUF];

  char product_device_prop_buf    [DEF_BUF] = "";
  char product_manuf_prop_buf     [DEF_BUF] = "";
  char product_board_prop_buf     [DEF_BUF] = "";

  void prop_buf_get (const char * prop, char * prop_buf) {
    __system_property_get (prop, prop_buf);
    logd ("prop_buf_get %s: %s", prop, prop_buf);
  }

  char def_prop_buf    [DEF_BUF] = "";

  char * prop_get (const char * prop) {
    __system_property_get (prop, def_prop_buf);
    logd ("prop_get %s: %s", prop, def_prop_buf);
    return (def_prop_buf);
  }



int file_find (char * dir, char * pat, char * path_buf, int path_len) {      // Find first file under subdir dir, with pattern pat. Put results in path_buf of size path_len.
  static int nest = 0;
  path_buf [0] = 0;
  if (nest == 0)
    logd ("file_find: %d %s %s", nest, dir, pat);

  nest ++;
  if (nest > 16) {                                                      // Routine is recursive; if more than 16 subdirectories nested... (/system/xbin/bb -> /system/xbin)
    logd ("file_find maximum nests: %d  dir: %s  path: %s", nest, dir, pat);
    nest --;
    return (0);                                                         // Done w/ no result
  }

  DIR  *dp;
  struct dirent *dirp;
  struct stat sb;
  int ret = 0;

  if ((dp = opendir (dir)) == NULL) {                                    // Open the directory. If error...
    //#define EACCES      13  /* Permission denied */
    if (errno == 13)                                                    // Common problem (Even w/ SU)
      logd ("file_find: can't open directory %s  errno: %d (EACCES Permission denied)", dir, errno);
    else
      logd ("file_find: can't open directory %s  errno: %d", dir, errno);
    nest --;
    return (0);//-13);                                                       // Done w/ no result & error code -EPERM
  }
  //logd ("file_find opened directory %s", dir);

  while ((dirp = readdir (dp)) != NULL) {                                // For all files/dirs in this directory... (Could terminate with errno set !!)
    //logd ("file_find: readdir returned file/dir %s", dirp->d_name);

    if (strlen (dirp->d_name) == 1)
      if (dirp->d_name[0] == '.')
        continue;                                                       // Ignore/Next if "." current dir

    if (strlen (dirp->d_name) == 2)
      if (dirp->d_name[0] == '.' && dirp->d_name[1] == '.')
        continue;                                                       // Ignore/Next if ".." parent dir

    char filename[DEF_BUF] = {0};
    strlcpy (filename, dir, sizeof (filename));
    strlcat (filename, "/", sizeof (filename));
    strlcat (filename, dirp->d_name, sizeof (filename));                // Set fully qualified filename

    ret = stat (filename, &sb);                                         // Get file/dir status.
    if (ret == -1) {
      logd ("file_find: stat errno: %d", errno);
      continue;                                                         // Ignore/Next if can't get status
    }

    if (S_ISDIR (sb.st_mode)) {                                         // If this is a directory...
      //logd ("file_find: dir %d", sb.st_mode);
      if (file_find (filename, pat, path_buf, path_len)) {              // Recursively call self: Go deeper to find the file, If found...
        closedir (dp);                                                  // Close the directory.
        nest --;
        return (1);                                                     // File found
      }
    }

    else if (S_ISREG (sb.st_mode)) {                                     // If this is a regular file...
      //logd ("file_find: reg %d", sb.st_mode);
      int pat_len = strlen (pat);
      int filename_len = strlen (filename);
      if (filename_len >= pat_len) {
        if (! strncasecmp (pat, &filename [filename_len - pat_len], pat_len)) {   // !! Case insensitive
          logd ("file_find pattern: %s  filename: %s", pat, filename);
          strlcpy (path_buf,filename, path_len);
          closedir (dp);                                                // Close the directory.
          nest --;
          return (1);                                                   // File found
        }
      }
    }
    else {
      logd ("file_find: unk %d", sb.st_mode);
    }
  }
  closedir (dp);                                                        // Close the directory.
  nest --;
  return (0);                                                           // Done w/ no result
}



int hcd_num = 0;

int hcd_file_find (char * path_buf, int path_len) {      // Find first file under subdir dir, with pattern pat. Put results in path_buf of size path_len.
  int ret = file_find ("/system", ".hcd", path_buf, path_len);          // Sometimes under system/vendor/firmware instead of system/etc/firmware
  logd ("HCD hcd file_find ret: %d", ret);
  hcd_num = 0;

  if (ret)                  // If we have at least one BC *.hcd or *.HCD firmware file in /system...
    logd ("hcd_file_find have *.hcd file: %s", path_buf);

  prop_buf_get ("ro.product.manufacturer",           product_manuf_prop_buf);

  if (! strncasecmp (product_manuf_prop_buf,"LG", strlen ("LG"))) {     // LG or LGE
    logd ("hcd_file_find LG G2");
    strncpy (path_buf, "/data/data/fm.a2d.sf/files/b2.bin", path_len);
    hcd_num = 2;
  }
  else if (! strncasecmp (product_manuf_prop_buf,"SONY", strlen ("SONY"))) {
    logd ("hcd_file_find Sony Z2+");
    //strncpy (path_buf, "/data/data/fm.a2d.sf/files/b3.bin", path_len);
    hcd_num = 0;    // Use actual file
  }
  else if (! strncasecmp (product_manuf_prop_buf,"HTC", strlen ("HTC"))) {
    logd ("hcd_file_find HTC One M7");
    strncpy (path_buf, "/data/data/fm.a2d.sf/files/b1.bin", path_len);
    hcd_num = 1;
  }
  else {
    logd ("hcd_file_find Unknown");
    hcd_num = 0;    // Use actual file
  }

  if (ret || hcd_num)   // If we have a file or can use internal...
    return (1);

  loge ("hcd_file_find no *.hcd file or no permission");
  return (0);                                                           // Done w/ no result
}


  int flags_file_get (const char * filename, int flags) {               // Return 1 if file, or directory, or device node etc. exists and we can open it

    int ret = 0;                                                        // 0 = File does not exist or is not accessible
    if ( file_get (filename)) {                                         // If file exists...
      int fd = open (filename, flags);
      if (fd < 0)
        loge ("flags_file_get open fd: %d  errno: %d", fd, errno);
      else
        logd ("flags_file_get open fd: %d", fd);
      if (fd >= 0) {                                                    // If open success...
        ret = 1;                                                        // 1 = File exists and is accessible
        close (fd);
      }
    }
    logd ("flags_file_get ret: %d  filename: %s", ret, filename);
    return (ret);                                                       // 0 = File does not exist or is not accessible         // 1 = File exists and is accessible
  }

  int file_get (const char * filename) {                                // Return 1 if file, or directory, or device node etc. exists
    struct stat sb = {0};
    int ret = 0;                                                        // 0 = No file
    errno = 0;
    if (stat (filename, & sb) == 0) {                                   // If file exists...
      ret = 1;                                                          // 1 = File exists
      logd ("file_get ret: %d  filename: %s", ret, filename);
    }
    else {
      if (errno == 2)
        logd ("file_get ret: %d  filename: %s  errno 2 = No File/Dir", ret, filename);
      else
        loge ("file_get ret: %d  filename: %s  errno: %d", ret, filename, errno);
    }
    return (ret);
  }

  int file_delete (const char * filename) {
    int ret = unlink (filename);
    logd ("file_delete ret: %d", ret);
    return (ret);
  }
  int file_create (const char * filename) {
    int ret = flags_file_get (filename, O_CREAT);
    logd ("file_create ret: %d", ret);
    return (ret);
  }


long ms_get () {
  struct timespec tspec = {0, 0};
  int res = clock_gettime (CLOCK_MONOTONIC, & tspec);
  //logd ("sec": %d  nsec: %d, tspec.tv_sec, tspec.tv_nsec);

  long millisecs= (tspec.tv_nsec / 1000000);
  millisecs += (tspec.tv_sec * 1000);       // remaining 22 bits good for monotonic time up to 4 million seconds =~ 46 days. (?? - 10 bits for 1024 ??)

  return (millisecs);
}

/*
  int chip_lock_val = 0;
  char * curr_lock_cmd = "none";
  int chip_lock_get (char * cmd) {
    return (0);
    int retries = 0;
    int max_msecs = 3030;
    int sleep_ms = 101; // 10
    while (retries ++ < max_msecs / sleep_ms) {
      chip_lock_val ++;
      if (chip_lock_val == 1) {
        curr_lock_cmd = cmd;
        return (0);
      }
      chip_lock_val --;
      if (chip_lock_val < 0)
        chip_lock_val = 0;
      loge ("sleep_ms: %d  retries: %d  cmd: %s  curr_lock_cmd: %s", sleep_ms, retries, cmd, curr_lock_cmd);
      ms_sleep (sleep_ms);
    }
    loge ("chip_lock_get retries exhausted");
    return (-1);
  }
  int chip_lock_ret () {
    if (chip_lock_val > 0)
      chip_lock_val --;
    if (chip_lock_val < 0)
      chip_lock_val = 0;
    return (0);
  }
*/


  #define HD_MW   256
  void hex_dump (char * prefix, int width, unsigned char * buf, int len) {
    if (! ena_log_hex_dump)
      return;
    char tmp  [3 * HD_MW + 8] = "";     // Handle line widths up to HD_MW
    char line [3 * HD_MW + 8] = "";
    if (width > HD_MW)
      width = HD_MW;
    int i, n;
    line [0] = 0;
    if (prefix)
      strlcpy (line, prefix, sizeof (line));
    for (i = 0, n = 1; i < len; i ++, n ++) {
      snprintf (tmp, sizeof (tmp), "%2.2x ", buf [i]);
      strncat (line, tmp, sizeof (line));
      if (n == width) {
        n = 0;
        loge (line);
        line [0] = 0;
        if (prefix)
          strlcpy (line, prefix, sizeof (line));
      }
      else if (i == len - 1 && n)
        loge (line);
    }
  }

/*
  static char sys_cmd [32768] = {0};
  static int sys_commit () {
    int ret = sys_run (sys_cmd);                                        // Run
    sys_cmd [0] = 0;                                                    // Done, so zero
    return (ret);
  }
  static int cached_sys_run (char * new_cmd) {                          // Additive single string w/ commit version
    char cmd [512] = {0};
    if (strlen (sys_cmd) == 0)                                          // If first command since commit
      snprintf (cmd, sizeof (cmd), "%s", new_cmd);
    else
      snprintf (cmd, sizeof (cmd), " ; %s", new_cmd);
    strncat (sys_cmd, cmd, sizeof (sys_cmd));
    int ret = sys_commit ();                                            // Commit every command now, due to GS3/Note problems
    return (ret);
  }
*/

/*
  int sys_run (char * cmd) {
    int ret = system (cmd);                                             // !! Binaries like ssd that write to stdout cause C system() to crash !
    logd ("sys_run ret: %d  cmd: \"%s\"", ret, cmd);
    return (ret);
  }
  int insmod_shell = 1;
*/
  int util_insmod (char * module) {    // ("/system/lib/modules/radio-iris-transport.ko");
    int ret = 0;
/*
    if (insmod_shell) {
      char cmd [DEF_BUF] = "insmod ";
      strlcat (cmd, module, sizeof (cmd));
      strlcat (cmd, " >/dev/null 2>/dev/null", sizeof (cmd));
      ret = sys_run (cmd);
      loge ("util_insmod module: \"%s\"  ret: %d  cmd: \"%s\"", module, ret, cmd);
    }
    else {
*/
      ret = insmod_internal (module);
      if (ret)
        loge ("util_insmod module: \"%s\"  ret: %d", module, ret);
      else
        logd ("util_insmod module: \"%s\"  success ret: %d", module, ret);
/*
    }
*/
    return (ret);
  }

  int file_write_many (const char * filename, int * pfd, char * data, int len, int flags) {
    logd ("file_write_many filename: %d  * pfd: %d  len: %d  flags: %d", filename, * pfd, len, flags);
    if (len > 0 && len == strlen (data))
      logd ("file_write_many data: \"%s\"", data);                      // All we use this for is ascii strings, so OK to display, else hexdump
    else if (len > 0)
      loge ("file_write_many NEED HEXDUMP !!");
    else
      logd ("file_write_many no data");
    int ret = 0;
    errno = 0;
    if (* pfd < 0) {
      if (flags | O_CREAT)
        * pfd = open (filename, flags, S_IRWXU | S_IRWXG | S_IRWXO);        //O_WRONLY);                                   // or O_RDWR    (But at least one codec_reg is write only, so leave!)
      else
        * pfd = open (filename, flags);
    }

    if (* pfd < 0) {
      loge ("file_write_many open * pfd: %d  flags: %d  errno: %d  len: %d  filename: %s", * pfd, flags, errno, len, filename);
      return (0);
    }

    logd ("file_write_many open * pfd: %d", * pfd);
    int written = 0;
    errno = 0;
    if (len > 0)
      written = write (* pfd, data, len);
    if (written != len)
      loge ("file_write_many written: %d  flags: %d  errno: %d  len: %d  filename: %s", written, flags, errno, len, filename);
    else
      logd ("file_write_many written: %d  flags: %d  len: %d  filename: %s", written, flags, len, filename);
    return (written);
  }

  int file_write (const char * filename, char * data, int len, int flags) {
    int ret = 0;
    int fd = -1;

    ret = file_write_many (filename, & fd, data, len, flags);
    logd ("file_write -> file_write_many ret: %d  flags: %d  len: %d  filename: %s", ret, flags, len, filename);

    errno = 0;
    ret = -1;
    if (fd >= 0)
      ret = close (fd);
    logd ("file_write close ret: %d", ret);
    return (ret);
  }

  void * file_read (const char * filename, ssize_t * size_ret) {
    int ret, fd;
    struct stat sb;
    ssize_t size;
    void * buffer = NULL;

    /* open the file */
    fd = open (filename, O_RDONLY);
    if (fd < 0)
      return (NULL);

    /* find out how big it is */
    if (fstat (fd, & sb) < 0)
      goto bail;
    size = sb.st_size;

    /* allocate memory for it to be read into */
    buffer = malloc (size);
    if (! buffer)
      goto bail;

    /* slurp it into our buffer */
    ret = read (fd, buffer, size);
    if (ret != size)
      goto bail;

    /* let the caller know how big it is */
    * size_ret = size;

  bail:
    close (fd);
    return (buffer);
  }

  extern int init_module (void *, unsigned long, const char *);

  int insmod_internal (const char * filename) {
    void * file;
    ssize_t size = 0;
    char opts [1024];
    int ret;

    file = file_read (filename, & size); // read the file into memory
    if (! file) {
      loge ("insmod_internal can't open \"%s\"", filename);
      return (-1);
    }

    opts [0] = '\0';
/*
    if (argc > 2) {
      int i, len;
      char *end = opts + sizeof(opts) - 1;
      char *ptr = opts;

      for (i = 2; (i < argc) && (ptr < end); i++) {
          len = MIN(strlen(argv[i]), end - ptr);
          memcpy(ptr, argv[i], len);
          ptr += len;
          *ptr++ = ' ';
      }
      *(ptr - 1) = '\0';
    }
*/

    ret = init_module (file, size, opts);   // pass it to the kernel
    if (ret != 0) {
      if (errno == EEXIST) { // 17
        ret = 0;
        logd ("insmod_internal: init_module '%s' failed EEXIST because already loaded", filename);
      }
      else
        loge ("insmod_internal: init_module '%s' failed (%s)", filename, strerror (errno));
    }

    free (file);    // free the file buffer
    return ret;
 }


/*
  char * holder_id = "None";

int lock_open (const char * id, volatile int * lock, int tmo) {
  int attempts = 0;
  volatile int lock_val = * lock;

  //logd ("lock_open %s  lock_val: %d  lock: %d  tmo: %d", id, lock_val, * lock, tmo);
  int start_time   = ms_get ();                                         // Set start time
  int timeout_time = start_time + tmo;                                  // Set end/timeout time
  int elapsed_time = ms_get () - start_time;
  long alt_sleep_ctr = 0;

  while (ms_get () < timeout_time) {                                    // Until timeout
    if (* lock < 0) {                                                   // If negative...
      * lock = 0;                                                       // Fix
      loge ("!!!!!!!!! lock_open clear negative lock id: %s  holder_id: %s", id, holder_id, attempts);
    }

    while ((* lock) && ms_get () < timeout_time) {                      // While the lock is NOT acquired and we have not timed out...
      if (elapsed_time > 100)
        loge ("lock_open sleep 10 ms id: %s  holder_id: %s  attempts: %d  lock_val: %d  lock: %d  elapsed_time: %d", id, holder_id, attempts, lock_val, * lock, elapsed_time);// Else we lost attempt
      if (attempts) {                                                   // If not 1st try...
        alt_sleep_ctr = 0;
        while (alt_sleep_ctr ++ < 10000);       // !!!! Because ms_sleep was crashing ?
      }
      else
        ms_sleep (10);                                                  // Sleep a while then try again
    }
    elapsed_time = ms_get () - start_time;

    (* lock) ++;                                                        // Attempt to acquire lock (1st or again)
    lock_val = (* lock);
    if (lock_val == 1) {                                                // If success...
      if (attempts)                                                     // If not 1st try...
        loge ("lock_open %s success id: %s  holder_id: %s  attempts: %d  lock_val: %d  lock: %d  elapsed_time: %d", id, holder_id, attempts, lock_val, * lock, elapsed_time);
      holder_id = (char *) id;                                          // We are last holder (though done now)
      return (0);                                                       // Lock acquired
    }
    else {
      (* lock) --;                                                      // Release lock attempt
      loge ("lock_open lost id: %s  holder_id: %s  attempts: %d  lock_val: %d  lock: %d  elapsed_time: %d", id, holder_id, attempts, lock_val, * lock, elapsed_time);// Else we lost attempt
      attempts ++;
    }
  }
  lock_val = (* lock);
  loge ("lock_open timeout id: %s  holder_id: %s  attempts: %d  lock_val: %d  lock: %d  elapsed_time: %d", id, holder_id, attempts, lock_val, * lock, elapsed_time);
  return (-1);                                                          // Error, no lock
}

int lock_close (const char * id, volatile int * lock) {
  //logd ("lock_close %s  lock: %d", id, (* lock));
  (* lock) --;
  //logd ("lock_close %s 2  lock: %d", id, (* lock));
  return (0);
}
*/


  #define ERROR_CODE_NUM 56
  static const char * error_code_str [ERROR_CODE_NUM + 1] = {
    "Success",
    "Unknown HCI Command",
    "Unknown Connection Identifier",
    "Hardware Failure",
    "Page Timeout",
    "Authentication Failure",
    "PIN or Key Missing",
    "Memory Capacity Exceeded",
    "Connection Timeout",
    "Connection Limit Exceeded",
    "Synchronous Connection to a Device Exceeded",
    "ACL Connection Already Exists",
    "Command Disallowed",
    "Connection Rejected due to Limited Resources",
    "Connection Rejected due to Security Reasons",
    "Connection Rejected due to Unacceptable BD_ADDR",
    "Connection Accept Timeout Exceeded",
    "Unsupported Feature or Parameter Value",
    "Invalid HCI Command Parameters",
    "Remote User Terminated Connection",
    "Remote Device Terminated Connection due to Low Resources",
    "Remote Device Terminated Connection due to Power Off",
    "Connection Terminated by Local Host",
    "Repeated Attempts",
    "Pairing Not Allowed",
    "Unknown LMP PDU",
    "Unsupported Remote Feature / Unsupported LMP Feature",
    "SCO Offset Rejected",
    "SCO Interval Rejected",
    "SCO Air Mode Rejected",
    "Invalid LMP Parameters",
    "Unspecified Error",
    "Unsupported LMP Parameter Value",
    "Role Change Not Allowed",
    "LMP Response Timeout",
    "LMP Error Transaction Collision",
    "LMP PDU Not Allowed",
    "Encryption Mode Not Acceptable",
    "Link Key Can Not be Changed",
    "Requested QoS Not Supported",
    "Instant Passed",
    "Pairing with Unit Key Not Supported",
    "Different Transaction Collision",
    "Reserved",
    "QoS Unacceptable Parameter",
    "QoS Rejected",
    "Channel Classification Not Supported",
    "Insufficient Security",
    "Parameter out of Mandatory Range",
    "Reserved",
    "Role Switch Pending",
    "Reserved",
    "Reserved Slot Violation",
    "Role Switch Failed",
    "Extended Inquiry Response Too Large",
    "Simple Pairing Not Supported by Host",
    "Host Busy - Pairing",
  };

  const char * hci_err_get (uint8_t status) {
    const char * str;
    if (status <= ERROR_CODE_NUM)
      str = error_code_str [status];
    else
      str = "Unknown HCI Error";
    return (str);
  }



  #include <netinet/in.h>
  #include <netdb.h> 

  int sock_tmo_set (int fd, int tmo) {                                 // tmo = timeout in milliseconds
    struct timeval tv = {0, 0};
    tv.tv_sec = tmo / 1000;                                               // Timeout in seconds
    tv.tv_usec = (tmo % 1000) * 1000;
    int ret = setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) & tv, sizeof (struct timeval));
    if (ret != 0) {
      loge ("sock_tmo_set setsockopt SO_RCVTIMEO errno: %d", errno);
    }
    else {
      //logd ("sock_tmo_set setsockopt SO_RCVTIMEO Success");
    }
    //ret = setsockopt (fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *) & tv, sizeof (struct timeval));
    //if (ret != 0) {
    //  loge ("timeout_set setsockopt SO_SNDTIMEO errno: %d", errno);
    //}
    return (0);
  }
  

  int pid_get (char * cmd, int start_pid) {
    DIR  * dp;
    struct dirent * dirp;
    FILE * fdc;
    struct stat sb;
    int pid = 0;
    int ret = 0;
    logd ("pid_get: %s  start_pid: %d", cmd, start_pid);

    if ((dp = opendir ("/proc")) == NULL) {                             // Open the /proc directory. If error...
      loge ("pid_get: opendir errno: %d", errno);
      return (0);                                                       // Done w/ no process found
    }
    while ((dirp = readdir (dp)) != NULL) {                             // For all files/dirs in this directory... (Could terminate with errno set !!)
      //logd ("pid_get: readdir: %s", dirp->d_name);
      errno = 0;
      pid = atoi (dirp->d_name);                                        // pid = directory name string to integer
      if (pid <= 0) {                                                   // Ignore non-numeric directories
        //loge ("pid_get: not numeric ret: %d  errno: %d", pid, errno);
        continue;
      }
      if (pid < start_pid) {                                            // Ignore PIDs we have already checked. Depends on directories in PID order which seems to always be true
        //loge ("pid_get: pid < start_pid");
        continue;
      }

      //logd ("pid_get: test pid: %d", pid);
    
      char fcmdline [DEF_BUF] = "/proc/";
      strlcat (fcmdline, dirp->d_name, sizeof (fcmdline));
      ret = stat (fcmdline, & sb);                                      // Get file/dir status.
      if (ret == -1) {
        logd ("pid_get: stat errno: %d", errno);                        // Common: pid_get: stat errno: 2
        continue;
      }
      if (S_ISDIR (sb.st_mode)) {                                       // If this is a directory...
        //logd ("pid_get: dir %d", sb.st_mode);
        char cmdline [DEF_BUF] = {0};
        strlcat (fcmdline, "/cmdline", sizeof (fcmdline));
        if ((fdc = fopen (fcmdline, "r")) == NULL) {                    // Open /proc/???/cmdline file read-only, If error...
          loge ("pid_get: fopen errno: %d", errno);
          continue;
        }
        ret = fread (cmdline, sizeof (char), sizeof (cmdline) - 1, fdc);// Read
        if (ret < 0 || ret > sizeof (cmdline) - 1) {                    // If error...
          loge ("pid_get fread ret: %d  errno: %d", ret, errno);
          fclose (fdc);
          continue;
        }
        cmdline [ret] = 0;
        fclose (fdc);
        int cmd_len = strlen (cmd);
        ret = strlen (cmdline);                                         // The buffer includes a trailing 0, so adjust ret to actual string length (in case not always true) (Opts after !)
        //logd ("pid_get: cmdline bytes: %d  cmdline: %s", ret, cmdline);

        if (ret >= cmd_len) {                                           // Eg: ret = strlen ("/bin/a") = 6, cmd_len = strlen ("a") = 1, compare 1 at cmd_line[5]

          if (! strncmp (cmd, & cmdline [ret - cmd_len], cmd_len)) {    // If a matching process name
            logd ("pid_get: got pid: %d for cmdline: %s  start_pid: %d", pid, cmdline, start_pid);
            closedir (dp);                                              // Close the directory.
            return (pid);                                               // SUCCESS: Done w/ pid
          }
        }
      }
      else if (S_ISREG (sb.st_mode)) {                                  // If this is a regular file...
        loge ("pid_get: reg %d", sb.st_mode);
      }
      else {
        loge ("pid_get: unk %d", sb.st_mode);
      }
    }
    closedir (dp);                                                      // Close the directory.
    return (0);                                                         // Done w/ no PID found
  }

  int kill_gentle_first = 1;
  int pid_kill (int pid, int brutal, char * cmd_to_verify) {
    logd ("pid_kill pid: %d  brutal: %d", pid, brutal);
    int ret = 0;
    int sig = SIGTERM;
    if (brutal) {
      if (kill_gentle_first) {
        errno = 0;
        ret = kill (pid, sig);
        if (ret) {
          loge ("pid_kill kill_gentle_first kill() errno: %d", errno);
        }
        else {
          logd ("pid_kill kill_gentle_first kill() success");
          errno = 0;
          int new_pid_check1 = pid_get (cmd_to_verify, pid);
          if (new_pid_check1 == pid) {
            loge ("pid_kill kill() success detected but same new_pid_check: %d  errno: %", new_pid_check1, errno);  // Fall through to brutal kill
          }
          else {
            logd ("Full Success pid != new_pid_check1: %d  errno: %", new_pid_check1, errno);
            return (ret);
          }
        }
      }
      sig = SIGKILL;
    }
    errno = 0;
    ret = kill (pid, sig);
    if (ret) {
      loge ("pid_kill kill() errno: %d", errno);
    }
    else {
      logd ("pid_kill kill() success");
      errno = 0;
      int new_pid_check2 = pid_get (cmd_to_verify, pid);
      if (new_pid_check2 == pid)
        loge ("pid_kill kill() success detected but same new_pid_check2: %d", new_pid_check2);
      else
        logd ("pid != new_pid_check: %d  errno: %", new_pid_check2, errno);
    }
    return (ret);
  }

  int killall (char * cmd, int brutal) {                                // Kill all OTHER instances of named process, except our own, if the same
    int ret = 0;
    int pid = 0;
    int our_pid = getpid ();
    logd ("killall cmd: %s  brutal: %d  our_pid: %d", cmd, brutal, our_pid);
    int idx = 0;
    int num_kill_attempts = 0;
    int max_kill_attempts = 16;                                         // Max of 16 kills (was to prevent blocking if can't kill, now just to limit kills)

    for (idx = 0; idx < max_kill_attempts; idx ++) {                    // For maximum kills...

      pid = pid_get (cmd, pid + 1);                                     // Get PID starting at last found PID + 1

      if (pid == our_pid) {
        logd ("pid == our_pid");                                        // If us, just log
      }
      else if (pid > 0) {
        ret = pid_kill (pid, brutal, cmd);                              // Else if valid external PID, kill it
        num_kill_attempts ++;
      }
      else {
        break;                                                          // Else if end of PID search, terminate loop
      }
    }
    logd ("killall num_kill_attempts: %d", num_kill_attempts);
    return (num_kill_attempts);
  }


#endif  //#ifndef UTILS_INCLUDED


    // Client/Server:

  //#ifdef  CS_AF_UNIX                                                      // For Address Family UNIX sockets
  //#include <sys/un.h>
  //#else                                                                   // For Address Family NETWORK sockets

    // Unix datagrams requires other write permission for /dev/socket, or somewhere else (ext, not FAT) writable.

  //#define CS_AF_UNIX        // Use network sockets to avoid filesystem permission issues w/ Unix Domain Address Family sockets
  #define CS_DGRAM            // Use datagrams, not streams/sessions

  #ifdef  CS_AF_UNIX                                                      // For Address Family UNIX sockets
  //#include <sys/un.h>
  #define DEF_API_SRVSOCK    "/dev/socket/srv_spirit"
  #define DEF_API_CLISOCK    "/dev/socket/cli_spirit"
  char api_srvsock [DEF_BUF] = DEF_API_SRVSOCK;
  char api_clisock [DEF_BUF] = DEF_API_CLISOCK;
  #define CS_FAM   AF_UNIX

  #else                                                                   // For Address Family NETWORK sockets
  //#include <netinet/in.h>
  //#include <netdb.h> 
  #define CS_FAM   AF_INET
  #endif

  #ifdef  CS_DGRAM
  #define CS_SOCK_TYPE    SOCK_DGRAM
  #else
  #define CS_SOCK_TYPE    SOCK_STREAM
  #endif

  #define   RES_DATA_MAX  1280


#ifdef  GENERIC_CLIENT
#ifndef  GENERIC_CLIENT_INCLUDED
#define  GENERIC_CLIENT_INCLUDED

    // Generic IPC API:

  int gen_client_cmd (unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max, int net_port, int rx_tmo) {
//    logd ("net_port: %d  cmd_buf: \"%s\"  cmd_len: %d", net_port, cmd_buf, cmd_len);
    static int sockfd = -1;
    int res_len, written;
    static socklen_t srv_len;
  #ifdef  CS_AF_UNIX
    static struct sockaddr_un  srv_addr;
    #ifdef  CS_DGRAM
    #define   CS_DGRAM_UNIX
      struct sockaddr_un  cli_addr;                                       // Unix datagram sockets must be bound; no ephemeral sockets.
      socklen_t cli_len;
    #endif
  #else
    //struct hostent *hp;
    struct sockaddr_in  srv_addr,cli_addr;
    socklen_t cli_len;
  #endif
  
    if (sockfd < 0) {
      if ((sockfd = socket (CS_FAM, CS_SOCK_TYPE, 0)) < 0) {                // Get an ephemeral, unbound socket
        loge ("gen_client_cmd: socket errno: %d", errno);
        return (0);//"Error socket");
      }
    #ifdef  CS_DGRAM_UNIX                                                 // Unix datagram sockets must be bound; no ephemeral sockets.
      strlcpy (api_clisock, DEF_API_CLISOCK, sizeof (api_clisock));
      char itoa_ret [MAX_ITOA_SIZE] = {0};
      strlcat (api_clisock, itoa (net_port, itoa_ret, 10), sizeof (api_clisock));
      unlink (api_clisock);                                                // Remove any lingering client socket
      memset ((char *) & cli_addr, sizeof (cli_addr), 0);
      cli_addr.sun_family = AF_UNIX;
      strncpy (cli_addr.sun_path, api_clisock, sizeof (cli_addr.sun_path));
      cli_len = strlen (cli_addr.sun_path) + sizeof (cli_addr.sun_family);
  
      if (bind (sockfd, (struct sockaddr *) & cli_addr,cli_len) < 0) {
        loge ("gen_client_cmd: bind errno: %d", errno);
        close (sockfd);
        sockfd = -1;
        return (0);//"Error bind");                                        // OK to continue w/ Internet Stream but since this is Unix Datagram and we ran unlink (), let's fail
      }
    #endif
    }
  //!! Can move inside above
  // Setup server address
    memset ((char *) & srv_addr, sizeof (srv_addr), 0);
  #ifdef  CS_AF_UNIX
    strlcpy (api_srvsock, DEF_API_SRVSOCK, sizeof (api_srvsock));
    char itoa_ret [MAX_ITOA_SIZE] = {0};
    strlcat (api_srvsock, itoa (net_port, itoa_ret, 10), sizeof (api_srvsock));
    srv_addr.sun_family = AF_UNIX;
    strlcpy (srv_addr.sun_path, api_srvsock, sizeof (srv_addr.sun_path));
    srv_len = strlen (srv_addr.sun_path) + sizeof (srv_addr.sun_family);
  #else
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr=htonl (INADDR_LOOPBACK);
    //hp = gethostbyname ("localhost");
    //if (hp == 0) {
    //  loge ("gen_client_cmd: Error gethostbyname  errno: %d", errno);
    //  return (0);//"Error gethostbyname");
    //}
    //bcopy ((char *) hp->h_addr, (char *) & srv_addr.sin_addr, hp->h_length);
    srv_addr.sin_port = htons (net_port);
    srv_len =sizeof (struct sockaddr_in);
  #endif
  
  
  // Send cmd_buf and get res_buf
  #ifdef CS_DGRAM
    written= sendto (sockfd, cmd_buf, cmd_len, 0,(const struct sockaddr *)&srv_addr,srv_len);
    if (written != cmd_len) {  // Dgram buffers should not be segmented
      loge ("gen_client_cmd: sendto errno: %d", errno);
    #ifdef  CS_DGRAM_UNIX
      unlink (api_clisock);
    #endif
      close (sockfd);
      sockfd = -1;
      return (0);//"Error sendto");
    }
  
    sock_tmo_set (sockfd, rx_tmo);
    res_len = recvfrom (sockfd, res_buf, res_max, 0,(struct sockaddr *)&srv_addr, &srv_len);
    if (res_len <= 0) {
      loge ("gen_client_cmd: recvfrom errno: %d", errno);
    #ifdef  CS_DGRAM_UNIX
      unlink (api_clisock);
    #endif
      close (sockfd);
      sockfd = -1;
      return (-1);
  //    return (0);//"Error recvfrom");
    }
    #ifndef CS_AF_UNIX
  // !!   ?? Don't need this ?? If srv_addr still set from sendto, should restrict recvfrom to localhost anyway ?
    if ( srv_addr.sin_addr.s_addr != htonl(INADDR_LOOPBACK) ) {
      loge ("gen_client_cmd: Unexpected suspicious packet from host");// %s", inet_ntop(srv_addr.sin_addr.s_addr)); //inet_ntoa(srv_addr.sin_addr.s_addr));
    }
    #endif
  #else
    if (connect(sockfd, (struct sockaddr *) &srv_addr, srv_len) < 0) {
      loge ("gen_client_cmd: connect errno: %d", errno);
      close (sockfd);
      sockfd = -1;
      return (0);//"Error connect");
    }
    written = write (sockfd, cmd_buf, cmd_len);                           // Write the command packet
    if (written != cmd_len) {                                             // Small buffers under 256 bytes should not be segmented ?
      loge ("gen_client_cmd: write errno: %d", errno);
      close (sockfd);
      sockfd = -1;
      return (0);//"Error write");
    }
  
    sock_tmo_set (sockfd, rx_tmo);
  
    res_len = read (sockfd, res_buf, res_max)); // Read response
    if (res_len <= 0) {
      loge ("gen_client_cmd: read errno: %d", errno);
      close (sockfd);
      sockfd = -1;
      return (0);//"Error read");
    }
  #endif
    //hex_dump ("", 32, res_buf, n);
  #ifdef  CS_DGRAM_UNIX
      unlink (api_clisock);
  #endif
    //close (sockfd);
    return (res_len);
  }

#endif      //#ifndef GENERIC_CLIENT_INCLUDED
#endif      //#ifdef  GENERIC_CLIENT


#ifdef  GENERIC_SERVER
#ifndef  GENERIC_SERVER_INCLUDED
#define  GENERIC_SERVER_INCLUDED

  int gen_server_exiting = 0;

  int gen_server_loop (int net_port, int poll_ms) {                     // Run until gen_server_exiting != 0, passing incoming commands to gen_server_loop_func() and responding with the results
    int sockfd = -1, newsockfd = -1, cmd_len = 0, ctr = 0;
    socklen_t cli_len = 0, srv_len = 0;
  #ifdef  CS_AF_UNIX
    struct sockaddr_un  cli_addr = {0}, srv_addr = {0};
    srv_len = strlen (srv_addr.sun_path) + sizeof (srv_addr.sun_family);
  #else
    struct sockaddr_in  cli_addr = {0}, srv_addr = {0};
    //struct hostent *hp;
  #endif
    unsigned char cmd_buf [DEF_BUF] ={0};
  
  #ifdef  CS_AF_UNIX
    strlcpy (api_srvsock, DEF_API_SRVSOCK, sizeof (api_srvsock));
    char itoa_ret [MAX_ITOA_SIZE] = {0};
    strlcat (api_srvsock, itoa (net_port, itoa_ret, 10), sizeof (api_srvsock));
    unlink (api_srvsock);
  #endif
    if ((sockfd = socket (CS_FAM, CS_SOCK_TYPE, 0)) < 0) {
      loge ("gen_server_loop socket  errno: %d", errno);
      return (-1);
    }

    if (poll_ms != 0)
      sock_tmo_set (sockfd, poll_ms);                                   // For polling every poll_ms milliseconds

    memset ((char *) & srv_addr, sizeof (srv_addr), 0);
  #ifdef  CS_AF_UNIX
    srv_addr.sun_family = AF_UNIX;
    strncpy (srv_addr.sun_path, api_srvsock, sizeof (srv_addr.sun_path));
    srv_len = strlen (srv_addr.sun_path) + sizeof (srv_addr.sun_family);
  #else
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK); //INADDR_ANY;
    //hp = gethostbyname ("localhost");
    //if (hp == 0) {
    //  loge ("Error gethostbyname  errno: %d", errno);
    //  return (-2);
    //}
    //bcopy ((char *) hp->h_addr, (char *) & srv_addr.sin_addr, hp->h_length);
    srv_addr.sin_port = htons (net_port);
    srv_len = sizeof (struct sockaddr_in);
  #endif
  
  #ifdef  CS_AF_UNIX
  logd ("srv_len: %d  fam: %d  path: %s", srv_len, srv_addr.sun_family, srv_addr.sun_path);
  #else
  logd ("srv_len: %d  fam: %d  addr: 0x%x  port: %d", srv_len, srv_addr.sin_family, ntohl (srv_addr.sin_addr.s_addr), ntohs (srv_addr.sin_port));
  #endif
    if (bind (sockfd, (struct sockaddr *) & srv_addr, srv_len) < 0) {
      loge ("Error bind  errno: %d", errno);
  #ifdef  CS_AF_UNIX
      return (-3);
  #endif
  #ifdef CS_DGRAM
      return (-3);
  #endif
      loge ("Inet stream continuing despite bind error");               // OK to continue w/ Internet Stream
    }

    if (poll_ms != 0)
      sock_tmo_set (sockfd, poll_ms);                                   // For polling every poll_ms ms

  // Get command from client
  #ifndef CS_DGRAM
    if (listen (sockfd, 5)) {                                           // Backlog= 5; likely don't need this
      loge ("Error listen  errno: %d", errno);
      return (-4);
    }
  #endif
  
    logd ("gen_server_loop Ready");
  
    while (! gen_server_exiting) {
      memset ((char *) & cli_addr, sizeof (cli_addr), 0);               // ?? Don't need this ?
      //cli_addr.sun_family = CS_FAM;                                   // ""
      cli_len = sizeof (cli_addr);
  
      //logd ("ms_get: %d",ms_get ());
  #ifdef  CS_DGRAM
      cmd_len = recvfrom (sockfd, cmd_buf, sizeof (cmd_buf), 0, (struct sockaddr *) & cli_addr, & cli_len);
      if (cmd_len <= 0) {
        if (errno == EAGAIN) {
          if (poll_ms != 0)                                             // If timeout polling is enabled...
            gen_server_poll_func (poll_ms);                             // Do the polling work
          else
            loge ("gen_server_loop EAGAIN !!!");                        // Else EGAIN is an unexpected error for blocking mode
        }
        else {                                                          // Else if some other error, sleep it off for 100 ms
          loge ("Error recvfrom errno: %d", errno);
          ms_sleep (101);
        }
        continue;
      }
    #ifndef CS_AF_UNIX
  // !! 
      if ( cli_addr.sin_addr.s_addr != htonl (INADDR_LOOPBACK) ) {
        //loge ("Unexpected suspicious packet from host %s", inet_ntop (cli_addr.sin_addr.s_addr));
        loge ("Unexpected suspicious packet from host");// %s", inet_ntoa (cli_addr.sin_addr.s_addr));
      }
    #endif
  #else
      newsockfd = accept (sockfd, (struct sockaddr *) & cli_addr, & cli_len);
      if (newsockfd < 0) {
        loge ("Error accept  errno: %d", errno);
        ms_sleep (101);   // Sleep 0.1 second
        continue;
      }
    #ifndef  CS_AF_UNIX
  // !! 
      if ( cli_addr.sin_addr.s_addr != htonl (INADDR_LOOPBACK) ) {
        //loge ("Unexpected suspicious packet from host %s", inet_ntop (cli_addr.sin_addr.s_addr));
        loge ("Unexpected suspicious packet from host");// %s", inet_ntoa (cli_addr.sin_addr.s_addr));
      }
    #endif
      cmd_len = read (newsockfd, cmd_buf, sizeof (cmd_buf));
      if (cmd_len <= 0) {
        loge ("Error read  errno: %d", errno);
        ms_sleep (101);   // Sleep 0.1 second
        close (newsockfd);
        ms_sleep (101);   // Sleep 0.1 second
        continue;
      }
  #endif
  
  #ifdef  CS_AF_UNIX
      //logd ("cli_len: %d  fam: %d  path: %s",cli_len,cli_addr.sun_family,cli_addr.sun_path);
  #else
      //logd ("cli_len: %d  fam: %d  addr: 0x%x  port: %d",cli_len,cli_addr.sin_family, ntohl (cli_addr.sin_addr.s_addr), ntohs (cli_addr.sin_port));
  #endif
      //hex_dump ("", 32, cmd_buf, n);
  
      unsigned char res_buf [RES_DATA_MAX] = {0};
      int res_len = 0;

      cmd_buf [cmd_len] = 0;    // Null terminate for string usage
      res_len = gen_server_loop_func ( cmd_buf, cmd_len, res_buf, sizeof (res_buf));    // Do server command function and provide response

      if (ena_log_verbose_tshoot)
        logd ("gen_server_loop gen_server_loop_func res_len: %d", res_len);

      if (res_len < 0) {  // If error
        res_len = 2;
        res_buf [0] = 0xff;//'?';   ?? 0xff for HCI ?
        res_buf [1] = 0xff;//'\n';
        res_buf [2] = 0;
      }
//      hex_dump ("", 32, res_buf, res_len);
  
  
  // Send response
  #ifdef  CS_DGRAM
      if (sendto (sockfd, res_buf, res_len, 0, (struct sockaddr *) & cli_addr, cli_len) != res_len) {
        loge ("Error sendto  errno: %d  res_len: %d", errno, res_len);
        ms_sleep (101);   // Sleep 0.1 second
      }
  #else
      if (write (newsockfd, res_buf, res_len) != res_len) {
        loge ("Error write  errno: %d", errno);
        ms_sleep (101);   // Sleep 0.1 second
      }
      close (newsockfd);
  #endif
    }
    close (sockfd);
  #ifdef  CS_AF_UNIX
    unlink (api_srvsock);
  #endif
  
    return (0);
  }

#endif      //#ifndef GENERIC_SERVER_INCLUDED
#endif      //#ifdef  GENERIC_SERVER


