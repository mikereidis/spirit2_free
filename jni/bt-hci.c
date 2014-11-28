// Description     Caller calls this function to get API instance
// Returns         API table

/*


a mount|grep system

a "su -c \"mount -o remount,rw /dev/block/mmcblk0p35 /system\""
  #a mv system/lib/libbt-hci.so system/lib/libbt-hcio.so

adb push libs/armeabi/libbt-hci.so /sdcard/
a cp /sdcard/libbt-hci.so /system/lib/libbt-hci.so 
al  /system/lib/libbt-hci*


a "su -c \"mount -o remount,ro /dev/block/mmcblk0p35 /system\""
adb reboot

-rw-rw-r-- root     root        13496 2013-07-03 00:28 libbt-hci.so
-rw-r--r-- root     root        17708 2008-08-01 08:00 libbt-hcio.so
*/


/*

08-09 00:11:54.106 I/fm_hci  ( 1185): init p_cb: 0x68b2a378  local_bdaddr: 84 7a 88 f4 ab 6c

08-09 00:11:54.126 I/fm_hci  ( 1185): set_power state: 0                // Done as a reset
08-09 00:11:54.136 I/fm_hci  ( 1185): set_power hcio_set_power done
08-09 00:11:54.136 I/fm_hci  ( 1185): set_power done

08-09 00:11:54.136 I/fm_hci  ( 1185): set_power state: 1
08-09 00:11:54.156 I/fm_hci  ( 1185): set_power hcio_set_power done
08-09 00:11:54.156 I/fm_hci  ( 1185): set_power done

08-09 00:11:54.156 I/fm_hci  ( 1185): bt_hc_worker_thread started
08-09 00:11:54.156 I/fm_hci  ( 1185): bt_hc_worker_thread lib_running

...

08-09 00:14:55.897 I/fm_hci  ( 1185): cleanup
08-09 00:14:55.927 I/fm_hci  ( 1185): cleanup done

08-09 00:14:55.927 I/fm_hci  ( 1185): set_power state: 0
08-09 00:14:55.937 I/fm_hci  ( 1185): set_power hcio_set_power done


*/

#define LOGTAG "fm_hci"

#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>

//#define _GNU_SOURCE
#include <unistd.h>

#include <pthread.h>

#define NELEM(x)                    (sizeof (x)/sizeof (*(x)))
#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
//#include <sys/system_properties.h>
//using namespace android;


#include <android/log.h>
#define  loge(...)  fm_log_print(ANDROID_LOG_ERROR, LOGTAG,__VA_ARGS__)
#define  logi(...)  fm_log_print(ANDROID_LOG_INFO,  LOGTAG,__VA_ARGS__)
#define  logd(...)  fm_log_print(ANDROID_LOG_DEBUG, LOGTAG,__VA_ARGS__)

//int __android_log_print(int prio, const char *tag, const char *fmt, ...);
//int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap);



  int no_log = 0;
  void * log_handle = NULL;

  int (* do_log) (int prio, const char * tag, const char * fmt, va_list ap);
  #include <stdarg.h>
  int fm_log_print (int prio, const char * tag, const char * fmt, ...) {

        //Disable debug logs
    if (prio == ANDROID_LOG_DEBUG)
      return (-1);

    if (no_log)
      return (-1);

    va_list ap;
    va_start ( ap, fmt ); 

    if (log_handle == NULL) {
      log_handle = dlopen ("liblog.so", RTLD_LAZY);
      if (log_handle == NULL) {
        no_log = 1;
        return (-1);
      }
      do_log = dlsym (log_handle, "__android_log_vprint");
      if (do_log == NULL) {
        no_log = 1;
        return (-1);
      }
    }
    //__android_log_vprint (prio, tag, fmt, ap);
    do_log (prio, tag, fmt, ap);
  }

  const char *copyright = "Copyright (c) 2011-2014 Michael A. Reid. All rights reserved.";

  #ifndef DEF_BUF
  #define DEF_BUF 512    // Raised from 256 so we can add headers to 255-256 byte buffers
  #endif

#define HCI_BLUEDROID

char bfm_rx_buf [288] = {0};
int  bfm_rx_len = 0;


int version_sdk = 18;   // props_log
char version_sdk_prop_buf       [DEF_BUF] = "";

#include "../../ul/jni/utils.c"
#include "../../ul/jni/acc_hci.c"


/*
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
*/


// bt_hci_lib.h:

/* Generic purpose transac returned upon request complete */
typedef void * TRANSAC;

/** Bluetooth Power Control States */
typedef enum {
    BT_HC_CHIP_PWR_OFF,
    BT_HC_CHIP_PWR_ON,
}  bt_hc_chip_power_state_t;

/** Bluetooth Low Power Mode */
typedef enum {
    BT_HC_LPM_DISABLE,
    BT_HC_LPM_ENABLE,
    BT_HC_LPM_WAKE_ASSERT,
    BT_HC_LPM_WAKE_DEASSERT,
} bt_hc_low_power_event_t;

/** Receive flow control */
typedef enum {
    BT_RXFLOW_OFF, /* add transport device fd to select set */
    BT_RXFLOW_ON,  /* remove transport device to from select set */
} bt_rx_flow_state_t;

/** HCI logging control */
typedef enum {
    BT_HC_LOGGING_OFF,
    BT_HC_LOGGING_ON,
} bt_hc_logging_state_t;

/** Result of write request */
typedef enum {
    BT_HC_TX_SUCCESS,  /* a buffer is fully processed and can be released */
    BT_HC_TX_FAIL,     /* transmit fail */
    BT_HC_TX_FRAGMENT, /* send split ACL pkt back to stack to reprocess */
} bt_hc_transmit_result_t;

/** Result of preload initialization */
typedef enum {
    BT_HC_PRELOAD_SUCCESS,
    BT_HC_PRELOAD_FAIL,
} bt_hc_preload_result_t;

/** Result of postload initialization */
typedef enum {
    BT_HC_POSTLOAD_SUCCESS,
    BT_HC_POSTLOAD_FAIL,
} bt_hc_postload_result_t;

/** Result of low power enable/disable request */
typedef enum {
    BT_HC_LPM_DISABLED,
    BT_HC_LPM_ENABLED,
} bt_hc_lpm_request_result_t;

