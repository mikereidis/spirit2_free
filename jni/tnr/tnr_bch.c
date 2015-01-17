
  #define LOGTAG "stnr_bch"

#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include <android/log.h>
#include "jni.h"

#include "tnr_tnr.h"

#define EVT_LOCK_BYPASS
  #include "tnr_tnr.c"

// Unix datagrams requires other write permission for /dev/socket, or somewhere else (ext, not FAT) writable.

//#define CS_AF_UNIX        // Use network sockets to avoid filesystem permission issues.
#define CS_DGRAM

#ifdef  CS_AF_UNIX
#define DEF_API_SRVSOCK    "/dev/socket/srv_spirit"
#define DEF_API_CLISOCK    "/dev/socket/cli_spirit"
char api_srvsock [DEF_BUF] = DEF_API_SRVSOCK;
char api_clisock [DEF_BUF] = DEF_API_CLISOCK;
#endif

#ifdef  CS_AF_UNIX
#include <sys/un.h>
#define CS_FAM   AF_UNIX
#else
#include <netinet/in.h>
#include <netdb.h> 
#define CS_PORT    2112
#define CS_FAM   AF_INET
#endif

#ifdef  CS_DGRAM
#define CS_SOCK_TYPE    SOCK_DGRAM
#else
#define CS_SOCK_TYPE    SOCK_STREAM
#endif


#include <arpa/inet.h>

    // FM Chip specific functions in this code called from generic plug.c code:

    // API start/stop
    // Functions called from this chip specific code to generic code:

  long ms_sleep (long ms);

  #define  loge(...)  fm_log_print(ANDROID_LOG_ERROR, LOGTAG,__VA_ARGS__)
  #define  logd(...)  fm_log_print(ANDROID_LOG_DEBUG, LOGTAG,__VA_ARGS__)

  int fm_log_print (int prio, const char * tag, const char * fmt, ...);

  extern int extra_log;// = 0;

    // Debug:
  int rds_error_debug = 0;                                                // 0 = Don't log it all
  extern int rds_dbg;// = 1;                                                        // But do log counts every 10, 000 blocks
  extern int evt_dbg;// = 1;
  int seek_dbg     = 1;

  int freq_up_get (int freq);
  int freq_dn_get (int freq);


    // FM Chip specific code

#ifndef DEF_BUF
#define DEF_BUF 512    // Raised from 256 so we can add headers to 255-256 byte buffers
#endif

// Debug:
//int hci_dbg      = 0;//1;
int reg_dbg = 0;//      = 0;//1;
//extern int evt_dbg;//      = 0;//1;


int pid_get (const char * cmd, int idx) {
  DIR  *dp;
  struct dirent *dirp;
  FILE *fdc; //*fd
  struct stat sb;
  int pid = 0, ret = 0, pid_idx = 0;
  //logd ("pid_get: %s", cmd);
  if ((dp = opendir ("/proc")) == NULL) {                               // Open the /proc directory. If error...
    loge ("pid_get: opendir errno: %d", errno);
    return (0);                                                         // Done w/ no process found
  }
  while ((dirp = readdir (dp)) != NULL) {                               // For all files/dirs in this directory... (Could terminate with errno set !!)
    //logd ("pid_get: readdir: %s", dirp->d_name);
    errno = 0;
    pid = atoi (dirp->d_name);
    if (pid <= 0) {
      //loge ("pid_get: not numeric ret: %d  errno: %d", pid, errno);
      continue;
    }
    //logd ("pid_get: test pid: %d", pid);
    
    char fcmdline[DEF_BUF] = "/proc/";
    strlcat (fcmdline, dirp->d_name, sizeof (fcmdline));
    ret = stat (fcmdline, &sb);                                         // Get file/dir status.
    if (ret == -1) {
      logd ("pid_get: stat errno: %d", errno);                          // Common: pid_get: stat errno: 2
      continue;
    }
    if (S_ISDIR (sb.st_mode)) {                                         // If this is a directory...
      //logd ("pid_get: dir %d", sb.st_mode);
      char cmdline[DEF_BUF] = {0};
      strlcat (fcmdline, "/cmdline", sizeof (fcmdline));
      if ((fdc = fopen (fcmdline, "r")) == NULL) {                      // Open /proc/???/cmdline file read-only, If error...
        loge ("pid_get: fopen errno: %d", errno);
        continue;
      }
      ret = fread (cmdline, sizeof (char), sizeof (cmdline) - 1, fdc);  // Read
      if (ret < 0 || ret > sizeof (cmdline) - 1) {                      // If error...
        loge ("pid_get fread ret: %d  errno: %d", ret, errno);
        fclose (fdc);
        continue;
      }
      cmdline[ret] = 0;
      fclose (fdc);
      int cmd_len = strlen (cmd);
      ret = strlen (cmdline);                                           // The buffer includes a trailing 0, so adjust ret to actual string length (in case not always true) (Opts after !)
      //logd ("pid_get: cmdline bytes: %d  cmdline: %s", ret, cmdline);
      if (ret >= cmd_len) {                                             // Eg: ret = strlen ("/bin/a") = 6, cmd_len = strlen ("a") = 1, compare 1 at cmd_line[5]
        if (! strncmp (cmd, &cmdline [ret - cmd_len], cmd_len)) {
          //logd ("pid_get: got pid: %d for cmdline: %s  pid_idx: %d  idx: %d", pid, cmdline, pid_idx, idx);
          if (idx != pid_idx ++)
            continue;
          closedir (dp);                                                // Close the directory.
          return (pid);                                                 // Done w/ pid
        }
      }
    }
    else if (S_ISREG (sb.st_mode)) {                                   // If this is a regular file...
      loge ("pid_get: reg %d", sb.st_mode);
    }
    else {
      loge ("pid_get: unk %d", sb.st_mode);
    }
  }
  closedir (dp);                                                        // Close the directory.
  return (0);                                                           // Done w/ no process found
}

#include <sys/socket.h>

int sock_rx_tmo_set (int fd, int tmo) {                                 // tmo = timeout in milliseconds
  struct timeval tv = {0, 0};
  tv.tv_sec = tmo/1000;                                                 // Timeout in seconds
  tv.tv_usec = (tmo%1000) * 1000;
  int ret = setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof (struct timeval));
  if (ret != 0) {
    loge ("timeout_set setsockopt SO_RCVTIMEO errno: %d", errno);
  }
  //ret = setsockopt (fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *) &tv, sizeof (struct timeval));
  //if (ret != 0) {
  //  loge ("timeout_set setsockopt SO_SNDTIMEO errno: %d", errno);
  //}
  return (0);
}


  char * holder_id = "None";

