
    // #include'd in tnr_bch.cpp

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>

#ifndef N_HCI
#define N_HCI  15
#endif

#define HCIUARTSETPROTO    _IOW('U', 200, int)
#define HCIUARTGETPROTO    _IOR('U', 201, int)
#define HCIUARTGETDEVICE  _IOR('U', 202, int)

#define HCI_UART_H4    0
#define HCI_UART_BCSP  1
#define HCI_UART_3WIRE  2
#define HCI_UART_H4DS  3
#define HCI_UART_LL    4

int exiting =       0;
int uart_fd =       -1;
int hcdfile_fd =    -1;
int bdaddr_flag =   0;
int enable_lpm =    0;
int enable_hci =    0;

struct termios termios = {0};
unsigned char hci_recv_buf [1024] = {0};             // Should only need 259 bytes ?

// Sending order
unsigned char hci_reset [] =             { 0, 0, 0, 0, 0x01, 0x03, 0x0c, 0x00 };                                     // OGF:    3    OCF:    3   (Host Controller & Baseband Commands, Reset)
unsigned char hci_patchram_start [] =    { 0, 0, 0, 0, 0x01, 0x2e, 0xfc, 0x00 };                                     // OGF: 0x3f    OCF: 0x2e
    // After this many packets sent from BCM4325D1_004.002.004.0218.0248.hcd: 48, 259,..., 114, 8