/** Host/Controller Library Return Status */
typedef enum {
    BT_HC_STATUS_SUCCESS,
    BT_HC_STATUS_FAIL,
    BT_HC_STATUS_NOT_READY,
    BT_HC_STATUS_NOMEM,
    BT_HC_STATUS_BUSY,
    BT_HC_STATUS_CORRUPTED_BUFFER
} bt_hc_status_t;


    // Bluetooth Host/Controller callback structure.

  typedef void  (*hostwake_ind_cb)      (bt_hc_low_power_event_t event);                    /* called upon bt host wake signal */
  typedef void  (*preload_result_cb)    (TRANSAC transac, bt_hc_preload_result_t result);   /* preload initialization callback */
  typedef void  (*postload_result_cb)   (TRANSAC transac, bt_hc_postload_result_t result);  /* postload initialization callback */
  typedef void  (*lpm_result_cb)        (bt_hc_lpm_request_result_t result);                /* lpm enable/disable callback */
  typedef char* (*alloc_mem_cb)         (int size);                                         /* datapath buffer allocation callback (callout) */
  typedef int   (*dealloc_mem_cb)       (TRANSAC transac, char *p_buf);                     /* datapath buffer deallocation callback (callout) */
  typedef int   (*tx_result_cb)         (TRANSAC transac, char *p_buf, bt_hc_transmit_result_t result); /* transmit result callback */
  typedef int   (*data_ind_cb)          (TRANSAC transac, char *p_buf, int len);            /* a previously setup buffer is read and available for processing buffer
                                                                                                is deallocated in stack when processed */

  typedef struct {
    size_t              size;                   /** set to sizeof(bt_hc_callbacks_t) */
    preload_result_cb   preload_cb;             /* notifies caller result of preload request */
    postload_result_cb  postload_cb;            /* notifies caller result of postload request */
    lpm_result_cb       lpm_cb;                 /* notifies caller result of lpm enable/disable */
    hostwake_ind_cb     hostwake_ind;           /* notifies hardware on host wake state */
    alloc_mem_cb        alloc;                  /* buffer allocation request */
    dealloc_mem_cb      dealloc;                /* buffer deallocation request */
    data_ind_cb         data_ind;               /* notifies stack data is available */
    tx_result_cb        tx_result;              /* notifies caller when a buffer is transmitted (or failed) */
  } bt_hc_callbacks_t;


    // Bluetooth Host/Controller Interface

  typedef struct {
    size_t  size;       /** Set to sizeof(bt_hc_interface_t) */
    int     (* init)        (const bt_hc_callbacks_t * p_cb, unsigned char * local_bdaddr);      /** Opens the interface and provides the callback routines to the implemenation of this interface. */
    void    (* set_power)   (bt_hc_chip_power_state_t state);           /** Chip power control */
    int     (* lpm)         (bt_hc_low_power_event_t event);            /** Set low power mode wake */
    void    (* preload)     (TRANSAC transac);                          /** Called prior to stack initialization */
    void    (* postload)    (TRANSAC transac);                          /** Called post stack initialization */
    int     (* transmit_buf)(TRANSAC transac, char * p_buf, int len);   /** Transmit buffer */
    int     (* set_rxflow)  (bt_rx_flow_state_t state);                 /** Controls receive flow */
    int     (* logging)     (bt_hc_logging_state_t state, char * p_path);   /** Controls HCI logging on/off */
    void    (* cleanup)     (void);                                     /** Closes the interface */
  } bt_hc_interface_t;



/******************************************************************************
**  Externs
******************************************************************************/
/*
extern bt_vendor_interface_t * bt_vnd_if;
extern int num_hci_cmd_pkts;
void lpm_init(void);
void lpm_cleanup(void);
void lpm_enable(uint8_t turn_on);
void lpm_wake_deassert(void);
void lpm_allow_bt_device_sleep(void);
void lpm_wake_assert(void);
void init_vnd_if(unsigned char *local_bdaddr);
void btsnoop_open(char *p_path);
void btsnoop_close(void);
*/
/******************************************************************************
**  Variables
******************************************************************************/

//bt_hc_callbacks_t *bt_hc_cbacks = NULL;
//BUFFER_Q tx_q;
//tHCI_IF * p_hci_if;

/******************************************************************************
**  Local type definitions
******************************************************************************/

/* Host/Controller lib thread control block */

  typedef struct {
    pthread_t       worker_thread;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
  } bt_hc_cb_t;

/******************************************************************************
**  Static Variables
******************************************************************************/
///*
static          bt_hc_cb_t  hc_cb;
static volatile uint8_t     thread_state = 0;       // 0 = stopped, 1 = started, 2 = not needed but thread still alive
static volatile uint16_t    ready_events = 0;
static volatile uint8_t     tx_cmd_pkts_pending = 0;//FALSE;
//*/
/******************************************************************************
**  Functions
******************************************************************************/