int lock_open (const char * id, volatile int * lock, int tmo) {
  int attempts = 0;
  volatile int lock_val = * lock;

  //logd ("lock_open %s  lock_val: %d  lock: %d  tmo: %d", id, lock_val, * lock, tmo);
  int start_time   = ms_get ();                                         // Set start time
  int timeout_time = start_time + tmo;                                  // Set end/timeout time
  int elapsed_time = ms_get () - start_time;

  while (ms_get () < timeout_time) {                                    // Until timeout
    if (* lock < 0) {                                                   // If negative...
      * lock = 0;                                                       // Fix
      loge ("!!!!!!!!! lock_open clear negative lock id: %s  holder_id: %s", id, holder_id, attempts);
    }

    while ((* lock) && ms_get () < timeout_time) {                      // While the lock is NOT acquired and we have not timed out...
      if (elapsed_time > 100)
        loge ("lock_open sleep 10 ms id: %s  holder_id: %s  attempts: %d  lock_val: %d  lock: %d  elapsed_time: %d", id, holder_id, attempts, lock_val, * lock, elapsed_time);// Else we lost attempt
      ms_sleep (10);                                                    // Sleep a while then try again
    }
    elapsed_time = ms_get () - start_time;

    (* lock) ++;                                                        // Attempt to acquire lock (1st or again)
    lock_val = (* lock);
    if (lock_val == 1) {                                                // If success...
      if (attempts)                                                     // If not 1st try...
        loge ("lock_open %s success id: %s  holder_id: %s  attempts: %d  lock_val: %d  lock: %d  elapsed_time: %d", id, holder_id, attempts, lock_val, * lock, elapsed_time);
      holder_id = (char *) id;
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


  int shim_hci_enable = 0;                                              // Default 0 = UART, 1 = Bluedroid SHIM
  #include "bch_hci.c"

  // Do internal HCI command:

int do_acc_hci (unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max, int rx_tmo ) {

  int hx_ret = hci_xact (cmd_buf, cmd_len);
  if (hx_ret < 8 || hx_ret > 270) {
    hci_recv_buf [0] = 0xff; // Error
    return (8);
  }
  hci_recv_buf [0] = 0;
  memcpy (res_buf, hci_recv_buf, hx_ret);
  //hex_dump ("aaaa", 32, hci_recv_buf,hx_ret);
  //hex_dump ("bbbb", 32, res_buf,hx_ret);
  
  if (res_buf [7])
    loge ("do_acc_hci hci err: %d %s", res_buf [7], hci_err_get (res_buf [7]));
  return (hx_ret);
}


#define MAX_HCI  264   // 8 prepended bytes + 255 max bytes HCI data/parameters + 1 trailing byte to align
//unsigned char res_buf [MAX_HCI];


/*
// Bluetooth:
//make -f /home/mike/Documents/android-ndk-r5b/build/core/build-local.mk
//make -f $NDK/build/core/build-local.mk
// Support:
#include <linux/socket.h>
//#include <errno.h>

// BD Address
typedef struct {
	__u8 b[6];
} __attribute__((packed)) bdaddr_t;

//ar r libbluetooth.a libbluetooth.so
//cd ../../../../android-3/arch-arm/usr/lib/;mkdir bt; mv libbluetooth.* bt
//../../android-ndk-r5b/platforms/android-5/arch-arm/usr/lib/libbluetooth.so



// !! open/socket, read, write etc. can be used without libbluetooth !!
// When BT is off:
//      hci_get_route()   returns errno 19 (ENODEV).
//      hci_open_dev DOES return direct_hci_btsock=3 (w/ dev_id= -1). This socket/handle can be saved for later if BT is switched on.
//      hci_send_req()    returns errno 77 (EBADFD).
// When BT is on, but FM is off:
//       HCI event has data len =1 and single byte = 0x0C error

//static int direct_hci_btsock = -1;  // Not usable

int direct_hci_cmd (uint8_t ogf, uint16_t ocf, char *cmd_buf, int cmd_len, char *res_buf, int res_max) {
  int ret = 0, dev_id;

  logd ("direct_hci_cmd");
  if (direct_hci_btsock < 0) {
    dev_id = hci_get_route (NULL);
    if (dev_id < 0) {
      loge ("hci_get_route errno: %d", errno);
      //return (-101);                                                    // -1 can work as default value
    }
    logd ("hci_get_route dev_id: %d",dev_id);

    direct_hci_btsock = hci_open_dev(dev_id);        // This does: socket (PF_BLUETOOTH, SOCK_RAW, 1);        PF_BLUETOOTH = 31
    if (direct_hci_btsock < 0) {
      loge ("hci_open_dev errno: %d", errno);
      hci_close_dev(direct_hci_btsock);    // Same as close (socket);
      direct_hci_btsock = -1;
      return (-102);
    }
  }
#define hci_cmd_DEBUG
#ifdef hci_cmd_DEBUG
  logd ("hci_open_dev direct_hci_btsock: %d", direct_hci_btsock);

  logd ("HCI Cmd: ogf 0x%x  ocf 0x%x  cmd_len %d",ogf,ocf, cmd_len);
  hex_dump (" ", 32, cmd_buf, cmd_len);
#endif
  struct hci_request hci_req = {0};
  hci_req.ogf = ogf;
  hci_req.ocf = ocf;
  hci_req.event = HCI_EV_CMD_COMPLETE;
  hci_req.cparam = &cmd_buf [8];
  hci_req.clen = cmd_len-8;
  hci_req.rparam = &res_buf [7];
  hci_req.rlen = res_max;//252;//256; //1 hci error byte + up to 251 bytes of response data/parameters

#define HCI_REQ_TIMEOUT 5000    // 5 seconds
  ret = hci_send_req (direct_hci_btsock, &hci_req, HCI_REQ_TIMEOUT);
  if (ret < 0) {
    loge ("hci_send_req errno: %d", errno);
    hci_close_dev (direct_hci_btsock);     // Same as close (socket);
    direct_hci_btsock = -1;
    return (-103);
  }
#ifdef hci_cmd_DEBUG
  logd ("hci_send_req ret: %d", ret);
  logd ("HCI Evt: 0x%x  hci_req.rlen %d", hci_req.event, hci_req.rlen);
  //hex_dump (" ", 32, &res_buf [7], hci_req.rlen);
  hex_dump (" ", 32, res_buf, hci_req.rlen + 7);
#endif

  res_buf [0] = 0;
  res_buf [1] = 4;
  res_buf [2] = hci_req.event; //0x0e;//HCI_EV_CMD_COMPLETE;
  res_buf [3] = hci_req.rlen + 3;
  res_buf [4] = 1;

  ogf = hci_req.ogf;
  ocf = hci_req.ocf;

  res_buf [5] = oc f& 0x00ff;
  res_buf [6] = ((ocf & 0x0300) >> 8) | ((ogf & 0x3f) << 2);

  return (hci_req.rlen + 7);
}

*/

/* Our/Serial HCI response: (Ours prepends 1 byte for internal use)
04 0e 05 01 0a fc 00 3f
0   -       Status / error ?
1   0       4 always    4 for response
2   1    0x0e always 0x0e Event Code: Should be 0x0e (Command Complete)
3   2       5 remaining length after this byte (So response data max size is actually 255-4 or 251 bytes !)
4   3       1 always 1
5   4    0x0a OCF lower 8 bits 
6   5    0xfc OGF<<2 + top 2 bits OCF
7   6       0 HCI error code
8   7    0x3f Response data...
  res_buf [0] = 0;
  res_buf [1] = 4;
  res_buf [2] = 0x0e;
  res_buf [3] = hci_req.rlen + 3;
  res_buf [4] = 1;
  res_buf [5] = ocf&0x00ff;
  res_buf [6] = ((ocf&0x0300)>>8) | ((ogf&0x3f)<<2);
  res_buf [7] = hci error
  res_buf [8] = data
request packets will be aligned to match w/ OCF / OGF
    Response HCI error maps to
    Command  Length of following HCI data bytes

prepend 4 bytes to:
unsigned char hci_fm_on[] = { 0x01, 0x15, 0xfc, 0x03, 0x00, 0x00, 0x01 };   // power reg 0 = 1
*/




// IPC API called by do_hci_cmd() (for HCI) or do_client_cmd() (For inquiry, stop or set debug_extra)
int do_client_hci (unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max, int rx_tmo) {
  static int sockfd = -1;
  int res_len,written;
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
      loge ("do_client_hci: socket errno: %d", errno);
      return (0);//"Error socket");
    }
  #ifdef  CS_DGRAM_UNIX                                                 // Unix datagram sockets must be bound; no ephemeral sockets.
    unlink (api_clisock);                                                // Remove any lingering client socket
    bzero((char *) &cli_addr, sizeof (cli_addr));
    cli_addr.sun_family = AF_UNIX;
    strncpy (cli_addr.sun_path, api_clisock, sizeof (cli_addr.sun_path));
    cli_len =strlen (cli_addr.sun_path) + sizeof (cli_addr.sun_family);

    if (bind(sockfd,(struct sockaddr *)&cli_addr,cli_len) < 0) {
      loge ("do_client_hci: bind errno: %d", errno);
      close (sockfd);
      sockfd = -1;
      return (0);//"Error bind");                                        // OK to continue w/ Internet Stream but since this is Unix Datagram and we ran unlink (), let's fail
    }
  #endif
  }
//!! Can move inside above
// Setup server address
  bzero((char *)&srv_addr, sizeof (srv_addr));
#ifdef  CS_AF_UNIX
  srv_addr.sun_family = AF_UNIX;
  strlcpy (srv_addr.sun_path, api_srvsock, sizeof (srv_addr.sun_path));
  srv_len = strlen (srv_addr.sun_path) + sizeof (srv_addr.sun_family);
#else
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  //hp = gethostbyname("localhost");
  //if (hp== 0) {
  //  loge ("do_client_hci: Error gethostbyname  errno: %d", errno);
  //  return (0);//"Error gethostbyname");
  //}
  //bcopy((char *)hp->h_addr, (char *)&srv_addr.sin_addr, hp->h_length);
  srv_addr.sin_port = htons(CS_PORT);
  srv_len =sizeof (struct sockaddr_in);
#endif


// Send cmd_buf and get res_buf
#ifdef CS_DGRAM
  written= sendto (sockfd, cmd_buf, cmd_len, 0,(const struct sockaddr *)&srv_addr,srv_len);
  if (written != cmd_len) {  // Dgram buffers should not be segmented
    loge ("do_client_hci: sendto errno: %d", errno);
  #ifdef  CS_DGRAM_UNIX
    unlink (api_clisock);
  #endif
    close (sockfd);
    sockfd = -1;
    return (0);//"Error sendto");
  }

  sock_rx_tmo_set (sockfd, 1000);
  res_len = recvfrom (sockfd, res_buf, res_max, 0,(struct sockaddr *)&srv_addr, &srv_len);
  if (res_len <= 0) {
    loge ("do_client_hci: recvfrom errno: %d", errno);
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
    loge ("do_client_hci: Unexpected suspicious packet from host");// %s", inet_ntop(srv_addr.sin_addr.s_addr)); //inet_ntoa(srv_addr.sin_addr.s_addr));
  }
  #endif
#else
  if (connect(sockfd, (struct sockaddr *) &srv_addr, srv_len) < 0) {
    loge ("do_client_hci: connect errno: %d", errno);
    close (sockfd);
    sockfd = -1;
    return (0);//"Error connect");
  }
  written = write (sockfd, cmd_buf, cmd_len);                           // Write the command packet
  if (written != cmd_len) {                                             // Small buffers under 256 bytes should not be segmented ?
    loge ("do_client_hci: write errno: %d", errno);
    close (sockfd);
    sockfd = -1;
    return (0);//"Error write");
  }

  sock_rx_tmo_set (sockfd, rx_tmo);

  res_len = read (sockfd, res_buf, res_max)); // Read response
  if (res_len <= 0) {
    loge ("do_client_hci: read errno: %d", errno);
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


// Do Remote or Local HCI command via do_client_hci() (For Remote via Shim Daemon) or do_acc_hci() (Access HCI via Local hcitool or BCM UART or TIFM)

int do_hci_cmd (int shim, uint8_t ogf, uint16_t ocf, unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max, int rx_tmo) {

  //char hci_cmd[MAX_HCI] = {0x01, 0x15, 0xfc, 0x03, 0x00, 0x01, 0x01};    // Get Sys Reg 0x00
  //hci_cmd[1] = ocf & 0x00ff;
  //hci_cmd[2] = ((ocf & 0x0300)>>8) | ((ogf & 0x003f)<<2);
  //hci_cmd[3] = cmd_len;
  //memcpy (&hci_cmd[4], cmd_buf, cmd_len);

  cmd_buf [4] = 1;
  cmd_buf [5] = ocf & 0x00ff;
  cmd_buf [6] = ((ocf & 0x0300) >> 8) | ((ogf & 0x3f) << 2);
  cmd_buf [7] = cmd_len - 8;

//#define DAEMON_DEBUG
#ifdef  DAEMON_DEBUG
    logd ("do_hci_cmd ogf: 0x%x  ocf: 0x%x  cmd_len: %d  res_max: %d", ogf, ocf, cmd_len, res_max);
//    hex_dump ("", 32, cmd_buf, cmd_len);
#endif

  int res_len = 0;
  if (shim)                                                            // If via shim daemon...
    res_len = do_client_hci (cmd_buf, cmd_len, res_buf, res_max, rx_tmo);
  else                                                                  // Else direct...
    res_len = do_acc_hci (cmd_buf, cmd_len, res_buf, res_max, rx_tmo);

  //int size_evt = res_buf [3]-3;

  int hci_err = res_buf [7];

#ifdef  DAEMON_DEBUG
  if (res_len > 1 + 8)
    logd ("do_hci_cmd hci_err: %d  res_len: %d  first data byte: 0x%x  last data byte: 0x%x", hci_err, res_len, res_buf [8], res_buf [res_len-1]);
  else if (res_len > 8)
    logd ("do_hci_cmd hci_err: %d  res_len: %d  one data byte: 0x%x", hci_err, res_len, res_buf [8]);
  else
    logd ("do_hci_cmd hci_err: %d %s  res_len: %d", hci_err, hci_err_get (hci_err), res_len);
  if (res_len > 0 && res_len < 1024)
//    hex_dump ("", 32, res_buf, res_len);
#endif

  if (res_len < 8 || res_len >  270)                                    // If error...
    loge ("do_hci_cmd error hci_err: %d %s  res_len: %d", hci_err, hci_err_get (hci_err), res_len);

  return (res_len);
}


/*
adb shell hcitool cmd 4 1
< HCI Command: ogf 0x04, ocf 0x0001, plen 0
> HCI Event: 0x0e plen 12
01 01 10 00 04 00 00 04 1D 00 C7 0D
*/

/* Our/Serial HCI response: (Ours prepends 1 byte for internal use)
04 0e 05 01 0a fc 00 3f
0   -       Status / error ?
1   0       4 always    4 for response
2   1    0x0e always 0x0e Event Code: Should be 0x0e (Command Complete)
3   2       5 remaining length after this byte (So response data max size is actually 255-4 or 251 bytes !)
4   3       1 always 1
5   4    0x0a OCF lower 8 bits 
6   5    0xfc OGF<<2 + top 2 bits OCF
7   6       0 HCI error code
8   7    0x3f Response data...
  res_buf [0] = 0;
  res_buf [1] = 4;
  res_buf [2] = 0x0e;
  res_buf [3] = hci_req.rlen + 3;
  res_buf [4] = 1;
  res_buf [5] = ocf&0x00ff;
  res_buf [6] = ((ocf&0x0300)>>8) | ((ogf&0x3f)<<2);
  res_buf [7] = hci error
  res_buf [8] = data
request packets will be aligned to match w/ OCF / OGF
    Response HCI error maps to
    Command  Length of following HCI data bytes

prepend 4 bytes to:
unsigned char hci_fm_on[] = { 0x01, 0x15, 0xfc, 0x03, 0x00, 0x00, 0x01 };   // power reg 0 = 1
*/


  static volatile int hci_cmd_lock = 0;                                 // Need a lock due to usage of non-thread safe hci_cmd_* routines; hcitool, do_client_hci etc. Also HCI Events are promiscuous
                                                                        // Still needed or get errors from simultaneous commands

int hci_cmd (uint8_t ogf, uint16_t ocf, unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max) {
  int hci_cmd_start_time = 0;

//if (cmd_len > 8)
//hex_dump ("", 32, cmd_buf + 8, cmd_len - 8);

  if (hci_dbg) {
    hci_cmd_start_time = ms_get ();
    logd ("hci_cmd ogf: 0x%x  ocf: 0x%x  cmd_len: %d  res_max: %d", ogf, ocf, cmd_len, res_max);
    hex_dump ("", 32, cmd_buf, cmd_len);
  }
  int res_len;
  if (res_max > 252)
    res_max = 252;
  //if (cmd_len > 255 || cmd_len < 8) {
  if (cmd_len > 263 || cmd_len < 8) {                                   // !! For ti_bulk_hci_write
    loge ("hci_cmd error cmd_len: %d", cmd_len);
    return (0);
  }

  if (lock_open ("hci_cmd", & hci_cmd_lock, 1000))//300))              // Acquire HCI command lock within 1 (was 0.3) second
    return (-1);                                                        // Done w/ error if timeout

  res_buf [7] = 0xfe;                                                   // Put something in HCI error field in case no response


  if (shim_hci_enable)                                                 // If hci command to Shim...
    res_len = do_hci_cmd (1, ogf, ocf, cmd_buf, cmd_len, res_buf, res_max, 1000);
  else                                                                  // If hci command to UART...
    res_len = do_hci_cmd (0, ogf, ocf, cmd_buf, cmd_len, res_buf, res_max, 1000);

  int hci_err = res_buf [7];
  if (hci_dbg) {
    if (res_len > 1 + 8)
      logd ("hci_cmd hci_err: %d %s   res_len: %d  first data byte: 0x%x  last data byte: 0x%x", hci_err, hci_err_get (hci_err), res_len, res_buf [8], res_buf [res_len-1]);
    else if (res_len > 8)
      logd ("hci_cmd hci_err: %d %s   res_len: %d  one data byte: 0x%x", hci_err, hci_err_get (hci_err), res_len, res_buf [8]);
    else
      logd ("hci_cmd hci_err: %d %s  res_len: %d", hci_err, hci_err_get (hci_err), res_len);
    //if (res_len > 0 && res_len < 1024)
    //  hex_dump ("", 32, res_buf, res_len);
  }
  else if (hci_err || res_len < 8 || res_len >= 270) {
    loge ("hci_cmd error res_len: %d  hci_err: %d %s", res_len, hci_err, hci_err_get (hci_err));     // Display the error
    int fix_res_len = res_len;
    if (fix_res_len < 8 || fix_res_len >= 270)
      fix_res_len = 16;
    //hex_dump ("", 32, res_buf, fix_res_len);
    logd ("hci_cmd failed command ogf: 0x%x  ocf: 0x%x  cmd_len: %d  res_max: %d",ogf,ocf, cmd_len, res_max);
    //hex_dump ("", 32, cmd_buf, cmd_len);
  }

  if (hci_dbg)
    logd ("hci_cmd took %d milliseconds", ms_get () - hci_cmd_start_time);
// 20 - 40/50 ms for hcitool popen2, same except spikes to 60-100+ for hcitool>file, 1-2 ms for UART
// Daemon HCI = 3/5-10/18 ms w/ open/bind/close every time, else 3-11 ms

  if (hci_err)
    res_len = 0;

  lock_close ("hci_cmd", & hci_cmd_lock);

  return (res_len);
}


int do_client_cmd (unsigned char cmd, int rx_tmo, int tmo) {
  if (tmo < 100)
    tmo = 100;
  int shim_ok_tmo = ms_get () + tmo;                                    // Wait up to tmo milliseconds, resending every rx_tmo milliseconds
  while (ms_get () < shim_ok_tmo) {
    unsigned char res_buf [DEF_BUF] = {0};
    int res_len = do_client_hci (& cmd, 1, res_buf, sizeof (res_buf), rx_tmo);    // 1 Byte: cmd
    if (res_len == 1 && res_buf [0] == cmd)
      return (0);                                                       // Daemon is ready
  }
  loge ("do_client_cmd timeout error");
  return (-1);
}



  int shim_hci_start () {
    logd ("shim_hci_start");
    if (do_client_cmd (0x73, 1000, 1200)) {                             // Inquiry / ping every 1 second for up to 1.2 seconds
      loge ("shim_hci_start error");
      return (-1);
    }
    logd ("shim_hci_start ok");
    return (0);
  }
  int shim_hci_stop () {
    logd ("shim_hci_stop");
    /*if (do_client_cmd (0x7f, 1000, 1200)) {                             // Terminate  every 1 second for up to 1.2 seconds        !!!! Leave running or problems !!!
      loge ("shim_hci_stop error");
      return (-1);
    }*/
    logd ("shim_hci_stop ok");
    return (0);
  }




enum BCM_FM_CMD {   //
  BC_REG_SYS = 0x00,        // 0x00  FM enable, RDS enable
  BC_REG_CTL,               // 0x01  Band select, mono/stereo blend, mono/stereo select
  BC_REG_RDS_CTL0,          // 0x02  RDS/RDBS, flush FIFO
  BC_REG_RDS_CTL1,          // 0x03  Not used
  BC_REG_AUD_PAUSE,         // 0x04  Pause level and time constant
  BC_REG_AUD_CTL0,          // 0x05  Mute, volume, de-emphasis, route parameters, BW select
  BC_REG_AUD_CTL1,          // 0x06  Mute, volume, de-emphasis, route parameters, BW select
  BC_REG_SRCH_CTL0,         // 0x07  Search parameters such as stop level, up/down
  BC_REG_SRCH_CTL1,         // 0x08  Not used (except on BCM4330/20780 2 bytes: 0x0206)
  BC_REG_SRCH_TUNE_MODE,    // 0x09  Search/tune mode and stop
  BC_REG_FREQ0,             // 0x0a  Set and get frequency
  BC_REG_FREQ1,             // 0x0b  Set and get frequency
  BC_REG_AF_FREQ0,          // 0x0c  Set alternate jump frequency
  BC_REG_AF_FREQ1,          // 0x0d  Set alternate jump frequency
  BC_REG_CARRIER,           // 0x0e  IF frequency error
  BC_REG_RSSI,              // 0x0f  Received signal strength indicator
  BC_REG_MSK0,              // 0x10  FM and RDS IRQ mask register
  BC_REG_MSK1,              // 0x11  FM and RDS IRQ mask register
  BC_REG_FLG0,              // 0x12  FM and RDS flag register
  BC_REG_FLG1,              // 0x13  FM and RDS flag register
  BC_REG_RDS_WLINE,         // 0x14  FIFO water line set level
  BC_REG_RDS_BLKB_MATCH0,   // 0x16  Block B match pattern
  BC_REG_RDS_BLKB_MATCH1,   // 0x17  Block B match pattern
  BC_REG_RDS_BLKB_MASK0,    // 0x18  Block B mask pattern
  BC_REG_RDS_BLKB_MASK1,    // 0x19  Block B mask pattern
  BC_REG_RDS_PI_MATCH0,     // 0x1a  PI match pattern
  BC_REG_RDS_PI_MATCH1,     // 0x1b  PI match pattern
  BC_REG_RDS_PI_MASK0,      // 0x1c  PI mask pattern
  BC_REG_RDS_PI_MASK1,      // 0x1d  PI mask pattern
  BC_REG_BOOT,              // 0x1e  FM_BOOT register
  BC_REG_TEST,              // 0x1f  FM_TEST register
  BC_REG_SPARE0,            // 0x20  Spare register #0
  BC_REG_SPARE1,            // 0x21  Spare register #1
                            // 0x21 - 0x26 Reserved
                            // 0x27  ??
  BC_REG_REV_ID = 0x28,     // 0x28  Revision ID of the FM demodulation core                                0xff                BCM2048_I2C_FM_RDS_REV
  BC_REG_SLAVE_CONF,        // 0x29  Enable/disable I2C slave. Configure I2C slave address
                            // 0x2a - 0x4c Reserved
  BC_REG_PCM_ROUTE = 0x4d,  // 0x4d  Controls routing of FM audio output to either PCM or Bluetooth SCO     0x44 or 0x40 on power-up
                            // 0x4e - 0x7f Reserved
  BC_REG_RDS_DATA = 0x80,   // 0x80  Read RDS tuples(3 bytes each)
                            // 0x81 - 0x8f Reserved
  BC_REG_BEST_TUNE = 0x90,  // 0x90  Best tune mode enable/disable for AF jump
                            // 0x91-0xfb Reserved
  BC_REG_SRCH_METH = 0xfc,  // 0xfc  Select search methods: normal, preset, RSSI monitoring
  BC_REG_SRCH_STEPS,        // 0xfd  Adjust search steps in units of 1kHz to 100kHz
  BC_REG_MAX_PRESET,        // 0xfe  Sets the maximum number of preset channels found for FM scan command
  BC_REG_PRESET_STATION,    // 0xff  Read the number of preset stations returned after a FM scan command
};

/* LG G2:

Writes:

    reg_set (reg  |       0,  val );                                    // Read 1 byte
    reg_set (reg  | 0x10000,  val );                                    // Read 2 bytes
    reg_set (reg  | 0x20000,  val );                                    // Read 4 bytes

 0, 03
 2, 02
fb, 00000000
 1, 02
            reg_get (0x4d           = 0x44
 5, 005c
fd, 0064
14, 40
            bulk_get (0x80, 0xf0)
            reg_get (0x12+10000     = 0x0008
10, 0200
 a, 5fb4
 9, 01
 

    reg_set (0x00 |       0,  0x03);                                    // FM + RDS = On        00 00 03 
    reg_set (0x02 |       0,  0x02);                                    //  RDS + Flush         02 00 02 
    reg_set (0xfb | 0x20000,  0x00000000);                              // Audio PCM ????       fb 00 00 00 00 00 
    reg_set (0x01 |       0,  0x02);                                    // NotJap + Stro Blnd   01 00 02 
    reg_set (0x05 | 0x10000,  0x005c);                                  // 75 DAC No Mute       05 00 5c 00 
    reg_set (0x05 | 0x10000,  0x006c);                                  // 75 I2S No Mute       05 00 6c 00 
    reg_set (0x0a | 0x10000,  0x5fb4);                                  // Freq 88.5            0a 00 b4 5f 

    reg_set (0x10 | 0x10000,  0x0003);                                  // IRQ MASKS            10 00 03 00 
    reg_set (0x09 |       0,  0x01);                                    // SrchTune Mode Prst   09 00 01 
    reg_set (0xf8 | 0x10000,  0x00ff);                                  // Vol Max              f8 00 ff 00 
    reg_set (0x14 |       0,  0x40);                                    // RDS WLINE 64         14 00 40 
    reg_set (0x10 | 0x10000,  0x0200);                                  // IRQ MASKS            10 00 00 02 
    reg_set (0x02 |       0,  0x03);                                    // RBDS + Flush         02 00 03 
    reg_set (0x0a | 0x10000,  0x5fb4);                                  // Freq 88.5            0a 00 b4 5f 

    reg_set (0x10 | 0x10000,  0x0003);                                  // IRQ MASKS            10 00 03 00 
    reg_set (0x09 |       0,  0x01);                                    // SrchTune Mode Prst   09 00 01 
    reg_set (0x10 | 0x10000,  0x0200);                                  // IRQ MASKS            10 00 00 02 
    reg_set (0x10 | 0x10000,  0x0200);                                  // IRQ MASKS            10 00 00 02 
    reg_set (0x10 | 0x10000,  0x0200);                                  // IRQ MASKS            10 00 00 02 

*/
// reg_set (0x00, BC_REG_SYS bits:
#define BC_VAL_SYS_OFF                      0x00
#define BC_VAL_SYS_FM                       0x01
#define BC_VAL_SYS_RDS                      0x02

// reg_set (0x01, BC_REG_CTL bits:
#define BC_VAL_CTL_BND_EUROPE_US            0x00
#define BC_VAL_CTL_BND_JAPAN                0x01    // BCM2048_BAND_SELECT

#define BC_VAL_CTL_MANUAL                   0x00
#define BC_VAL_CTL_AUTO                     0x02    // BCM2048_STEREO_MONO_AUTO_SELECT

#define BC_VAL_CTL_MONO                     0x00
#define BC_VAL_CTL_STEREO                   0x04    // BCM2048_STEREO_MONO_MANUAL_SELECT

#define BC_VAL_CTL_BLEND                    0x00
#define BC_VAL_CTL_SWITCH                   0x08    // BCM2048_STEREO_MONO_BLEND_SWITCH

#define BCM2048_HI_LO_INJECTION			0x10

/* BCM2048_I2C_RDS_CTRL0 */
#define BCM2048_RBDS_RDS_SELECT		0x01
#define BCM2048_FLUSH_FIFO		0x02

/* BCM2048_I2C_FM_AUDIO_PAUSE */
#define BCM2048_AUDIO_PAUSE_RSSI_TRESH	0x0f
#define BCM2048_AUDIO_PAUSE_DURATION	0xf0



// reg_set (0x05, BC_REG_AUD_CTL0 bits:
#define BC_VAL_AUD_CTL0_RF_MUTE_DISABLE     0x00
#define BC_VAL_AUD_CTL0_RF_MUTE_ENABLE      0x01

#define BC_VAL_AUD_CTL0_MANUAL_MUTE_OFF     0x00
#define BC_VAL_AUD_CTL0_MANUAL_MUTE_ON      0x02

#define BC_VAL_AUD_CTL0_DAC_OUT_LEFT_OFF    0x00
#define BC_VAL_AUD_CTL0_DAC_OUT_LEFT_ON     0x04

#define BC_VAL_AUD_CTL0_DAC_OUT_RIGHT_OFF   0x00
#define BC_VAL_AUD_CTL0_DAC_OUT_RIGHT_ON    0x08

#define BC_VAL_AUD_CTL0_ROUTE_DAC_DISABLE   0x00
#define BC_VAL_AUD_CTL0_ROUTE_DAC_ENABLE    0x10

#define BC_VAL_AUD_CTL0_ROUTE_I2S_DISABLE   0x00
#define BC_VAL_AUD_CTL0_ROUTE_I2S_ENABLE    0x20

#define BC_VAL_AUD_CTL0_DEMPH_50US          0x00
#define BC_VAL_AUD_CTL0_DEMPH_75US          0x40

#define BCM2048_AUDIO_BANDWIDTH_SELECT	0x80


// reg_set (0x07, BC_REG_SRCH_CTL0 bits:
#define BC_VAL_SRCH_CTL0_DOWN               0x00
#define BC_VAL_SRCH_CTL0_UP                 0x80
#define BC_VAL_SRCH_CTL0_RSSI_60DB          0x5e        //94
#define BC_VAL_SRCH_CTL0_RSSI_MID           0x40        // ?? Is this my define ?

/* BCM2048_I2C_FM_SEARCH_CTRL0 */
#define BCM2048_SEARCH_RSSI_THRESHOLD	0x7f
#define BCM2048_SEARCH_DIRECTION	0x80


// reg_set (0x09, BC_REG_SRCH_TUNE_MODE:
#define BC_REG_SRCH_TUNE_MODE_TERMINATE     0x00
#define BC_VAL_SRCH_TUNE_MODE_PRESET        0x01
#define BC_REG_SRCH_TUNE_MODE_AUTO          0x02
#define BC_REG_SRCH_TUNE_MODE_AF_JUMP       0x03        // BCM2048_FM_AUTO_SEARCH

// reg_set (0x12, BC_REG_FLG0 / BC_REG_MSK0 bits:
#define BC_VAL_FLG0_SRCH_TUNE_FINISHED      0x01
#define BC_VAL_FLG0_SRCH_TUNE_FAIL          0x02
#define BC_VAL_FLG0_RSSI_LOW                0x04
#define BC_VAL_FLG0_CARRIER_ERROR_HIGH      0x08
#define BC_VAL_FLG0_AUDIO_PAUSE_INDICATION  0x10
#define BC_VAL_FLG0_STEREO_DETECTION        0x20
#define BC_VAL_FLG0_STEREO_ACTIVE           0x40

// reg_set (0x13, BC_REG_FLG1 / BC_REG_MSK1 bits:
#define BC_VAL_FLG1_RDS_FIFO_WLINE          0x02
#define BC_VAL_FLG1_RDS_B_BLOCK_MATCH       0x08
#define BC_VAL_FLG1_RDS_SYNC_LOST           0x10
#define BC_VAL_FLG1_RDS_PI_MATCH            0x20

// reg_set (0xfc, BC_REG_SRCH_METH:
#define BC_VAL_SRCH_METH_NORMAL             0x00
#define BC_VAL_SRCH_METH_PRESET             0x01
#define BC_VAL_SRCH_METH_RSSI               0x02


/* BCM2048_I2C_RDS_DATA */      // Reg 0x80
#define BCM2048_SLAVE_ADDRESS			0x3f
#define BCM2048_SLAVE_ENABLE			0x80

/* BCM2048_I2C_FM_BEST_TUNE_MODE */     // Reg 0x90
#define BCM2048_BEST_TUNE_MODE			0x80


#define BCM2048_RDS_MARK_END_BYTE0		0x7C
#define BCM2048_RDS_MARK_END_BYTEN		0xFF

#define BCM2048_DEFAULT_TIMEOUT		1500
#define BCM2048_AUTO_SEARCH_TIMEOUT	3000

#define BCM2048_FREQDEV_UNIT		10000

#define BCM2048_DEFAULT_POWERING_DELAY	20  // Sleep 20 milliseconds after power on

int reg_disable = 0;//1;

// For RDS: (but on BCM can read multiple registers at once!)
int bulk_get (int reg, int size, unsigned char * res_buf, int res_max) {
  if (reg_disable)
    return (0);
  int res_len = -1;
  if (size > 252 || size < 0) {
    loge ("bulk_get error size: %d", size);
    return (0);
  }
  //char * val = NULL;
  //int got = 0;
  unsigned char cmd_buf [MAX_HCI] = {0};//3 + 8] = {0};
  int  cmd_buf_size = 3 + 8;// ! 11 works for both TI and BCM !           sizeof (cmd_buf);

  if (reg_dbg)
    logd ("bulk_get reg: %x  size: %d", reg, size);

  cmd_buf [8] = reg;

      cmd_buf [9] = 1;                                                   // a hcitool cmd 3f 15 reg 1(read) size
      cmd_buf [10] = 0xff & size;
      res_len = hci_cmd (0x3f, 0x15, cmd_buf, cmd_buf_size, res_buf, res_max);
      if (res_len >= 3 + 8 && ! res_buf [7]) {                           // !! ?? Actual value ??
        //val = &res_buf [10];
        //got = res_len-3;
        memcpy (&res_buf [8],&res_buf [10], res_len-8-2);   // Copy data down 2 bytes to match TI response w/ bulk data starting at res_buf [8]  // !!! Assume data is copied from bottom up since we are copying in place
        res_len -= 2;
      }

  if (res_len <= 0)
    return (0);
  if (! res_buf [7])
    return (res_len);

  loge ("bulk_get hci error: %d %s  reg: 0x%x  size: %d", res_buf [7], hci_err_get (res_buf [7]), reg, size);     // Display the error
  return (0);
}


  int reg_verify = 0;

  int reg_get (int reg);

int reg_set (int reg, int val) {     // !! So far nothing TI checks/uses the return value, BC does)
  if (reg_disable)
    return (0);
  unsigned char res_buf [MAX_HCI];

  int res_len = -1;
  //char bcw_buf [] = {0, 0, 0};                                          // reg, 0, val
  //char tiw_buf [] = {0x01, 0x02, 0x00, 0x00, 0x00};                       // reg, 2, 0, valhi, vallo

  unsigned char cmd_buf [MAX_HCI] = {0};//5+8] = {0};   // !! Only supports up to 3 bytes on BC !!!!!!!!!!!!!!!!!!!!!!!!
  
  //ms_sleep (100);   //100 ms to stabilize

  if (reg_dbg)
    logd ("reg_set reg: %x  val: 0x%x (%d)", reg, val, val);

  int size = 1;
  if (reg >= 65536) {
    if (reg >= 131072) {
      size = 4;
      reg -= 131072;
    }
    else {
      size = 2;
      reg -= 65536;
    }
  }
  cmd_buf [8] = reg;
      //if (val>255 || val<0) {
      //  loge ("reg_set val out of range reg: 0x%x  val: 0x%x (%d)  api_mode: %d", reg, val, val, api_mode);
      //  return (-1);
      //}
      cmd_buf [9] = 0;        // a hcitool cmd 3f 15 reg 0(write) val
      if (size == 2) {
        cmd_buf [11] = val / 256;
        cmd_buf [10] = val % 256;
      }
      else if (size == 4) {
        cmd_buf [13] = (0xff) & (val >> 24);//(val/16777216);
        cmd_buf [12] = (0xff) & (val >> 16);//(val/65536);
        cmd_buf [11] = (0xff) & (val >> 8);//(val/256);
        cmd_buf [10] = (0xff) & (val >> 0);//(val%256);
      }
      else {
        cmd_buf [10] = val;
      }
      res_len = hci_cmd (0x3f, 0x15, cmd_buf, size + 2 + 8, res_buf, sizeof (res_buf));            // BC HCI: 0x15
      //if (res_len >= 4 + 8 && ! res_buf [7])    // !!!! WHY IS THIS HERE ???? REMOVE !!!!
      //  val = res_buf [10];                 // !!!! WHY IS THIS HERE ???? REMOVE !!!!
  //if (reg_dbg)
  //  logd ("reg_set res_len: %d", res_len);
  if (res_len < 8) {
    loge ("reg_set hci_cmd error res_len: %d hci error: %d %s  reg: 0x%x  val: 0x%x", res_len, res_buf [7], hci_err_get (res_buf [7]), reg, val);
    return (-1);//return (0);
  }
  if (! res_buf [7]) {

    if (reg_verify) {
      int read = 0;
      if (size == 2)                                                    // If 2 bytes...
        read = reg_get (reg | 0x10000);                                 // Read 2 bytes
      else if (size == 4)                                               // If 4 bytes...
        read = reg_get (reg | 0x20000);                                 // Read 4 bytes
      else
        read = reg_get (reg);

      if (read != val) {                                                // If read value different than write value...
          if (reg == 0x01 && (val & ~0x20) == read)
            return (0);

        loge ("reg_set verify error reg: 0x%x  write val: 0x%x  read val: 0x%x", reg, val, read);
        return (-1);
      }
    }
    return (0);
  }
  loge ("reg_set hci error: %d %s  reg: 0x%x  val: 0x%x", res_buf [7], hci_err_get (res_buf [7]), reg, val);
  return (res_buf [7]);
}

int reg_get_err_enab = 1;        // Displays reg_get () error messages; disable for api_mode_get () tests.
int reg_get (int reg) {  // If error, return 0 instead of error code.
  if (reg_disable)
    return (0);
  unsigned char res_buf [MAX_HCI];
  int val = 0, res_len = -1;

  unsigned char cmd_buf [MAX_HCI] = {0};//3+8] = {};; //= {0xff, 0x01, 0x01};    // TI: {0xff, 0x02, 0x00};

  if (reg_dbg)
    logd ("reg_get reg: 0x%x", reg);//  api_mode: %d", reg, api_mode);

  int size = 1;
  if (reg >= 65536) {
    if (reg >= 131072) {
      size = 4;
      reg -= 131072;
    }
    else {
      size = 2;
      reg -= 65536;
    }
  }

  cmd_buf [8] = reg;
      cmd_buf [9] = 1;        // a hcitool cmd 3f 15 reg 1(read) 1(size)
      cmd_buf [10] = size;//1;
      res_len = hci_cmd (0x3f, 0x15, cmd_buf, 3 + 8, res_buf, sizeof (res_buf));          // BC HCI: 0x15
      //if (res_len >= 3+8 && ! res_buf [7])
      if (size == 2)
        val = 256 * res_buf [11] + res_buf [10];
      else if (size == 4)
        //val = 16777216 * res_buf [13] + 65536 * res_buf [12] + 256 * res_buf [11] + res_buf [10];
        val = res_buf [13] << 24 + res_buf [12] << 16 + res_buf [11] << 8 + res_buf [10] << 0;
      else
        val = res_buf [10];
  //if (reg_dbg)
  //  logd ("reg_get res_len: %d", res_len);
  if (res_len < 8) {
    if (reg_get_err_enab)                           // If we display reg_get () error messages
      loge ("reg_get hci_cmd error: %d hci error: %d %s  reg: 0x%x  val: 0x%x", res_len, res_buf [7], hci_err_get (res_buf [7]), reg, val);    // Display the error
    return (0);
  }
  if (! res_buf [7]) {
    if (reg_dbg)
      logd ("reg_get reg: 0x%x  val: 0x%x (%d)", reg, val, val);
    return (val);
  }
  if (reg_get_err_enab)                             // If we display reg_get () error messages
    loge ("reg_get hci error: %d %s  reg: 0x%x  val: 0x%x", res_buf [7], hci_err_get (res_buf [7]), reg, val);     // Display the error
  return (0);
}


/*

	if (bdev->power_state) {
// Report frequencies with high carrier errors as zero signal level
		f_error = bcm2048_get_fm_carrier_error(bdev);
		if (f_error < BCM2048_FREQ_ERROR_FLOOR ||
		    f_error > BCM2048_FREQ_ERROR_ROOF) {
			tuner->signal = 0;
		} else {
// RSSI level -60 dB is defined to report full signal strength

			rssi = bcm2048_get_fm_rssi(bdev);
			if (rssi >= BCM2048_RSSI_LEVEL_BASE) {
				tuner->signal = 0xFFFF;
			} else if (rssi > BCM2048_RSSI_LEVEL_ROOF) {
				tuner->signal = (rssi +
						 BCM2048_RSSI_LEVEL_ROOF_NEG)
						 * BCM2048_SIGNAL_MULTIPLIER;
			} else {
				tuner->signal = 0;
			}
		}
	} else {
		tuner->signal = 0;
	}


Seek:

	//bcm2048_set_fm_search_mode_direction (bdev, seek->seek_upward);
	reg_set (0x07, 0xc0);     // Up, mid RSSI

    //flags =	BCM2048_FM_FLAG_SEARCH_TUNE_FINISHED | BCM2048_FM_FLAG_SEARCH_TUNE_FAIL;
    //bcm2048_send_command(bdev, BCM2048_I2C_FM_RDS_MASK0, flags);      //#define BCM2048_I2C_FM_RDS_MASK0	0x10

    reg_set (0x10, 0x03);

    //mode = BCM2048_FM_PRE_SET_MODE              for chip_imp_freq_set        1.5 second timeout
    //mode = BCM2048_FM_AUTO_SEARCH_MODE          for chip_imp_seek_start      3 second timeout
    //bcm2048_send_command(bdev, BCM2048_I2C_FM_SEARCH_TUNE_MODE, mode;

    reg_set (0x09, 2);

    Wait...
    
*/

    

  int dev_hndl      =     -1;
  int intern_band     =      0;   // 0 = EU, 1 = US
  extern int curr_freq_val;
  extern int curr_freq_inc;
  extern int curr_freq_lo;
  extern int curr_freq_hi;

  int chip_imp_api_on (int freq_lo, int freq_hi, int freq_inc) {
    logd ("chip_imp_api_on freq_lo: %d  freq_hi: %d  freq_inc: %d", freq_lo, freq_hi, freq_inc);

    hci_cmd_lock = 0;
    curr_freq_lo  = freq_lo;
    curr_freq_hi  = freq_hi;
    curr_freq_inc = freq_inc;

    shim_hci_enable = 0;                                                // Default access = UART

    struct stat sb;
    if (stat ("/system/lib/libbt-hci.so", & sb) == 0) {                 // If Bluedroid file exists...
      if (sb.st_size > 60000 && file_get ("/system/lib/libbt-hcio.so")){// If our lib and have old lib to call  (If just large but no old, assume original
        if (file_get ("/dev/ttyHS0") || file_get ("/dev/ttyHS99")) {
          //if (bt_get ())                                              // com.android.bluetooth always runs after BT turned on, even when off
          if (file_get ("/data/data/fm.a2d.sf/files/use_shim")) {
            logd ("chip_imp_api_on will use SHIM");
            shim_hci_enable = 1;                                        // Use shim
          }
        }
      }
    }
    else if (stat ("/system/vendor/lib/libbt-vendor.so", & sb) == 0) {                 // If Bluedroid file exists...
      if (sb.st_size > 60000 && file_get ("/system/vendor/lib/libbt-vendoro.so")){// If our lib and have old lib to call  (If just large but no old, assume original
        if (file_get ("/dev/ttyHS0") || file_get ("/dev/ttyHS99")) {
          //if (bt_get ())                                              // com.android.bluetooth always runs after BT turned on, even when off
          if (file_get ("/data/data/fm.a2d.sf/files/use_shim")) {
            logd ("chip_imp_api_on will use SHIM");
            shim_hci_enable = 1;                                        // Use shim
          }
        }
      }
    }


    int ret = 0;
    if (shim_hci_enable) {
      ret = shim_hci_start ();
//bc_g2_pcm_set ();
    }
    else
      ret = uart_hci_start ();

/* 
    int ret = shim_hci_start ();                                        // Always try shim mode first (Can't use, doesn't return error
    loge ("shim_hci_start (): %d", ret);
    if (ret == 0)
      return (ret);
    else
      return (uart_hci_start ());
*/
  }

  int chip_imp_api_off () {
    hci_cmd_lock = 0;
    if (shim_hci_enable)
      return (shim_hci_stop ());
    else
      return (uart_hci_stop ());
  }


/*
void test_stuff() {
    // bcm2048_set_fm_automatic_stereo_mono
  reg_set (0x01, 0x02);               //BCM2048_I2C_FM_CTRL |= BCM2048_STEREO_MONO_AUTO_SELECT (BC_VAL_CTL_AUTO)
}

    // BCM2048 default initialization sequence
int bcm2048_init(struct bcm2048_device *bdev) {
	int err;
	err = bcm2048_set_power_state (bdev, BCM2048_POWER_ON);
	if (err < 0)
		goto exit;
	err = bcm2048_set_audio_route (bdev, BCM2048_AUDIO_ROUTE_DAC);
	if (err < 0)
		goto exit;
	err = bcm2048_set_dac_output (bdev, BCM2048_DAC_OUTPUT_LEFT |
		BCM2048_DAC_OUTPUT_RIGHT);
exit:
	return err;
}

    // BCM2048 default deinitialization sequence
int bcm2048_deinit(struct bcm2048_device *bdev) {
	int err;
	err = bcm2048_set_audio_route(bdev, 0);
	if (err < 0)
		goto exit;
	err = bcm2048_set_dac_output(bdev, 0);
	if (err < 0)
		goto exit;
	err = bcm2048_set_power_state(bdev, BCM2048_POWER_OFF);
	if (err < 0)
		goto exit;
exit:
	return err;
}

    // BCM2048 probe sequence
int bcm2048_probe(struct bcm2048_device *bdev) {
	int err;
	err = bcm2048_set_power_state(bdev, BCM2048_POWER_ON);
	if (err < 0)
		goto unlock;
	err = bcm2048_checkrev(bdev);
	if (err < 0)
		goto unlock;
	err = bcm2048_set_mute(bdev, BCM2048_DEFAULT_MUTE);
	if (err < 0)
		goto unlock;
	err = bcm2048_set_region (bdev, BCM2048_DEFAULT_REGION);
	if (err < 0)
		goto unlock;
	err = bcm2048_set_fm_search_rssi_threshold(bdev,
					BCM2048_DEFAULT_RSSI_THRESHOLD);
	if (err < 0)
		goto unlock;
	err = bcm2048_set_fm_automatic_stereo_mono(bdev, BCM2048_ITEM_ENABLED);
	if (err < 0)
		goto unlock;
	err = bcm2048_get_rds_wline(bdev);
	if (err < BCM2048_DEFAULT_RDS_WLINE)
		err = bcm2048_set_rds_wline(bdev, BCM2048_DEFAULT_RDS_WLINE);
	if (err < 0)
		goto unlock;
	err = bcm2048_set_power_state(bdev, BCM2048_POWER_OFF);
unlock:
	return err;
}
*/

/*
void bc_reg_dump (int lo, int hi, int bytes) {
  ms_sleep (20);
  int reg = 0, val = 0;
  for (reg = lo; reg <= hi; reg += bytes) {
    if (bytes == 1)
      val = reg_get (reg);
    else if (bytes == 2)
      val = reg_get (reg | 0x10000);
    else
      val = reg_get (reg | 0x20000);

    logd ("bc_reg_dump 0x%x: 0x%x (%d)", reg, val, val);
    ms_sleep (20);
  }
}
*/

int bc_reg_ctl       = 0;                                               // BC_REG_CTL       // 0x01  Band select, mono/stereo blend, mono/stereo select
int bc_reg_aud_ctl0  = 0;                                               // BC_REG_AUD_CTL0  // 0x05  Mute, volume, de-emphasis, route parameters, BW select

/* OneV w/ 4330                                                         DHD 4329
1:
0...
09-01 23:53:07.606 D/fm_hrdw (15238): bc_reg_dump 0x5: 0x2 (2)
0...
09-01 23:53:07.687 D/fm_hrdw (15238): bc_reg_dump 0x8: 0x6 (6)          0
0...
09-01 23:53:07.877 D/fm_hrdw (15238): bc_reg_dump 0xf: 0xde (222)       0x87
0...
09-02 00:11:59.591 D/fm_hrdw (15795): bc_reg_dump 0x20: 0x1 (1)
09-02 00:11:59.611 D/fm_hrdw (15795): bc_reg_dump 0x21: 0x58 (88)       0x50
0...
09-02 00:11:59.811 D/fm_hrdw (15795): bc_reg_dump 0x28: 0xff (255)
09-02 00:11:59.831 D/fm_hrdw (15795): bc_reg_dump 0x29: 0x22 (34)
0...
09-02 00:11:59.891 D/fm_hrdw (15795): bc_reg_dump 0x2b: 0x3a (58)
0...
09-02 00:12:00.061 D/fm_hrdw (15795): bc_reg_dump 0x30: 0x1d (29)
09-02 00:12:00.081 D/fm_hrdw (15795): bc_reg_dump 0x31: 0x2a (42)
09-02 00:12:00.111 D/fm_hrdw (15795): bc_reg_dump 0x32: 0x4a (74)
09-02 00:12:00.141 D/fm_hrdw (15795): bc_reg_dump 0x33: 0x24 (36)
09-02 00:12:00.161 D/fm_hrdw (15795): bc_reg_dump 0x34: 0xea (234)
0...
09-02 00:12:00.221 D/fm_hrdw (15795): bc_reg_dump 0x36: 0x60 (96)
09-02 00:12:00.241 D/fm_hrdw (15795): bc_reg_dump 0x37: 0x88 (136)      0
09-02 00:12:00.271 D/fm_hrdw (15795): bc_reg_dump 0x38: 0x48 (72)       0
0...                                                                    3b=xd
09-02 00:12:00.432 D/fm_hrdw (15795): bc_reg_dump 0x3e: 0x14 (20)       0x3a
09-02 00:12:00.462 D/fm_hrdw (15795): bc_reg_dump 0x3f: 0x72 (114)
0...                                                                    42=x14,x4
09-02 00:12:00.602 D/fm_hrdw (15795): bc_reg_dump 0x44: 0xab (171)
09-02 00:12:00.632 D/fm_hrdw (15795): bc_reg_dump 0x45: 0x32 (50)
0...
09-02 00:12:00.712 D/fm_hrdw (15795): bc_reg_dump 0x48: 0x98 (152)
0...
09-02 00:12:00.762 D/fm_hrdw (15795): bc_reg_dump 0x4a: 0x12 (18)       0xd
0...                                                                    4c=xa0
09-02 00:12:00.852 D/fm_hrdw (15795): bc_reg_dump 0x4d: 0x44 (68)
0...
0x50 = 0x50

2 only diff:
09-01 23:53:08.457 D/fm_hrdw (15238): bc_reg_dump 0x8: 0x206 (518)          2 byte

4:
0...
*/

// 09-01 23:53:07.386 D/fm_hrdw (15238): hci_manuf_get HCI Version: 6  Revision: 4786  LMP Version: 6  Subversion: 16643  Manuf: 15
// 4786 = 0x12b2
// 16643 = 0x4103
// 0x00: Bluetooth HCI Specification 1.0
// LMP = Link Manager Protocol
// Revision = Manufacturer controlled
// Others = BT SIG standards

/* Good after BT:
09-01 23:52:59.589 D/fm_hrdw (15238): uart_baudrate_get termios_osp: 4098  termios_isp: 4098
09-01 23:52:59.589 D/fm_hrdw (15238): uart_baudrate_get: 115200
09-01 23:52:59.589 D/fm_hrdw (15238): baudrate_set: 115200
09-01 23:52:59.589 D/fm_hrdw (15238): reset_start
09-01 23:52:59.619 D/fm_hrdw (15238): patchram_set
...
09-01 23:52:59.619 D/fm_hrdw (15238): patchram_set fd: 85
09-01 23:53:01.631 E/fm_hrdw (15238): tmo_read timeout reached of 2000 milliseconds

09-01 23:53:01.631 D/fm_hrdw (15238): patchram_set read 1 ret: 0
09-01 23:53:01.691 D/fm_hrdw (15238): patchram_set read 3 ret: 139  len: 139
...
*/
/* Bad before BT:
09-02 00:37:30.828 D/fm_hrdw ( 3845): uart_baudrate_get termios_osp: 13  termios_isp: 13
09-02 00:37:30.828 D/fm_hrdw ( 3845): uart_baudrate_get: 9600
09-02 00:37:30.828 D/fm_hrdw ( 3845): baudrate_set: 115200
09-02 00:37:30.838 D/fm_hrdw ( 3845): reset_start
09-02 00:37:30.858 D/fm_hrdw ( 3845): patchram_set
...
09-02 00:37:30.858 D/fm_hrdw ( 3845): patchram_set fd: 98
09-02 00:37:32.870 E/fm_hrdw ( 3845): tmo_read timeout reached of 2000 milliseconds

09-02 00:37:32.870 D/fm_hrdw ( 3845): patchram_set read 1 ret: 0
09-02 00:37:32.930 D/fm_hrdw ( 3845): patchram_set read 3 ret: 139  len: 139
09-02 00:37:33.730 E/fm_hrdw ( 3845): tmo_read timeout reached of 800 milliseconds
09-02 00:37:33.730 E/fm_hrdw ( 3845): uart_recv error 1 rret: 0  flush: 0
09-02 00:37:33.730 E/fm_hrdw ( 3845): hci_xact error rret: -1
09-02 00:37:33.730 E/fm_hrdw ( 3845): patchram_set hci_xact 2 error: 139
09-02 00:37:33.730 E/fm_hrdw ( 3845): uart_start patchram_set error
*/



// 3f 00 = "Customer_Extension"
/*
01-15 19:46:39.924 D/fm_hrdw ( 2104): bc_mute_set: 0

01-15 19:46:40.064 D/fm_hrdw ( 2104): bc_g2_pcm_set 1
01-15 19:46:40.064 E/fm_hrdw ( 2104): do_acc_hci hci err: 252 Unknown HCI Error
01-15 19:46:40.064 E/fm_hrdw ( 2104): hci_cmd error res_len: 8  hci_err: 252 Unknown HCI Error
01-15 19:46:40.064 D/fm_hrdw ( 2104): 00 04 0f 04 00 01 00 fc 
01-15 19:46:40.064 D/fm_hrdw ( 2104): hci_cmd failed command ogf: 0x3f  ocf: 0x0  cmd_len: 13  res_max: 252
01-15 19:46:40.064 D/fm_hrdw ( 2104): 00 00 00 00 01 00 fc 05 f3 88 01 02 05 
01-15 19:46:40.064 E/fm_hrdw ( 2104): bc_g2_pcm_set hci error: 252 Unknown HCI Error

01-15 19:46:40.114 D/fm_hrdw ( 2104): uart_recv flushed bytes: 7
01-15 19:46:40.114 D/fm_hrdw ( 2104): ff 04 ff 04 f3 00 88 00 
01-15 19:46:40.114 D/fm_hrdw ( 2104): uart_cmd uart_recv fret: 8  flushed: 0xf3


01-15 19:46:40.194 D/fm_hrdw ( 2104): regional_set
01-15 19:46:40.194 D/fm_hrdw ( 2104): bc_rbds_set: 1
*/


int bc_g2_pcm_set () {
loge ("bc_g2_pcm_set");
      ms_sleep (50);
      reg_set (0xfb | 0x20000,  0x00000000);                            // Audio PCM ????       fb 00 00 00 00 00 
      ms_sleep (50);


//  bc_g2_pcm_set_orig ();

      int inc = 100;
      ms_sleep (50);
      reg_set (0xfd | 0x10000, inc);
      ms_sleep (50);
      reg_set (0xf8 | 0x10000,  0x00ff);                                // Vol Max              f8 00 ff 00 

  bc_g2_pcm_set_orig ();
}

int bc_g2_pcm_set_orig () {
loge ("bc_g2_pcm_set_orig");
  unsigned char res_buf [MAX_HCI];
  int res_len;
  logd ("bc_g2_pcm_set 1");     // hcitool cmd 3f 00 f3 88 01 02 05

  unsigned char hci_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 0xf3, 0x88, 0x01, 0x02, 0x05};

  res_len = hci_cmd (0x3f, 0x00, hci_buf, sizeof (hci_buf), res_buf, sizeof (res_buf));
  if (res_buf [7]) {
    loge ("bc_g2_pcm_set hci error: %d %s", res_buf [7], hci_err_get (res_buf [7]));
    return (res_buf [7]);
  }
  if (res_len < 1 + 8)
    loge ("bc_g2_pcm_set hci_cmd error res_len: %d", res_len);
  else 
    loge ("bc_g2_pcm_set OK");
  return (0);
}


  int version_sdk;
  char version_sdk_prop_buf       [DEF_BUF];



  int band_setup ();

  int curr_pwr_rds = 1;

  #define bc_freq_lo    64000 // 0x6720        For BC chip calculations. Constant regardless of band.

  #define MAX_RDS_BLOCKS 20 //40//41//40 //42

  int is_lg2 = 0;
  int is_m7  = 0;

    // Chip API:
  int chip_imp_pwr_on (int pwr_rds) {
    int ret = 0;
    logd ("chip_imp_pwr_on: %d", pwr_rds);
    curr_pwr_rds = pwr_rds;

    //logd ("chip_imp_pwr_on reg dump");
    //bc_reg_dump (0x00, 0xff, 1);
    //bc_reg_dump (0x00, 0x7f, 1);
    //bc_reg_dump (0x80, 0xff, 1);
    //bc_reg_dump (0x00, 0x4f, 1);
    //bc_reg_dump (0x00, 0x4f, 2);
    //bc_reg_dump (0x01, 0x50, 2);
    //bc_reg_dump (0x00, 0x4f, 4);

    int pwr_val = 0x01;                                                 // No RDS
    if (pwr_rds)
      pwr_val = 0x03;
    if (reg_set (0x00, pwr_val) < 0)                                    // Write power reg.
      loge ("chip_imp_pwr_on 1 error writing 0x00");
    else
      logd ("chip_imp_pwr_on 1 success writing 0x00");

    ms_sleep (20);                                                      // We are supposed to sleep 20 milliseconds here

    bc_reg_aud_ctl0 = 0;//0x23; //0x03;   //!! Mute for now     0x5c;   // radio-bcm2048.c also sets I2S     ROUTE_I2S_ENABLE |= 0x20 !!!! Test for Galaxy Tab etc.

    ret = reg_get (0x00);
    if (ret < 0) {                                                      // Read power reg.
      loge ("chip_imp_pwr_on 1 error reading 0x00  ret: %d", ret);
    }
    else
      logd ("chip_imp_pwr_on 1 success reading 0x00  ret: %d", ret);
    ms_sleep (20);                                                       // We are supposed to sleep 20 milliseconds here

    if (reg_set (0x00, pwr_val) < 0) {                                  // Write power reg again. If this fails, the rest is useless.
      loge ("chip_imp_pwr_on 2 error writing 0x00");
    }
    else
      logd ("chip_imp_pwr_on 2 success writing 0x00");
    ms_sleep (20);
    ret = reg_get (0x00);
    if (ret < 0) {                                                      // Read power reg.
      loge ("chip_imp_pwr_on 2 error reading 0x00  ret: %d", ret);
    }
    else
      logd ("chip_imp_pwr_on 2 success reading 0x00  ret: %d", ret);
    ms_sleep (20);                                                      // We are supposed to sleep 20 milliseconds here

    reg_set (0x10 | 0x10000, 0x0000);                                   // Write an Interrupt Mask of 0x0000 so we don't get Interrupt packets such as this on the UART: 04 ff 01 08

/*  //if (reg_dump) {
    logd ("chip_imp_pwr_on reg dump");
    //bc_reg_dump (0x00, 0xff, 1);
    bc_reg_dump (0x00, 0x7f, 1);
    bc_reg_dump (0x80, 0xff, 1);
  //} */

    bc_reg_ctl = 4;                                                     // Set: CTL: BND_EUROPE_US + MANUAL + STEREO + BLEND write the band setting, mono/stereo blend setting.
    bc_reg_ctl |= 0x02;                                                 // Automatic Stereo/Mono (When stereo selected)

    if (reg_set (0x01, bc_reg_ctl) < 0) {
      loge ("chip_imp_pwr_on error writing 0x01");
    }
    else
      logd ("chip_imp_pwr_on success writing 0x01");
    ms_sleep (20);

        // Always get error here ; don't need ? Default anyway ?
    if (reg_set (0x14, MAX_RDS_BLOCKS * 3) < 0) {//0x78) < 0) {                                         //0x7e);  //BC_REG_RDS_WLINE,         // 0x14  FIFO water line set level
      loge ("chip_imp_pwr_on error writing 0x14");                            // ?? Usually fails ??
    }
    else
      logd ("chip_imp_pwr_on success writing 0x14");
    ms_sleep (20);

    //reg_set (0xfb | 0x20000,  0x00000000);                              // Audio PCM ????       fb 00 00 00 00 00 


    bc_reg_aud_ctl0 = 0;//0x23; //0x03;   //!! Mute for now     0x5c;   // radio-bcm2048.c also sets I2S     ROUTE_I2S_ENABLE |= 0x20 !!!! Test for Galaxy Tab etc.

                            // Set: AUD_CTL0: RF_MUTE_DISABLE + MANUAL_MUTE_OFF + DAC_OUT_LEFT_ON + DAC_OUT_RIGHT_ON + ROUTE_DAC_ENABLE + ROUTE_I2S_DISABLE + DEMPH_75US


    //bc_reg_aud_ctl0 = 0x60; // 
    //bc_reg_aud_ctl0 = 0x5c; // 

    bc_reg_aud_ctl0 = 0x7c;//0x6c;     // 75 us, I2S, DAC, DAC left & right, no RF mute

    //bc_reg_aud_ctl0 = 0xff;

    int bc_rev_id = reg_get (0x28);                                     // REV_ID always 0xff
    logd ("chip_imp_pwr_on bc_rev_id: %d", bc_rev_id);
    //logd ("reg 0: %d", reg_get (0x00));

    chip_imp_mute_set (0);                                                  // Unmute

    //band_setup ();  // !! Done explicitly later via chip_imp_extra_cmd() with intern_band set

    //chip_stro_req_set (curr_stro_req);
    //chip_autoaf_set (curr_autoaf);
    //chip_imp_freq_set (curr_freq_val);                                    // Don't set frequency until band is set


    //!!!!!!!!!!!!!!!1
    //chip_imp_vol_set (65535);                                             // Set volume...



/*
    reg_set (0x00 |       0,  0x03);                                    // FM + RDS = On        00 00 03 
    reg_set (0x02 |       0,  0x02);                                    //  RDS + Flush         02 00 02 
ms_sleep (50);
    reg_set (0xfb | 0x20000,  0x00000000);                              // Audio PCM ????       fb 00 00 00 00 00 
    reg_set (0x01 |       0,  0x02);                                    // NotJap + Stro Blnd   01 00 02 

// Read 1 byte 0x44 from 0x4d        BC_REG_PCM_ROUTE = 0x4d,  // 0x4d  Controls routing of FM audio output to either PCM or Bluetooth SCO     0x44 or 0x40 on power-up

ms_sleep (50);
int reg = reg_get (0x4d);
//ms_sleep (50);
//reg_set (0x05 | 0x10000,  0x005c);                                  // 75 DAC No Mute       05 00 5c 00 

ms_sleep (50);
int inc = 100;
reg_set (0xfd | 0x10000, inc);
    //reg_set (0x14 |       0,  0x40);                                    // RDS WLINE 64         14 00 40 

//ms_sleep (50);
//    reg_set (0x05 | 0x10000,  0x006c);                                  // 75 I2S No Mute       05 00 6c 00 

//    reg_set (0x0a | 0x10000,  0x5fb4);                                  // Freq 88.5            0a 00 b4 5f 

//    reg_set (0x10 | 0x10000,  0x0003);                                  // IRQ MASKS            10 00 03 00 
//    reg_set (0x09 |       0,  0x01);                                    // SrchTune Mode Prst   09 00 01 
ms_sleep (50);
    reg_set (0xf8 | 0x10000,  63);//0x00ff);                                  // Vol Max              f8 00 ff 00 

    logd ("chip_imp_pwr_on done before sleep");
    ms_sleep (500);
*/

    props_log ("ro.product.board",            product_board_prop_buf);
    if (! strncasecmp (product_board_prop_buf, "GALBI", strlen ("GALBI")))        is_lg2 = 1;

    props_log ("ro.product.device",           product_device_prop_buf);
    if (! strncasecmp (product_device_prop_buf, "G2", strlen ("G2")))             is_lg2 = 1;
    if (! strncasecmp (product_device_prop_buf, "LS980", strlen ("LS980")))       is_lg2 = 1;
    if (! strncasecmp (product_device_prop_buf, "VS980", strlen ("VS980")))       is_lg2 = 1;
    if (! strncasecmp (product_device_prop_buf,  "D800", strlen ( "D800")))       is_lg2 = 1;
    if (! strncasecmp (product_device_prop_buf,  "D801", strlen ( "D801")))       is_lg2 = 1;
    if (! strncasecmp (product_device_prop_buf,  "D802", strlen ( "D802")))       is_lg2 = 1;
    if (! strncasecmp (product_device_prop_buf,  "D803", strlen ( "D803")))       is_lg2 = 1;
    if (! strncasecmp (product_device_prop_buf,   "D80", strlen (  "D80")))       is_lg2 = 1;

    props_log ("ro.product.manufacturer",           product_manuf_prop_buf);
    if (! strncasecmp (product_manuf_prop_buf,   "LG", strlen (   "LG")))       is_lg2 = 1;     // LG or LGE

    if (! strncasecmp (product_manuf_prop_buf,  "HTC", strlen (  "HTC")))       is_m7  = 1;

    props_log ("ro.build.version.sdk",        version_sdk_prop_buf);      // Android 4.4 KitKat = 19
    version_sdk = atoi (version_sdk_prop_buf);


    if (is_lg2 && shim_hci_enable == 0)   // UART only; see bt-ven.c not yet in bt-hci.c
      bc_g2_pcm_set ();
    else if (is_lg2) {                                                  // Special LG G2 stuff needed, or no audio
      ms_sleep (50);
      reg_set (0xfb | 0x20000,  0x00000000);                            // Audio PCM ????       fb 00 00 00 00 00 
      ms_sleep (50);
      //if (shim_hci_enable == 0)   // UART only; see bt-ven.c not yet in bt-hci.c
      //  bc_g2_pcm_set ();
      int inc = 100;
      ms_sleep (50);
      reg_set (0xfd | 0x10000, inc);
      ms_sleep (50);
      reg_set (0xf8 | 0x10000,  0x00ff);                                // Vol Max              f8 00 ff 00 
    }
    else {  // For HTC One M7 and Sony Z2/Z3:
      ms_sleep (50);
      reg_set (0xfb | 0x20000,  0x00000000);                            // Audio PCM ????       fb 00 00 00 00 00 
      ms_sleep (50);
      int inc = 100;
      ms_sleep (50);
      reg_set (0xfd | 0x10000, inc);
      ms_sleep (50);

      int vol_val = 0x0040;                                             // Target max about 27,000
      if (version_sdk >= 21 && file_get ("/system/framework/htcirlibs.jar")) // If HTC One M7 GPE       (Stock Android 5 too ????)
        vol_val = 0x0060;
      else if (version_sdk >= 21 && is_m7)
        vol_val = 0x0040;
      //else if (version_sdk >= 21 && is_xz2)   // Early ROM needed 0x0040 but FXP-cm-12-20150102-UNOFFICIAL-castor_windy does not
      //  vol_val = 0x0040;
      else
        vol_val = 0x00ff;

      if (vol_val != 0x00ff)
        reg_set (0xf8 | 0x10000,  vol_val);                               // Vol Max              f8 00 ff 00 
    }


    //bc_reg_dump (0x00, 0x7f, 1);

    logd ("chip_imp_pwr_on done");
    return (0);
  }

  int chip_imp_pwr_off (int pwr_rds) {
    int ret = 0;
    logd ("chip_imp_pwr_off");
    //if (pwr_rds) {

    chip_imp_mute_set (1);                                                  // Mute

    // !! Shouldn't need two mutes here !!

    // This was disabled, re-enable...
    bc_reg_aud_ctl0 = 0x02;//0x03;                                      // DACs and I2S off, mute audio + rf mute, resets band but power off so OK
    if (reg_set (0x05 | 0x10000, bc_reg_aud_ctl0) < 0) {                //
      loge ("chip_imp_pwr_off error writing 0x05");
    }

    if (reg_set (0x00, 0) < 0) {                                          // Set: SYS: OFF turn off FM
      loge ("chip_imp_pwr_off error writing 0x00");
    }
    return (0);
  }

    // Set:
  int chip_imp_freq_set (int freq) {                                        // 1 KHz resolution
    logd ("chip_imp_freq_set: %3.3d", freq);
    reg_set (0x0a | 0x10000, freq - bc_freq_lo);                        // Freq is offset from 64MHz
    //logd ("bc_freq_set reg 0: %d", reg_get (0x00));
    if ( reg_set (0x09, 1) < 0){                                      // Set: SRCH_TUNE_MODE: PRESET
      loge ("bc_freq_set error");
      return (-1);
    }
    curr_freq_val = freq;
    return (0);
  }

  int chip_imp_mute_set (int mute) {
    int ret = 0;
    logd ("chip_imp_mute_set: %3.3d", mute);
    //bc_reg_aud_ctl0 &= ~0x3f;                                           // Low 6 bits = 0
    bc_reg_aud_ctl0 &= ~0x1f;                                             // Don't turn I2S off (Or it interrupts transfer ??)
    if (mute) {
      //bc_reg_aud_ctl0 |= 0x02;//0x03;                                   // Set: AUD_CTL0: MANUAL_MUTE_ON 0x02 + RF Mute 0x01 = 0x03     No dacs, dac enable or i2s enable
      bc_reg_aud_ctl0 |= 0x03;                                            // !! Must set FM mute bit so no blast of high volume on 4330/20780 !!  ?? RF Mute ???
    }
    else {
bc_reg_aud_ctl0 |= 0x1d;//0x1c;                                     // Set: AUD_CTL0: dacs on, dac enable, no mutes, no i2s         !! Now RF mute
//bc_reg_aud_ctl0 |= 0x01;
      //!! No volume control without low bit set
      //  bc_reg_aud_ctl0 |= 0x1c;                                            // Set: AUD_CTL0: dacs on, dac enable, no mutes, no i2s
      //bc_reg_aud_ctl0 |= 0x20;                                          // Set: AUD_CTL0: + i2s !!!! Test for Galaxy Tab etc.
    }

//bc_reg_aud_ctl0 = 0x20; // I2S only
    if (reg_set (0x05 | 0x10000, bc_reg_aud_ctl0) < 0) {                  //
      loge ("bc_mute_set error writing 0x05");
    }
    return (0);
  }

/*
#define BC_VAL_CTL_MANUAL                   0x00
#define BC_VAL_CTL_AUTO                     0x02    // BCM2048_STEREO_MONO_AUTO_SELECT      Automatic only works when BCM2048_STEREO_MONO_MANUAL_SELECT = 1 = 0x04

#define BC_VAL_CTL_MONO                     0x00
#define BC_VAL_CTL_STEREO                   0x04    // BCM2048_STEREO_MONO_MANUAL_SELECT

#define BC_VAL_CTL_BLEND                    0x00
#define BC_VAL_CTL_SWITCH                   0x08    // BCM2048_STEREO_MONO_BLEND_SWITCH
*/
  int chip_imp_stro_set (int stereo) {                                        //
    int ret = 0;

//stereo = 0;
    logd ("chip_imp_stro_set: %3.3d", stereo);
    // Default 0 = Blend Auto, 1 = Switch Auto, 2 = Stereo Force, 3 = Mono Force
    bc_reg_ctl &= ~0x0e;                                                  // bits 1-3 = 0
    if (stereo != 0)
      bc_reg_ctl |= 0x06;                                                 // Set CTL: + STEREO + AUTO
    /*else if (stereo == 1)
      bc_reg_ctl |= 0x0e;                                                 // Set CTL: + STEREO + AUTO + SWITCH
    else if (stereo == 2)
      bc_reg_ctl |= 0x04;//0x0c;                                          // Set CTL: + STEREO + SWITCH (? Or 4 for STEREO only ?
    else if (stereo == 0)*/
    else
      bc_reg_ctl |= 0x00;                                                 // Set CTL: MONO
    if (reg_set (0x01, bc_reg_ctl) < 0) {
      loge ("chip_imp_stro_set error writing 0x01");
      //return (-1);
    }
    return (0);
  }


//xfc

/* 4330/20780:
// Galaxy Mini S5570 : v111001 -> v111004       if (!bc_20780) -> if (1)
// ... vol_poke () created. Same code except if error doesn't ms_sleep (4000); and bc_20780 = 1;
// -> chip_imp_pwr_on () does bc_vol0xf8_set = 0;

// After one time (each power up on New) reg 0xF8 test which sets bc_vol0xf8 = 0; or bc_vol0xf8 = 1; ...
Old:
If bc_20780 and bc_vol0xf8 are both 0, do a volpoke.
If timeout error, sleep 4 seconds and set bc_20780 = 1;
If bc_20780 | bc_vol0xf8, set reg F8 and done.
Else if (! bc_20780) do bc_ar_set (); if never before done

New:
Commented Same as line 1 above
No timout error set as in 2 above
Same as 3 above
Else unconditional bc_ar_set (); and bc_vol_poke ();

Difference: Assume bc_20780 = 1
Old: set reg F8 and done

*/

/*
Galaxy Y        Stock:  /system/bin/BCM4330B1_002.001.003.0485.0506.hcd

Galaxy Nexus    Stock:  /system/vendor/firmware/bcm4330.hcd

Galaxy 5 gt-i5500m Mad: /system/bin/BCM2049B0_BCM20780B0_002.001.022.0170.0174.hcd
*/

int bc_ar_set () {                                                      // Broadcom Audio route set (probably does something else?)
  unsigned char res_buf [MAX_HCI];
  int res_len;

  logd ("bc_ar_set 1");


    // bc_ar_1 Poke: a hcitool cmd 3f a 5 c0 41 f 0 20 00 00 00
    // bc_ar_1 Peek: a hcitool cmd 3f a 4 c0 41 f 0


  unsigned char bc_ar_1_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xc0, 0x41, 0x0f, 0, 0x20};   // Cmd 0x0a = Super_Peek_Poke, 5 = ARM_Memory_Poke, Address: 0x000f41c0, Data: 0x00000020 (Don't need the 3 trailing 0's)
  //  char bc_ar_1_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xc0, 0x41, 0x0f, 0, 0x20, 0, 0, 0};  // Add 3 trailing 0's

  res_len = hci_cmd (0x3f, 0x0a, bc_ar_1_buf, sizeof (bc_ar_1_buf), res_buf, sizeof (res_buf));
  if (res_buf [7]) {
    loge ("bc_ar_set 1 hci error: %d %s", res_buf [7], hci_err_get (res_buf [7]));
    return (res_buf [7]);
  }
  if (res_len < 1 + 8)
    loge ("bc_ar_set 1 hci_cmd error res_len: %d", res_len);

  logd ("bc_ar_set 2");

    // bc_ar_2 Poke: a hcitool cmd 3f a 5 e4 41 f 0 00 00 00 00
    // bc_ar_2 Peek: a hcitool cmd 3f a 4 e4 41 f 0
    
  unsigned char bc_ar_2_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xe4, 0x41, 0x0f, 0, 0};      // Don't need the 3 trailing 0's
  //  char bc_ar_2_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xe4, 0x41, 0x0f, 0,    0, 0, 0, 0};  // Add 3 trailing 0's
  res_len = hci_cmd (0x3f, 0x0a, bc_ar_2_buf, sizeof (bc_ar_2_buf), res_buf, sizeof (res_buf));
  if (res_buf [7]) {
    loge ("bc_ar_set 2 hci error: %d %s", res_buf [7], hci_err_get (res_buf [7]));
    return (res_buf [7]);
  }
  if (res_len < 1 + 8)
    loge ("bc_ar_set 2 hci_cmd error res_len: %d", res_len);

  return (0);
}

