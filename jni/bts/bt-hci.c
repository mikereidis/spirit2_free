
/*
a mount|grep system

a "su -c \"mount -o remount,rw /system\""
  #a mv system/lib/libbt-hci.so system/lib/libbt-hcio.so

adb push libs/armeabi/libbt-hci.so /sdcard/
a cp /sdcard/libbt-hci.so /system/lib/libbt-hci.so 
al  /system/lib/libbt-hci*
*/

#define LOGTAG "sfbt-hci"

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <dlfcn.h>


  const char * copyright = "Copyright (c) 2011-2015 Michael A. Reid. All rights reserved.";

  #include "hcd/hcd_bch.c"                                              // Patchram data !!!!  //unsigned char make_over_60k_for_hal_file_size_get [32768] = {0};

  char bfm_rx_buf [288] = {0};
  int  bfm_rx_len = 0;

  int shim_hci_enable = 1;//0;                                          // Default 0 = UART, 1 = Bluedroid SHIM

#define  GENERIC_SERVER
  #include "utils.c"

  #include "halhci.h"

  void * bfm_send (char * buf, int len);

  #include "bts/bt-svc.c"

    // Host/Controller lib thread control block
  typedef struct {
    pthread_t       server_thread;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
  } bt_hc_cb_t;

  static          bt_hc_cb_t  hc_cb;
  static volatile uint8_t     thread_state = 0;       // 0 = stopped, 1 = started, 2 = not needed but thread still alive

  static void * bt_server_thread (void * arg) {
    uint16_t events;
    logd ("bt_server_thread started");

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

    return (NULL);    // compiler friendly
  }


    // BLUETOOTH HOST/CONTROLLER INTERFACE LIBRARY FUNCTIONS

  int   (* hcio_init)           (const bt_hc_callbacks_t * p_cb, unsigned char * local_bdaddr);
  void  (* hcio_set_power)      (bt_hc_chip_power_state_t state);
  int   (* hcio_lpm)            (bt_hc_low_power_event_t event);
  void  (* hcio_preload)        (TRANSAC transac);
  void  (* hcio_postload)       (TRANSAC transac);
  int   (* hcio_transmit_buf)   (TRANSAC transac, char * p_buf, int len);