/*
  static void * bt_hc_worker_thread (void * arg);

  void bthc_signal_event (uint16_t event) {

    logd ("bt_signal_event event: %d", event);

    pthread_mutex_lock (& hc_cb.mutex);
    ready_events |= event;
    pthread_cond_signal (& hc_cb.cond);
    pthread_mutex_unlock (& hc_cb.mutex);
  }
*/

  //#define HC_EVENT_LPM_ALLOW_SLEEP       0x0080
  //TRANSAC bfm_send (char * buf, int len);
  //int   (* hcio_lpm)            (bt_hc_low_power_event_t event);

  //int need_bfm_send = 1;

  static void * bt_hc_worker_thread (void * arg) {
    uint16_t events;
    //HC_BT_HDR *p_msg, *p_next_msg;

    logi ("bt_hc_worker_thread started");

//sh_run ("s2d server &");

    //prctl (PR_SET_NAME, (unsigned long) "bt_hc_worker", 0, 0, 0);     // ??
    tx_cmd_pkts_pending = 0;//FALSE;

    //raise_priority_a2dp (TASK_HIGH_HCI_WORKER);

// From sprtd.c :
    if (acc_hci_start (4))                                              // Start Access to HCI for Bluedroid mode only
      return (NULL);                                                    // If not available, terminate... (never an error March 2014)

    while (thread_state == 1) {                                         // While needed and running...
      logi ("bt_hc_worker_thread thread_state = needed and running");

// From sprtd.c :
      do_server ();     // !! Doesn't return if no packets sent

//      if (exiting) {
//        acc_hci_stop ();
//        return (0);
//      }
      //else {
      //  loge ("Terminating due to do_server error");  // !! NEW !! due to double daemons (low timeout ???)
      //  return (-1);  // !!
      //}

    if (thread_state == 1)
      sleep (1);                                                          // Keep trying if error return


  //return (-1);                                                          // Should never get here
  //return (NULL);    // compiler friendly


/*
      pthread_mutex_lock (& hc_cb.mutex);
      logd ("bt_hc_worker_thread done pthread_mutex_lock");

      while (ready_events == 0) {
        logd ("bt_hc_worker_thread pthread_cond_wait start");
        pthread_cond_wait (& hc_cb.cond, & hc_cb.mutex);
        logd ("bt_hc_worker_thread pthread_cond_wait end");
      }

      events = ready_events;
      ready_events = 0;

      logd ("bt_hc_worker_thread pthread_mutex_unlock");
      pthread_mutex_unlock (& hc_cb.mutex);

      //if (events & HC_EVENT_LPM_ALLOW_SLEEP) {
        if (need_bfm_send) {
          need_bfm_send = 0;
          bfm_send ();
          logd ("bt_hc_worker_thread lpm bfm_send done");
        }
        logd ("bt_hc_worker_thread hcio_lpm");
        int ret = hcio_lpm (BT_HC_LPM_WAKE_DEASSERT);
        logd ("bt_hc_worker_thread lpm ret: %d", ret);
      //}

*/

/*
#ifndef HCI_USE_MCT
        if (events & HC_EVENT_RX)
        {
            p_hci_if->rcv();

            if ((tx_cmd_pkts_pending == TRUE) && (num_hci_cmd_pkts > 0))
            {
                // Got HCI Cmd Credits from Controller.
                // Prepare to send prior pending Cmd packets in the
                // following HC_EVENT_TX session.

                events |= HC_EVENT_TX;
            }
        }
#endif
        if (events & HC_EVENT_PRELOAD)
        {
            userial_open(USERIAL_PORT_1);

            // Calling vendor-specific part
            if (bt_vnd_if)
            {
                bt_vnd_if->op(BT_VND_OP_FW_CFG, NULL);
            }
            else
            {
                if (bt_hc_cbacks)
                    bt_hc_cbacks->preload_cb(NULL, BT_HC_PRELOAD_FAIL);
            }
        }

        if (events & HC_EVENT_POSTLOAD)
        {
            // Start from SCO related H/W configuration, if SCO configuration
            // is required. Then, follow with reading requests of getting
            // ACL data length for both BR/EDR and LE.

            int result = -1;

            // Calling vendor-specific part
            if (bt_vnd_if)
                result = bt_vnd_if->op(BT_VND_OP_SCO_CFG, NULL);

            if (result == -1)
                p_hci_if->get_acl_max_len();
        }

        if (events & HC_EVENT_TX)
        {
             // We will go through every packets in the tx queue. Fine to clear tx_cmd_pkts_pending.
            tx_cmd_pkts_pending = FALSE;
            HC_BT_HDR * sending_msg_que[64];
            int sending_msg_count = 0;
            utils_lock();
            p_next_msg = tx_q.p_first;
            while (p_next_msg && sending_msg_count <
                            (int)sizeof(sending_msg_que)/sizeof(sending_msg_que[0]))
            {
                if ((p_next_msg->event & MSG_EVT_MASK)==MSG_STACK_TO_HC_HCI_CMD)
                {
                     / if we have used up controller's outstanding HCI command credits (normally is 1), skip all HCI command packets in the queue.
                     //  The pending command packets will be sent once controller gives back us credits through CommandCompleteEvent or CommandStatusEvent.
                    if ((tx_cmd_pkts_pending == TRUE) || (num_hci_cmd_pkts <= 0))
                    {
                        tx_cmd_pkts_pending = TRUE;
                        p_next_msg = utils_getnext(p_next_msg);
                        continue;
                    }
                }

                p_msg = p_next_msg;
                p_next_msg = utils_getnext(p_msg);
                utils_remove_from_queue_unlocked(&tx_q, p_msg);
                sending_msg_que[sending_msg_count++] = p_msg;
            }
            utils_unlock();
            int i;
            for(i = 0; i < sending_msg_count; i++)
                p_hci_if->send(sending_msg_que[i]);
            if (tx_cmd_pkts_pending == TRUE)
                BTHCDBG("Used up Tx Cmd credits");

        }

        if (events & HC_EVENT_LPM_ENABLE)
        {
            lpm_enable(TRUE);
        }

        if (events & HC_EVENT_LPM_DISABLE)
        {
            lpm_enable(FALSE);
        }

        if (events & HC_EVENT_LPM_IDLE_TIMEOUT)
        {
            lpm_wake_deassert();
        }

        if (events & HC_EVENT_LPM_ALLOW_SLEEP)
        {
            lpm_allow_bt_device_sleep();
        }

        if (events & HC_EVENT_LPM_WAKE_DEVICE)
        {
            lpm_wake_assert();
        }

        if (events & HC_EVENT_EXIT)
            break;
*/
    }
    logi ("bt_hc_worker_thread exiting");

    acc_hci_stop ();

    thread_state = 0;

    pthread_exit (NULL);

    return (NULL);    // compiler friendly
  }


/*****************************************************************************
**
**   BLUETOOTH HOST/CONTROLLER INTERFACE LIBRARY FUNCTIONS
**
*****************************************************************************/

  int   (* hcio_init)           (const bt_hc_callbacks_t * p_cb, unsigned char * local_bdaddr);
  void  (* hcio_set_power)      (bt_hc_chip_power_state_t state);
  int   (* hcio_lpm)            (bt_hc_low_power_event_t event);
  void  (* hcio_preload)        (TRANSAC transac);
  void  (* hcio_postload)       (TRANSAC transac);
  int   (* hcio_transmit_buf)   (TRANSAC transac, char * p_buf, int len);
  int   (* hcio_set_rxflow)     (bt_rx_flow_state_t state);
  int   (* hcio_logging)        (bt_hc_logging_state_t state, char * p_path);
  void  (* hcio_cleanup)        (void);

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
    logd ("hcib_alloc_mem_cb size: %d", size);
    //if (allocs ++ % 32 == 0)
    //  loge ("hcib_alloc_mem_cb allocs: %d  deallocs: %d", allocs, deallocs);
    //logd ("hcib_alloc_mem_cb hcib: %p", hcib);
    //if (hcib)
    //  logd ("hcib_alloc_mem_cb hcib->alloc: %p", hcib->alloc);
    char * ret = NULL;
    if (hcib && hcib -> alloc)
      ret = hcib -> alloc (size);
    logd ("hcib_alloc_mem_cb done ret: %p", ret);
    return (ret);
  }
  static int   hcib_dealloc_mem_cb       (TRANSAC transac, char *p_buf) {                    /* datapath buffer deallocation callback (callout) */
    logd ("hcib_dealloc_mem_cb transac: %p  p_buf: %p", transac, p_buf);
    //if (deallocs ++ % 32 == 0)
    //  loge ("hcib_alloc_mem_cb allocs: %d  deallocs: %d", allocs, deallocs);
    int ret = 0;
    ret = hcib -> dealloc (transac, p_buf);
    logd ("hcib_dealloc_mem_cb done ret: %d", ret);
    return (ret);
  }

