
    // Spirit2 Tuner Plugin for "Broadcom HCI" API:

    // See https://github.com/ljalves/linux_media/blob/36eb6c41ce499c88719702be2dd86869eae5264d/drivers/staging/media/bcm2048/radio-bcm2048.c

  #define LOGTAG "sftnrbch"

  #include <stdio.h>
  #include <errno.h>
  #include <sys/stat.h>
  #include <fcntl.h>

  #include "tnr_tnr.c"

  #define  GENERIC_CLIENT                                               // BCH plugin is a Client to remote Bluetooth shim, if UART mode is not used because BT power is on/BT headset desired
  #include "utils.c"

    // Internal functions:

  int bc_reg_ctl       = 0;                                             // BC_REG_CTL       // 0x01  Band select, mono/stereo blend, mono/stereo select
  int bc_reg_aud_ctl0  = 0;                                             // BC_REG_AUD_CTL0  // 0x05  Mute, volume, de-emphasis, route parameters, BW select

  #define bc_freq_lo    64000 // 0x6720        For BC chip calculations. Constant regardless of band.

  #define MAX_RDS_BLOCKS 20 //40//41//40 //42

  int hci_cmd_tmo = 1000;
  int reg_verify = 0;


    // UART Layer:                                                      // UART HCI functions

  #include <termios.h>

  // 100 / 50 / 100 was OK, but @ 4 MBits/second on the M7 the 50 for normal_uart_recv resulted in a patchram error
  int patchram_uart_recv_tmo_ms = 501;//201;//100;//500;//1000;         // Initial patchram command only
  int normal_uart_recv_tmo_ms   = 502;//202;// 50;//200;//400;
  int normal_uart_send_tmo_ms   = 203;//100;//400;    //800);//1000);      // Write HCI command, waiting up to 0.8 (1) second to complete

  int gen_server_loop_func (unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max );

  int uart_fd        = -1;
  int hcdfile_fd     = -1;
  int start_baudrate = -1;
  int high_baudrate  = 3000000;//4000000;//3000000;     Back to 3 MBits/second for M7 reliability ??

  unsigned char hci_recv_buf [1024] = {0};             // Should only need 259 bytes ?

    // Sending order
  unsigned char hci_cmd_full_reset     [] = { 0, 0, 0, 0, 0x01, 0x03, 0x0c, 0x00 };                                     // OGF:    3    OCF:    3   (Host Controller & Baseband Commands, Reset)
  unsigned char hci_cmd_baudrate_reset [] = { 0, 0, 0, 0, 0x01, 0x18, 0xfc, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // OGF: 0x3f    OCF: 0x18   Parameter Total Length:  6
  unsigned char hci_cmd_patchram_start [] = { 0, 0, 0, 0, 0x01, 0x2e, 0xfc, 0x00 };                                     // OGF: 0x3f    OCF: 0x2e


    // Each command is assigned a 2 byte Opcode used to uniquely identify different types of commands.
    // The Opcode parameter is divided into two fields, called the OpCode Group Field (OGF) and OpCode Command Field (OCF).
    // The OGF occupies the upper 6 bits of the Opcode, while the OCF occupies the remaining 10 bits. The OGF of 0x3F is reserved for vendor-specific debug commands. 

  struct termios termios = {0};
  typedef struct {
    int integer_baudrate;
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

  int termios_baudrate_get (int baudrate) {                             // Convert normal numeric baudrate to termios baudrate.
    int idx = 0;
    for (idx = 0; idx < (sizeof (baudrates) / sizeof (tbaudrates)); idx ++)
      if (baudrates [idx].integer_baudrate == baudrate)
        return (baudrates [idx].termios_baudrate);
    return (0);
  }
  int integer_baudrate_get (int termios_baudrate) {                     // Convert termios baudrate to normal numeric baudrate.
    int idx = 0;
    for (idx = 0; idx < (sizeof (baudrates) / sizeof (tbaudrates)); idx ++)
      if (baudrates [idx].termios_baudrate == termios_baudrate)
        return (baudrates [idx].integer_baudrate);
    return (0);
  }
  int uart_baudrate_get () {                                            // Get current UART baudrate
    speed_t termios_osp = cfgetospeed (& termios);
    speed_t termios_isp = cfgetispeed (& termios);
    logd ("uart_baudrate_get termios_osp: %d  termios_isp: %d", termios_osp, termios_isp);
    int baudrate = integer_baudrate_get (termios_osp);
    logd ("uart_baudrate_get: %d", baudrate);                           // LG G2 after BT on, then off:     uart_baudrate_get: 4000000
    return (baudrate);
  }

  int uart_baudrate_set (int baudrate) {                                // Set ONLY UART baudrate (doesn't reset chip baudrate)
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

  int baudrate_reset (int baudrate) {                                   // Reset the HCI interface baudrate, then set the port baudrate; Assumes we have valid communications first
    logd ("baudrate_reset: %d", baudrate);
    int ret = -1;
    int termios_baudrate = termios_baudrate_get (baudrate);             // Get termios_baudrate

    if (termios_baudrate) {                                             // If valid baudrate
      hci_cmd_baudrate_reset [13] = (unsigned char) (baudrate >> 24);
      hci_cmd_baudrate_reset [12] = (unsigned char) (baudrate >> 16);
      hci_cmd_baudrate_reset [11] = (unsigned char) (baudrate >> 8);
      hci_cmd_baudrate_reset [10] = (unsigned char) (baudrate & 0xFF);
                                                                        // Send Baudrate reset command
      if (uart_hci_xact (hci_cmd_baudrate_reset, sizeof (hci_cmd_baudrate_reset)) < 0)
        return (-1);                                                    // If error, return error and don't change UART baudrate

      ret = uart_baudrate_set (baudrate);                               // Change UART baudrate. Validated so this never return an error
      return (ret);
    }
    loge ("baudrate_reset invalid: %d",baudrate);
    return (-1);
  }


  int uart_init (int set_baudrate) {
    logd ("uart_init baud: ", set_baudrate);

    tcflush (uart_fd, TCIOFLUSH);
    tcgetattr (uart_fd, & termios);

    cfmakeraw (& termios);
    /* cfmakeraw() equals this:
    termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    termios.c_oflag &= ~OPOST;
    termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    termios.c_cflag &= ~(CSIZE | PARENB);
    termios.c_cflag |= CS8;  */

    termios.c_cflag |= CSTOPB;                                          // !!!! Set 2 stop bits as Broadcom code does...

    termios.c_cflag |= CRTSCTS;                                         // RTS - CTS flow control

    tcsetattr (uart_fd, TCSANOW, & termios);                            // Set values
    tcflush (uart_fd, TCIOFLUSH);                                       // Throw in extra flushes ?

    tcsetattr (uart_fd, TCSANOW, & termios);                            // Set values again ?
    tcflush (uart_fd, TCIOFLUSH);                                       // Throw in extra flushes ?
    tcflush (uart_fd, TCIOFLUSH);                                       // Throw in extra flushes ?

    start_baudrate = uart_baudrate_get ();                              // Save start_baudrate: used to restore original baud if error...

    uart_baudrate_set (set_baudrate);                                   // Set UART baudrate to parameter passed

    tcflush (uart_fd, TCIOFLUSH);                                       // !! Flush RX + Tx to prevent reset_start () bogus first response rx byte 0xf0
    tcflush (uart_fd, TCIFLUSH);                                        // !! Flush RX      to prevent reset_start () bogus first response rx byte 0xf0

    return (0);
  }

  int reset_start () {
    tcflush (uart_fd, TCIOFLUSH);                                       // !! Flush RX + Tx to prevent reset_start () bogus first response rx byte 0xf0
    tcflush (uart_fd, TCIFLUSH);                                        // !! Flush RX      to prevent reset_start () bogus first response rx byte 0xf0

    logd ("reset_start");

    if (uart_hci_xact (hci_cmd_full_reset, sizeof (hci_cmd_full_reset)) < 0) {            // Send the reset command. If error...
      loge ("reset_start uart_hci_xact hci_cmd_full_reset error 1");
      if (uart_hci_xact (hci_cmd_full_reset, sizeof (hci_cmd_full_reset)) < 0) {          // Send a second reset. Second attempt works better when switching baudrates.
        loge ("reset_start uart_hci_xact hci_cmd_full_reset error 2");
        return (-1);                                                    // If error, return error
      }
    }
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
    else if (ret && flags_file_get (hcd_buf, O_RDONLY)) {               // If we have this hcd file and read accessible...
      logd ("hcd_get from find         have HCD: %s", hcd_buf);
      return (hcd_buf);
    }
    loge ("hcd_get no HCD found");
    return (NULL);
  }


  char * hcd_ptr = NULL;
  int hcd_bytes_left = 0;

  #include "hcd/hcd_bch.c"                                              // Patchram data

  int hcd_open (char * hcd) {                                           // Open patchram file (if used) or setup hcd_ptr and hcd_bytes_left (for ram method)
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

  int hcd_close (int fd) {                                              // Close patchram file if used
    int ret = 0;
    if (hcd_num == 0) {
      ret = close (fd);
      return (ret);
    }
    return (-1);
  }

  ssize_t hcd_read (int fd, void * buf, size_t count) {                 // Read patchram file (if used) or read data from ram
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

  int patchram_set () {                                                 // Do the patchram process to load initial Broadcom chip RAM
    unsigned char hci_cmd_patchram_send_buf [1024] = {0};               // Should only need 259 bytes or so
    int len= 0, ret = 0;
    logd ("patchram_set");

    char * hcd = hcd_get ();                                            // Get *.hcd file
    if (! hcd)
      return (-1);

    if ((hcdfile_fd = hcd_open (hcd)) < 0) {                            // Open hcd file
      loge ("patchram_set open errno: %d", errno);
      return (-5);
    }
    logd ("patchram_set fd: %d", hcdfile_fd);
                                                                        // Start patchram
    ret = uart_hci_xact (hci_cmd_patchram_start, sizeof (hci_cmd_patchram_start));
    if (ret < 0) {
      loge ("patchram_set uart_hci_xact 1 error: %d", ret);
      return (-1);
    }
                                                                        // Always times out now ?
    ret = tmo_read (uart_fd, & hci_cmd_patchram_send_buf [4], 2, patchram_uart_recv_tmo_ms, 0);   // W/ timeout, Read 2 bytes from UART (uses neither byte), doing multiple reads if needed
    logd ("patchram_set read 1 ret: %d", ret);


// !! patchram takes 5 seconds regardless !!
    baudrate_reset (high_baudrate);                                     // Reset baudrate high

    quiet_ms_sleep (55);                                            // !! ?? Need ??

    while (ret = hcd_read (hcdfile_fd, & hci_cmd_patchram_send_buf [5], 3) > 0) { // Read 3 bytes from hcdfile until returns 0   (!! could mess up!!)
      if (ret < 0) {                                                    // If error...
        loge ("patchram_set read 2 ret: %d  errno: %d", ret, errno);
        return (-1);                                                    // Done w/ error
      }
      if (ret != 1)
        logd ("patchram_set read 2 ret: %d", ret);                      // Always shows 1 ??
      hci_cmd_patchram_send_buf [4] = 0x01;
      len = hci_cmd_patchram_send_buf [7];
      ret = hcd_read (hcdfile_fd, & hci_cmd_patchram_send_buf [8], len);// Read specified length of file
      if (ret < 0) {
        loge ("patchram_set read 3 ret: %d  len: %d  errno: %d", ret, len, errno);
        return (-1);
      }
      logd ("patchram_set read 3 ret: %d  len: %d", ret, len);

      if (uart_hci_xact (hci_cmd_patchram_send_buf, len + 8) < 0) {     // Send to UART
        loge ("patchram_set uart_hci_xact 2 error: %d", ret);
        return (-1);
      }
    }
    logd ("patchram_set read 4 last ret: %d", ret);
    hcd_close (hcdfile_fd);
    hcdfile_fd = -1;
    return (0);
  }


    // BT power utilities:

  void rfkill_state_file_get (char * rfkill_state_file, size_t rsf_size, const char * type) {   // Get rfkill state filename for passed type
    if (rsf_size < 1)
      return;

    if (rfkill_state_file [0])                                          // If already set...
      return;

    char type_path [64] = {0};
    char type_buf  [64] = {0};
    int type_len = 0;
    int fd = -1;
    int id = 0;

    #define MAX_RFKILL 16 //4  // 1 was OK, but Nexus9 needs at least 3
    for (id = 0; id < MAX_RFKILL; id ++) {                                // For all possible values of id that have a type file...
      snprintf (type_path, sizeof (type_path), "/sys/class/rfkill/rfkill%d/type", id);

      fd = open (type_path, O_RDONLY);                                  // Open type file
      if (fd < 0) {
        loge ("rfkill_state_file_get open %s errno: %s (%d)", type_path, strerror (errno), errno);
        return;
      }
      type_len = read (fd, & type_buf, sizeof (type_buf));              // Get contents of type file
      if (type_len <= 0) {
        loge ("rfkill_state_file_get read %s errno: %s (%d)", type_path, strerror (errno), errno);
        close (fd);
        return;
      }
      close (fd);
      if (type_len > sizeof (type_buf) - 1)
        type_len = sizeof (type_buf) - 1;
      type_buf [type_len] = 0;                                          // ASCIIZ terminate for C string
      int ctr = 0;
      for (ctr = 0; ctr < type_len ; ctr ++)
        if (type_buf [ctr] == '\r' || type_buf [ctr] == '\n')
          type_buf [ctr] = 0;                                           // Replace newlines w/ 0

      logd ("rfkill_state_file_get for type_path: \"%s\"  type_buf: \"%s\"", type_path, type_buf );

      if (! strncmp (type_buf, type, type_len)) {                       // If type starts with passed type, eg "bluetooth" or "fm"
        snprintf (rfkill_state_file, rsf_size, "/sys/class/rfkill/rfkill%d/state", id);
        return;
      }
    }
    return;
  }

  int rfkill_state_get (char * rfkill_state_file, size_t rsf_size, const char * type) {
    int fd = -1;
    int ret = -1;
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

    if (on)                                                             // If turning on...
      file_write (rfkill_state_file, "1", 1, O_WRONLY);                 // echo 1 > state

    else                                                                // Else if turning off...
      file_write (rfkill_state_file, "0", 1, O_WRONLY);                 // echo 0 > state

    return (0);
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
          ms_sleep (22);                                                  // Wait a bit: 20 ms better than 10 ms on battery with Broadcom RDS ?
          continue;
        }
        loge ("tmo_write write errno: %s (%d)", strerror (errno), errno);
        return (buf_sent);
      }
      buf_left -= wret;
      buf_sent += wret;
      if (wret == 0) {                                                  // If wrote 0 bytes (but no error)
        loge ("tmo_write wrote 0 errno: %s (%d)", strerror (errno), errno);
        return (buf_sent);                                              // Just return now
      }
      else if (buf_left > 0) {
        loge ("tmo_write write partial buf_len: %d  written: %d", buf_len, wret);
      }
    }
    buf_sent = buf_len - buf_left;
    return (buf_sent);
  }


  int tmo_read (int fd, unsigned char * buf, int buf_len, int tmo_ms, int single_read) {
    //logd ("tmo_read buf_len: %d  tmo_ms: %d", buf_len, tmo_ms);
    int partial = 0, rret = 0;
    int buf_left = buf_len;                                             // Buffer left = count to read
    int buf_recv = 0;                                                   // Bytes read = 0

    int tmo_time = ms_get () + tmo_ms;                                  // Time to timeout

    while (buf_left > 0) {                                              // While we still have bytes to read
      if (ms_get () >= tmo_time) {
        if (tmo_ms != 1) {                                              // Suppress for UART flush
          loge ("tmo_read timeout reached of %d milliseconds", tmo_ms);
        }
        return (buf_recv);                                              // If timeout,... return number bytes read so far
      }
      errno = 0;
      rret = read (fd, & buf [buf_recv], buf_left);                     // Try to read up to number of bytes left to read
      if (rret < 0) {                                                   // If error
        if (errno == EAGAIN || errno == EWOULDBLOCK) {                  // If would block
          //logd ("tmo_read waiting errno: %s (%d)", strerror (errno), errno);
          //ms_sleep (1);
          quiet_ms_sleep (10);  // !! Will this improve battery on Broadcom w/ RDS ???? !!!!
          continue;                                                     // Sleep a bit and continue loop
        }
        loge ("tmo_read read errno: %s (%d)", strerror (errno), errno); // If other error...
        return (buf_recv);                                              // Return number bytes read so far
      }
      buf_left -= rret;                                                 // If bytes were read,... left reduced by number read this loop
      buf_recv += rret;                                                 // Received increased by number read this loop
      if (single_read)
            //!!!! For BT just return; if buf was big enough we have all we nead; a 2nd read never returns more data
        return (buf_recv);                                              // Return number bytes read (should be all of them)

      if (buf_left > 0) {                                               // If still bytes to get,... display debug message
        logd ("tmo_read read partial buf_len: %d  read: %d", buf_len, rret);
        partial = 1;                                                    // Flag as partial read
      }
    }
    if (partial && buf_len == 3 && buf_recv == 3) {                     // To attempt to fix extra byte response to reset_start ()...
      if (buf [0] != 4 && buf [1] == 4 && buf [2] == 0x0e) {
        loge ("tmo_read removing bogus byte: 0x%x", buf [0]);
        ms_sleep (1);                                                   // Sleep a bit to ensure next byte
        buf [0] = 4;
        buf [1] = 0x0e;
        //buf [2] =4;
        rret = read (fd, & buf [2], 1);                                 // Read size byte
        if (rret < 0) {                                                 // If error
          loge ("tmo_read removed bogus byte but can't get size byte");
          return (2);
        }
        loge ("tmo_read added size byte: 0x%x", buf [2]);
        return (3);
      }
    }
    return (buf_recv);                                                  // Return number bytes read (should be all of them)
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
    int wret = tmo_write (uart_fd, & buf [4], len - 4, normal_uart_send_tmo_ms);      // Write HCI command, waiting up to 0.8 (1) second to complete
    if (wret != len - 4) {
      loge ("uart_send wret: %d", wret);
      return (-1);
    }
    return (0);
  }


  int uart_recv (int fd, unsigned char * buf, int flush) {
    //flush = 0;  // !!!! Get this followed by problems: "uart_recv flushed bytes: 11"

    int rret = 0;
    if (flush)
      rret = tmo_read (fd, & buf [1], 3,   1, 0);                         // W/ timeout 1 ms, Read first 3 bytes, doing multiple reads if needed
    else
      rret = tmo_read (fd, & buf [1], 3, normal_uart_recv_tmo_ms, 0);     // W/ timeout 400 ms, Read first 3 bytes, doing multiple reads if needed
    if (rret != 3) {                                                      // If a read error or 3 bytes not read
      //if (! flush) loge ("uart_recv error 1 rret: %d  flush: %d", rret, flush);   // tmo_read already logged an error message
      return (-1);
    }

    if (buf [2] == 0x0f) {
      loge ("!!!!!!!!!    LG G2 BC detected, return success buf [0] buf [7]"); // ????
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
      
    rret = tmo_read (fd, & buf [4], read_remain, normal_uart_recv_tmo_ms, 0); // W/ timeout 400 ms, Read remaining bytes, doing multiple reads if needed
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


  int uart_hci_xact (unsigned char * cmd, int cmd_len) {                  // Do UART mode command
    int ctr = 0;
    int fret = 0;
    int wret = uart_send (cmd, cmd_len);                                  // Send command via UART
    if (wret) {
      loge ("uart_hci_xact uart_send error wret: %d", wret);
      return (-1);
    }
    int rret = uart_recv (uart_fd, hci_recv_buf, 0);                      // Receive response event via UART, no flush
    return (rret);
  }


#define RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?

  char * uart_list [] = {
#ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?
    "/dev/ttyHSs20",                                                    // Ours
    "/dev/ttyHSs299",                                                   // Ours
#endif  // #ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?
    "/dev/ttyHS0",                                                      // Most: HTC One M7, Xperia Z2
    "/dev/ttyHS99",                                                     // LG G2
  };

#ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?
  char uart_orig [DEF_BUF] = {0};
#endif  // #ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?

  #define AID_BLUETOOTH     1002                                        // bluetooth subsystem user

  char * uart_get () {
    int ctr = 0;
    char * uart = NULL;                                                 // /dev/ttyHS0...
    char * uart_check = NULL;

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

    uart = user_char_dev_get ("/dev", AID_BLUETOOTH);                   // !! should we look for tty* etc ? : al /dev|grep -i blu
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

  char bt_rfkill_state_file [DEF_BUF] = {0};                            // "/sys/class/rfkill/rfkill0/state";

  int bt_rfkill_state_set (int state) {
    if (rfkill_state_set (state, bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth") < 0) { // UART off/on w/ 0/1: If error...
      int rfkill_state = rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth");
      loge ("bt_rfkill_state_set rfkill_state_set (%d) error rfkill_state_get: %d", state, rfkill_state);
      return (-1);                                                      // Error
    }
    return (0);                                                         // Done OK
  }

  int uart_hci_stop () {
    int uart_pwr = rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth");
    logd ("uart_hci_stop uart_pwr: %d  uart_fd: %d", uart_pwr, uart_fd);

    if (uart_fd >= 0)                                                   // If UART open...
      close (uart_fd);                                                  // Close
    uart_fd = -1;

    if (uart_pwr <= 0) {                                                // If UART is already off or error...
      loge ("uart_hci_stop BT is off or error; will not stop UART");
      return (-1);                                                      // Done w/ error
    }

    if (bt_rfkill_state_set (0) < 0) {                                  // UART off: If error...
      loge ("uart_hci_stop bt_rfkill_state_set (0) error");
      return (-1);                                                      // Error
    }

    logd ("uart_hci_stop OK rfkill_state_set: %d", rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth"));

#ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?
    char * renamed  = "/dev/ttyHS0";
    char * orig     = "/dev/ttyHS0";
    if (file_get ("/dev/ttyHSs20")) {
      renamed = "/dev/ttyHSs20";
      orig    = "/dev/ttyHS0";
    }
    else if (file_get ("/dev/ttyHSs299")) {
      renamed = "/dev/ttyHSs299";
      orig    = "/dev/ttyHS99";
    }
    else {
      loge ("uart_hci_stop no renamed UART");
    }

    errno = 0;
    //int ret = rename ("/dev/ttyHSs2", uart_orig);
    int ret = rename (renamed, orig);
    if (ret == 0)
      logd ("uart_hci_stop rename ret: %d  renamed: %s  orig: %s", ret, renamed, orig);
    else
      loge ("uart_hci_stop rename error ret: %d  errno: %d (%s)  renamed: %s  orig: %s", ret, errno, strerror (errno), renamed, orig);
#endif  // #ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?

    return (0);
  }

  int uart_hci_start () {
    //logd ("uart_hci_start rfkill_state_get: %d", rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth"));

    const char * uart = uart_get ();                                    // Get UART filename...
    if (! uart) {                                                       // If can't get UART filename, abort
      loge ("uart_hci_start can't get uart filename");
      return (-1);
    }
    logd ("uart_hci_start uart: %s", uart);

    int uart_pwr = rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth");
    logd ("uart_hci_start uart_pwr: %d  uart_fd: %d", uart_pwr, uart_fd);

    //uart_fd = -1;                                                     // Invalidate any previous UART file handle for if error we don't close invalid handle

    if (uart_pwr > 0) {                                                 // If UART is already on...
      loge ("uart_hci_start BT is already on !!!!  UART %s due to %s", uart, bt_rfkill_state_file);
      //loge ("uart_hci_start BT is on; will not start UART %s due to %s", uart, bt_rfkill_state_file);   // Ignore error in case we crashed ??
      //return (-1);                                                    // Done w/ error
    }

    if (bt_rfkill_state_set (1) < 0) {                                  // UART on: If error...
      loge ("uart_hci_start bt_rfkill_state_set (0) error");
      //return (-1);                                                    // Error                  // Ignore error ??
    }

    //logd ("uart_hci_start rfkill_state_get: %d", rfkill_state_get (bt_rfkill_state_file, sizeof (bt_rfkill_state_file), "bluetooth"));

    if ( (uart_fd = open (uart, O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1) // Open UART
      loge ("uart_hci_start open uart: %s  errno: %d", uart, errno);
    else
      logd ("uart_hci_start open uart: %s", uart);

    if (uart_fd < 0) {                                                  // If UART can't be opened...
      bt_rfkill_state_set (0);                                          // UART off
      return (-1);                                                      // Done w/ error
    }

    uart_init (115200);                                                 // Initialize the UART @ 115200
    //uart_init (high_baudrate);

    // https://android.googlesource.com/platform/hardware/broadcom/libbt/+/master/src/hardware.c
    // The look-up table of recommended firmware settlement delay (milliseconds) on known chipsets.
    //static const fw_settlement_entry_t fw_settlement_table[] = {
    //{"BCM43241", 200},
    //{"BCM43341", 100},
    //{(const char *) NULL, 100} // Giving the generic fw settlement delay setting.
    //};

    if (reset_start ()) {                                               // If reset start error...  !!!! Doesn't work; error comes in patchram_set () !!!!
      loge ("uart_hci_start reset_start error @ 115200");
      bt_rfkill_state_set (0);                                          // UART off
      return (-1);                                                      // Done w/ error
    }

    if (patchram_set ()) {                                              // Do patchram to init chip. If error...
      loge ("uart_hci_start patchram_set error");
      uart_baudrate_set (start_baudrate);                               // Restore port to start baudrate
      bt_rfkill_state_set (0);                                          // UART off !! No longer works !!
      return (-1);                                                      // Done w/ error
    }
/*
    if (reset_start ()) {                                               // Another reset prevents some strange issues like no audio or 64 MHz frequency
      loge ("uart_hci_start reset_start 2 error");
      //bt_rfkill_state_set (0);                                        // UART off
      //return (-1);                                                    // ?? No UART mode on Dell Streak due to error ?
    }

    baudrate_reset (high_baudrate);                                     // Set to high baudrate (Already set ?)
*/

#ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?
    strlcpy (uart_orig, uart, sizeof (uart_orig));
    errno = 0;
    int ret = 0;
    if (! strcmp ("/dev/ttyHS0", uart))
      ret = rename (uart, "/dev/ttyHSs20");
    else if (! strcmp ("/dev/ttyHS99", uart))
      ret = rename (uart, "/dev/ttyHSs299");
    if (ret == 0)
      logd ("uart_hci_start rename ret: %d", ret);
    else
      loge ("uart_hci_start rename error ret: %d  errno: %d (%s)", ret, errno, strerror (errno));
#endif  // #ifdef  RENAME_TTY      // Intended to protect against Bluetooth changes, but unstable for some ?

    return (0);                                                         // Done success
  }




    // HCI Layer:

  #define MAX_HCI  264   // 8 prepended bytes + 255 max bytes HCI data/parameters + 1 trailing byte to align

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

  int hci_cmd (uint8_t ogf, uint16_t ocf, unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max, int rx_tmo) {
    int hci_cmd_start_time = 0;
    //if (cmd_len > 8)
    //hex_dump ("", 32, cmd_buf + 8, cmd_len - 8);
    if (ena_log_bch_hci) {
      hci_cmd_start_time = ms_get ();
      logd ("hci_cmd ogf: 0x%x  ocf: 0x%x  cmd_len: %d  res_max: %d", ogf, ocf, cmd_len, res_max);
      hex_dump ("", 32, cmd_buf, cmd_len);
    }
    int res_len = 0;
    if (res_max > 252)
      res_max = 252;
    //if (cmd_len > 255 || cmd_len < 8) {
    if (cmd_len > 263 || cmd_len < 8) {                                 // !! For ti_bulk_hci_write
      loge ("hci_cmd error cmd_len: %d", cmd_len);
      return (0);
    }

    res_buf [7] = 0xfe;                                                 // Put something in HCI error field in case no response
    cmd_buf [4] = 1;
    cmd_buf [5] = ocf & 0x00ff;
    cmd_buf [6] = ((ocf & 0x0300) >> 8) | ((ogf & 0x3f) << 2);
    cmd_buf [7] = cmd_len - 8;
    if (curr_api_mode)                                                  // If hci command to Shim...
      res_len = gen_client_cmd (cmd_buf, cmd_len, res_buf, res_max, NET_PORT_HCI, rx_tmo);
    else {                                                              // Else direct...
      hci_recv_buf [0] = 0xff;                                          // Default = error
      res_len = uart_hci_xact (cmd_buf, cmd_len);                       // Do UART mode transaction
      if (res_len < 8 || res_len > 270) {
        hci_recv_buf [0] = 0xff; // Error
        res_len = 8;
      }
      hci_recv_buf [0] = 0;
      memcpy (res_buf, hci_recv_buf, res_len);
    }

    //int size_evt = res_buf [3] - 3;
    int hci_err = res_buf [7];

    if (ena_log_bch_hci) {
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
    if (ena_log_bch_hci)
      logd ("hci_cmd took %d milliseconds", ms_get () - hci_cmd_start_time);
          // 20 - 40/50 ms for hcitool popen2, same except spikes to 60-100+ for hcitool>file, 1-2 ms for UART
          // Daemon HCI = 3/5-10/18 ms w/ open/bind/close every time, else 3-11 ms
    if (hci_err)
      res_len = 0;
    return (res_len);
  }


  int shim_client_cmd (unsigned char cmd, int rx_tmo, int tmo) {        // Only used for startup test by shim_hci_start()
    if (tmo < 100)
      tmo = 100;
    int shim_ok_tmo = ms_get () + tmo;                                    // Wait up to tmo milliseconds, resending every rx_tmo milliseconds
    while (ms_get () < shim_ok_tmo) {
      unsigned char res_buf [DEF_BUF] = {0};
      int res_len = gen_client_cmd (& cmd, 1, res_buf, sizeof (res_buf), NET_PORT_HCI, rx_tmo);    // 1 Byte: cmd
      if (res_len == 1 && res_buf [0] == cmd)
        return (0);                                                       // Daemon is ready
    }
    loge ("shim_client_cmd timeout error");
    return (-1);
  }

  int shim_hci_start () {
    logd ("shim_hci_start");
    if (shim_client_cmd (0x73, 1000, 1200)) {                             // Inquiry / ping every 1 second for up to 1.2 seconds
      loge ("shim_hci_start error");
      return (-1);
    }
    logd ("shim_hci_start ok");
    return (0);
  }
  int shim_hci_stop () {
    logd ("shim_hci_stop");
    /*if (shim_client_cmd (0x7f, 1000, 1200)) {                             // Terminate  every 1 second for up to 1.2 seconds        !!!! Leave running or problems !!!
      loge ("shim_hci_stop error");
      return (-1);
    }*/
    logd ("shim_hci_stop ok");
    return (0);
  }


    // Chip code definitions:

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
  
  #define BCM2048_HI_LO_INJECTION           0x10
  
  /* BCM2048_I2C_RDS_CTRL0 */
  #define BCM2048_RBDS_RDS_SECT             0x01
  #define BCM2048_FLUSH_FIFO                0x02
  
  /* BCM2048_I2C_FM_AUDIO_PAUSE */
  #define BCM2048_AUDIO_PAUSE_RSSI_TRESH    0x0f
  #define BCM2048_AUDIO_PAUSE_DURATION      0xf0
  
  
  
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
  
  //#define BCM2048_DEFAULT_POWERING_DELAY	20  // Sleep 20 milliseconds after power on


    // Chip code:

  int bulk_get (int reg, int size, unsigned char * res_buf, int res_max) {  // For RDS or otherwise reading multiple registers at once; use reg_get() for 1-4 byte/register reads
    int res_len = -1;
    if (size > 252 || size < 0) {
      loge ("bulk_get error size: %d", size);
      return (0);
    }
    unsigned char cmd_buf [MAX_HCI] = {0};//3 + 8] = {0};
    int  cmd_buf_size = 3 + 8;                                          // ! 11 works for both TI and BCM !           sizeof (cmd_buf);
    if (ena_log_bch_reg)
      logd ("bulk_get reg: %x  size: %d", reg, size);

    cmd_buf [8] = reg;
    cmd_buf [9] = 1;                                                    // a hcitool cmd 3f 15 reg 1(read) size
    cmd_buf [10] = 0xff & size;
    res_len = hci_cmd (0x3f, 0x15, cmd_buf, cmd_buf_size, res_buf, res_max, hci_cmd_tmo);
    if (res_len >= 3 + 8 && ! res_buf [7]) {                            // !! ?? Actual value ??
      memcpy (&res_buf [8],&res_buf [10], res_len-8-2);                 // Copy data down 2 bytes to match TI response w/ bulk data starting at res_buf [8]
                                                                        // !!! Assume data is copied from bottom up since we are copying in place
      res_len -= 2;
    }

    if (res_len <= 0)                                                   // If error...
      return (0);

    if (res_buf [7]) {                                                  // If HCI error...
      loge ("bulk_get hci error: %d %s  reg: 0x%x  size: %d", res_buf [7], hci_err_get (res_buf [7]), reg, size);     // Display the error
      return (0);
    }

    return (res_len);                                                   // If success, return size
  }


  int reg_get (int reg) {                                               // If error, return 0 instead of error code. This helps protect against strange values for frequency etc.
    unsigned char res_buf [MAX_HCI];
    int val = 0;
    int res_len = -1;
    unsigned char cmd_buf [MAX_HCI] = {0};                              // 3+8] = {};; //= {0xff, 0x01, 0x01};    // TI: {0xff, 0x02, 0x00};

    if (ena_log_bch_reg)
      logd ("reg_get reg: 0x%x", reg);

    int size = 1;                                                       // Hack to allow different register sizes w/ all bytes read simultaneously encoded in register number as higher order bits
    if (reg & 0x00010000)
      size = 2;
    else if (reg & 0x00020000)
      size = 4;
    reg &= 0x0000ffff;

    cmd_buf [8] = reg;
    cmd_buf [9] = 1;        // a hcitool cmd 3f 15 reg 1(read) 1(size)
    cmd_buf [10] = size;//1;
    res_len = hci_cmd (0x3f, 0x15, cmd_buf, 3 + 8, res_buf, sizeof (res_buf), hci_cmd_tmo);          // BC HCI: 0x15
    //if (res_len >= 3+8 && ! res_buf [7])
    if (size == 2)
      val = 256 * res_buf [11] + res_buf [10];
    else if (size == 4)
      //val = 16777216 * res_buf [13] + 65536 * res_buf [12] + 256 * res_buf [11] + res_buf [10];
      val = res_buf [13] << 24 + res_buf [12] << 16 + res_buf [11] << 8 + res_buf [10] << 0;
    else
      val = res_buf [10];
    if (res_len < 8) {
      //if (ena_log_bch_reg)
        loge ("reg_get hci_cmd error: %d hci error: %d %s  reg: 0x%x  val: 0x%x", res_len, res_buf [7], hci_err_get (res_buf [7]), reg, val);    // Display the error
      return (0);
    }
    else if (ena_log_bch_reg)
      logd ("reg_get res_len: %d", res_len);
    if (! res_buf [7]) {
      if (ena_log_bch_reg)
        logd ("reg_get reg: 0x%x  val: 0x%x (%d)", reg, val, val);
      return (val);
    }
    //if (ena_log_bch_reg)
      loge ("reg_get hci error: %d %s  reg: 0x%x  val: 0x%x", res_buf [7], hci_err_get (res_buf [7]), reg, val);     // Display the error
    return (0);
  }

  int reg_set (int reg, int val) {
    unsigned char res_buf [MAX_HCI];
    int res_len = -1;
    unsigned char cmd_buf [MAX_HCI] = {0};//5+8] = {0};   // !! Only supports up to 3 bytes on BC !!!!!!!!!!!!!!!!!!!!!!!!
  
    if (ena_log_bch_reg)
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
    cmd_buf [9] = 0;        // a hcitool cmd 3f 15 reg 0(write) val
    if (size == 2) {
      cmd_buf [11] = val / 256;
      cmd_buf [10] = val % 256;
    }
    else if (size == 4) {
      cmd_buf [13] = (0xff) & (val >> 24);      // (val / 16777216);
      cmd_buf [12] = (0xff) & (val >> 16);      // (val /    65536);
      cmd_buf [11] = (0xff) & (val >>  8);      // (val /      256);
      cmd_buf [10] = (0xff) & (val >>  0);      // (val %      256);
    }
    else {
      cmd_buf [10] = val;
    }
    res_len = hci_cmd (0x3f, 0x15, cmd_buf, size + 2 + 8, res_buf, sizeof (res_buf), hci_cmd_tmo);            // BC HCI: 0x15
    if (res_len < 8) {
      loge ("reg_set hci_cmd error res_len: %d hci error: %d %s  reg: 0x%x  val: 0x%x", res_len, res_buf [7], hci_err_get (res_buf [7]), reg, val);
      return (-1);//return (0);
    }
    else if (ena_log_bch_reg)
      logd ("reg_set res_len: %d", res_len);
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

  int vol_get () {
    int vol = 257 * reg_get (0xf8 | 0x00010000);
    logd ("chip_imp_vol_get: %d", vol);
    return (vol);
  }

  int freq_get () {
    //logd ("freq_get");
    int ret = reg_get (0x0a | 0x10000);
    int freq = bc_freq_lo + ret;
    curr_freq_int = freq;
    if (ena_log_tnr_extra)
      logd ("freq_get: %d", freq);
    return (freq);
  }

  int pwr_off () {
    int ret = 0;
    logd ("pwr_off");
    chip_imp_mute_sg (1);                                               // Mute

    // !! Shouldn't need two mutes here !!
    // This was disabled, re-enable...
    bc_reg_aud_ctl0 = 0x02;//0x03;                                      // DACs and I2S off, mute audio + rf mute, resets band but power off so OK
    if (reg_set (0x05 | 0x10000, bc_reg_aud_ctl0) < 0) {                //
      loge ("pwr_off error writing 0x05");
    }

    if (reg_set (0x00, 0) < 0) {                                          // Set: SYS: OFF turn off FM
      loge ("pwr_off error writing 0x00");
    }

    curr_state = 0;
    return (curr_state);
  }

/*
  int bc_ar_set () {                                                      // Broadcom Audio route set (probably does something else?)
    unsigned char res_buf [MAX_HCI];
    int res_len;

    logd ("bc_ar_set 1");

    // bc_ar_1 Poke: a hcitool cmd 3f a 5 c0 41 f 0 20 00 00 00
    // bc_ar_1 Peek: a hcitool cmd 3f a 4 c0 41 f 0

    unsigned char bc_ar_1_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xc0, 0x41, 0x0f, 0, 0x20};   // Cmd 0x0a = Super_Peek_Poke, 5 = ARM_Memory_Poke, Address: 0x000f41c0, Data: 0x00000020 (Don't need the 3 trailing 0's)
    //  char bc_ar_1_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 0xc0, 0x41, 0x0f, 0, 0x20, 0, 0, 0};  // Add 3 trailing 0's

    res_len = hci_cmd (0x3f, 0x0a, bc_ar_1_buf, sizeof (bc_ar_1_buf), res_buf, sizeof (res_buf), hci_cmd_tmo);
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
    res_len = hci_cmd (0x3f, 0x0a, bc_ar_2_buf, sizeof (bc_ar_2_buf), res_buf, sizeof (res_buf), hci_cmd_tmo);
    if (res_buf [7]) {
      loge ("bc_ar_set 2 hci error: %d %s", res_buf [7], hci_err_get (res_buf [7]));
      return (res_buf [7]);
    }
    if (res_len < 1 + 8)
      loge ("bc_ar_set 2 hci_cmd error res_len: %d", res_len);

    return (0);
  }
*/


  // Event flags requiring callback:
//  extern int need_freq_chngd;//     = 0;

  int reg_set_slow (int reg, int val) {
    int ret = reg_set (reg, val);
//    ms_sleep (20);
    return (ret);
  }


  int bc_seek_handle (int flags, int seek_state) {
    logd ("bc_seek_handle flags: 0x%x  seek_state: %d", flags, seek_state);
    if (! (flags & 0x01)) {                                             // If seek NOT complete (bit0 = 0 = even)...
      return (0);                                                       // Seek not finished
    }
                                                                        // Else if seek IS complete (bit0 = 1 = odd)...
    if (flags & 0x0c)                                                   // If carrier error high or rssi low
      logv ("bc_seek_handle carrier error high or rssi low flags: 0x%2.2x    curr_freq_int: %d", flags, curr_freq_int); // Get this at limits

    curr_freq_int = freq_get ();

    if (flags & 0x02) {                                                 // If band limit reached
      if (curr_freq_int <= curr_freq_lo) {                              // If lower limit (must have been seek down)
        logd ("bc_seek_handle restart seek down    flags: 0x%x    curr_freq_int: %d", flags, curr_freq_int);
        //chip_imp_seek_state_sg (seek_state);                          // Restart seek in original direction (down)
        chip_imp_freq_sg (curr_freq_hi);
        reg_set_slow (0x09, 2);                                         // Set SRCH_TUNE_MODE: TUNE_MODE_AUTO
      }
      else if (curr_freq_int >= curr_freq_hi) {                         // If upper limit (must have been seek up)
        logd ("bc_seek_handle restart seek up    flags: 0x%x    curr_freq_int: %d", flags, curr_freq_int);
        //chip_imp_seek_state_sg (seek_state);                          // Restart seek in original direction (up)
        chip_imp_freq_sg (curr_freq_lo);
        reg_set_slow (0x09, 2);                                         // Set SRCH_TUNE_MODE: TUNE_MODE_AUTO
      }
      else {                                                            // If not at a limit. MIGHT be hung here
        logd ("bc_seek_handle unknown seek error    flags: 0x%x    curr_freq_int: %d", flags, curr_freq_int);     // 0x27, 0x2b bc_reg
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

  int seek_stop () {
    logd ("seek_stop");
    reg_set (0x09, 0);                                                    // Set SRCH_TUNE_MODE: TUNE_MODE_TERMINATE to stop searching.

    curr_seek_state = 0;
    return (curr_seek_state);
  }


  int band_set (int low , int high, int band) {                 // ? Do we need to stop/restart RDS power in reg 0x00 ? Or rbds_set to flush ?
    logd ("band_set low: %d  high: %d  band: %d", low, high, band);
    bc_reg_ctl &= ~0x01;                                                  // bit0  = 0 (BND_EUROPE_US)
    if (low < 87500)
      bc_reg_ctl |= 0x01;                                                 // Set CTL: + BND_JAPAN Japan
    if (reg_set (0x01, bc_reg_ctl) < 0) {                                 //
      loge ("bc_band_set error writing 0x01");
    }
    return (0);
  }

  int freq_inc_set (int inc) {
    logd ("freq_inc_set: %d", inc);
    //reg_set (0xfd | 0x10000, inc);                                      // Set: BC_REG_SRCH_STEPS  Reg 0xfd write/read bad on BCM4325 !!, perhaps because last byte RDS data, sets 256 KHz jumps
    //ms_sleep (1);//20000);
    return (0);
  }
  int emph75_set (int emph75) {
    logd ("emph75_set: %d", emph75);
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
    logd ("rbds_set: %d",rbds);
    if (rbds)
      reg_set (0x02, 0x03); //  ?? correct ? + flush
    else
      reg_set (0x02, 0x02); //  ?? correct ? + flush
    return (rbds);
  }


    // Chip API:   

//#define  SUPPORT_RDS
#ifdef  SUPPORT_RDS
  #include "rds_bch.c"
#else
  int rds_poll (unsigned char * rds_grpd) {return (EVT_GET_NONE);}
#endif
                                                                        // Polling function called every event_sg_ms milliseconds. Not used remotely but could be in future.
  int chip_imp_event_sg (unsigned char * rds_grpd) {
    //logd ("chip_imp_event_sg: %p", rds_grpd);
    int ret = 0;
    ret = rds_poll (rds_grpd);
    return (ret);
  }

  int chip_imp_api_mode_sg (int api_mode) {
    if (api_mode == GET)
      return (curr_api_mode);
    curr_api_mode = api_mode;
    logd ("chip_imp_api_mode_sg curr_api_mode: %d", curr_api_mode);
    return (curr_api_mode);
  }

  int chip_imp_api_state_sg (int state) {
    logd ("chip_imp_api_state_sg state: %d", state);
    if (state == GET)
      return (curr_api_state);

    int ret = 0;
    if (state == 0) {
      if (curr_api_mode)
        ret = shim_hci_stop ();
      else
        ret = uart_hci_stop ();
      curr_api_state = 0;
      return (curr_api_state);
    }

    if (curr_api_mode)
      ret = shim_hci_start ();
    else
      ret = uart_hci_start ();

    if (ret)
      curr_api_state = 0;
    else
      curr_api_state = 1;
    return (curr_api_state);
  }

  int chip_imp_mode_sg (int mode) {
    if (mode == GET)
      return (curr_mode);
    curr_mode = mode;
    logd ("chip_imp_mode_sg curr_mode: %d", curr_mode);
    return (curr_mode);
  }

  int chip_imp_state_sg (int state) {
    if (state == GET)
      return (curr_state);

    logd ("chip_imp_state_sg state: %d", state);
    if (state == 0)
      return (pwr_off ());

    int ret = 0;
    //logd ("chip_imp_state_sg reg dump");
    //bc_reg_dump (0x00, 0xff, 1);
    //bc_reg_dump (0x00, 0x7f, 1);
    //bc_reg_dump (0x80, 0xff, 1);
    //bc_reg_dump (0x00, 0x4f, 1);
    //bc_reg_dump (0x00, 0x4f, 2);
    //bc_reg_dump (0x01, 0x50, 2);
    //bc_reg_dump (0x00, 0x4f, 4);

    int pwr_val = 0x01;                                                 // No RDS
    if (curr_rds_state)
      pwr_val = 0x03;
    if (reg_set (0x00, pwr_val) < 0)                                    // Write power reg.
      loge ("chip_imp_state_sg 1 error writing 0x00");
    else
      logd ("chip_imp_state_sg 1 success writing 0x00");

    quiet_ms_sleep (50);//22);                                                      // We are supposed to sleep 20 milliseconds here    //#define BCM2048_DEFAULT_POWERING_DELAY	20  // Sleep 20 milliseconds after power on

    bc_reg_aud_ctl0 = 0;//0x23; //0x03;   //!! Mute for now     0x5c;   // radio-bcm2048.c also sets I2S     ROUTE_I2S_ENABLE |= 0x20 !!!! Test for Galaxy Tab etc.

    ret = reg_get (0x00);
    if (ret < 0)                                                        // Read power reg.
      loge ("chip_imp_state_sg 1 error reading 0x00  ret: %d", ret);
    else
      logd ("chip_imp_state_sg 1 success reading 0x00  ret: %d", ret);

    if (reg_set (0x00, pwr_val) < 0) {                                  // Write power reg again. If this fails, the rest is useless.
      curr_state = 0;
      loge ("chip_imp_state_sg error writing 0x00 curr_state: %d", curr_state);
      return (curr_state);
    }
    else
      logd ("chip_imp_state_sg 2 success writing 0x00");

    ret = reg_get (0x00);
    if (ret < 0)                                                        // Read power reg.
      loge ("chip_imp_state_sg 2 error reading 0x00  ret: %d", ret);
    else
      logd ("chip_imp_state_sg 2 success reading 0x00  ret: %d", ret);

    reg_set (0x10 | 0x10000, 0x0000);                                   // Write an Interrupt Mask of 0x0000 so we don't get Interrupt packets such as this on the UART: 04 ff 01 08

    bc_reg_ctl = 4;                                                     // Set: CTL: BND_EUROPE_US + MANUAL + STEREO + BLEND write the band setting, mono/stereo blend setting.
    bc_reg_ctl |= 0x02;                                                 // Automatic Stereo/Mono (When stereo selected)

    if (reg_set (0x01, bc_reg_ctl) < 0)
      loge ("chip_imp_state_sg error writing 0x01");
    else
      logd ("chip_imp_state_sg success writing 0x01");

    if (reg_set (0x14, MAX_RDS_BLOCKS * 3) < 0)                         // 0x78 // 0x7e);  // BC_REG_RDS_WLINE, // 0x14  FIFO water line set level
      loge ("chip_imp_state_sg error writing 0x14");                    // ?? Usually fails ??
    else
      logd ("chip_imp_state_sg success writing 0x14");

    bc_reg_aud_ctl0 = 0x7c;                                 // 0x6c;    // 75 us, I2S, DAC, DAC left & right, no RF mute

    int bc_rev_id = reg_get (0x28);                                     // REV_ID always 0xff
    logd ("chip_imp_state_sg bc_rev_id: %d", bc_rev_id);

    //chip_imp_mute_sg (0);                                               // Unmute

    reg_set (0xfb | 0x20000,  0x00000000);                              // Audio PCM ????       fb 00 00 00 00 00 
    int inc = 100;
    reg_set (0xfd | 0x10000, inc);

    if (lg_get ()) {                                                       // If LG G2 and SHIM:   Special LG G2 stuff needed, or no audio
      if (curr_api_mode == 0) {                                         // If LG G2 and UART:   See bt-ven.c not yet in bt-hci.c
        unsigned char res_buf [MAX_HCI];
        int res_len;
        unsigned char hci_buf [] = {0, 0, 0, 0, 0, 0, 0, 0, 0xf3, 0x88, 0x01, 0x02, 0x05};  // hcitool cmd 3f 00 f3 88 01 02 05
        res_len = hci_cmd (0x3f, 0x00, hci_buf, sizeof (hci_buf), res_buf, sizeof (res_buf), hci_cmd_tmo);
        if (res_buf [7]) {
          logd ("chip_imp_state_sg g2 hci error: %d %s", res_buf [7], hci_err_get (res_buf [7]));      // !!!! Always error
        }
        if (res_len < 1 + 8)
          logd ("chip_imp_state_sg g2 hci_cmd error res_len: %d", res_len);                            // !!!! Always error
        else 
          logd ("chip_imp_state_sg g2 OK");
      }
    }

/*
    int vol_val = 0x00ff;                                               // Target audio level max about 27,000
    if (htc_get () && android_version >= 21) {                                   // For HTC One M7 Lollipop
      vol_val = 0x0040;                                                 // Target max about 27,000
      if (file_get ("/system/framework/htcirlibs.jar"))                 // If HTC One M7 GPE       (Stock Android 5 too ????)
        vol_val = 0x0060;
    }
    if (vol_val != 0x00ff)
      reg_set (0xf8 | 0x10000,  vol_val);                               // Vol Max              f8 00 ff 00 
*/

    chip_imp_vol_sg (65535);

    curr_state = 1;
    logd ("chip_imp_state_sg curr_state: %d", curr_state);
    return (curr_state);
  }

  int chip_imp_antenna_sg (int antenna) {
    if (antenna == GET)
      return (curr_antenna);
/*
    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_ANTENNA, antenna) < 0)     // 0 = common external, 1 = Sony Z/Z1 internal
      loge ("chip_imp_antenna_sg ANTENNA error");
    else
      logd ("chip_imp_antenna_sg ANTENNA success");
*/
    curr_antenna = antenna;
    logd ("chip_imp_antenna_sg curr_antenna: %d", curr_antenna);
    return (curr_antenna);
  }

  int chip_imp_band_sg (int band) {
    if (band == GET)
      return (curr_band);

    logd ("chip_imp_band_sg band: %d", band);

    curr_freq_lo =  87500;
    curr_freq_hi = 108000;

    if (band == 0)
      curr_freq_inc = 200;
    else
      curr_freq_inc = 100;

    band_set (curr_freq_lo, curr_freq_hi, band);

    freq_inc_set (curr_freq_inc);

    emph75_set (band);

    rbds_set (band);

    return (band);
  }

  int chip_imp_freq_sg (int freq) {                                        // 1 KHz resolution
    if (freq == GET)
      return (freq_get ());

    logd ("chip_imp_freq_sg: %d", freq);
    reg_set (0x0a | 0x10000, freq - bc_freq_lo);                        // Freq is offset from 64MHz
    //logd ("bc_freq_sg reg 0: %d", reg_get (0x00));
    if ( reg_set (0x09, 1) < 0){                                      // Set: SRCH_TUNE_MODE: PRESET
      loge ("chip_imp_freq_sg error");
      return (curr_freq_int);
    }
    curr_freq_int = freq;
    rds_init ();
    return (curr_freq_int);
  }

  int chip_imp_vol_sg (int vol) {
    if (vol == GET)
      return (curr_vol);

    reg_set (0xf8 | 0x00010000, vol / 256);//63);//vol / 256);

    curr_vol = vol;
    logd ("chip_imp_vol_sg curr_vol: %d", curr_vol);
    return (curr_vol);
  }

  int chip_imp_thresh_sg (int thresh) {
    if (thresh == GET)
      return (curr_thresh);
    curr_thresh = thresh;
    logd ("chip_imp_thresh_sg curr_thresh: %d", curr_thresh);
    return (curr_thresh);
  }

  int chip_imp_mute_sg (int mute) {
    if (mute == GET)
      return (curr_mute);

    //bc_reg_aud_ctl0 &= ~0x3f;                                           // Low 6 bits = 0
    bc_reg_aud_ctl0 &= ~0x1f;                                             // Don't turn I2S off (Or it interrupts transfer ??)

    if (mute)
      //bc_reg_aud_ctl0 |= 0x02;//0x03;                                   // Set: AUD_CTL0: MANUAL_MUTE_ON 0x02 + RF Mute 0x01 = 0x03     No dacs, dac enable or i2s enable
      bc_reg_aud_ctl0 |= 0x03;                                            // !! Must set FM mute bit so no blast of high volume on 4330/20780 !!  ?? RF Mute ???
    else
      bc_reg_aud_ctl0 |= 0x1d;//0x1c;                                     // Set: AUD_CTL0: dacs on, dac enable, no mutes, no i2s         !! Now RF mute

    if (reg_set (0x05 | 0x10000, bc_reg_aud_ctl0) < 0)                    //
      loge ("bc_mute_sg error writing 0x05");

    curr_mute = mute;
    logd ("chip_imp_mute_sg curr_mute: %d", curr_mute);
    return (curr_mute);
  }

  int chip_imp_softmute_sg (int softmute) {
    if (softmute == GET)
      return (curr_softmute);

    //#define BC_VAL_AUD_CTL0_RF_MUTE_DISABLE     0x00
    //#define BC_VAL_AUD_CTL0_RF_MUTE_ENABLE      0x01
    if (softmute)
      bc_reg_aud_ctl0 |= 0x01;
    else
      bc_reg_aud_ctl0 &= 0xfffe;    //~ 0x02;
    if (reg_set (0x05 | 0x10000, bc_reg_aud_ctl0) < 0)
      loge ("chip_imp_softmute_sg error writing 0x05");
    else
      logd ("chip_imp_softmute_sg success writing 0x05");
  
    curr_softmute = softmute;
    logd ("chip_imp_softmute_sg curr_softmute: %d", curr_softmute);
    return (curr_softmute);
  }

/*
#define BC_VAL_CTL_MANUAL                   0x00
#define BC_VAL_CTL_AUTO                     0x02    // BCM2048_STEREO_MONO_AUTO_SELECT      Automatic only works when BCM2048_STEREO_MONO_MANUAL_SELECT = 1 = 0x04

#define BC_VAL_CTL_MONO                     0x00
#define BC_VAL_CTL_STEREO                   0x04    // BCM2048_STEREO_MONO_MANUAL_SELECT

#define BC_VAL_CTL_BLEND                    0x00
#define BC_VAL_CTL_SWITCH                   0x08    // BCM2048_STEREO_MONO_BLEND_SWITCH
*/
  int chip_imp_stereo_sg (int stereo) {                                        //
    if (stereo == GET)
      return (curr_stereo);

    int ret = 0;

    logd ("chip_imp_stereo_sg: %d", stereo);                          // Default 0 = Blend Auto, 1 = Switch Auto, 2 = Stereo Force, 3 = Mono Force
    bc_reg_ctl &= ~0x0e;                                                // bits 1-3 = 0

    if (stereo)
      bc_reg_ctl |= 0x06;                                               // Set CTL: + STEREO + AUTO
    /*else if (stereo == 1)
      bc_reg_ctl |= 0x0e;                                               // Set CTL: + STEREO + AUTO + SWITCH
    else if (stereo == 2)
      bc_reg_ctl |= 0x04;//0x0c;                                        // Set CTL: + STEREO + SWITCH (? Or 4 for STEREO only ?
    else if (stereo == 0)*/
    else
      bc_reg_ctl |= 0x00;                                               // Set CTL: MONO

    if (reg_set (0x01, bc_reg_ctl) < 0)
      loge ("chip_imp_stereo_sg error writing 0x01");
    else
      logd ("chip_imp_stereo_sg success writing 0x01");

    curr_stereo = stereo;
    logd ("chip_imp_stereo_sg curr_stereo: %d", curr_stereo);
    return (curr_stereo);
  }

  int chip_imp_seek_state_sg (int seek_state) {
    if (seek_state == GET)
      return (curr_seek_state);

    if (seek_state == 0)
      return (seek_stop ());

    int ret = 0;
    curr_seek_state = seek_state;                                       // ?? Need for bc_seek_handle ??
    logd ("chip_imp_seek_state_sg seek_state: %d  bc_ctl_rssi_base: %d  curr_freq_int: %d", seek_state, bc_ctl_rssi_base, curr_freq_int);
    //seek_stop ();

    freq_inc_set (curr_freq_inc);                                       // !! Issues ! // Does nothing without "bc/ss"

    reg_set_slow (0xfc, 0x0);                                           // BC_REG_SRCH_METH Search method 0 = SRCH_METH_NORMAL normal search; 1 = preset search; 2 = rssi search.
    //reg_get (0xfc);                                                   // Read BC_REG_SRCH_METH Search method

    reg_set_slow (0x08, 0x0c);                                          // BC_REG_SRCH_CTL1 = 0x0c = ?? (2 bytes read as 0x020c)
    reg_set_slow (0xfe, 0x0);                                           // BC_REG_MAX_PRESET Max Preset = 0
 
    if (seek_state == 1) {
      reg_set_slow (0x07, bc_ctl_rssi_base + 0x80);                     // Direction = up
      chip_imp_freq_sg (freq_up_get (curr_freq_int));                   // Move up to next channel
    }
    else {//if (seek_state == 2) {
      reg_set_slow (0x07, bc_ctl_rssi_base);                            // Direction = down
      chip_imp_freq_sg (freq_dn_get (curr_freq_int));                   // Move dn to next channel
    }

    reg_set_slow (0x09, 2);                                             // Set SRCH_TUNE_MODE: TUNE_MODE_AUTO

    int ctr = 0;
    for (ctr = 0; ctr < 40; ctr ++) {                                   // With 4 second timeout by 40 times, each 100 ms...
      int flags = reg_get (0x12 | 0x10000);                             // Read 2 bytes event/RDS event register  For some reason, this needs to be done before
                                                                        //  RDS flags get and process, still fails some time
      //if (ena_log_tnr_evt)
      //  logd ("chip_imp_seek_state_sg flags: 0x%x", flags);
      if (bc_seek_handle (flags, seek_state))
        break;                                                          // Terminate loop
      else
        ms_sleep (101);
    }

    curr_seek_state = 0;
    rds_init ();
    return (curr_seek_state);
  }

  int chip_imp_rds_state_sg (int rds_state) {
    if (rds_state == GET)
      return (curr_rds_state);

    curr_rds_state = rds_state;
    logd ("chip_imp_rds_state_sg curr_rds_state: %d", curr_rds_state);
    return (curr_rds_state);
  }

  int chip_imp_rds_af_state_sg (int rds_af_state) {
    if (rds_af_state == GET)
      return (curr_rds_af_state);

    curr_rds_af_state = rds_af_state;
    logd ("chip_imp_rds_af_state_sg curr_rds_af_state: %d", curr_rds_af_state);
    return (curr_rds_af_state);
  }

  int chip_imp_rssi_sg (int fake_rssi) {
    int rssi = reg_get (0x0f);                          // Get RSSI
    rssi -= 144;
    if (ena_log_tnr_extra)
      logd ("chip_imp_rssi_sg: %d",rssi);

    curr_rssi = rssi;
    return (curr_rssi);
  }

  int chip_imp_pilot_sg (int fake_pilot) {
    return (curr_pilot);
  }

  int chip_imp_rds_pi_sg (int rds_pi) {
    if (rds_pi == GET)
      return (curr_rds_pi);
    int ret = -1;
    //ret = rds_pi_set (rds_pi);
    curr_rds_pi = rds_pi;
    return (curr_rds_pi);
  }
  int chip_imp_rds_pt_sg (int rds_pt) {
    if (rds_pt == GET)
      return (curr_rds_pt);
    int ret = -1;
    //ret = rds_pt_set (rds_pt);
    curr_rds_pt = rds_pt;
    return (curr_rds_pt);
  }
  char * chip_imp_rds_ps_sg (char * rds_ps) {
    if (rds_ps == GETP)
      return (curr_rds_ps);
    int ret = -1;
    //ret = rds_ps_set (rds_ps);
    strlcpy (curr_rds_ps, rds_ps, sizeof (curr_rds_ps));
    return (curr_rds_ps);
  }
  char * chip_imp_rds_rt_sg (char * rds_rt) {
    if (rds_rt == GETP)
      return (curr_rds_rt);
    int ret = -1;
    //ret = rds_rt_set (rds_rt);
    strlcpy (curr_rds_rt, rds_rt, sizeof (curr_rds_rt));
    return (curr_rds_rt);
  }

  char * chip_imp_extension_sg (char * reg) {
    if (reg == GETP)
      return (curr_extension);
    int ret = -1;
    //ret = reg_set (reg);
    strlcpy (curr_extension, reg, sizeof (curr_extension));
    return (curr_extension);
  }