unsigned char hci_baudrate_reset [] =    { 0, 0, 0, 0, 0x01, 0x18, 0xfc, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // OGF: 0x3f    OCF: 0x18   Parameter Total Length:  6


// Each command is assigned a 2 byte Opcode used to uniquely identify different types of commands.
// The Opcode parameter is divided into two fields, called the OpCode Group Field (OGF) and OpCode Command Field (OCF).
// The OGF occupies the upper 6 bits of the Opcode, while the OCF occupies the remaining 10 bits. The OGF of 0x3F is reserved for vendor-specific debug commands. 

  int uart_recv (int fd, unsigned char * buf, int flush);
  int uart_send (unsigned char * buf, int len);
  int hci_xact  (unsigned char * buf, int len);


/* Default power up permissions:

drwxr-xr-x   14 root     root          3240 May 16 05:04 dev
drwxrwxr-x    2 root     net_raw        220 May 16 05:04 socket

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



#define MAX_HCI  264   // 8 prepended bytes + 255 max bytes HCI data/parameters + 1 trailing byte to align


// Unix datagrams requires other write permission for /dev/socket, or somewhere else (ext, not FAT) writable.

//#define CS_AF_UNIX        // Use network sockets to avoid filesystem permission issues.
#define CS_DGRAM

#ifdef  CS_AF_UNIX
#define DEF_API_SRVSOCK    "/dev/socket/srv_sprt"
#define DEF_API_CLISOCK    "/dev/socket/cli_sprt"
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

//unsigned char res_buf [MAX_HCI] = {0};

int do_daemon_hci (char * cmd_buf, int cmd_len, char * res_buf, int res_max ) {
  if (cmd_len == 1 && cmd_buf [0] == 0x73) {
    logd ("do_daemon_hci got ready inquiry");
    res_buf [0] = cmd_buf [0];
    return (1);
  }
  else if (cmd_len == 1 && cmd_buf [0] == 0x7f) {
    logd ("do_daemon_hci got stop");
    res_buf [0] = cmd_buf [0];
    exiting = 1;
    return (1);
  }

  int hx_ret = hci_xact (cmd_buf, cmd_len);                     // Do HCI transaction
  if (hx_ret < 8 || hx_ret > 270) {
    hci_recv_buf [0] = 0xff; // Error
    return (8);
  }
  hci_recv_buf [0] = 0;
  memcpy (res_buf, hci_recv_buf, hx_ret);
  //hex_dump ("aaaa", 32, hci_recv_buf, hx_ret);
  //hex_dump ("bbbb", 32, res_buf, hx_ret);
  
  if (res_buf [7])
    loge ("do_daemon_hci hci err: %d %s", res_buf [7], hci_err_get (res_buf [7]));
  
  return (hx_ret);
}

//char stop_resp[] ={2, 0xff, 0, 0, 0, 0, 0, 0};
//char err_resp[] = {2, 0xfe, 0, 0, 0, 0, 0, 0};

int do_server () {
  int sockfd = -1, newsockfd = -1, cmd_len = 0, ctr = 0;
  socklen_t cli_len = 0, srv_len = 0;
#ifdef  CS_AF_UNIX
  struct sockaddr_un  cli_addr = {0}, srv_addr = {0};
  srv_len = strlen (srv_addr.sun_path) + sizeof (srv_addr.sun_family);
#else
  struct sockaddr_in  cli_addr = {0}, srv_addr = {0};
  //struct hostent *hp;
#endif
  char cmd_buf [DEF_BUF] ={0};

  //system("chmod 666 /dev");            // !! Need su if in JNI
  //system("chmod 666 /dev/socket");

#ifdef  CS_AF_UNIX
  unlink (api_srvsock);
#endif
  if ((sockfd = socket (CS_FAM,CS_SOCK_TYPE, 0)) < 0) {
    loge ("do_server socket  errno: %d", errno);
    return (-1);
  }

  memset ((char *) & srv_addr, sizeof (srv_addr), 0);
#ifdef  CS_AF_UNIX
  srv_addr.sun_family = AF_UNIX;
  strncpy (srv_addr.sun_path, api_srvsock, sizeof (srv_addr.sun_path));
  srv_len = strlen (srv_addr.sun_path) + sizeof (srv_addr.sun_family);
#else
  srv_addr.sin_family=AF_INET;
  srv_addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK); //INADDR_ANY;
  //hp = gethostbyname("localhost");
  //if (hp== 0) {
  //  loge ("Error gethostbyname  errno: %d", errno);
  //  return (-2);
  //}
  //bcopy((char *)hp->h_addr, (char *)&srv_addr.sin_addr, hp->h_length);
  srv_addr.sin_port = htons (CS_PORT);
  srv_len = sizeof (struct sockaddr_in);
#endif

#ifdef  CS_AF_UNIX
logd ("srv_len: %d  fam: %d  path: %s", srv_len, srv_addr.sun_family, srv_addr.sun_path);
#else
logd ("srv_len: %d  fam: %d  addr: 0x%x  port: %d", srv_len, srv_addr.sin_family, ntohl (srv_addr.sin_addr.s_addr), ntohs (srv_addr.sin_port));
#endif
  if (bind (sockfd,(struct sockaddr *)&srv_addr, srv_len) < 0) {
    loge ("Error bind  errno: %d", errno);
#ifdef  CS_AF_UNIX
    return (-3);
#endif
#ifdef CS_DGRAM
    return (-3);
#endif
    loge ("Inet stream continuing despite bind error");      // OK to continue w/ Internet Stream
  }

// Get command from client
#ifndef CS_DGRAM
  if (listen(sockfd, 5)) {                           // Backlog= 5; likely don't need this
    loge ("Error listen  errno: %d", errno);
    return (-4);
  }
#endif

  logd ("do_server Ready");

  while (!exiting) {
    memset ((char *) & cli_addr, sizeof (cli_addr), 0);                        // ?? Don't need this ?
    //cli_addr.sun_family = CS_FAM;                                     // ""
    cli_len = sizeof (cli_addr);

    //logd ("ms_get: %d",ms_get ());
#ifdef  CS_DGRAM
    cmd_len = recvfrom(sockfd, cmd_buf, sizeof (cmd_buf), 0,(struct sockaddr *)&cli_addr,&cli_len);
    if (cmd_len <= 0) {
      loge ("Error recvfrom  errno: %d", errno);
      ms_sleep (101);   // Sleep 0.1 second
      continue;
    }
  #ifndef CS_AF_UNIX
// !! 
    if ( cli_addr.sin_addr.s_addr != htonl (INADDR_LOOPBACK) ) {
      loge ("Unexpected suspicious packet from host");// %s",inet_ntoa(cli_addr.sin_addr.s_addr));//inet_ntop(cli_addr.sin_addr.s_addr)); //
    }
  #endif
#else
    newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&cli_len);
    if (newsockfd < 0) {
      loge ("Error accept  errno: %d", errno);
      ms_sleep (101);   // Sleep 0.1 second
      continue;
    }
  #ifndef  CS_AF_UNIX
// !! 
    if ( cli_addr.sin_addr.s_addr != htonl (INADDR_LOOPBACK) ) {
      loge ("Unexpected suspicious packet from host");// %s",inet_ntoa(cli_addr.sin_addr.s_addr));//inet_ntop(cli_addr.sin_addr.s_addr)); //
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

    unsigned char res_buf [MAX_HCI] = {0};
    int res_len= 0;
      res_len = do_daemon_hci ( cmd_buf, cmd_len, res_buf, sizeof (res_buf));    // Do HCI (or other) function
      //logd ("do_server do_daemon_hci res_len: %d", res_len);
      if (res_len < 0) {  // If error
        res_len = 2;
        res_buf [0] = 0xff;
        res_buf [1] = 0xff;
      }
      //hex_dump ("", 32, res_buf, res_len);
//    }

// Send response
#ifdef  CS_DGRAM
    if (sendto(sockfd, res_buf, res_len, 0,(struct sockaddr *)&cli_addr,cli_len) != res_len) {
      loge ("Error sendto  errno: %d", errno);
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

  //acc_hci_stop ();

  return (0);//stop_resp);//"server finished end");
}



typedef struct {
  int baudrate;
  int termios_baudrate;
} tbaudrates;

//#define REAL_LOW_BAUDRATES                                            // Doesn't work below 9600
#define LOW_BAUDRATES
tbaudrates baudrates[] = {
#ifdef  REAL_LOW_BAUDRATES
  {      50,      B50 },
  {      75,      B75 },
  {     110,     B110 },
  {     134,     B134 },
  {     150,     B150 },
  {     200,     B200 },
  {     300,     B300 },
  {     600,     B600 },
  {    1200,    B1200 },
  {    1800,    B1800 },
  {    2400,    B2400 },
  {    4800,    B4800 },
#endif
  {    9600,    B9600 },
#ifdef  LOW_BAUDRATES
  {   19200,   B19200 },
  {   38400,   B38400 },
  {   57600,   B57600 },
#endif
  {  115200,  B115200 },
  {  230400,  B230400 },
  {  460800,  B460800 },
  {  500000,  B500000 },
  {  576000,  B576000 },
  {  921600,  B921600 },
  { 1000000, B1000000 },
  { 1152000, B1152000 },
  { 1500000, B1500000 },
  { 2000000, B2000000 },
  { 2500000, B2500000 },
  { 3000000, B3000000 },
  { 3500000, B3500000 },
  { 4000000, B4000000 },
};

//#define ALL_RATES
int rate_idx = 0;

int termios_baudrate_get (int baudrate) {                               // Convert normal numeric baudrate to termios baudrate.
  int idx = 0;
  for (idx = 0; idx < (sizeof (baudrates) / sizeof (tbaudrates)); idx ++)
    if (baudrates [idx].baudrate == baudrate)
      return (baudrates [idx].termios_baudrate);
  return (0);
}

int baudrate_get (int termios_baudrate) {                       // Convert termios baudrate to normal numeric baudrate.
  int idx= 0;
  for (idx = 0; idx < (sizeof (baudrates) / sizeof (tbaudrates)); idx ++)
    if (baudrates [idx].termios_baudrate == termios_baudrate)
      return (baudrates [idx].baudrate);
  return (0);
}

int uart_baudrate_set (int baudrate) {                                  // Set ONLY port baudrate (doesn't reset chip baudrate)
  logd ("baudrate_set: %d", baudrate);
  int termios_baudrate = termios_baudrate_get (baudrate);
  if (termios_baudrate) {
    cfsetospeed (& termios, termios_baudrate);
    cfsetispeed (& termios, termios_baudrate);
    tcsetattr (uart_fd, TCSANOW, & termios);
    return (0);
  }
  loge ("baudrate_set invalid: %d", baudrate);
  return (-1);
}

int uart_baudrate_get () {                                              // Get current port baudrate
  speed_t termios_osp = cfgetospeed (& termios);
  speed_t termios_isp = cfgetispeed (& termios);
  logd ("uart_baudrate_get termios_osp: %d  termios_isp: %d", termios_osp, termios_isp);
  int baudrate = baudrate_get (termios_osp);
  logd ("uart_baudrate_get: %d", baudrate);                             // LG G2 after BT on, then off:     uart_baudrate_get: 4000000
  return (baudrate);
}
///* No longer used
int baudrate_reset (int baudrate) {                                     // Reset the HCI interface baudrate, then set the port baudrate; Assumes we have valid communications first
  logd ("baudrate_reset: %d", baudrate);
  int ret = -1;
  int termios_baudrate = termios_baudrate_get (baudrate);
  if (termios_baudrate) {                                               // If valid baudrate
    hci_baudrate_reset [13] = (unsigned char) (baudrate >> 24);
    hci_baudrate_reset [12] = (unsigned char) (baudrate >> 16);
    hci_baudrate_reset [11] = (unsigned char) (baudrate >> 8);
    hci_baudrate_reset [10] = (unsigned char) (baudrate & 0xFF);
    if (hci_xact (hci_baudrate_reset, sizeof (hci_baudrate_reset)) < 0)
      return (-1);
    ret = uart_baudrate_set (baudrate);                                 // Validated so should never return an error
    return (ret);
  }
  loge ("baudrate_reset invalid: %d",baudrate);
  return (-1);
}
//*/
int start_baudrate = -1;

int uart_init (int set_baudrate) {
  logd ("uart_init baud: ", set_baudrate);

  tcflush (uart_fd, TCIOFLUSH);
  tcgetattr (uart_fd, & termios);

  cfmakeraw (& termios);
/* Equivalent without cfmakeraw()
  termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  termios.c_oflag &= ~OPOST;
  termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  termios.c_cflag &= ~(CSIZE | PARENB);
  termios.c_cflag |= CS8;
*/

  termios.c_cflag |= CSTOPB;                                            // !!!! 2 stop bits as Broadcom code does...

  termios.c_cflag |= CRTSCTS;                       // RTS - CTS flow control
  tcsetattr (uart_fd, TCSANOW, & termios);
  tcflush (uart_fd, TCIOFLUSH);
  tcsetattr (uart_fd, TCSANOW, & termios);
  tcflush (uart_fd, TCIOFLUSH);
  tcflush (uart_fd, TCIOFLUSH);

  start_baudrate = uart_baudrate_get ();    // Now used to restore baud if error...

  uart_baudrate_set (set_baudrate);

  tcflush (uart_fd, TCIOFLUSH);                                          // !! Flush RX + Tx to prevent reset_start () bogus first response rx byte 0xf0
  tcflush (uart_fd, TCIFLUSH);                                           // !! Flush RX      to prevent reset_start () bogus first response rx byte 0xf0

  return (0);
}

//int xact_ms_sleep = 0;//2000;

int reset_start () {

  tcflush (uart_fd, TCIOFLUSH);                                         // !! Flush RX + Tx to prevent reset_start () bogus first response rx byte 0xf0
  tcflush (uart_fd, TCIFLUSH);                                          // !! Flush RX      to prevent reset_start () bogus first response rx byte 0xf0

  logd ("reset_start");

  //xact_ms_sleep = 2000;
  if (hci_xact (hci_reset, sizeof (hci_reset)) < 0) {                   // Send the reset. If error...
    loge ("reset_start hci_xact hci_reset error 1");
    //xact_ms_sleep = 0;
    if (hci_xact (hci_reset, sizeof (hci_reset)) < 0) {                 // Send a second reset. Second attempt works better when switching baudrates.
      loge ("reset_start hci_xact hci_reset error 2");
      return (-1);                                                      // If error, return error
    }
  }
  //xact_ms_sleep = 0;
  return (0);
}

char hcd_main [DEF_BUF] = "/sdcard/spirit/broadcomp.hcd";

char * hcd_list [] = {
  hcd_main,
};  
char hcd_buf [DEF_BUF] = {0};

char * hcd_get () {
  int ctr = 0, ret = 0;
  char *hcd = NULL;

  hcd_buf [0] = 0;

  for (ctr = 0; ctr < (sizeof (hcd_list) / sizeof (char *)); ctr ++) {  // For all entries in hcd file list
    hcd = hcd_list [ctr];
    if (flags_file_get (hcd, O_RDONLY)) {                               // If we have this hcd file and read accessible...
      logd ("hcd_get from hcd list     have HCD");                      // !!    Don't display b1.bin file name !!    : %s", hcd);
      return (hcd);
    }
    else {
      logd ("hcd_get no                     HCD: %s", hcd);
    }
  }
  hcd_buf [0] = 0;
  ret = hcd_file_find (hcd_buf, sizeof (hcd_buf));
  if (hcd_num == 1) {
    logd ("hcd_get from find         have internal HCD: %d", hcd_num);
    return ((char *) "b1.bin");
  }
  else if (hcd_num == 2) {
    logd ("hcd_get from find         have internal HCD: %d", hcd_num);
    return ((char *) "b2.bin");
  }
  else if (ret && flags_file_get (hcd_buf, O_RDONLY)) {                 // If we have this hcd file and read accessible...
    logd ("hcd_get from find         have HCD: %s", hcd_buf);
    return (hcd_buf);
  }
  loge ("hcd_get no HCD found");
  return (NULL);
}

#include "bch_hcd.c"

int tmo_read (int fd, unsigned char * buf, int buf_len, int tmo_ms, int single_read);

char * hcd_ptr = NULL;
int hcd_bytes_left = 0;

int hcd_open (char * hcd) {
  int ret = 0;
  if (hcd_num == 0) {
    ret = open (hcd, O_RDONLY);
    return (ret);
  }
  else if (hcd_num == 1) {
    hcd_ptr = b1_bin;
    hcd_bytes_left = sizeof (b1_bin);
    return (hcd_num);
  }
  else if (hcd_num == 2) {
    hcd_ptr = b2_bin;
    hcd_bytes_left = sizeof (b2_bin);
    return (hcd_num);
  }
  return (-1);
}

int hcd_close (int fd) {
  int ret = 0;
  if (hcd_num == 0) {
    ret = close (fd);
    return (ret);
  }
  return (-1);
}

ssize_t hcd_read (int fd, void * buf, size_t count) {
  if (count < 0 || buf == NULL)
    return (-1);
  ssize_t ret = 0;
  if (hcd_num == 0) {
    ret = read (fd, buf, count);
    return (ret);
  }
  else if (hcd_num == 1 || hcd_num == 2) {
    if (hcd_bytes_left <= 0)
      return (0);
    if (count > hcd_bytes_left)
      count = hcd_bytes_left;
    memcpy (buf, hcd_ptr, count);
    hcd_ptr += count;
    hcd_bytes_left -= count;
    return (count);
  }
  return (-1);
}

int patchram_set () {
  unsigned char patchram_send_buf [1024] = {0};                         // Should only need 259 bytes or so
  int len= 0, ret = 0;
  logd ("patchram_set");

  char * hcd = hcd_get ();                                              // Get *.hcd file
  if (! hcd)
    return (-1);


  if ((hcdfile_fd = hcd_open (hcd)) < 0) {                              // Open hcd file
    loge ("patchram_set open errno: %d", errno);
    return (-5);
  }
  logd ("patchram_set fd: %d", hcdfile_fd);

  ret = hci_xact (hci_patchram_start, sizeof (hci_patchram_start));      // Start patchram
  if (ret < 0) {
    loge ("patchram_set hci_xact 1 error: %d", ret);
    return (-1);
  }
        //Always times out now ??
  ret = tmo_read (uart_fd, & patchram_send_buf [4], 2, 2000, 0);        // W/ timeout 2 s, Read 2 bytes from UART (uses neither byte), doing multiple reads if needed
                                                                        // Change timeout to 2 seconds due to Samsung removal for "//for BCM4330B1"
  logd ("patchram_set read 1 ret: %d", ret);

  baudrate_reset (3000000);

  ms_sleep (55);
  while (ret = hcd_read (hcdfile_fd, & patchram_send_buf [5], 3) > 0) { // Read 3 bytes from hcdfile until returns 0   (!! could mess up!!)
    if (ret < 0) {                                                      // If error...
      loge ("patchram_set read 2 ret: %d  errno: %d", ret, errno);
      return (-1);                                                      // Done w/ error
    }
    if (ret != 1)
      logd ("patchram_set read 2 ret: %d", ret);                        // Always shows 1 ??
    patchram_send_buf [4] = 0x01;
    len = patchram_send_buf [7];
    ret = hcd_read (hcdfile_fd, & patchram_send_buf [8], len);          // Read specified length of file
    if (ret < 0) {
      loge ("patchram_set read 3 ret: %d  len: %d  errno: %d", ret, len, errno);
      return (-1);
    }
    logd ("patchram_set read 3 ret: %d  len: %d", ret, len);

    if (hci_xact (patchram_send_buf, len + 8) < 0) {                    // Send to UART
      loge ("patchram_set hci_xact 2 error: %d", ret);
      return (-1);
    }
  }
  logd ("patchram_set read 4 last ret: %d", ret);
  hcd_close (hcdfile_fd);
  hcdfile_fd = -1;
  return (0);
}


unsigned char hci_ver_get[] = { 0, 0, 0, 0, 0x01, 0x01, 0x10, 0x00 };   // HCI Version get

#ifdef  FM_MODE
unsigned char hci_fm_on[] = { 0, 0, 0, 0, 0x01, 0x15, 0xfc, 0x03, 0x00, 0x00, 0x01 };   // power reg 0 = 1
unsigned char hci_ct_on[] = { 0, 0, 0, 0, 0x01, 0x15, 0xfc, 0x03, 0x01, 0x00, 0x04 };   // aud ctrl reg 1 = 4
unsigned char hci_au_on[] = { 0, 0, 0, 0, 0x01, 0x15, 0xfc, 0x03, 0x05, 0x00, 0x5c };   // aud ctrl reg 5 = 5c
unsigned char hci_fa_on[] = { 0, 0, 0, 0, 0x01, 0x15, 0xfc, 0x03, 0x0a, 0x00, 0xb4 };   // freqa 88.5
unsigned char hci_fb_on[] = { 0, 0, 0, 0, 0x01, 0x15, 0xfc, 0x03, 0x0b, 0x00, 0x5f };   // freqb 88.5
unsigned char hci_fp_on[] = { 0, 0, 0, 0, 0x01, 0x15, 0xfc, 0x03, 0x09, 0x00, 0x01 };   // freq preset

unsigned char hci_ra_on[] = { 0, 0, 0, 0, 0x01, 0x0a, 0xfc, 0x09, 0x05, 0xc0, 0x41, 0x0f, 0x00, 0x20, 0x00, 0x00, 0x00 };   // audio routing a
unsigned char hci_rb_on[] = { 0, 0, 0, 0, 0x01, 0x0a, 0xfc, 0x09, 0x05, 0xe4, 0x41, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00 };   // audio routing b
unsigned char hci_vo_on[] = { 0, 0, 0, 0, 0x01, 0x0a, 0xfc, 0x09, 0x05, 0xe0, 0x41, 0x0f, 0x00, 0x7f, 0x00, 0x00, 0x00 };   // volume 0x7f = half
unsigned char hci_rg_on[] = { 0, 0, 0, 0, 0x01, 0x15, 0xfc, 0x03, 0x0f, 0x01, 0x01 };   // RSSI get


int hci_lens[] = {
    sizeof (hci_fm_on),
    sizeof (hci_ct_on),
    sizeof (hci_au_on),
    sizeof (hci_fa_on),
    sizeof (hci_fb_on),
    sizeof (hci_fp_on),
    sizeof (hci_ra_on),
    sizeof (hci_rb_on),
    sizeof (hci_vo_on),
    sizeof (hci_rg_on),
    sizeof (hci_ver_get),
};
unsigned char * hci_cmds[] = {
    hci_fm_on,
    hci_ct_on,
    hci_au_on,
    hci_fa_on,
    hci_fb_on,
    hci_fp_on,
    hci_ra_on,
    hci_rb_on,
    hci_vo_on,
    hci_rg_on,
    hci_ver_get,
};

int bulk_hci_xact () {
  int idx = 0;
  int num_cmds = sizeof (hci_lens) / sizeof (int);
  int num_cmdsb = sizeof (hci_cmds) / sizeof (int);
  if (num_cmdsb != num_cmds) {
    return (-1);
  }
  for (idx = 0; idx < num_cmds; idx++) {
    logd ("bulk_hci_xact: %d", idx);
    if (hci_xact (hci_cmds [idx], hci_lens [idx]) < 0)
      return (-1);
  }
  return (0);
}
#endif  //#ifdef  FM_MODE


// BT power utilities derived from cm7src/system/bluetooth/bluedroid/bluetooth.c :

void rfkill_state_file_get (char * rfkill_state_file, size_t rsf_size, const char * type) {       // !!!! CM9 has rfkill for bcm4329 AND hci0 !!!!

  if (rsf_size < 1)
    return;

  if (rfkill_state_file [0])
    return;

  char path[64] = {0};
  char buf [64] = {0};
  int fd = -1, ret = 0, id = 0;

  #define MAX_RFKILL 4  //1 was OK, but N9 needs at least 3
  for (id = 0; id < MAX_RFKILL; id ++) {                                // For all possible values of id that have a type file...

    snprintf (path, sizeof (path), "/sys/class/rfkill/rfkill%d/type", id);

    fd = open (path, O_RDONLY);                                         // Open type file
    if (fd < 0) {
      logd ("rfkill_state_file_get open %s errno: %s (%d)", path, strerror (errno), errno);   // Normal error
      return;
    }

    ret = read (fd, & buf, sizeof (buf));                               // Get contents of type file
    close (fd);

    int ctr = 0;
    int len = 64;
    buf [len - 1] = 0;
    if (strlen (buf) < len)
      len = strlen (buf);
    for (ctr = 0; ctr < len; ctr ++) {
      if (buf [ctr] == '\r' || buf [ctr] == '\n')
        buf [ctr] = 0;                                                  // Replace newlines w/ 0
    }

    logd ("rfkill_state_file_get for path: \"%s\"  read: \"%s\"", path, buf );

    if (ret >= strlen (type) && memcmp (buf, type, strlen (type)) == 0){// If type starts with variable tpe, eg "bluetooth" or "fm"

      if (rfkill_state_file [0] == 0) {                                 // If not set yet, IE if first found...
        snprintf (rfkill_state_file, rsf_size, "/sys/class/rfkill/rfkill%d/state", id);
      }
    }
  }
  return;
}

int rfkill_state_get (char * rfkill_state_file, size_t rsf_size, const char * type) {     // !!!! CM9 has rfkill for bcm4329 AND hci0 !!!!
  int fd = -1, ret = -1;
  char rbuf = 0;

  rfkill_state_file_get (rfkill_state_file, rsf_size, type);            // Get RFKill state file, eg: "bluetooth" or "fm"
  if (rfkill_state_file [0] == 0)                                       // If no state file of type desired found...
    return (-1);

  fd = open (rfkill_state_file, O_RDONLY);                              // Open state file
  if (fd < 0) {
    loge ("rfkill_state_get open \"%s\" errno: %s (%d)", rfkill_state_file, strerror (errno), errno);
    return (-2);
  }
  ret = read (fd, & rbuf, 1);                                           // Read 1 byte
  if (ret < 1) {                                                        // If don't have at least 1 byte...
    loge ("rfkill_state_get read \"%s\" ret: %d  errno: %s (%d)", rfkill_state_file, ret, strerror (errno), errno);
    close (fd);
    return (-3);
  }
  close (fd);

  if (rbuf == '1')
    return (1);

  else if (rbuf == '0')
    return (0);
 
  return (-4);
}

  int rfkill_state_set (int on, char * rfkill_state_file, size_t rsf_size, const char * type) {
    int fd = -1, ret = -1;

    rfkill_state_file_get (rfkill_state_file, rsf_size, type);            // Get RFKill state file, eg: "bluetooth" or "fm"
    if (rfkill_state_file [0] == 0)
      return (-1);                                                        // Abort error if we can't get state file name

    char rfk_cmd [DEF_BUF] = "echo ";                                     // Works in root code but writing directly doesn't work in app; get EPERM even when permissions set to 666
    if (on)
      strlcat (rfk_cmd, "1 > ", sizeof (rfk_cmd));
    else
      strlcat (rfk_cmd, "0 > ", sizeof (rfk_cmd));

    strlcat (rfk_cmd, rfkill_state_file , sizeof (rfk_cmd));
    //strlcat (rfk_cmd, " 2>/dev/null", sizeof (rfk_cmd));

    logd ("rfkill_state_set about to run: %s", rfk_cmd);
                                                                        // Previously done in svc_svc : hci_init
    system (rfk_cmd);                                                   // Run the shell command as SU
    system (rfk_cmd);                                                   // Repeat to ensure

    return (0);
  }

typedef struct {    // BD Address
    __u8 b[6];
} __attribute__((packed)) bdaddr_t;

#define SOL_HCI        0
#define BTPROTO_HCI     1

#ifdef  HCI_MAX_FRAME_SIZE
#undef  HCI_MAX_FRAME_SIZE
#endif

    
int ba2str_size (const bdaddr_t *btaddr, char *straddr, int straddr_size) {
  unsigned char *b = (unsigned char *)btaddr;
  return snprintf (straddr, straddr_size, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",b[7], b[6], b[5], b[4], b[3], b[2]);
}
int str2ba(const char *str, bdaddr_t *ba) {
  int i= 0;
  for (i = 5; i >= 0; i--) {
    ba->b[i] = (uint8_t) strtoul (str, (char **)&str, 16);
    str++;
  }
  return 0;
}

int tmo_write (int fd, unsigned char * buf, int buf_len, int tmo_ms) {
  //logd ("tmo_write buf_len: %d  tmo_ms: %d", buf_len, tmo_ms);
  int buf_left = buf_len, buf_sent = 0;
  int tmo_time = ms_get () + tmo_ms;
  while (buf_left > 0) {
    if (ms_get () >= tmo_time) {
      loge ("tmo_write timeout reached of %d milliseconds", tmo_ms);
      return (buf_sent);
    }
    int wret = write (fd, & buf [buf_sent], buf_left);
    if (wret < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        loge ("tmo_write waiting errno: %s (%d)", strerror (errno), errno);

        //ms_sleep (10);                                                  // Wait 10 milliseconds
        ms_sleep (22);  // !! Will this improve battery on Broadcom w/ RDS ???? !!!!

        continue;
      }
      loge ("tmo_write write errno: %s (%d)", strerror (errno), errno);
      return (buf_sent);
    }
    buf_left -= wret;
    buf_sent += wret;
    if (wret == 0) {                                                    // If wrote 0 bytes (but no error)
      logd ("tmo_write wrote 0 errno: %s (%d)", strerror (errno), errno);
      ////ms_sleep (10);                                                   // USED TO Treat like waiting...: Wait 10 milliseconds
      //ms_sleep (100);                                                  // 100 ms
      //continue;
      return (buf_sent);    // Just return now
    }
    else if (buf_left > 0) {
      logd ("tmo_write write partial buf_len: %d  written: %d", buf_len, wret);
    }
  }
  buf_sent = buf_len - buf_left;
  return (buf_sent);
}


int tmo_read_tmo_error_times = 0;

int tmo_read (int fd, unsigned char * buf, int buf_len, int tmo_ms, int single_read) {

  //logd ("tmo_read tmo_ms: %d", tmo_ms);
  // 1, 800, 2000

  if (tmo_ms == 2000)
    tmo_ms = 1000;//200;//1000;
  else if (tmo_ms == 800)
    tmo_ms = 400;//100;//400;

//tmo_ms = 2000; For Nexus Player testing

  //logd ("tmo_read buf_len: %d  tmo_ms: %d", buf_len, tmo_ms);
  int partial = 0, rret = 0;
  int buf_left = buf_len;                                               // Buffer left = count to read
  int buf_recv = 0;                                                     // Bytes read = 0

  if (tmo_read_tmo_error_times >= 10) {                                 // If excessive errors...
    if (tmo_read_tmo_error_times ++ < 1000)                             // Mark as another error. If under 1000....
      return (0);                                                       // Return as if no error, but 0 bytes read
    tmo_read_tmo_error_times = 0;                                       // Reset after 1000 times
  }

  int tmo_time = ms_get () + tmo_ms;                                    // Time to timeout

/* Always get this:
01-29 05:35:25.806 D/s2tnrbch( 7101): hcd_get from find         have internal HCD: 2
01-29 05:35:25.806 D/s2tnrbch( 7101): patchram_set fd: 2
01-29 05:35:26.826 D/s2tnrbch( 7101): tmo_read timeout reached of 1000 milliseconds
01-29 05:35:26.826 D/s2tnrbch( 7101): patchram_set read 1 ret: 0
01-29 05:35:26.826 D/s2tnrbch( 7101): baudrate_reset: 3000000
01-29 05:35:27.226 D/s2tnrbch( 7101): tmo_read timeout reached of 400 milliseconds

01-29 05:35:27.226 E/s2tnrbch( 7101): uart_recv error 1 rret: 0  flush: 0
01-29 05:35:27.226 E/s2tnrbch( 7101): hci_xact error rret: -1
01-29 05:35:27.226 E/s2tnrbch( 7101): 00 00 00 00 01 18 fc 06 00 00 c0 c6 2d 00 

01-29 05:35:27.286 D/s2tnrbch( 7101): patchram_set read 3 ret: 139  len: 139
01-29 05:35:27.306 D/s2tnrbch( 7101): patchram_set read 3 ret: 254  len: 254
01-29 05:35:27.336 D/s2tnrbch( 7101): patchram_set read 3 ret: 254  len: 254
*/

  while (buf_left > 0) {                                                // While we still have bytes to read
    if (ms_get () >= tmo_time) {
      if (tmo_ms != 1) {                                                // Suppress for UART flush
        tmo_read_tmo_error_times ++;                                    // Another timeout error
        logd ("tmo_read timeout reached of %d milliseconds", tmo_ms);
      }
      return (buf_recv);                                                // If timeout,... return number bytes read so far
    }
    tmo_read_tmo_error_times = 0;                                       // Reset errors
    rret = read (fd, & buf [buf_recv], buf_left);                         // Try to read up to number of bytes left to read
    if (rret < 0) {                                                     // If error
      if (errno == EAGAIN || errno == EWOULDBLOCK) {                      // If would block
        //logd ("tmo_read waiting errno: %s (%d)", strerror (errno), errno);

        //ms_sleep (1);
        ms_sleep (10);  // !! Will this improve battery on Broadcom w/ RDS ???? !!!!

        continue;                                                       // Sleep a bit and continue loop
      }
      loge ("tmo_read read errno: %s (%d)", strerror (errno), errno);   // If other error...
      return (buf_recv);                                                // Return number bytes read so far
    }
    buf_left -= rret;                                                   // If bytes were read,... left reduced by number read this loop
    buf_recv += rret;                                                   // Received increased by number read this loop
    if (single_read)
        //!!!! For BT just return; if buf was big enough we have all we nead; a 2nd read never returns more data
      return (buf_recv);                                                // Return number bytes read (should be all of them)

    if (buf_left > 0) {                                                 // If still bytes to get,... display debug message
      logd ("tmo_read read partial buf_len: %d  read: %d", buf_len, rret);
      partial = 1;                                                      // Flag as partial read
    }
    
  }
  if (partial && buf_len == 3 && buf_recv == 3) {                       // To attempt to fix extra byte response to reset_start ()...
    if (buf [0] != 4 && buf [1] == 4 && buf [2] == 0x0e) {
      loge ("tmo_read removing bogus byte: 0x%x", buf [0]);
      ms_sleep (1);                                                    // Sleep a bit to ensure next byte
      buf [0] = 4;
      buf [1] = 0x0e;
      //buf [2] =4;
      rret = read (fd, & buf [2], 1);                                    // Read size byte
      if (rret < 0) {                                                   // If error
        loge ("tmo_read removed bogus byte but can't get size byte");
        return (2);
      }
      loge ("tmo_read added size byte: 0x%x", buf [2]);
      return (3);
    }
  }
  return (buf_recv);                                                    // Return number bytes read (should be all of them)
}


// Don't need noblock_set as uart_fd was opened with O_NONBLOCK
int uart_send (unsigned char * buf, int len) {
                                                                        // First do a non-blocking read to remove any previous stuff
  int rret = read (uart_fd, hci_recv_buf, sizeof (hci_recv_buf));
  if (rret > 0) {
    loge ("uart_send rret: %d", rret);
    hex_dump ("", 32, hci_recv_buf, rret);
  }
  //logd ("uart_send bytes: %d", len);
  //if (len < 128)
  //  hex_dump ("", 32, buf, len);                                      // Only dump smaller packets
  int wret = tmo_write (uart_fd, & buf [4], len - 4, 800);//1000);      // Write HCI command, waiting up to 0.8 (1) second to complete
  if (wret != len - 4) {
    loge ("uart_send wret: %d", wret);
    return (-1);
  }
  return (0);
}

typedef void * TRANSAC;
TRANSAC bfm_send (char * buf, int len);
static void   hcib_dealloc_mem_cb       (TRANSAC transac);//, char *p_buf);

/*
Good UART ONE:

12-13 02:15:30.859 D/stnr_bch(32312): hci_cmd ogf: 0x3f  ocf: 0x0  cmd_len: 13  res_max: 264
12-13 02:15:30.859 D/stnr_bch(32312): 00 00 00 00 00 00 00 00 f3 88 01 02 05 
12-13 02:15:30.859 D/stnr_bch(32312): hci_xact cmd_len: 13
12-13 02:15:30.859 D/stnr_bch(32312): 00 00 00 00 01 00 fc 05 f3 88 01 02 05 

12-13 02:15:30.869 D/stnr_bch(32312): hci_xact rret: 8
12-13 02:15:30.869 D/stnr_bch(32312): ff 04 0f 04 00 01 00 fc 
12-13 02:15:30.869 E/stnr_bch(32312): do_acc_hci hci err: 252 Unknown HCI Error
12-13 02:15:30.869 D/stnr_bch(32312): hci_cmd hci_err: 252 Unknown HCI Error  res_len: 8


12-13 02:15:30.871 D/stnr_bch(32312): hci_cmd ogf: 0x3f  ocf: 0x15  cmd_len: 11  res_max: 264
12-13 02:15:30.871 D/stnr_bch(32312): 00 00 00 00 00 00 00 00 0a 01 02 
12-13 02:15:30.871 D/stnr_bch(32312): hci_xact cmd_len: 11
12-13 02:15:30.871 D/stnr_bch(32312): 00 00 00 00 01 15 fc 03 0a 01 02 

12-13 02:15:30.871 E/stnr_bch(32312): uart_send rret: 7
12-13 02:15:30.871 D/stnr_bch(32312): 04 ff 04 f3 00 88 03 
12-13 02:15:30.882 D/stnr_bch(32312): hci_xact rret: 12
12-13 02:15:30.882 D/stnr_bch(32312): 04 04 0e 08 01 15 fc 00 0a 01 b4 5f 
12-13 02:15:30.882 D/stnr_bch(32312): hci_cmd hci_err: 0 Success   res_len: 12  first data byte: 0xa  last data byte: 0x5f

Bluedroid ONE:

12-13 04:15:12.223 D/stnr_bch( 6858): bc_g2_pcm_set 1
12-13 04:15:12.223 D/stnr_bch( 6858): hci_cmd ogf: 0x3f  ocf: 0x0  cmd_len: 13  res_max: 264
12-13 04:15:12.223 D/stnr_bch( 6858): 00 00 00 00 00 00 00 00 f3 88 01 02 05 
12-13 04:15:12.223 D/sven   ( 6641): bfm_send buf: 0xa2df3b71  len: 8
12-13 04:15:12.223 D/sven   ( 6641): bfm_send ogf: 0x3f  ocf: 0x0  opcode: 0xfc00  hci_data: 0xa2df3b74  hci_len: 5
12-13 04:15:12.223 D/sven   ( 6641): TXB 00 20 08 00 00 00 00 fc 00 fc 05 f3 88 01 02 05 
12-13 04:15:12.223 D/sven   ( 6641): bfm_send ret: 1


12-13 04:08:56.796 D/stnr_bch( 6858): chip_imp_freq_set: 88500
12-13 04:08:56.796 D/stnr_bch( 6858): hci_cmd ogf: 0x3f  ocf: 0x15  cmd_len: 12  res_max: 264
12-13 04:08:56.796 D/stnr_bch( 6858): 00 00 00 00 00 00 00 00 0a 00 b4 5f 
12-13 04:08:56.796 D/sven   ( 6641): bfm_send buf: 0xa2df3b71  len: 7
12-13 04:08:56.796 D/sven   ( 6641): bfm_send ogf: 0x3f  ocf: 0x15  opcode: 0xfc15  hci_data: 0xa2df3b74  hci_len: 4
12-13 04:08:56.796 D/sven   ( 6641): TXB 00 20 07 00 00 00 15 fc 15 fc 04 0a 00 b4 5f 
12-13 04:08:56.797 D/sven   ( 6641): bfm_send ret: 1

12-13 04:08:56.800 D/sven   ( 6641): bfm_cback p_mem: 0xaf258690
12-13 04:08:56.801 D/sven   ( 6641): bfm_ 00 10 08 00 00 00 00 00 0e 06 01 15 fc 00 0a 00 
12-13 04:08:56.801 D/sven   ( 6641): bfm_cback p_buf: 0xaf258690  len: 16  bfm_rx_len: 0
12-13 04:08:56.801 D/sven   ( 6641): bfm_cback p_buf: 0xaf258690  len: 16
12-13 04:08:56.801 D/sven   ( 6641): bfm_cback done
12-13 04:08:56.801 D/stnr_bch( 6858): hci_cmd hci_err: 0 Success   res_len: 10  first data byte: 0xa  last data byte: 0x0


12-13 04:08:56.801 D/stnr_bch( 6858): hci_cmd ogf: 0x3f  ocf: 0x15  cmd_len: 11  res_max: 264
12-13 04:08:56.801 D/stnr_bch( 6858): 00 00 00 00 00 00 00 00 09 00 01 
12-13 04:08:56.801 D/sven   ( 6641): bfm_send buf: 0xa2df3b71  len: 6
12-13 04:08:56.801 D/sven   ( 6641): bfm_send ogf: 0x3f  ocf: 0x15  opcode: 0xfc15  hci_data: 0xa2df3b74  hci_len: 3
12-13 04:08:56.801 D/sven   ( 6641): TXB 00 20 06 00 00 00 15 fc 15 fc 03 09 00 01 
12-13 04:08:56.802 D/sven   ( 6641): bfm_send ret: 1

12-13 04:08:56.805 D/sven   ( 6641): bfm_cback p_mem: 0xaf25880c
12-13 04:08:56.805 D/sven   ( 6641): bfm_ 00 10 08 00 00 00 00 00 0e 06 01 15 fc 00 09 00 
12-13 04:08:56.805 D/sven   ( 6641): bfm_cback p_buf: 0xaf25880c  len: 16  bfm_rx_len: 0
12-13 04:08:56.805 D/sven   ( 6641): bfm_cback p_buf: 0xaf25880c  len: 16
12-13 04:08:56.805 D/sven   ( 6641): bfm_cback done
12-13 04:08:56.806 D/stnr_bch( 6858): hci_cmd hci_err: 0 Success   res_len: 10  first data byte: 0x9  last data byte: 0x0


*/
int bluedroid_cmd (unsigned char * cmd, int cmd_len) {                           // Do bluedroid mode command

//Tx:
//if (cmd [4] == 0x01

  uint16_t ocf = cmd [5];                                               // 0x15
  uint16_t ocf_hi = (cmd [6] & 0x03) << 8;
  ocf |= ocf_hi;
  uint16_t ogf = (cmd [6] & 0xfc) >> 2;

  unsigned char * hci_data = & cmd [8];
  int hci_len = cmd_len - 8; // cmd [7]

  //logd ("bluedroid_cmd ogf: 0x%x  ocf: 0x%x  hci_data: %p  hci_len: %d", ogf, ocf, hci_data, hci_len);

#ifdef  HCI_BLUEDROID
  bfm_rx_len = 0;
// NO gv_mem
//bfm_rx_
  TRANSAC tx_mem = bfm_send (& cmd [5], hci_len + 3);

// ff 04 0f 04 00 01 00 fc
// 00  
///* Doesn't help:
  if (ogf == 0x3f && ocf == 0) {
    hci_recv_buf [0] = 0;
    hci_recv_buf [1] = 1;
    hci_recv_buf [2] = 0x0e;
    hci_recv_buf [3] = 4;
    hci_recv_buf [4] = 0;
    hci_recv_buf [5] = 1;
    hci_recv_buf [6] = 0;
    hci_recv_buf [7] = 0;
    hci_recv_buf [8] = 0;
    ms_sleep (1);

loge ("bluedroid_cmd LG G2 BC intercept before ms_sleep(700)");
ms_sleep (700);
loge ("bluedroid_cmd LG G2 BC intercept after ms_sleep(700)");

    return (9);
  }
//*/

  int tmo_ctr = 0;
  while (tx_mem && tmo_ctr ++ < 2000) {   // 2 seconds

    if (bfm_rx_len) {
      if (bfm_rx_len > 0 && bfm_rx_len < 288) {
        hci_recv_buf [0] = 0;
        hci_recv_buf [1] = 1;
        memcpy (& hci_recv_buf [2], bfm_rx_buf, bfm_rx_len);
      }
      int rret1 = bfm_rx_len - 6;
      int rret2 = bfm_rx_buf [1] + 4;
      if (rret1 != rret2)
        loge ("bluedroid_cmd rret1: %d  rret2: %d", rret1, rret2);
      return (rret1);
    }
    ms_sleep (1);
  }

  if (tmo_ctr >= 2000)
    return (-1);
#endif

  return (-2);
}


int uart_recv (int fd, unsigned char * buf, int flush) {

  //flush = 0;  // !!!! Get this followed by problems: "uart_recv flushed bytes: 11"

  int rret = 0;
  if (flush)
    rret = tmo_read (fd, & buf [1], 3,   1, 0);                         // W/ timeout 1 ms, Read first 3 bytes, doing multiple reads if needed
  else
    //rret = tmo_read (fd, & buf [1], 3, 5000, 0);                      // W/ timeout 5 s, Read first 3 bytes, doing multiple reads if needed
    rret = tmo_read (fd, & buf [1], 3, 800, 0);                         // W/ timeout 0.8 s, Read first 3 bytes, doing multiple reads if needed
  if (rret != 3) {                                                      // If a read error or 3 bytes not read
    if (! flush)
      loge ("uart_recv error 1 rret: %d  flush: %d", rret, flush);
    return (-1);
  }

  if (buf [2] == 0x0f) {
    logd ("LG G2 BC detected, return success buf [0] buf [7]"); // ????
    buf [0] = 0;
    buf [7] = 0;
  }
// ff 04 0f 04 00 01 00 fc
// 00  
        // Else 3 bytes read OK...
  // buf [1]        = 1 for HCI Command packet, 4 for HCI Event packet, ff for HCI Vendor packet
  // buf [2]        = Event Code: 0x0e = Command Complete HCI_EV_CMD_COMPLETE, 0x0f = HCI_EV_CMD_STATUS 
  // buf [3]        = Remaining length to read        "Parameter Total Length"
  int read_remain = (0xff & buf [3]);

  // buf [4]        = Num_HCI_Command_Packets (1 on BC)
  // buf [5],buf [6]= Command_Opcode (That caused this event)
  // buf [7]        = HCI Error code (or length on Tx)
  // buf [8]...     = Return_Parameter(s) (Optional)
      
  rret = tmo_read (fd, & buf [4], read_remain, 800, 0);                 // W/ timeout 0.8 (1) s, Read remaining bytes, doing multiple reads if needed
  if (rret != read_remain) {                                            // If read error or partial read...
    loge ("uart_recv error 2 rret %d  read_remain %d  flush: %d", rret, read_remain, flush);
    return (-1);
  }
  if (flush) {
    logd ("uart_recv flushed bytes: %d", rret + 3);
    //if (rret < 128)                                                   // Only dump smaller packets
    hex_dump ("", 32, & buf [0],rret + 4);
  }
  return (rret + 4);                                                    // Return positive total length of normalized standard HCI response packet
}


int uart_cmd (unsigned char * cmd, int cmd_len) {                                // Do UART mode command
  int ctr = 0;
  int fret = 0;
  int wret = uart_send (cmd, cmd_len);                                  // Send command via UART
  if (wret) {
    loge ("uart_cmd uart_send error wret: %d", wret);
    return (-1);
  }
  int rret = uart_recv (uart_fd, hci_recv_buf, 0);                      // Receive response event via UART, no flush
  return (rret);
}



int hci_xact_error_times = 0;

int hci_xact (unsigned char * cmd, int cmd_len) {                       // Do HCI transaction; Bluedroid SHIM or UART mode (app)
  int rret = 0;
  if (ena_log_bch_hci) {
    logd ("hci_xact cmd_len: %d", cmd_len);
    hex_dump ("", 32, cmd, cmd_len);
  }
  hci_recv_buf [0] = 0xff;                                              // Default = error

  if (cmd_len < 8 || cmd_len > 270) {                                   // If invalid size...
    loge ("hci_xact error cmd_len: %d", cmd_len);
    return (-1);                                                        // Done w/ error
  }

  if (hci_xact_error_times >= 10) {                                     // If excessive errors... (Aborts most requests if too many errors to prevent hang)
    if (hci_xact_error_times ++ < 1000)                                 // Mark as another error. If under 1000....
      return (-1);                                                      // Done w/ error
    hci_xact_error_times = 0;                                           // Reset after 1000 times
  }

  if (shim_hci_enable)                                                  // If Bluedroid mode...
    rret = bluedroid_cmd (cmd, cmd_len);                                // Do Bluedroid mode transaction
  else
    rret = uart_cmd (cmd, cmd_len);                                     // Do UART mode transaction

  if (ena_log_bch_hci) {
    logd ("hci_xact rret: %d",rret);
    if (rret > 0)
      hex_dump ("", 32, hci_recv_buf, rret);
    else
      hex_dump ("", 32, hci_recv_buf, 16);
  }
  if (rret < 8 || rret > 270) {
    loge ("hci_xact error rret: %d", rret);
    hex_dump ("", 32, cmd, cmd_len);

    hci_recv_buf [0] = 0xff;                                            // Error
    rret = -1;
    hci_xact_error_times ++;                                            // Another error
  }
  else {
    hci_xact_error_times = 0;                                           // Reset errors
  }
  return (rret);
}

char * uart_list [] = {
  "/dev/ttyHS0",                                                        // Most
  "/dev/ttyHS99",                                                       // LG G2
};
char uart_buf [DEF_BUF] = {0};

#define AID_BLUETOOTH     1002  /* bluetooth subsystem */
char * uart_get () {
  int ctr = 0;
  char * uart;// = uart_buf;                                      // /dev/ttyHS0...
  char * uart_check;// = NULL;

  for (ctr = 0; ctr < (sizeof (uart_list) / sizeof (char *)); ctr ++) {
    uart = uart_list [ctr];
    if (file_get (uart)) {                                              // If we have this UART device...
      logd ("uart_get have possible UART: %s", uart);

      uart_check = user_char_dev_get (uart, AID_BLUETOOTH);
      if (uart_check != NULL) {
        logd ("uart_get have bluetooth UART: %s", uart_check);
        return (uart);
      }
      else
        logd ("uart_get not  bluetooth UART: %s", uart_check);
    }
    else {
      logd ("uart_get not exist UART: %s", uart);
    }
  }

  uart = user_char_dev_get ("/dev", AID_BLUETOOTH);                     // !! should we look for tty* etc ? : al /dev|grep -i blu
                                                                        // crw-rw---- system   bluetooth  10, 223 2011-11-20 19:22 uinput
                                                                        // crw-rw-rw- bluetooth bluetooth 248,   0 2011-11-21 00:53 ttyHS0
  if (uart) {
    logd ("uart_get found UART via AID_BLUETOOTH: ", uart);
    return (uart);
  }

  for (ctr = 0; ctr < (sizeof (uart_list) / sizeof (char *)); ctr ++) {
    uart = uart_list [ctr];
    if (file_get (uart)) {                                              // If we have this UART device...
      logd ("uart_get have UART: %s", uart);
      return (uart);
    }
    else {
      logd ("uart_get no   UART: %s", uart);
    }
  }

  logd ("uart_get no UART found");
  return (NULL);
}

char bt_rfkill_state_file [DEF_BUF] = {0};                              // "/sys/class/rfkill/rfkill0/state";

int bt_rfkill_state_set (int state) {
  if (rfkill_state_set (state, bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth") < 0) { // UART off/on w/ 0/1: If error...
    int rfkill_state = rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth");
    loge ("bt_rfkill_state_set rfkill_state_set (%d) error rfkill_state_get: %d", state, rfkill_state);
    return (-1);                                                        // Error
  }
  return (0);                                                           // Done OK
}

int uart_hci_stop () {

  int uart_pwr = rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth");
  logd ("uart_hci_stop uart_pwr: %d  uart_fd: %d", uart_pwr, uart_fd);

  if (uart_fd >= 0)                                                     // If UART open...
    close (uart_fd);                                                    // Close

  uart_fd = -1;

  if (uart_pwr <= 0) {                                                  // If UART is already off or error...
    loge ("uart_hci_stop BT is off or error; will not stop UART");
    return (-1);                                                        // Done w/ error
  }

  if (bt_rfkill_state_set (0) < 0) {                                    // UART off: If error...
    loge ("uart_hci_stop bt_rfkill_state_set (0) error");
    return (-1);                                                        // Error
  }

  logd ("uart_hci_stop OK rfkill_state_set: %d", rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth"));

  return (0);
}

int uart_start () {
  //logd ("uart_start rfkill_state_get: %d", rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth"));

  const char * uart = uart_get ();                                      // Get UART filename...
  if (! uart) {                                                         // If can't get UART filename, abort
    loge ("uart_start can't get uart filename");
    return (-1);
  }

  logd ("uart_start uart: %s", uart);

  int uart_pwr = rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth");
  logd ("uart_start uart_pwr: %d  uart_fd: %d", uart_pwr, uart_fd);

  //uart_fd = -1;                                                       // Invalidate any previous UART file handle for if error we don't close invalid handle

  if (uart_pwr > 0) {                                                   // If UART is already on...
    loge ("uart_start BT is already on !!!!  UART %s due to %s", uart, bt_rfkill_state_file);
    //loge ("uart_start BT is on; will not start UART %s due to %s", uart, bt_rfkill_state_file);   // Ignore error in case we crashed ??
    //return (-1);                                                        // Done w/ error
  }

  if (bt_rfkill_state_set (1) < 0) {                                    // UART on: If error...
    loge ("uart_start bt_rfkill_state_set (0) error");
    //return (-1);                                                      // Error                  // Ignore error ??
  }

  //logd ("uart_start rfkill_state_get: %d", rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth"));

  if ( (uart_fd = open (uart, O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1)   // Open UART
    loge ("uart_start open uart: %s  errno: %d", uart, errno);
  else
    logd ("uart_start open uart: %s", uart);

  if (uart_fd < 0) {                                                    // If UART can't be opened...
    bt_rfkill_state_set (0);                                            // UART off
    return (-1);                                                        // Done w/ error
  }

  uart_init (115200);                                                   // Initialize the UART @ 115200
  //uart_init (3000000);

    // https://android.googlesource.com/platform/hardware/broadcom/libbt/+/master/src/hardware.c
    // The look-up table of recommended firmware settlement delay (milliseconds) on known chipsets.
    //static const fw_settlement_entry_t fw_settlement_table[] = {
    //{"BCM43241", 200},
    //{"BCM43341", 100},
    //{(const char *) NULL, 100} // Giving the generic fw settlement delay setting.
    //};

  if (reset_start ()) {                                                 // If reset start error...  !!!! Doesn't work; error comes in patchram_set () !!!!
    loge ("uart_start reset_start error @ 115200");
    bt_rfkill_state_set (0);                                            // UART off
    return (-1);                                                        // Done w/ error
  }

  if (patchram_set ()) {                                                // If patchram error...
    loge ("uart_start patchram_set error");
    uart_baudrate_set (start_baudrate);                                 // Restore port to start baudrate
    bt_rfkill_state_set (0);                                            // UART off !! No longer works !!
    return (-1);                                                        // Done w/ error
  }

  if (reset_start ()) {                                                 // Another reset prevents some strange issues like no audio or 64 MHz frequency
    loge ("uart_start reset_start 2 error");
    //bt_rfkill_state_set (0);                                          // UART off
    //return (-1);                                                      // ?? No UART mode on Dell Streak due to error ?
  }


  baudrate_reset (3000000);


  return (0);                                                           // Done success
}


                                                                        // UART HCI start:
  int uart_hci_start () {
    if (uart_start ()) {                                                // Start UART mode, if error...
      loge ("acc_hci_start error no uart mode");
      //uart_hci_stop (); // !!
      return (-1);                                                      // Done w/ error
    }
    logd ("acc_hci_start success uart mode");
    return (0);
  }