/*
07-08 01:02:19.724 D/fm_hci  ( 1090): RXB 0e 0c 01 01 10 00 06 5d 00 06 0f 00 06 41 09 02 
07-08 01:02:19.724 D/fm_hci  ( 1090): RXB 3a 00 30 00 00 00 

len: 22
pre         0e 0c 01
Sent        01 10 00
hci_ver     06
Revision    5d 00
LMP Version 06
hci_manuf   0f 00        = Broadcom
Subversion  06 41
8 extra ?   09 02 3a 00 30 00 00 00 

*/

  static int   hcib_data_ind_cb          (TRANSAC transac, char *p_buf, int len) {           /* a previously setup buffer is read and available for processing buffer */

    if ((len >= 5 && p_buf [3] == 0x15 && p_buf [4] == 0xfc) ||         // Broadcom 3F  15
        (len >= 5 && p_buf [3] == 0x33 && p_buf [4] == 0xfd) ||         // TI       3F 133
        (len >= 5 && p_buf [3] == 0x01 && p_buf [4] == 0x10 && bfm_rx_len == 0)) {// 4   1      "ssd 4 1"
      logd ("bfm_ hcib_data_ind_cb transac: %p  p_buf: %p  len: %d  bfm_rx_len: %d", transac, p_buf, len, bfm_rx_len);
      hex_dump ("bfm_ ", 16, (unsigned char *) p_buf, len);

      bfm_rx_len = len;
      memcpy (bfm_rx_buf, p_buf, len);

      //return (0);     // Can't return or stack will never release buffer
    }

    logd ("hcib_data_ind_cb transac: %p  p_buf: %p  len: %d", transac, p_buf, len);
    int shortlen = len;
    if (shortlen < 0)
      shortlen = 0;
    if (shortlen > 32)
      shortlen = 32;
    hex_dump ("RXB ", 16, (unsigned char *) p_buf, shortlen);

    int ret = 0;
    ret = hcib -> data_ind (transac, p_buf, len);
    logd ("hcib_data_ind_cb done ret: %d", ret);
    return (ret);
  }
  static int   hcib_tx_result_cb         (TRANSAC transac, char *p_buf, bt_hc_transmit_result_t result) {/* transmit result callback */
    logd ("hcib_tx_result_cb : transac: %p  p_buf: %p  result: %d", transac, p_buf, result);
    int ret = 0;
    ret = hcib -> tx_result (transac, p_buf, result);
    logd ("hcib_tx_result_cb done ret: %d", ret);
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

/*
    size_t              size;                   // set to sizeof(bt_hc_callbacks_t)
    preload_result_cb   preload_cb;             // notifies caller result of preload request
    postload_result_cb  postload_cb;            // notifies caller result of postload request
    lpm_result_cb       lpm_cb;                 // notifies caller result of lpm enable/disable
    hostwake_ind_cb     hostwake_ind;           // notifies hardware on host wake state
    alloc_mem_cb        alloc;                  // buffer allocation request
    dealloc_mem_cb      dealloc;                // buffer deallocation request
    data_ind_cb         data_ind;               // notifies stack data is available
    tx_result_cb        tx_result;              // notifies caller when a buffer is transmitted (or failed)
*/
/*
07-03 01:47:53.664 D/fm_hci  ( 2514): transmit_buf transac: 0x704bb4a8  p_buf: 0x704bb4b0  len: 3
07-03 01:47:53.664 D/fm_hci  ( 2514): TXB 05 10 00 
07-03 01:47:53.664 D/fm_hci  ( 2514): transmit_buf ret: 0

 #define HCI_READ_BUFFER_SIZE            (0x0005 | HCI_GRP_INFORMATIONAL_PARAMS)

07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_tx_result_cb : transac: 0x704bb4a8  p_buf: 0x704bb4b0  result: 0
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_tx_result_cb done ret: 0
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_alloc_mem_cb size: 1024
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_alloc_mem_cb done ret: 0x72454064
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_alloc_mem_cb size: 21
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_alloc_mem_cb done ret: 0x723554d0
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_dealloc_mem_cb transac: 0x72453048  p_buf: 0x72453050
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_dealloc_mem_cb done ret: 0
07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_data_ind_cb transac: 0x723554d0  p_buf: 0x723554d8  len: 21

07-03 01:47:53.664 D/fm_hci  ( 2514): RXB 0e 0b 01 05 10 00 fd 03 40 08 00 01 00 00 48 00 
07-03 01:47:53.664 D/fm_hci  ( 2514): RXB 48 00 3a 00 6d 

15 return: fd 03 40 08 00 01 00 00 48 00 48 00 3a 00 6d 

2:  03fd    HC_ACL_Data_Packet_Length       = 1024 - 3
1:    40    HC_SCO_Data_Packet_Length       = 64
2:  0008    HC_Total_Num_ACL_Data_Packets
2:  0001    HC_Total_Num_SCO_Data_Packets

Followed by:    00 48 00 48 00 3a 00 6d 


07-03 01:47:53.664 D/fm_hci  ( 2514): hcib_data_ind_cb done ret: 0
*/


  static void worker_thread_stop () {
    if (thread_state == 1) {
      thread_state = 2; // not needed but still running
      //bthc_signal_event (HC_EVENT_EXIT);

// !! Disable wait !!
//      pthread_join (hc_cb.worker_thread, NULL);
    }
  }


  static int worker_thread_start () {
    pthread_attr_t thread_attr;
    struct sched_param param;
    int policy, result;

    if (thread_state == 1) {
      logi ("worker_thread_start & thread_state = running");
      return (BT_HC_STATUS_SUCCESS);
    }

    if (thread_state == 2) {
      logi ("worker_thread_start & thread_state = not needed and running");
      return (BT_HC_STATUS_SUCCESS);
    }

    thread_state = 1;


    ready_events = 0;
    pthread_mutex_init (& hc_cb.mutex, NULL);
    pthread_cond_init  (& hc_cb.cond, NULL);
    pthread_attr_init  (& thread_attr);

    if (pthread_create (& hc_cb.worker_thread, & thread_attr, bt_hc_worker_thread, NULL) != 0) {
      loge ("pthread_create failed");
      thread_state = 0;
      return (BT_HC_STATUS_FAIL);
    }

#ifndef BTHC_LINUX_BASE_POLICY
#define BTHC_LINUX_BASE_POLICY SCHED_NORMAL
#endif

/*#if (BTHC_LINUX_BASE_POLICY != SCHED_NORMAL)
#ifndef BTHC_LINUX_BASE_PRIORITY
#define BTHC_LINUX_BASE_PRIORITY 30
#endif*/

    if (pthread_getschedparam (hc_cb.worker_thread, & policy, & param) == 0) {
      policy = BTHC_LINUX_BASE_POLICY;
#if (BTHC_LINUX_BASE_POLICY!=SCHED_NORMAL)
      param.sched_priority = BTHC_MAIN_THREAD_PRIORITY;
#endif
      result = pthread_setschedparam (hc_cb.worker_thread, policy, & param);
      if (result != 0) {
        logd ("libbt-hci init: pthread_setschedparam failed (%s)", strerror(result));
      }
    }
    return (BT_HC_STATUS_SUCCESS);
  }

  static int init (const bt_hc_callbacks_t * p_cb, unsigned char * local_bdaddr) {
    logi ("init p_cb: %p  local_bdaddr: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x", p_cb, local_bdaddr [0], local_bdaddr [1], local_bdaddr [2], local_bdaddr [3], local_bdaddr [4], local_bdaddr [5] );
    if (p_cb == NULL) {
      loge ("init no user callbacks");
      return (BT_HC_STATUS_FAIL);
    }
    hcib = p_cb;

    //int ret = hcio_init (p_cb, local_bdaddr);
    int ret = hcio_init (& bt_hc_callbacks, local_bdaddr);
    logd ("init ret: %d", ret);

    if (ret != BT_HC_STATUS_SUCCESS) 
      return (ret);
    //return (BT_HC_STATUS_SUCCESS);
    //return (BT_HC_STATUS_FAIL);

    // store reference to user callbacks
    //bt_hc_cbacks = (bt_hc_callbacks_t *) p_cb;

/*
    init_vnd_if(local_bdaddr);

    utils_init();
#ifdef HCI_USE_MCT
    extern tHCI_IF hci_mct_func_table;
    p_hci_if = &hci_mct_func_table;
#else
    extern tHCI_IF hci_h4_func_table;
    p_hci_if = &hci_h4_func_table;
#endif

    p_hci_if->init();

    userial_init();
    lpm_init();

    utils_queue_init(&tx_q);
*/

    ret = 0;//worker_thread_start ();
    return (ret);

  }


/** Chip power control */
  static void set_power (bt_hc_chip_power_state_t state) {
    logi ("set_power state: %d", state);
    hcio_set_power (state);
    logi ("set_power hcio_set_power done");

    int ret = 0;
    if (state == BT_HC_CHIP_PWR_ON)
      ret = worker_thread_start ();
    else
      worker_thread_stop ();

    logi ("set_power done");

    return;
/*
    int pwr_state;
    // Calling vendor-specific part
    pwr_state = (state == BT_HC_CHIP_PWR_ON) ? BT_VND_PWR_ON : BT_VND_PWR_OFF;

    if (bt_vnd_if)
        bt_vnd_if->op(BT_VND_OP_POWER_CTRL, &pwr_state);
    else
        ALOGE("vendor lib is missing!");
*/
}


/** Configure low power mode wake state */
  static int lpm (bt_hc_low_power_event_t event) {
    logd ("lpm event: %d", event);

//Was for bfm_send
//    if (event == BT_HC_LPM_WAKE_DEASSERT) {                             // Delay
//      bthc_signal_event (HC_EVENT_LPM_ALLOW_SLEEP);
//      return (BT_HC_STATUS_SUCCESS);
//    }

    int ret = hcio_lpm (event);
    logd ("lpm ret: %d", ret);

    return (ret);
/*
//bt_hci_bdroid.h
#define TRUE   (!0)
    uint8_t status = TRUE;
    switch (event)
    {
        case BT_HC_LPM_DISABLE:
            bthc_signal_event (HC_EVENT_LPM_DISABLE);
            break;

        case BT_HC_LPM_ENABLE:
            bthc_signal_event (HC_EVENT_LPM_ENABLE);
            break;

        case BT_HC_LPM_WAKE_ASSERT:
            bthc_signal_event (HC_EVENT_LPM_WAKE_DEVICE);
            break;

        case BT_HC_LPM_WAKE_DEASSERT:
            bthc_signal_event (HC_EVENT_LPM_ALLOW_SLEEP);
            break;
    }
    return (status == TRUE) ? BT_HC_STATUS_SUCCESS : BT_HC_STATUS_FAIL;
*/
  }


    /** Called prio to stack initialization */
  static void preload (TRANSAC transac) {
    logd ("preload transac: %p", transac);
    hcio_preload (transac);
    logd ("preload done");
    return;
    //bthc_signal_event (HC_EVENT_PRELOAD);
  }


    /** Called post stack initialization */
  static void postload (TRANSAC transac) {
    logd ("postload transac: %p", transac);
    hcio_postload (transac);
    logd ("postload done");
    return;
    //bthc_signal_event (HC_EVENT_POSTLOAD);
  }


/** Transmit frame */
  static int transmit_buf (TRANSAC transac, char * p_buf, int len) {
    logd ("transmit_buf transac: %p  p_buf: %p  len: %d", transac, p_buf, len);
    int shortlen = len;
    if (shortlen < 0)
      shortlen = 0;
    if (shortlen > 32)
      shortlen = 32;
    hex_dump ("TXB ", 16, (unsigned char *) p_buf, shortlen);
//return 0;
    int ret = hcio_transmit_buf (transac, p_buf, len);
    logd ("transmit_buf ret: %d", ret);
    return (ret);
    //utils_enqueue (& tx_q, (void *) transac);
    //bthc_signal_event (HC_EVENT_TX);
    //return (BT_HC_STATUS_SUCCESS);
}


/** Controls receive flow */
  static int set_rxflow (bt_rx_flow_state_t state) {
    logd ("set_rxflow state: %d", state);
    int ret = hcio_set_rxflow (state);
    logd ("set_rxflow ret: %d", ret);
    return (ret);

    //userial_ioctl(\
     ((state == BT_RXFLOW_ON) ? USERIAL_OP_RXFLOW_ON : USERIAL_OP_RXFLOW_OFF), \
     NULL);
    //return (BT_HC_STATUS_SUCCESS);
}


