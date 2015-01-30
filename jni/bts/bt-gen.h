
    // Bluetooth/Bluedroid definitions
    // Derived from Google/Broadcom bt_hci_bdroid.h
    // Like Google's "cleaned Linux headers" this file is not copyrightable as it contains only definitions/facts


/******************************************************************************
 *
 *  Filename:      bt_hci_bdroid.h
 *
 *  Description:   A wrapper header file of bt_hci_lib.h
 *
 *                 Contains definitions specific for interfacing with Bluedroid
 *                 Bluetooth stack
 *
 ******************************************************************************/

#pragma once

#ifdef HAS_BDROID_BUILDCFG
#include "bdroid_buildcfg.h"
#endif

/******************************************************************************
**  Constants & Macros
******************************************************************************/

#if __STDC_VERSION__ < 199901L
#  ifndef FALSE
#    define FALSE 0
#  endif
#  ifndef TRUE
#    define TRUE (!FALSE)
#  endif
#else
#  include <stdbool.h>
#  ifndef FALSE
#    define FALSE  false
#  endif
#  ifndef TRUE
#    define TRUE   true
#  endif
#endif

#define HCI_ACL_MAX_SIZE 1024
#define HCI_MAX_FRAME_SIZE (HCI_ACL_MAX_SIZE + 4)

/* Host/Controller lib internal event ID */
typedef enum {
  HC_EVENT_LPM_IDLE_TIMEOUT,
} bthc_event_t;

#define MSG_CTRL_TO_HC_CMD             0x0100 /* evt mask used by HC_EVENT_TX_CMD */

/* Message event mask across Host/Controller lib and stack */
#define MSG_EVT_MASK                    0xFF00 /* eq. BT_EVT_MASK */
#define MSG_SUB_EVT_MASK                0x00FF /* eq. BT_SUB_EVT_MASK */

/* Message event ID passed from Host/Controller lib to stack */
#define MSG_HC_TO_STACK_HCI_ERR        0x1300 /* eq. BT_EVT_TO_BTU_HCIT_ERR */
#define MSG_HC_TO_STACK_HCI_ACL        0x1100 /* eq. BT_EVT_TO_BTU_HCI_ACL */
#define MSG_HC_TO_STACK_HCI_SCO        0x1200 /* eq. BT_EVT_TO_BTU_HCI_SCO */
#define MSG_HC_TO_STACK_HCI_EVT        0x1000 /* eq. BT_EVT_TO_BTU_HCI_EVT */
#define MSG_HC_TO_STACK_L2C_SEG_XMIT   0x1900 /* eq. BT_EVT_TO_BTU_L2C_SEG_XMIT */

/* Message event ID passed from stack to vendor lib */
#define MSG_STACK_TO_HC_HCI_ACL        0x2100 /* eq. BT_EVT_TO_LM_HCI_ACL */
#define MSG_STACK_TO_HC_HCI_SCO        0x2200 /* eq. BT_EVT_TO_LM_HCI_SCO */
#define MSG_STACK_TO_HC_HCI_CMD        0x2000 /* eq. BT_EVT_TO_LM_HCI_CMD */

/* Local Bluetooth Controller ID for BR/EDR */
#define LOCAL_BR_EDR_CONTROLLER_ID      0

/* Definitions of audio codec type
 *      inherited from AG callout function "codec" parameter
 */
#define SCO_CODEC_NONE      0x0000 /* BTA_AG_CODEC_NONE/BTM_SCO_CODEC_NONE */
#define SCO_CODEC_CVSD      0x0001 /* BTA_AG_CODEC_CVSD/BTM_SCO_CODEC_CVSD */
#define SCO_CODEC_MSBC      0x0002 /* BTA_AG_CODEC_MSBC/BTM_SCO_CODEC_MSBC */

/******************************************************************************
**  Type definitions and return values
******************************************************************************/

typedef struct
{
    uint16_t          event;
    uint16_t          len;
    uint16_t          offset;
    uint16_t          layer_specific;
    uint8_t           data[];
} HC_BT_HDR;

#define BT_HC_HDR_SIZE (sizeof(HC_BT_HDR))

typedef struct _hc_buffer_hdr
{
    struct _hc_buffer_hdr *p_next;   /* next buffer in the queue */
    uint8_t   reserved1;
    uint8_t   reserved2;
    uint8_t   reserved3;
    uint8_t   reserved4;
} HC_BUFFER_HDR_T;

#define BT_HC_BUFFER_HDR_SIZE (sizeof(HC_BUFFER_HDR_T))

/******************************************************************************
**  Extern variables and functions
******************************************************************************/

extern bt_hc_callbacks_t *bt_hc_cbacks;

/******************************************************************************
**  Functions
******************************************************************************/

// Called when a buffer has been produced by the serial layer and should be
// processed by the HCI layer.
void bthc_rx_ready(void);
void bthc_tx(HC_BT_HDR *buf);
void bthc_idle_timeout(void);
