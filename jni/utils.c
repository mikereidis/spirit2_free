
    // Utilities: Used by many

  #include <android/log.h>

  #define  loge(...)  fm_log_print(ANDROID_LOG_ERROR, LOGTAG,__VA_ARGS__)
  #define  logd(...)  fm_log_print(ANDROID_LOG_DEBUG, LOGTAG,__VA_ARGS__)

  int extra_log = 0;

  int ena_verbo_log = 0;
  int ena_debug_log = 0;
  int ena_error_log = 1;
  void * log_hndl = NULL;

  int (* do_log) (int prio, const char * tag, const char * fmt, va_list ap);
  #include <stdarg.h>
  int fm_log_print (int prio, const char * tag, const char * fmt, ...) {

    if (! ena_error_log && prio == ANDROID_LOG_ERROR)
      return -1;

    if (! ena_debug_log && prio == ANDROID_LOG_DEBUG)
      return -1;

    //if (! ena_verbo_log)
    //  return -1;

    va_list ap;
    va_start (ap, fmt); 

    if (log_hndl == NULL) {
      log_hndl = dlopen ("liblog.so", RTLD_LAZY);
      if (log_hndl == NULL) {
        ena_verbo_log = 0;                                              // Don't try again
        ena_debug_log = 0;
        ena_error_log = 1;
        return (-1);
      }
      do_log = dlsym (log_hndl, "__android_log_vprint");
      if (do_log == NULL) {
        ena_verbo_log = 0;                                              // Don't try again
        ena_debug_log = 0;
        ena_error_log = 1;
        return (-1);
      }
    }
    do_log (prio, tag, fmt, ap);
    return (0);
  }


    // !!!! Eliminate duplicates, such as ms_get, hcd_file_find, ...

#ifndef DEF_BUF
#define DEF_BUF 512    // Raised from 256 so we can add headers to 255-256 byte buffers
#endif

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


  long ms_sleep (long ms) {
    if (ms > 10 && (ms % 101 != 0) && (ms % 11 != 0))
      loge ("ms_sleep ms: %d", ms);
    usleep (ms * 1000);
    return (ms);
  }


#include <dirent.h>                                 // For opendir (), readdir (), closedir (), DIR, struct dirent.

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

void props_log (const char * prop, char * prop_buf) {
  //char prop_buf [DEF_BUF] = "";
  __system_property_get (prop, prop_buf);
  logd ("props_log %32.32s: %s", prop, prop_buf);
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

  props_log ("ro.product.manufacturer",           product_manuf_prop_buf);

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


  int flags_file_get (const char * file, int flags) {                   // Return 1 if file, or directory, or device node etc. exists and we can open it
    if (! file_get (file))
      return (0);
    int fd = open (file, flags);
    if (fd < 0)
      return (0);
    close (fd);
    return (1);                                                         // File is accessible
  }



int file_get (char * file) {                                            // Return 1 if file, or directory, or device node etc. exists
  struct stat sb;
  if (stat (file, & sb) == 0)                                           // If file exists...
    return (1);                                                         // Return 1
  return (0);                                                           // Else if no file return 0
}

long ms_get () {
  struct timespec tspec = {0, 0};
  int res = clock_gettime (CLOCK_MONOTONIC, & tspec);
  //logd ("sec": %d  nsec: %d, tspec.tv_sec, tspec.tv_nsec);

  long millisecs= (tspec.tv_nsec / 1000000);
  millisecs += (tspec.tv_sec * 1000);       // remaining 22 bits good for monotonic time up to 4 million seconds =~ 46 days. (?? - 10 bits for 1024 ??)

  return (millisecs);
}


  #define HD_MW   256
  static void hex_dump (char * prefix, int width, unsigned char * buf, int len) {
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
        logd (line);
        line [0] = 0;
        if (prefix)
          strlcpy (line, prefix, sizeof (line));
      }
      else if (i == len - 1 && n)
        logd (line);
    }
  }