/** Controls HCI logging on/off */
  static int logging (bt_hc_logging_state_t state, char * p_path) {
    logd ("logging state: %d  p_path: \"%s\"", state, p_path);
    int ret = hcio_logging (state, p_path);
    logd ("logging ret: %d", ret);
    return (ret);
/*
    if (state == BT_HC_LOGGING_ON)
    {
        if (p_path != NULL)
            btsnoop_open(p_path);
    }
    else
    {
        btsnoop_close();
    }
*/
    return (BT_HC_STATUS_SUCCESS);
}


/** Closes the interface */
  static void cleanup (void) {
    logi ("cleanup");
    hcio_cleanup ();
    logi ("cleanup done");

    //worker_thread_stop ();

    return;
/*

    lpm_cleanup ();
    userial_close ();
    p_hci_if->cleanup ();
    utils_cleanup ();

    // Calling vendor-specific part
    if (bt_vnd_if)
        bt_vnd_if->cleanup ();
    bt_hc_cbacks = NULL;
*/
  }


  static const bt_hc_interface_t bluetoothHCLibInterface = {
    sizeof (bt_hc_interface_t),
    init,
    set_power,
    lpm,
    preload,
    postload,
    transmit_buf,
    set_rxflow,
    logging,
    cleanup
  };

  const void * btsnoop_reg (void) {
    logi ("btsnoop_reg");
    return (NULL);
  }

  int  have_hcio = 1;
  char hcio_lib [64] = "libbt-hcio.so";
  void * hcio_handle = NULL;

  const bt_hc_interface_t * (* hcio_bt_hc_get_interface) (void);

  const bt_hc_interface_t * hcio_bt_hc_interface = NULL;

  const bt_hc_interface_t * bt_hc_get_interface (void) {

    logi ("bt_hc_get_interface have_hcio: %d", have_hcio);
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
      logi ("bt_hc_get_interface hcio_bt_hc_interface: %p", hcio_bt_hc_interface);

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
      hcio_set_rxflow = hcio_bt_hc_interface->set_rxflow;
      if (hcio_set_rxflow == NULL)
        return (NULL);
      hcio_logging = hcio_bt_hc_interface->logging;
      if (hcio_logging == NULL)
        return (NULL);
      hcio_cleanup = hcio_bt_hc_interface->cleanup;
      if (hcio_cleanup == NULL)
        return (NULL);
    }
    have_hcio = 1;
    logi ("bt_hc_get_interface done have_hcio: %d", have_hcio);


    return (& bluetoothHCLibInterface);
  }