/* Unknown: Seen on stock app for i5500:

//COMMAND "Super_Peek_Poke" 0x00A: ARM_Memory_Poke 0x0020169c, 0x0000bd74           0xbd74 = 48, 500
//[pid  5731] write (8, " 01 0a fc 09 05 9c 16 20 00 74 bd 00 00", 13) = 13
//[pid  5740] read (8, " 04 0e 05 01 0a fc 00 74", 1704) = 8

    // 4330/20780 Poke: a hcitool cmd 3f a 5 9c 16 20 0 74 bd 00 00
    // 4330/20780 Peek: a hcitool cmd 3f a 4 9c 16 20 0                                  (Hangs on 4329)

    char bc_ar_a_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0x9c, 0x16, 0x20, 0x00, 0x74, 0xbd, 0x00, 0x00};
    res_len = hci_cmd (0x3f, 0x0a,bc_ar_a_buf, sizeof (bc_ar_a_buf), res_buf, sizeof (res_buf));
    if (res_buf [7])
      loge ("bc_ar_set a hci error: %d %s", res_buf [7], hci_err_get (res_buf [7]));
    if (res_len < 1+8)
      loge ("bc_ar_set a hci_cmd error res_len: %d", res_len);

    //int ret = reg_set (0xfb | 0x20000, 0);
*/

