
/*
a mount|grep system

a mount -o remount,rw /system
  #a mv system/vendor/lib/libbt-vendor.so system/vendor/lib/libbt-vendoro.so

adb push libs/armeabi/libbt-vendor.so /sdcard/
a cp /sdcard/libbt-vendor.so /system/vendor/lib/libbt-vendor.so 
a chmod 644 /system/vendor/lib/libbt-vendor.so
al  /system/vendor/lib/libbt-vendor*


a mount -o remount,rw /system ; adb push libs/armeabi/libbt-vendor.so /sdcard/ ; a cp /sdcard/libbt-vendor.so /system/vendor/lib/libbt-vendor.so ; al  /system/vendor/lib/libbt-vendor*
*/

#define LOGTAG "sfbt-ven"

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <dlfcn.h>

#include "halven.h"
#include "halhci.h"
#include "bt-gen.h" //"bt_hci_bdroid.h"


  const char * copyright = "Copyright (c) 2011-2015 Michael A. Reid. All rights reserved.";

  #include "hcd/hcd_bch.c"                                                  // Patchram data !!!!  //unsigned char make_over_60k_for_hal_file_size_get [32768] = {0};

  char bfm_rx_buf [288] = {0};
  int  bfm_rx_len = 0;

  int shim_hci_enable = 1;//0;                                              // Default 0 = UART, 1 = Bluedroid SHIM

#define  GENERIC_SERVER
  #include "utils.c"


  static bt_vendor_callbacks_t * bt_ven_cbacks = NULL;

    // Host/Controller lib thread control block
  typedef struct {
    pthread_t       server_thread;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
  } bt_server_thread_t;

  static          bt_server_thread_t  server_thread;

  static volatile uint8_t     thread_state = 0;                         // 0 = stopped, 1 = started, 2 = not needed but thread still alive

  void * bfm_send (char * buf, int len);

  #include "bts/bt-svc.c"