/*
  static char* hcib_alloc_mem_cb         (int size) {                                        // datapath buffer allocation callback (callout)
    logd ("hcib_alloc_mem_cb size: %d", size);
    char * ret = NULL;
    ret = hcib -> alloc (size);
    logd ("hcib_alloc_mem_cb done ret: %p", ret);
    return (ret);
  }
  static int   hcib_dealloc_mem_cb       (TRANSAC transac, char *p_buf) {                    // datapath buffer deallocation callback (callout)
    logd ("hcib_dealloc_mem_cb transac: %p  p_buf: %p", transac, p_buf);
    int ret = 0;
    ret = hcib -> dealloc (transac, p_buf);
    logd ("hcib_dealloc_mem_cb done ret: %d", ret);
    return (ret);
  }

*/


#ifdef  MY_DISABLED
  TRANSAC bfm_send (char * buf, int len) {
      logd ("bfm_send");
      return (NULL);
    }
#endif
#ifndef  MY_DISABLED
  static char * gv_mem = NULL;

  TRANSAC bfm_send (char * buf, int len) {
      logd ("bfm_send");
//return;

      int data_len = len - 3;//3;//0;
      //int len = 3 + data_len;                                                      // len not even used by bt_hci_bdroid.c: transmit_buf ; should be 3 based on observed calls
      int full_len = len + 8;
//return;
//char gv_mem [1024] = {0};
      //logd ("bfm_send hcib_alloc_mem_cb: %p\n", hcib_alloc_mem_cb);


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

      //uint16_t ogf = 0x3f;//4;
      //uint16_t ocf = 0x15;//1;

      //uint16_t opcode = 0xfc15;//ogf << 10 + ocf;
      uint16_t opcode = (ogf * 1024) | ocf;

  logd ("bfm_send ogf: 0x%x  ocf: 0x%x  opcode: 0x%x  hci_data: %p  hci_len: %d", ogf, ocf, opcode, hci_data, hci_len);

      uint16_t HC_BT_HDR_event          = 0x2000;   // MSG_STACK_TO_HC_HCI_CMD | 0 / LOCAL_BR_EDR_CONTROLLER_ID
      uint16_t HC_BT_HDR_len            = len;//0x0003;
      uint16_t HC_BT_HDR_offset         = 0x0000;   // Starts at [8] so offset = 0
      uint16_t HC_BT_HDR_layer_specific = opcode;//0x1001;   // = Opcode means internal command means just dealloc, don't callback tx_result

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
/*//TXB 
      gv_mem [8] = opcode & 0x003f;//0x01;
      gv_mem [9] = (opcode & 0xff00) >> 8;//0x10;
      gv_mem [10]= data_len;//0x00;                                                // 0 bytes of data follow

      gv_mem [11]= 0;   // reg 0
      gv_mem [12]= 1;   // 1 = read
      gv_mem [13]= 0x40;// size = 64 bytes from 0x00 - 0x3f
*/
//return;
      TRANSAC itransac = gv_mem;//buf;
      int ret = transmit_buf (itransac, & gv_mem [8], len);
      //sleep (1);

      //hcib_dealloc_mem_cb (itransac, & gv_mem [8]);

      logd ("bfm_send ret: %d", ret);

      return (gv_mem);
  }