/*  I/AudioFlinger      steps 0-15          Simply multiplies these by 2.5
setFmVolume 0, 6, 13, 20, 26, 33, 40, 46, 53, 60, 66, 73, 80, 86, 93, 100 */

/*
    // 4330/20780 Poke: a hcitool cmd 3f a 5 9c 16 20 0 74 bd 00 00                      (Hangs on 4329)
    // 4330/20780 Peek: a hcitool cmd 3f a 4 9c 16 20 0                                  (Hangs on 4329)

    // bc_ar_1 Poke: a hcitool cmd 3f a 5 c0 41 f 0 20 00 00 00
    // bc_ar_1 Peek: a hcitool cmd 3f a 4 c0 41 f 0

    // bc_ar_2 Poke: a hcitool cmd 3f a 5 e4 41 f 0 00 00 00 00
    // bc_ar_2 Peek: a hcitool cmd 3f a 4 e4 41 f 0

    // bc_vol Poke: a hcitool cmd 3f a 5 e0 41 f 0 ff 0 0 0
    // bc_vol Peek: a hcitool cmd 3f a 4 e0 41 f 0

get F8:
a hcitool cmd 3f 15 f8 1 2

*/

int bc_vol_poke (int vol) {
  logd ("bc_vol_poke: %d", vol);

  unsigned char res_buf [MAX_HCI];
    // bc_vol Poke: a hcitool cmd 3f a 5 e0 41 f 0 ff 0 0 0
    // bc_vol Peek: a hcitool cmd 3f a 4 e0 41 f 0

    // Cmd 0x0a = Super_Peek_Poke, 5 = ARM_Memory_Poke, Address: 0x000f41e0, Data: 0x000000?? (volume)
    // Poke: a hcitool cmd 3f a 5 e0 41 f 0 ff 0 0 0                    // Volume = 255 = Max
    unsigned char cmd_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xe0, 0x41, 0x0f, 0, 0};// Last byte here written with volume. Don't need 3 trailing 0's

    //char cmd_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xe0, 0x41, 0x0f, 0, 0, 0, 0, 0};  // Add 3 trailing 0's

    cmd_buf [5 + 8] = vol / 256;                                           // Write last byte above (14th) with volume value 0 - 0xff

    int res_len = hci_cmd (0x3f, 0x0a, (unsigned char *) cmd_buf, sizeof (cmd_buf), res_buf, sizeof (res_buf));
    if (res_buf [7])
      loge ("bc_vol_poke hci error: %d %s", res_buf [7], hci_err_get (res_buf [7]));
    if (res_len < 1 + 8)
      loge ("bc_vol_poke hci_cmd error res_len: %d", res_len);

}


  int chip_imp_vol_set (int vol) {
    logd ("chip_imp_vol_set: %d", vol);
    reg_set (0xf8 | 0x00010000, 63);//vol / 256);
    return (0);
  }

    // Get:
  int chip_imp_freq_get () {
    //logd ("chip_imp_freq_get");
    int ret = reg_get (0x0a | 0x10000);
    int freq = bc_freq_lo + ret;
    curr_freq_val = freq;
    if (extra_log)
      logd ("chip_imp_freq_get: %d", freq);
    return (freq);
  }

  int chip_imp_rssi_get () {
    //logd ("chip_imp_rssi_get");
    int rssi = reg_get (0x0f);                          // Get RSSI
    rssi -= 144;
    //logd ("bc_rssi_get: %d",rssi);
    //rssi*= 2;
    //rssi/= 3;                                         // Adjusted to TI levels of around 0 - 40/50
    if (extra_log)
      logd ("chip_imp_rssi_get: %d",rssi);
    return (rssi);
  }

  extern int curr_stro_sig;// = 0;
  int stro_sig = 0;
  int need_stro_sig_chngd = 0;
  extern int stro_evt_enable;// = 1;

  int chip_imp_stro_get () {
    //logd ("chip_imp_stro_get");
    return (curr_stro_sig);
  }

    // RDS:

//#define  SUPPORT_RDS
#ifdef  SUPPORT_RDS
  #include "bch_rds.c"
#else
  int rds_events_process (unsigned char * rds_grpd) {return (-1);}
#endif

    // Seek
    //int seek_in_progress    = 0;
  extern int seek_in_progress;

  int chip_imp_events_process (unsigned char * rds_grpd) {
    //logd ("chip_imp_events_process: %p", rds_grpd);
    int ret = 0;

    ret = rds_events_process (rds_grpd);
    return (ret);
  }

    // Seek:

  // Event flags requiring callback:
  extern int need_freq_chngd;//     = 0;

  int reg_set_slow (int reg, int val) {
    int ret = reg_set (reg, val);
//    ms_sleep (20);
    return (ret);
  }


  int bc_seek_handle (int flags, int dir) {
    logd ("bc_seek_handle flags: 0x%x  dir: %d", flags, dir);
    if (! (flags & 0x01)) {                                             // If seek NOT complete (bit0 = 0 = even)...
      return (0);                                                       // Seek not finished
    }
                                                                        // Else if seek IS complete (bit0 = 1 = odd)...
    if (flags & 0x0c)                                                   // If carrier error high or rssi low
      if (seek_dbg)
        logd ("bc_seek_handle carrier error high or rssi low flags: 0x%2.2x    curr_freq_val: %d", flags, curr_freq_val); // Get this at limits

    curr_freq_val = chip_imp_freq_get ();

    if (flags & 0x02) {                                                 // If band limit reached
      if (curr_freq_val <= curr_freq_lo) {                                  // If lower limit (must have been seek down)
        logd ("bc_seek_handle restart seek down    flags: 0x%x    curr_freq_val: %d", flags, curr_freq_val);
        //chip_imp_seek_start (dir);                                          // Restart seek in original direction (down)
        chip_imp_freq_set (curr_freq_hi);
        reg_set_slow (0x09, 2);                                             // Set SRCH_TUNE_MODE: TUNE_MODE_AUTO
      }
      else if (curr_freq_val >= curr_freq_hi) {                             // If upper limit (must have been seek up)
        logd ("bc_seek_handle restart seek up    flags: 0x%x    curr_freq_val: %d", flags, curr_freq_val);
        //chip_imp_seek_start (dir);                                          // Restart seek in original direction (up)
        chip_imp_freq_set (curr_freq_lo);
        reg_set_slow (0x09, 2);                                             // Set SRCH_TUNE_MODE: TUNE_MODE_AUTO
      }
      else {                                                            // If not at a limit. MIGHT be hung here
        logd ("bc_seek_handle unknown seek error    flags: 0x%x    curr_freq_val: %d", flags, curr_freq_val);     // 0x27, 0x2b bc_reg
      }
    }
    else {                                                              // Else if not a band limit reached error... (Likely OK ?)
      return (1);                                                       // Seek finished
    }
    return (0);                                                         // Seek not finished
  }

  int freq_inc_set (int inc);
  int bc_ctl_rssi_base = 0x60;
    //0x20          // Finds 0
    //0x40          // Finds 14  Set: SRCH_CTL0: CTL0_UP + CTL0_RSSI_MID                            // Now 2, min=48
    //0x48          // Now 9, min=39
    //0x50          // 14 on p500 ?? // ?? Finds 24+ and seems good, other than some seek hangs     // Now 15 @ 00:24, min=34
    //0x58          // Finds 24                                                                     // Now 15, mon=34
    //0x60          // Finds 25 stations and 6 non-stations                                         // Now 17/18 @ 00:25, min=17
    //0x68          Now 19. min=12
    //0x70          Stops all the time.

  int chip_imp_seek_start (int dir) {
    int ret = 0;
    seek_in_progress = 1;
    logd ("chip_imp_seek_start dir: %d  bc_ctl_rssi_base: %d  curr_freq_val: %d", dir, bc_ctl_rssi_base, curr_freq_val);
    //chip_imp_seek_stop ();

    freq_inc_set (curr_freq_inc);                                       // !! Issues ! // Does nothing without "bc/ss"

    reg_set_slow (0xfc, 0x0);                                           // BC_REG_SRCH_METH Search method 0 = SRCH_METH_NORMAL normal search; 1 = preset search; 2 = rssi search.
    //reg_get (0xfc);                                                     // Read BC_REG_SRCH_METH Search method

    reg_set_slow (0x08, 0x0c);                                          // BC_REG_SRCH_CTL1 = 0x0c = ?? (2 bytes read as 0x020c)
    reg_set_slow (0xfe, 0x0);                                           // BC_REG_MAX_PRESET Max Preset = 0
 
    if (dir) {
      reg_set_slow (0x07, bc_ctl_rssi_base + 0x80);                     // Direction = up
      chip_imp_freq_set (freq_up_get (curr_freq_val));                      // Move up to next channel
    }
    else {
      reg_set_slow (0x07, bc_ctl_rssi_base);                            // Direction = down
      chip_imp_freq_set (freq_dn_get (curr_freq_val));                      // Move dn to next channel
    }

    reg_set_slow (0x09, 2);                                             // Set SRCH_TUNE_MODE: TUNE_MODE_AUTO

    int ctr = 0;
    for (ctr = 0; ctr < 40; ctr ++) {                                   // With 4 second timeout by 40 times, each 100 ms...
      int flags = reg_get (0x12 | 0x10000);                             // Read 2 bytes event/RDS event register  For some reason, this needs to be done before
                                                                        //  RDS flags get and process, still fails some time
      //if (evt_dbg)
      //  logd ("chip_imp_seek_start flags: 0x%x", flags);
      if (bc_seek_handle (flags, dir))
        break;                                                          // Terminate loop
      else
        ms_sleep (101);
    }

    seek_in_progress = 0;
    return (curr_freq_val);
  }

  int chip_imp_seek_stop () {
    logd ("chip_imp_seek_stop");
    reg_set (0x09, 0);                                                    // Set SRCH_TUNE_MODE: TUNE_MODE_TERMINATE to stop searching.
    return (0);
  }


  int band_set (int low , int high, int band) {                 // ? Do we need to stop/restart RDS power in reg 0x00 ? Or rbds_set to flush ?
    logd ("band_set low: %3.3d  high: %3.3d  band: %3.3d", low, high, band);
    bc_reg_ctl &= ~0x01;                                                  // bit0  = 0 (BND_EUROPE_US)
    if (low < 87500)
      bc_reg_ctl |= 0x01;                                                 // Set CTL: + BND_JAPAN Japan
    if (reg_set (0x01, bc_reg_ctl) < 0) {                                 //
      loge ("bc_band_set error writing 0x01");
    }
    return (0);
  }

  int freq_inc_set (int inc) {
    logd ("freq_inc_set: %3.3d", inc);
    //reg_set (0xfd | 0x10000, inc);                                      // Set: BC_REG_SRCH_STEPS  Reg 0xfd write/read bad on BCM4325 !!, perhaps because last byte RDS data, sets 256 KHz jumps
    ms_sleep (1);//20000);
    return (0);
  }
  int emph75_set (int emph75) {
    logd ("emph75_set: %3.3d", emph75);
    bc_reg_aud_ctl0 &= ~0x40;                                             // bit6  = 0
    if (emph75)
      bc_reg_aud_ctl0 |= 0x40;                                            // Set: AUD_CTL0: + DEMPH_75US for North America (and a few other countries).
    if (reg_set (0x05 | 0x10000, bc_reg_aud_ctl0) < 0) {                  //
      loge ("emph75_set error writing 0x05");
      //return (-1);
    }
    return (0);
  }
  int rbds_set (int rbds) {
    logd ("rbds_set: %3.3d",rbds);
// BCM2048_I2C_RDS_CTRL0 
//#define BCM2048_RBDS_RDS_SELECT		0x01
//#define BCM2048_FLUSH_FIFO		0x02

  logd ("bc_rbds_set: %d", rbds);

  if (rbds)
    reg_set (0x02, 0x03); //  ?? correct ? + flush
  else
    reg_set (0x02, 0x02); //  ?? correct ? + flush

// BCM2048_I2C_RDS_CTRL0
//#define BCM2048_RBDS_RDS_SELECT		0x01
//#define BCM2048_FLUSH_FIFO		0x02


// 
/*
  int orig= reg_get (0x02);
  if (rbds)
    reg_set (0x02, orig + ???);              // Set: RDS_CTL0: FLUSH_FIFO + RBDS for North America.
  else
    reg_set (0x02, orig + ???);              // Set: RDS_CTL0: FLUSH_FIFO + RDS  for everywhere else.

//Reg ops same order as in TI wl128x driver:
  //reg_set (0x2f, 1);                        // Flush the RDS FIFO

  reg_get (0x12);                            // Read the flags to clear pending events

  //reg_set (0x14,MAX_RDS_BLOCKS);           // RDS mem/FIFO set to 64/80 blocks = 192/240 bytes
  ////reg_set (0x14,(MAX_RDS_BLOCKS*3)/4);   // RDS mem set to 48 blocks = 144 bytes
  return (0);
*/
    return (0);
  }