//  int   (* hcio_set_rxflow)     (bt_rx_flow_state_t state);
  int   (* hcio_logging)        (bt_hc_logging_state_t state, char * p_path, bool save_existing);
  void  (* hcio_cleanup)        (void);
  int   (*hcio_tx_cmd)          (TRANSAC transac, char *p_buf, int len);    /** sends commands to hc layer (e.g. SCO state) */

  static const bt_hc_callbacks_t * hcib = NULL;

  static int transmit_buf (TRANSAC transac, char * p_buf, int len);

  static void  hcib_preload_result_cb    (TRANSAC transac, bt_hc_preload_result_t result) {  /* preload initialization callback */
    logd ("hcib_preload_result_cb transac: %p  result: %d", transac, result);

    hcib -> preload_cb (transac, result);
    logd ("hcib_preload_result_cb done");
  }
  static void  hcib_postload_result_cb   (TRANSAC transac, bt_hc_postload_result_t result) { /* postload initialization callback */
    logd ("hcib_postload_result_cb transac: %p  result: %d", transac, result);
    hcib -> postload_cb (transac, result);
    logd ("hcib_postload_result_cb done");
  }
  static void  hcib_lpm_result_cb        (bt_hc_lpm_request_result_t result) {               /* lpm enable/disable callback */
    logd ("hcib_lpm_result_cb result: %d", result);
    hcib -> lpm_cb (result);
    logd ("hcib_lpm_result_cb done");
  }
  static void  hcib_hostwake_ind_cb      (bt_hc_low_power_event_t event) {                   /* called upon bt host wake signal */
    logd ("hcib_hostwake_ind_cb event: %d", event);
    hcib -> hostwake_ind (event);
    logd ("hcib_hostwake_ind_cb done");
  }
  int allocs = 0, deallocs = 0;
  static char * hcib_alloc_mem_cb         (int size) {                                        /* datapath buffer allocation callback (callout) */
    logv ("hcib_alloc_mem_cb size: %d", size);
    char * ret = NULL;
    if (hcib && hcib -> alloc)
      ret = hcib -> alloc (size);
    logv ("hcib_alloc_mem_cb done ret: %p", ret);
    return (ret);
  }
  static void hcib_dealloc_mem_cb       (TRANSAC transac) {//, char *p_buf) {                    /* datapath buffer deallocation callback (callout) */
    logv ("hcib_dealloc_mem_cb transac: %p");//  p_buf: %p", transac, p_buf);
    /*int ret = 0;
    ret =*/ hcib -> dealloc (transac);//, p_buf);
    logv ("hcib_dealloc_mem_cb done");// ret: %d", ret);
    return;// (ret);
  }

  static int   hcib_data_ind_cb          (TRANSAC transac, char *p_buf, int len) {           /* a previously setup buffer is read and available for processing buffer */

    if ((len >= 5 && p_buf [3] == 0x15 && p_buf [4] == 0xfc) ||         // Broadcom 3F  15
        (len >= 5 && p_buf [3] == 0x33 && p_buf [4] == 0xfd) ||         // TI       3F 133
        (len >= 5 && p_buf [3] == 0x01 && p_buf [4] == 0x10 && bfm_rx_len == 0)) {// 4   1      "ssd 4 1"
      logv ("bfm_ hcib_data_ind_cb transac: %p  p_buf: %p  len: %d  bfm_rx_len: %d", transac, p_buf, len, bfm_rx_len);
      hex_dump ("bfm_ ", 16, (unsigned char *) p_buf, len);

      bfm_rx_len = len;
      memcpy (bfm_rx_buf, p_buf, len);
    }

    logv ("hcib_data_ind_cb transac: %p  p_buf: %p  len: %d", transac, p_buf, len);
    int shortlen = len;
    if (shortlen < 0)
      shortlen = 0;
    if (shortlen > 32)
      shortlen = 32;
    hex_dump ("RXB ", 16, (unsigned char *) p_buf, shortlen);

    int ret = 0;
    ret = hcib -> data_ind (transac, p_buf, len);
    logv ("hcib_data_ind_cb done ret: %d", ret);
    return (ret);
  }
  static int   hcib_tx_result_cb         (TRANSAC transac, char *p_buf, bt_hc_transmit_result_t result) {/* transmit result callback */
    logv ("hcib_tx_result_cb : transac: %p  p_buf: %p  result: %d", transac, p_buf, result);
    int ret = 0;
    ret = hcib -> tx_result (transac, p_buf, result);
    logv ("hcib_tx_result_cb done ret: %d", ret);
    return (ret);
  }

  static const bt_hc_callbacks_t bt_hc_callbacks = {
    sizeof (bt_hc_callbacks_t),
    hcib_preload_result_cb,
    hcib_postload_result_cb,
    hcib_lpm_result_cb,
    hcib_hostwake_ind_cb,
    hcib_alloc_mem_cb,
    hcib_dealloc_mem_cb,
    hcib_data_ind_cb,
    hcib_tx_result_cb
  };


  static void server_thread_stop () {
    if (thread_state == 1) {
      thread_state = 2; // not needed but still running
    }
  }

  static int server_thread_start () {
    pthread_attr_t thread_attr;
    struct sched_param param;
    int policy, result;

    if (thread_state == 1) {
      logd ("server_thread_start & thread_state = running");
      return (BT_HC_STATUS_SUCCESS);
    }

    if (thread_state == 2) {
      logd ("server_thread_start & thread_state = not needed and running");
      return (BT_HC_STATUS_SUCCESS);
    }

    thread_state = 1;
    pthread_mutex_init (& hc_cb.mutex, NULL);
    pthread_cond_init  (& hc_cb.cond, NULL);
    pthread_attr_init  (& thread_attr);

    if (pthread_create (& hc_cb.server_thread, & thread_attr, bt_server_thread, NULL) != 0) {
      loge ("pthread_create failed");
      thread_state = 0;
      return (BT_HC_STATUS_FAIL);
    }

    if (pthread_getschedparam (hc_cb.server_thread, & policy, & param) == 0) {
      policy = SCHED_NORMAL;
      //param.sched_priority = BTHC_MAIN_THREAD_PRIORITY;
      result = pthread_setschedparam (hc_cb.server_thread, policy, & param);
      if (result != 0) {
        logd ("libbt-hci init: pthread_setschedparam failed (%s)", strerror (result));  // errno ??
      }
    }
    return (BT_HC_STATUS_SUCCESS);
  }

  static int init (const bt_hc_callbacks_t * p_cb, unsigned char * local_bdaddr) {

    logd ("init p_cb: %p  local_bdaddr: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x", p_cb, local_bdaddr [0], local_bdaddr [1], local_bdaddr [2], local_bdaddr [3], local_bdaddr [4], local_bdaddr [5] );

    utils_init ();

    if (p_cb == NULL) {
      loge ("init no user callbacks");
      return (BT_HC_STATUS_FAIL);
    }
    hcib = p_cb;

    int ret = hcio_init (& bt_hc_callbacks, local_bdaddr);
    logd ("init ret: %d", ret);

    if (ret != BT_HC_STATUS_SUCCESS) 
      return (ret);

    ret = 0;
    return (ret);
  }


    /** Chip power control */
  static void set_power (bt_hc_chip_power_state_t state) {
    logd ("set_power state: %d", state);
    hcio_set_power (state);
    logd ("set_power hcio_set_power done");

    int ret = 0;
    if (state == BT_HC_CHIP_PWR_ON)
      ret = server_thread_start ();
    else
      server_thread_stop ();

    logd ("set_power done");

    return;
  }

    /** Configure low power mode wake state */
  static int lpm (bt_hc_low_power_event_t event) {
    logv ("lpm event: %d", event);
    int ret = hcio_lpm (event);
    logv ("lpm ret: %d", ret);
    return (ret);
  }

    /** Called prio to stack initialization */
  static void preload (TRANSAC transac) {
    logd ("preload transac: %p", transac);
    hcio_preload (transac);
    logd ("preload done");
    return;
  }

    /** Called post stack initialization */
  static void postload (TRANSAC transac) {
    logd ("postload transac: %p", transac);
    hcio_postload (transac);
    logd ("postload done");
    return;
  }

    /** Transmit frame */
  static int transmit_buf (TRANSAC transac, char * p_buf, int len) {
    logv ("transmit_buf transac: %p  p_buf: %p  len: %d", transac, p_buf, len);
    int shortlen = len;
    if (shortlen < 0)
      shortlen = 0;
    if (shortlen > 32)
      shortlen = 32;
    hex_dump ("TXB ", 16, (unsigned char *) p_buf, shortlen);
    int ret = hcio_transmit_buf (transac, p_buf, len);
    logv ("transmit_buf ret: %d", ret);
    return (ret);
  }

    /** Controls receive flow */