#else
/*
07-08 01:02:19.724 D/fm_hci  ( 1090): RXB 0e 0c 01 01 10 00 06 5d 00 06 0f 00 06 41 09 02 
07-08 01:02:19.724 D/fm_hci  ( 1090): RXB 3a 00 30 00 00 00 

len: 22
pre         0e 0c 01
Sent        01 10 00

8 hci data
hci_ver     06
Revision    5d 00
LMP Version 06
hci_manuf   0f 00        = Broadcom
Subversion  06 41
8 extra ?   09 02 3a 00 30 00 00 00 

*/


    // Just for testing as binary:
  int main (int argc, char * argv []) {

    loge ("!!!!!!!!!!!!");

    strlcpy (hcio_lib, "libbt-hci.so", sizeof (hcio_lib));
    printf ("main hcio_lib: \"%s\"", hcio_lib);

    const bt_hc_interface_t * main_bt_hc_interface = bt_hc_get_interface ();
    printf ("main main_bt_hc_interface: %p\n", main_bt_hc_interface);

    char bfm_send_data [1024] = {0, 0, 0, 0,    1,     1, 0x10, 0};
    void * tx_mem = bfm_send (bfm_send_data, 8);

    while (1) {
      //if (cbs
      sleep (1);
    }
    return (0);
  }
/*

07-09 00:22:15.692 D/fm_hci  ( 1877): bt_hc_get_interface have_hcio: 1
07-09 00:22:15.692 D/fm_hci  ( 1877): bt_hc_get_interface hcio_bt_hc_interface: 0x7055cde0
07-09 00:22:15.692 D/fm_hci  ( 1877): bt_hc_get_interface done have_hcio: 1

07-09 00:22:15.812 D/fm_hci  ( 1877): init p_cb: 0x74349828  local_bdaddr: 22 22 84 7a 4e e6
07-09 00:22:15.822 D/fm_hci  ( 1877): init ret: 0

07-09 00:22:15.822 D/fm_hci  ( 1877): set_power state: 0
07-09 00:22:15.822 D/fm_hci  ( 1877): bt_hc_worker_thread started
07-09 00:22:15.822 D/fm_hci  ( 1877): bt_hc_worker_thread lib_running
07-09 00:22:15.822 D/fm_hci  ( 1877): bt_hc_worker_thread done pthread_mutex_lock
07-09 00:22:15.822 D/fm_hci  ( 1877): bt_hc_worker_thread pthread_cond_wait start
07-09 00:22:15.832 D/fm_hci  ( 1877): set_power done

07-09 00:22:15.832 D/fm_hci  ( 1877): set_power state: 1
07-09 00:22:15.842 D/fm_hci  ( 1877): set_power done

07-09 00:22:15.842 D/fm_hci  ( 1877): preload transac: 0x0
07-09 00:22:15.842 D/fm_hci  ( 1877): preload done

07-09 00:22:15.842 D/fm_hci  ( 1877): hcib_alloc_mem_cb size: 11
07-09 00:22:15.842 D/fm_hci  ( 1877): hcib_alloc_mem_cb done ret: 0x723610f4
07-09 00:22:15.842 D/fm_hci  ( 1877): hcib_dealloc_mem_cb transac: 0x723610f4  p_buf: 0x723610fc

07-09 00:22:15.842 D/fm_hci  ( 1877): hcib_alloc_mem_cb size: 1024
07-09 00:22:15.852 D/fm_hci  ( 1877): hcib_alloc_mem_cb done ret: 0x72421010
07-09 00:22:15.852 D/fm_hci  ( 1877): hcib_dealloc_mem_cb done ret: 0

07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_alloc_mem_cb size: 1024
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_alloc_mem_cb size: 14
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_alloc_mem_cb done ret: 0x7242202c
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_alloc_mem_cb done ret: 0x72361140
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_dealloc_mem_cb transac: 0x72421010  p_buf: 0x72421018
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_dealloc_mem_cb done ret: 0
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_alloc_mem_cb size: 266
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_alloc_mem_cb done ret: 0x60f6ffd0
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_dealloc_mem_cb transac: 0x72361140  p_buf: 0x72361148
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_dealloc_mem_cb done ret: 0
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_dealloc_mem_cb transac: 0x60f6ffd0  p_buf: 0x60f6ffd8
07-09 00:22:15.922 D/fm_hci  ( 1877): hcib_dealloc_mem_cb done ret: 0
.........


07-09 00:22:16.523 D/fm_hci  ( 1877): transmit_buf transac: 0x7362c168  p_buf: 0x7362c170  len: 3
07-09 00:22:16.523 D/fm_hci  ( 1877): TXB 03 0c 00 
07-09 00:22:16.523 D/fm_hci  ( 1877): transmit_buf ret: 0
07-09 00:22:16.523 D/fm_hci  ( 1877): hcib_tx_result_cb : transac: 0x7362c168  p_buf: 0x7362c170  result: 0
07-09 00:22:16.523 D/fm_hci  ( 1877): hcib_tx_result_cb done ret: 0

07-09 00:22:16.543 D/fm_hci  ( 1877): hcib_data_ind_cb transac: 0x72361484  p_buf: 0x7236148c  len: 14
07-09 00:22:16.543 D/fm_hci  ( 1877): RXB 0e 04 01 03 0c 00 00 00 00 00 00 00 00 00 
07-09 00:22:16.543 D/fm_hci  ( 1877): hcib_data_ind_cb done ret: 0


07-09 00:22:16.543 D/fm_hci  ( 1877): transmit_buf transac: 0x7362c408  p_buf: 0x7362c410  len: 3
07-09 00:22:16.543 D/fm_hci  ( 1877): TXB 05 10 00 
07-09 00:22:16.543 D/fm_hci  ( 1877): hcib_tx_result_cb : transac: 0x7362c408  p_buf: 0x7362c410  result: 0
07-09 00:22:16.543 D/fm_hci  ( 1877): hcib_tx_result_cb done ret: 0
07-09 00:22:16.543 D/fm_hci  ( 1877): transmit_buf ret: 0

07-09 00:22:16.543 D/fm_hci  ( 1877): hcib_data_ind_cb transac: 0x723614d0  p_buf: 0x723614d8  len: 21
07-09 00:22:16.543 D/fm_hci  ( 1877): RXB 0e 0b 01 05 10 00 fd 03 40 08 00 01 00 61 9a 6f 
07-09 00:22:16.543 D/fm_hci  ( 1877): RXB 65 6e 5f 55 53 


*/