/*
//regional_set (curr_freq_lo, curr_freq_hi, curr_freq_inc, curr_freq_odd, curr_emph75, curr_rbds);
int regional_set (int lo, int hi, int inc, int odd, int emph75, int rbds) {
  logd ("regional_set");

  //if (inc < 100)                                                            // !!!! Hack !!!!          !!!! ????
  //  inc = 100;

  if (inc < 50)
    inc = 50;

  chip_rbds_set (rbds);                                                      // Rbds before band for Tavarua V4L
  intern_band_set (lo, hi);
  curr_freq_inc_set (inc);
  freq_odd_set (odd);
  chip_emph75_set (emph75);
  return (0);
}
*/

  int band_setup () {
    //logd ("band_setup");
    if (intern_band)
      curr_freq_inc = 200;
    else
      curr_freq_inc = 100;
    band_set (curr_freq_lo, curr_freq_hi, intern_band);
    freq_inc_set (curr_freq_inc);
    emph75_set (intern_band);
    rbds_set (intern_band);
  }

/*
  int band_setup () {
    band_set (curr_freq_lo, curr_freq_hi);
    freq_inc_set (curr_freq_inc);
    if (curr_freq_inc >= 200) {                                         // If US
      emph75_set (1);
      rbds_set (1);
    }
    else {
      emph75_set (0);
      rbds_set (0);
    }
  }
*/

    // Disabled internal functions:
/*  int chip_info_log () {
  }*/


  int chip_imp_extra_cmd (const char * command, char ** parameters) {
    if (command == NULL)
      return (-1);

    int full_val = atoi (command);              // full_val = -2^31 ... +2^31 - 1       = Over 9 digits
    int ctrl = (full_val / 1000) - 200;         // ctrl = hi 3 digits - 200     (control to write to)
    int val  = (full_val % 1000);               // val  = lo 3 digits           (value to write)
    logd ("chip_imp_extra_cmd command: %s  full_val: %d  ctrl: %d  val: %d", command, full_val, ctrl, val);

      if (val == 990) {
        intern_band = 0;  // EU
        band_setup ();
      }
      else if (val == 991) {
        intern_band = 1;  // US
        band_setup ();
      }
      else if (full_val >= 200000 && full_val <= 200255) {
        int vol_reg_val = full_val - 200000;
        reg_set (0xf8 | 0x10000,  vol_reg_val);//0x000f);//0x00ff);                                // Vol Max              f8 00 ff 00 
      }
      else if (full_val == 230000) {
        bc_g2_pcm_set ();
      }

    return (0);
  }

//  #include "plug.c"