/*
int bc_g2_pcm_set () {
  unsigned char res_buf [MAX_HCI];
  int res_len;
  logd ("bc_g2_pcm_set 1");     // hcitool cmd 3f 00 f3 88 01 02 05

unsigned char hci_g2 [] =    {0, 0, 0, 0, 0x01, 0x00, 0xfc, 0x05, 0xf3, 0x88, 0x01, 0x02, 0x05 }; // OGF: 0x3f    OCF: 0x00   Parameter Total Length:  5
int hci_len = sizeof (hci_g2) - 8;    // 13 - 8 = 5
void * res_ptr = bfm_send (& hci_g2 [5], hci_len + 3);    // 5 + 3 = 8

//unsigned char hci_buf [] = {0, 0, 0, 0, 0,    0,    0,    0,    0xf3, 0x88, 0x01, 0x02, 0x05};
//hci_buf [5] = 0;    // ocf
//hci_buf [6] = 0xfc; // ogf << 2
//hci_buf [7] = 5;    // ?
//void * res_ptr = bfm_send (& hci_buf [5], sizeof (hci_buf) - 5);

  if (res_ptr == NULL)
    loge ("bc_g2_pcm_set bfm_send error res_ptr: %p", res_ptr);
  else 
    loge ("bc_g2_pcm_set OK");
  return (0);
}
*/


  static void * bt_server_thread (void * arg) {
    uint16_t events;
    logd ("bt_server_thread start");

    //bc_g2_pcm_set ();

    int poll_tmo_ms = 0;                                                // No polling
    int hci_port = NET_PORT_HCI;

    while (thread_state == 1) {                                         // While needed and running...
      logd ("bt_server_thread thread_state = needed and running");
      gen_server_loop (hci_port, poll_tmo_ms);                          // !! Doesn't return if no packets sent

      if (thread_state == 1)
        sleep (1);                                                      // Keep trying if error return
    }
    logd ("bt_server_thread gen_server_exiting");

    thread_state = 0;
    logd ("bt_server_thread exit");

    pthread_exit (NULL);
    loge ("bt_server_thread exit 2");

    return (NULL);                                                      // Compiler friendly ; never reach
  }


  static void server_thread_stop () {
    logd ("server_thread_stop thread_state: %d", thread_state);
    if (thread_state == 1) {
      thread_state = 2;                                                 // Thread not needed but still running
    }
  }

  static int server_thread_start () {
    pthread_attr_t thread_attr;
    struct sched_param param;
    int policy, result;

    logd ("server_thread_start thread_state: %d", thread_state);

    if (thread_state == 1) {
      logd ("server_thread_start & thread_state = running");
      return (BT_HC_STATUS_SUCCESS);
    }

    if (thread_state == 2) {
      logd ("server_thread_start & thread_state = not needed and running");
      return (BT_HC_STATUS_SUCCESS);
    }

    thread_state = 1;                                                   // Thread started
    pthread_mutex_init (& server_thread.mutex, NULL);
    pthread_cond_init  (& server_thread.cond, NULL);
    pthread_attr_init  (& thread_attr);

    if (pthread_create (& server_thread.server_thread, & thread_attr, bt_server_thread, NULL) != 0) {
      loge ("server_thread_start pthread_create failed");
      thread_state = 0;
      return (BT_HC_STATUS_FAIL);
    }

    if (pthread_getschedparam (server_thread.server_thread, & policy, & param) == 0) {
      policy = SCHED_NORMAL;
      //param.sched_priority = BTHC_MAIN_THREAD_PRIORITY;
      result = pthread_setschedparam (server_thread.server_thread, policy, & param);
      if (result != 0) {
        loge ("server_thread_start pthread_setschedparam failed (%s)", strerror (result));  // errno ??
      }
    }
    logd ("server_thread_start done");
    return (BT_HC_STATUS_SUCCESS);
  }


  void                  * veno_handle   = NULL;
  bt_vendor_interface_t * veno_if       = NULL;

  int   (* veno_init)       (const bt_vendor_callbacks_t * p_cb, unsigned char * local_bdaddr);
  int   (* veno_op)         (bt_vendor_opcode_t opcode, void * param);
  void  (* veno_cleanup)    (void);

  void shim_stop () {
    logd ("shim_stop veno_handle: %p", veno_handle);
    if (veno_handle != NULL)
      dlclose (veno_handle);
    veno_handle = NULL;
    veno_if = NULL;
    veno_init = NULL;
    veno_op = NULL;
    veno_cleanup = NULL;
    logd ("shim_stop done");
  }

  char veno_lib [64] = "libbt-vendoro.so";

  bt_vendor_interface_t * shim_start () {
    logd ("shim_start veno_handle: %p", veno_handle);

    if (veno_handle != NULL && veno_if != NULL)
      return (veno_if);

    shim_stop ();

    veno_handle = dlopen (veno_lib, RTLD_LAZY);
    if (veno_handle != NULL)
      veno_if = dlsym (veno_handle, "BLUETOOTH_VENDOR_LIB_INTERFACE");
    if (veno_if != NULL)
      veno_init = veno_if->init;
    if (veno_init != NULL)
      veno_op = veno_if->op;
    if (veno_op != NULL)
      veno_cleanup = veno_if->cleanup;

    if (veno_handle == NULL || veno_if == NULL || veno_init == NULL || veno_op == NULL || veno_cleanup == NULL) {
      shim_stop ();
      loge ("shim_start done veno_if: %p", veno_if);
    }
    else
      logd ("shim_start done veno_if: %p", veno_if);

    return (veno_if);
  }
    

    // Open interface
  static int ven_init (const bt_vendor_callbacks_t * p_cb, unsigned char * local_bdaddr) {
    logd ("ven_init p_cb: %p  local_bdaddr: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x", p_cb, local_bdaddr [0], local_bdaddr [1], local_bdaddr [2], local_bdaddr [3], local_bdaddr [4], local_bdaddr [5] );
    if (p_cb == NULL) {
      loge ("ven_init no user callbacks");
      return (BT_HC_STATUS_FAIL);
    }
    bt_ven_cbacks = (bt_vendor_callbacks_t *) p_cb;                     // Store reference to user callbacks

    if (shim_start () == NULL) {
      loge ("ven_init shim_start error");
      return (BT_HC_STATUS_FAIL);
    }

    int ret = -88;
    if (veno_init)
      ret = veno_init (p_cb, local_bdaddr);
    logd ("ven_init ret: %d", ret);
    if (ret == 0)
      ret = server_thread_start ();

    return (ret);
  }


  static char * op_str_get (bt_vendor_opcode_t opcode) {
    switch (opcode) {
      case BT_VND_OP_POWER_CTRL:            return ("BT_VND_OP_POWER_CTRL");
      case BT_VND_OP_FW_CFG:                return ("BT_VND_OP_FW_CFG");
      case BT_VND_OP_SCO_CFG:               return ("BT_VND_OP_SCO_CFG");
      case BT_VND_OP_USERIAL_OPEN:          return ("BT_VND_OP_USERIAL_OPEN");
      case BT_VND_OP_USERIAL_CLOSE:         return ("BT_VND_OP_USERIAL_CLOSE");
      case BT_VND_OP_GET_LPM_IDLE_TIMEOUT:  return ("BT_VND_OP_GET_LPM_IDLE_TIMEOUT");
      case BT_VND_OP_LPM_SET_MODE:          return ("BT_VND_OP_LPM_SET_MODE");
      case BT_VND_OP_LPM_WAKE_SET_STATE:    return ("BT_VND_OP_LPM_WAKE_SET_STATE");
      case BT_VND_OP_SET_AUDIO_STATE:       return ("BT_VND_OP_SET_AUDIO_STATE");
      case BT_VND_OP_EPILOG:                return ("BT_VND_OP_EPILOG");
    }
    return ("UNKNOWN");
  }

  int need_bc_g2 = 1;
    // Requested operation
  static int ven_op (bt_vendor_opcode_t opcode, void * param) {
    if (ena_log_ven_extra && opcode != BT_VND_OP_LPM_WAKE_SET_STATE)
      logd ("ven_op opcode: %d (%s) param: %p", opcode, op_str_get (opcode), param);
    int ret = -77;
    if (veno_op)
      ret = veno_op (opcode, param);
    if (need_bc_g2 && opcode == 7){//BT_VND_OP_LPM_WAKE_SET_STATE   1) {    // BT_VND_OP_FW_CFG
      need_bc_g2 = 0;
      //bc_g2_pcm_set ();
    }
    if (ena_log_ven_extra && opcode != BT_VND_OP_LPM_WAKE_SET_STATE)
      logd ("ven_op ret: %d", ret);
    return (ret);
  }

    // Close interface
  static void ven_cleanup (void) {
    logd ("ven_cleanup");
    server_thread_stop ();

    if (veno_cleanup)
      veno_cleanup ();

    bt_ven_cbacks = NULL;

    shim_stop ();

    logd ("ven_cleanup done");
    return;
  }

    // Entry point of DLib
  const bt_vendor_interface_t BLUETOOTH_VENDOR_LIB_INTERFACE = {
    sizeof (bt_vendor_interface_t),
    ven_init,
    ven_op,
    ven_cleanup
  };


  static void bfm_cback (void * p_mem) {
    if (ena_log_ven_extra)
      logd ("bfm_cback p_mem: %p", p_mem);
    if (p_mem == NULL)
      return;

    char * p_buf = (char *) p_mem;
    int len = (p_buf [1 + 8] & 0x00ff) + 2 + 8;       // x + 4 + 6 = len

    if (ena_log_ven_extra)
      hex_dump ("bfm_cb", 16, (unsigned char *) p_buf, len);//32);//288-16);//len);

    if (len > 288 || len < 0) {
      loge ("bfm_cback len: %d", len);
      if (bt_ven_cbacks != NULL)
        bt_ven_cbacks->dealloc (p_mem);
      return;
    }

    if (ena_log_ven_extra)
      logd ("bfm_cback p_buf: %p  len: %d  bfm_rx_len: %d", p_buf, len, bfm_rx_len);

    memcpy (bfm_rx_buf, & p_buf [8], len - 8);                          // Copy HCI data to bfm_rx_buf
    bfm_rx_len = len;                                                   // Return length to flag ready now that data is written

    if (ena_log_ven_extra)
      logd ("bfm_cback p_buf: %p  len: %d", p_buf, len);
    int shortlen = len;
    if (shortlen < 0)
      shortlen = 0;
    if (shortlen > 32)
      shortlen = 32;

//    if (ena_log_ven_extra)
//      hex_dump ("RXB ", 16, (unsigned char *) p_buf, shortlen);

        // Must free the RX event buffer    - Or re-use ??
    if (bt_ven_cbacks != NULL)
      bt_ven_cbacks->dealloc (p_mem);

    if (ena_log_ven_extra)
      logd ("bfm_cback done");
  }


  static char * gv_mem = NULL;

  void * bfm_send (char * buf, int len) {
    if (ena_log_ven_extra)
      logd ("bfm_send buf: %p  len: %d", buf, len);
    int data_len = len - 3;
    int full_len = len + 8;

    if (! bt_ven_cbacks)
      return (NULL);

    gv_mem = NULL;
    if (gv_mem == NULL) {
      if (bt_ven_cbacks->alloc != NULL)
        //p_buf = (HC_BT_HDR *) bt_ven_cbacks->alloc ();
        gv_mem = bt_ven_cbacks->alloc (288);    //full_len);            // BT_HC_HDR_SIZE + HCI_CMD_PREAMBLE_SIZE
      else
        loge ("bfm_send bt_ven_cbacks->alloc = NULL");
    }
    if (gv_mem == NULL) {
      loge ("bfm_send alloc error");
      return (NULL);
    }
//    int ctr = 0;
//    for (ctr = 0; ctr < full_len; ctr ++)                               // Zero memory
//      gv_mem [ctr] = 0;

    uint16_t ocf = buf [0];                                             // 0x15
    uint16_t ocf_hi = (buf [1] & 0x03) << 8;
    ocf |= ocf_hi;
    uint16_t ogf = (buf [1] & 0xfc) >> 2;

    char * hci_data = & buf [3];
    int hci_len = len - 3; // buf [2]
    uint16_t opcode = (ogf * 1024) | ocf;

    if (ena_log_ven_extra)
      logd ("bfm_send ogf: 0x%x  ocf: 0x%x  opcode: 0x%x  hci_data: %p  hci_len: %d", ogf, ocf, opcode, hci_data, hci_len);

    int event_mask = 0x2000;
    //if (ocf == 0)
    //  event_mask = 0x2100;//0x0;//0;//0xff00;        0: Ctlr H/w error event - code:0x0
    uint16_t HC_BT_HDR_event          = event_mask;//0xffff;//Event Mask 0x2000;                         // MSG_STACK_TO_HC_HCI_CMD | 0 / LOCAL_BR_EDR_CONTROLLER_ID
    uint16_t HC_BT_HDR_len            = len;                            // 0x0003;
    uint16_t HC_BT_HDR_offset         = 0x0000;                         // Starts at [8] so offset = 0
    uint16_t HC_BT_HDR_layer_specific = opcode; // Layer Specific = Our code to use.

    gv_mem [0] = HC_BT_HDR_event             & 0x00ff;
    gv_mem [1] = HC_BT_HDR_event         >>8 & 0x00ff;
    gv_mem [2] = HC_BT_HDR_len               & 0x00ff;
    gv_mem [3] = HC_BT_HDR_len           >>8 & 0x00ff;
    gv_mem [4] = HC_BT_HDR_offset            & 0x00ff;
    gv_mem [5] = HC_BT_HDR_offset        >>8 & 0x00ff;
    gv_mem [6] = HC_BT_HDR_layer_specific    & 0x00ff;
    gv_mem [7] = HC_BT_HDR_layer_specific>>8 & 0x00ff;

    if (len >= 3 && len <= 258) {
      memcpy (& gv_mem [8], buf, len);                                  // Copy HCI data to gv_mem
    }
    else {
      loge ("bfm_send len error: %d", len);
      //len = 3;
      return (NULL);
    }
    int ret = -100;

    int shortlen = full_len;
    if (shortlen < 0)
      shortlen = 0;
    if (shortlen > 32)
      shortlen = 32;
    if (ena_log_ven_extra)
      hex_dump ("TXB ", 16, (unsigned char *) gv_mem, shortlen);

// hcitool cmd 3f 00 f3 88 01 02 05
/*
    if (ogf == 0x3f && ocf == 0) {
      ret = bt_ven_cbacks->xmit_cb (opcode, gv_mem, NULL);           // Send HCI command ; Callback bfm_cback() with response
      loge ("!!!!!! NULL cback !!!");
    }
    else
*/
      ret = bt_ven_cbacks->xmit_cb (opcode, gv_mem, bfm_cback);

    if (ena_log_ven_extra)
      logd ("bfm_send ret: %d", ret);
    return (gv_mem);
  }