/*
  static int set_rxflow (bt_rx_flow_state_t state) {
    logd ("set_rxflow state: %d", state);
    int ret = hcio_set_rxflow (state);
    logd ("set_rxflow ret: %d", ret);
    return (ret);
  }
*/
    /** Controls HCI logging on/off */
  static int logging (bt_hc_logging_state_t state, char * p_path, bool save_existing) {
    logd ("logging state: %d  p_path: \"%s\"  save_existing: %d", state, p_path, save_existing);
    int ret = hcio_logging (state, p_path, save_existing);
    logd ("logging ret: %d", ret);
    return (ret);
  }

    /** Closes the interface */
  static void cleanup (void) {
    logd ("cleanup");
    hcio_cleanup ();
    logd ("cleanup done");
    return;
  }

    /** sends commands to hc layer (e.g. SCO state) */
    // Same as transmit_buf:
  static int tx_cmd (TRANSAC transac, char * p_buf, int len) {
    logd ("tx_cmd transac: %p  p_buf: %p  len: %d", transac, p_buf, len);
    /*int shortlen = len;
    if (shortlen < 0)
      shortlen = 0;
    if (shortlen > 32)
      shortlen = 32;
    hex_dump ("TXB ", 16, (unsigned char *) p_buf, shortlen);*/
    int ret = hcio_tx_cmd (transac, p_buf, len);
    logd ("tx_cmd ret: %d", ret);
    return (ret);
  }


  static const bt_hc_interface_t bluetoothHCLibInterface = {
    sizeof (bt_hc_interface_t),
    init,
    set_power,
    lpm,
    preload,
    postload,
    transmit_buf,
//    set_rxflow,   // !! Delete
    logging,
    cleanup
, tx_cmd    // !! Add
  };

  int  have_hcio = 1;
  char hcio_lib [64] = "libbt-hcio.so";
  void * hcio_handle = NULL;
  const bt_hc_interface_t * (* hcio_bt_hc_get_interface) (void);
  const bt_hc_interface_t * hcio_bt_hc_interface = NULL;

  const bt_hc_interface_t * bt_hc_get_interface (void) {
    logd ("bt_hc_get_interface have_hcio: %d", have_hcio);
    if (! have_hcio)
      return (NULL);

    have_hcio = 0;

    if (hcio_handle == NULL) {
      hcio_handle = dlopen (hcio_lib, RTLD_LAZY);
      if (hcio_handle == NULL)
        return (NULL);

      hcio_bt_hc_get_interface = dlsym (hcio_handle, "bt_hc_get_interface");
      if (hcio_bt_hc_get_interface == NULL)
        return (NULL);

      hcio_bt_hc_interface = hcio_bt_hc_get_interface ();
      logd ("bt_hc_get_interface hcio_bt_hc_interface: %p", hcio_bt_hc_interface);

      hcio_init = hcio_bt_hc_interface->init;
      if (hcio_init == NULL)
        return (NULL);
      hcio_set_power = hcio_bt_hc_interface->set_power;
      if (hcio_set_power == NULL)
        return (NULL);
      hcio_lpm = hcio_bt_hc_interface->lpm;
      if (hcio_lpm == NULL)
        return (NULL);
      hcio_preload = hcio_bt_hc_interface->preload;
      if (hcio_preload == NULL)
        return (NULL);
      hcio_postload = hcio_bt_hc_interface->postload;
      if (hcio_postload == NULL)
        return (NULL);
      hcio_transmit_buf = hcio_bt_hc_interface->transmit_buf;
      if (hcio_transmit_buf == NULL)
        return (NULL);
/*      hcio_set_rxflow = hcio_bt_hc_interface->set_rxflow;
      if (hcio_set_rxflow == NULL)
        return (NULL);*/
      hcio_logging = hcio_bt_hc_interface->logging;
      if (hcio_logging == NULL)
        return (NULL);
      hcio_cleanup = hcio_bt_hc_interface->cleanup;
      if (hcio_cleanup == NULL)
        return (NULL);
      hcio_tx_cmd = hcio_bt_hc_interface->tx_cmd;
      if (tx_cmd == NULL)
        return (NULL);
    }
    have_hcio = 1;
    logd ("bt_hc_get_interface done have_hcio: %d", have_hcio);

    return (& bluetoothHCLibInterface);
  }

  static char * gv_mem = NULL;

  TRANSAC bfm_send (char * buf, int len) {
    logv ("bfm_send");
    int data_len = len - 3;//3;//0;
    int full_len = len + 8;

    gv_mem = NULL;
    if (gv_mem == NULL) {
      if (hcib_alloc_mem_cb != NULL)
        gv_mem = hcib_alloc_mem_cb (288);//full_len);
      else
        loge ("bfm_send hcib_alloc_mem_cb = NULL");
    }
    if (gv_mem == NULL) {
      loge ("bfm_send alloc error");
      return (NULL);
    }
    int ctr = 0;
    for (ctr = 0; ctr < full_len; ctr ++)
      gv_mem [ctr] = 0;

    uint16_t ocf = buf [0];                                               // 0x15
    uint16_t ocf_hi = (buf [1] & 0x03) << 8;
    ocf |= ocf_hi;
    uint16_t ogf = (buf [1] & 0xfc) >> 2;

    char * hci_data = & buf [3];
    int hci_len = len - 3; // buf [2]
    uint16_t opcode = (ogf * 1024) | ocf;

    logv ("bfm_send ogf: 0x%x  ocf: 0x%x  opcode: 0x%x  hci_data: %p  hci_len: %d", ogf, ocf, opcode, hci_data, hci_len);

    uint16_t HC_BT_HDR_event          = 0x2000;   // MSG_STACK_TO_HC_HCI_CMD | 0 / LOCAL_BR_EDR_CONTROLLER_ID
    uint16_t HC_BT_HDR_len            = len;//0x0003;
    uint16_t HC_BT_HDR_offset         = 0x0000;   // Starts at [8] so offset = 0
    uint16_t HC_BT_HDR_layer_specific = opcode;//0x1001;   // Layer Specific = Our code to use. (WAS Opcode means internal command means just dealloc, don't callback tx_result)

    gv_mem [0] = HC_BT_HDR_event             & 0x00ff;
    gv_mem [1] = HC_BT_HDR_event         >>8 & 0x00ff;
    gv_mem [2] = HC_BT_HDR_len               & 0x00ff;
    gv_mem [3] = HC_BT_HDR_len           >>8 & 0x00ff;
    gv_mem [4] = HC_BT_HDR_offset            & 0x00ff;
    gv_mem [5] = HC_BT_HDR_offset        >>8 & 0x00ff;
    gv_mem [6] = HC_BT_HDR_layer_specific    & 0x00ff;
    gv_mem [7] = HC_BT_HDR_layer_specific>>8 & 0x00ff;

    if (len >= 3 && len <= 258) {
      memcpy (& gv_mem [8], buf, len);
    }
    else
      loge ("bfm_send len error: %d", len);
    TRANSAC itransac = gv_mem;//buf;
    int ret = transmit_buf (itransac, & gv_mem [8], len);
    logv ("bfm_send ret: %d", ret);
    return (gv_mem);
  }