/*
GPE system/lib/libandroid_runtime.so:

_ZN7android53register_com_broadcom_bt_service_fm_FmReceiverServiceEP7_JNIEnv
_ZN7android60register_com_broadcom_bt_service_bpp_BluetoothPrinterServiceEP7_JNIEnv
_ZN7android62register_com_broadcom_bt_service_bpp_BluetoothPrinterEventLoopEP7_JNIEnv
_ZN7android56register_com_broadcom_bt_service_fm_FmTransmitterServiceEP7_JNIEnv

BTL_IFC_UnregisterSubSystem
check_fmtx_antenna
set_fmtx_antenna
dump_msg_id

get_current_state
fm_disable
fm_enable
BTL_IFC_SendMsgNoParams

com_broadcom_bt_service_fm_FmReceiverService.cpp
[JNI] - setFMVolumeNative volume =%d
[JNI] - setAudioPathNative :    audioPath = %i
[JNI] - setAudioModeNative :    audioMode = %i
[JNI] - setRdsModeNative :
[JNI] - muteNative :    toggle = %i
[JNI] - searchAbortNative :
[JNI] - searchNative :    scanMode = %i  rssiThreshold = %i  condVal = %i  condType = %i
[JNI] - tuneNative :    freq = %i
[JNI] - disableNative :
[JNI] - enableNative :    functionalityMask = %i
[JNI] - enableNative :    INIT = %i
[JNI] - enableNative :    REGISTER = %i
[JNI] - enableNative :    ENABLE = %i
onRadioOnEvent
onRadioOffEvent
onRadioMuteEvent
onRadioTuneEvent
onRadioSearchEvent
onRadioSearchCompleteEvent
onRadioAfJumpEvent
onRadioAudioPathEvent
onRadioAudioModeEvent
onRadioAudioDataEvent
onRadioRdsModeEvent
(IZZI)V
onRadioRdsTypeEvent
onRadioRdsUpdateEvent
onRadioDeemphEvent
onRadioScanStepEvent
onRadioRegionEvent
onRadioNflEstimationEvent
onRadioVolumeEvent
%s: event ID: %d, ATTACHING THREAD
%s: THREAD NOT ATTACHED
%s: THREAD ATTACHED OK
[JNI] - TRANSMITTING EVENT UP :    event = %i
[JNI] - BTA_FM_SEARCH_EVT :    rssi = %i, freq = %i
[JNI] - BTA_FM_SEARCH_CMPL_EVT :    status = %i rssi = %i freq = %i
[JNI] - TRANSMITTING EVENT BTA_FM_AF_JMP_EVT :    status = %i rssi = %i freq = %i
%s: BTA_FM_RDS_MODE_EVT
[JNI] - setRdsRdbsNative :
%s: BTA_FM_RDS_TYPE_EVT
%s: BTA_FM_RDS_UPD_EVT, 0x%8x
%s: BTA_FM_RDS_UPD_EVT, previous_rds%s
%s: BTA_FM_RDS_UPD_EVT, new_rds%s
%s: BTA_FM_RDS_UPD_EVT, memcmp 0x%8x
[JNI_CALLBACK] - androidFmRxCback :    unknown event received
%s: DetachCurrentThread() failed
%s: Event ID: %d
BTLIF_FM_ENABLE:%d
BTLIF_FM_DISABLE:%d
%s: RDS MODE EVENT
%s: RDS TYPE EVENT
%s: RDS UPDATE EVENT
com/broadcom/bt/service/fm/FmReceiverService
initNativeDataNative
tuneNative
muteNative
searchNative
searchAbortNative
setRdsModeNative
(ZZI)Z
setAudioModeNative
setAudioPathNative
setScanStepNative
setRegionNative
configureDeemphasisNative
estimateNoiseFloorNative
getAudioQualityNative
configureSignalNotificationNative
setFMVolumeNative
classFmInitNative
initLoopNative
cleanupLoopNative
com_broadcom_bt_service_ftp_FTPService.cpp
Unregister Sub system SUB_FTPS
#### Dead Beef ####
Dead Beef
FTPS Client handle has been changed from %d to %d



BTLIF_UNREGISTER_SUBSYS_REQ
BTLIF_UNREGISTER_SUBSYS_RSP
.....
BTLIF_HCIUTILS_CMD_ENABLE
BTLIF_HCIUTILS_CMD_DISABLE
BTLIF_HCIUTILS_CMD_SET_AFH_CHANNELS
BTLIF_HCIUTILS_CMD_SET_AFH_CHANNEL_ASSESSMENT
BTLIF_HCIUTILS_CMD_ADD_FILTER
BTLIF_HCIUTILS_CMD_REMOVE_FILTER
.....
BTLIF_TST_SET_TESTMODE
BTLIF_TST_GET_TST_STATE
BTLIF_TST_TX_RX_TEST
BTLIF_TST_SEND_TST_CMD
BTLIF_TST_BLE_PTS_TEST
BTLIF_TST_BLE_PTS_EVT
.....
BTLIF_FM_ENABLE
BTLIF_FM_DISABLE
BTLIF_FM_TUNE
BTLIF_FM_MUTE
BTLIF_FM_SEARCH
BTLIF_FM_SEARCH_ABORT
BTLIF_FM_SET_RDS_MODE
BTLIF_FM_SET_RDS_RBDS
BTLIF_FM_RDS_UPDATE
BTLIF_FM_AUDIO_MODE
BTLIF_FM_AUDIO_PATH
BTLIF_FM_SCAN_STEP
BTLIF_FM_SET_REGION
BTLIF_FM_CONFIGURE_DEEMPHASIS
BTLIF_FM_ESTIMATE_NFL
BTLIF_FM_GET_AUDIO_QUALITY
BTLIF_FM_CONFIGURE_SIGNAL_NOTIFICATION
BTLIF_FM_AF_JMP_EVT
BTLIF_FM_SET_VOLUME
BTLIF_FM_SET_VOLUME_EVT
BTLIF_FM_SEARCH_CMPL_EVT

BTLIF_FMTX_ENABLE
BTLIF_FMTX_DISABLE
BTLIF_FMTX_TXPOWER
BTLIF_FMTX_TXFREQ
BTLIF_FMTX_CONFIG
BTLIF_FMTX_SETMUTE
BTLIF_FMTX_INTF_NOTIF
BTLIF_FMTX_SEARCH_BEST_CHANS
BTLIF_FMTX_ABORT_SEARCH
BTLIF_FMTX_AUDIO_CHIRP
.....
BTLIF_AG_ConnectReq
BTLIF_AG_ConnectIndAck
NONE
CTRL
DTUN
FMTX
FTPS
TEST
SAPS
HCIUTILS
AVRC
FLICK
SLIDESHOW
GATT
PANU
brcm.bt.btlif



*/

#endif  //#ifdef  MY_DISABLED

