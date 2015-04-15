

  unsigned char hci_recv_buf [1024] = {0};             // Should only need 259 bytes ?

  int gen_server_poll_func (int poll_ms) {
    //loge ("!!!! NOT USED");
  }

  char * gv_mem_alloc ();
  int gv_mem_xmit (int ogf, int ocf, int opcode, int len);
  static char * gv_mem;// = NULL;

  TRANSAC bfm_send (char * buf, int len) {

    if (ena_log_ven_extra)
      logd ("bfm_send buf: %p  len: %d", buf, len);

    if (len < 0 + 3 || len > 255 + 3) {                                 // If invalid len...
      loge ("bfm_send len error: %d", len);
      return (NULL);
    }

    gv_mem = gv_mem_alloc ();
    if (gv_mem == NULL) {
      loge ("bfm_send alloc error");
      return (NULL);
    }

    uint16_t ocf    = buf [0] + ((buf [1] & 0x03) << 8);                // OCF = 0x15 (FM) or 0x00 (G2 audio switch)
    uint16_t ogf    = (buf [1] & 0xfc) >> 2;                            // OGF = 0x3F

    char * hci_data = & buf [3];
    int hci_len     = len - 3;                                          // Also at buf [2]
    uint16_t opcode = (ogf << 10) | ocf;                                // opcode = 0xFC15 (FM) or 0xFC00 (G2 audio switch)

    if (ena_log_ven_extra)
      logd ("bfm_send ogf: 0x%x  ocf: 0x%x  opcode: 0x%x  hci_data: %p  hci_len: %d", ogf, ocf, opcode, hci_data, hci_len);

    int event_mask = 0x2000;
    if (ocf == 0) {
      //event_mask = 0; //event_mask = 0xffff;//2100;//0x0;//0;//0xff00;        0: Ctlr H/w error event - code:0x0
    }

    uint16_t HC_BT_HDR_event          = event_mask;//0xffff;//Event Mask 0x2000;    // MSG_STACK_TO_HC_HCI_CMD | 0 / LOCAL_BR_EDR_CONTROLLER_ID
    uint16_t HC_BT_HDR_len            = len;                            // 0x0003;
    uint16_t HC_BT_HDR_offset         = 0x0000;                         // Starts at [8] so offset = 0
    uint16_t HC_BT_HDR_layer_specific = opcode;                         // Layer Specific = Our code to use (Non-Zero identifies this as bt-vendor internal

    gv_mem [0] = HC_BT_HDR_event             & 0x00ff;
    gv_mem [1] = HC_BT_HDR_event         >>8 & 0x00ff;
    gv_mem [2] = HC_BT_HDR_len               & 0x00ff;
    gv_mem [3] = HC_BT_HDR_len           >>8 & 0x00ff;
    gv_mem [4] = HC_BT_HDR_offset            & 0x00ff;
    gv_mem [5] = HC_BT_HDR_offset        >>8 & 0x00ff;
    gv_mem [6] = HC_BT_HDR_layer_specific    & 0x00ff;
    gv_mem [7] = HC_BT_HDR_layer_specific>>8 & 0x00ff;

    memcpy (& gv_mem [8], buf, len);                                    // Copy HCI data to gv_mem

    if (ena_log_hex_dump) {
      int shortlen = len + 8;
      if (shortlen < 0)
        shortlen = 0;
      if (shortlen > 32)
        shortlen = 32;
      hex_dump ("TXB ", 16, (unsigned char *) gv_mem, shortlen);
    }
//03-13 06:16:35.707 D/s2bt-ven( 9505): bc_g2_pcm_set 1
//03-13 06:16:35.707 W/s2bt-ven( 9505): TXB 00 20 08 00 00 00 00 fc 00 fc 05 f3 88 01 02 05 
//03-13 06:16:35.717 E/s2bt-ven( 9505): bc_g2_pcm_set OK



    int ret = gv_mem_xmit (ogf, ocf, opcode, len);
    if (ena_log_ven_extra)
      logd ("bfm_send gv_mem_xmit ret: %d", ret);

    return (gv_mem);
  }


  static int num_hci_send = 0;
  static int num_hci_cback = 0;

  int shim_hci_xact (unsigned char * cmd, int cmd_len) {                // Do HCI command

    //Tx:
    //if (cmd [4] == 0x01

    uint16_t ocf = cmd [5];                                             // 0x15
    uint16_t ocf_hi = (cmd [6] & 0x03) << 8;
    ocf |= ocf_hi;
    uint16_t ogf = (cmd [6] & 0xfc) >> 2;

    unsigned char * hci_data = & cmd [8];
    int hci_len = cmd_len - 8; // cmd [7]

    //logd ("shim_hci_xact ogf: 0x%x  ocf: 0x%x  hci_data: %p  hci_len: %d", ogf, ocf, hci_data, hci_len);

    bfm_rx_len = 0;

    // NO gv_mem
    //bfm_rx_

    logv ("shim_hci_xact before bfm_send() num_hci_cback: %d  num_hci_send: %d", num_hci_cback, num_hci_send);
    num_hci_send ++;
    TRANSAC tx_mem = bfm_send (& cmd [5], hci_len + 3);                   // Send command to Bluedroid and call bfm_cback() when response

    logv ("shim_hci_xact after  bfm_send() num_hci_cback: %d  num_hci_send: %d", num_hci_cback, num_hci_send);

    // ff 04 0f 04 00 01 00 fc
    // 00  

    int bad_g2_hack = 1;
    if (ogf == 0x3f && ocf == 0) {
      int len = bfm_rx_len;
      logw ("!!!!!!!!!    shim_hci_xact LG G2 BC intercept len: %d", len);
      hex_dump ("rxb ", 16, (unsigned char *) bfm_rx_buf, 16);//len);
/*
      //bfm_rx_len = 0;
      ms_sleep (2000);
      logw ("!!!!!!!!!    shim_hci_xact LG G2 BC intercept after ms_sleep() bfm_rx_len: ", bfm_rx_len);
      if (bfm_rx_len) {
        logw ("!!!!!!!!!    shim_hci_xact LG G2 BC intercept bfm_rx_len: %d", bfm_rx_len);
        hex_dump ("g2 ", 16, (unsigned char *) bfm_rx_buf, bfm_rx_len);
      }
      else
        logw ("!!!!!!!!!    shim_hci_xact LG G2 BC intercept bfm_rx_len: %d", bfm_rx_len);
*/

      if (bad_g2_hack) {                                                // Fake a response
        hci_recv_buf [0] = 0;                                           // [0] = Success
        hci_recv_buf [1] = 1;                                           // [1] = 1 for HCI Command packet
        hci_recv_buf [2] = 0x0e;                                        // [2] = Event Code: 0x0e = Command Complete HCI_EV_CMD_COMPLETE
        hci_recv_buf [3] = 4;                                           // [3] = Length remaining = 4
        hci_recv_buf [4] = 0;                                           //
        hci_recv_buf [5] = 1;                                           //
        hci_recv_buf [6] = 0;                                           //
        hci_recv_buf [7] = 0;                                           // [7] = No Error
        hci_recv_buf [8] = 0;
        ms_sleep (200);
        loge ("!!!!!!!!!    shim_hci_xact LG G2 BC intercept after ms_sleep(200)");
        return (8);//9);
      }
    }


    int tmo_ctr = 0;
    int sleep_ms = 10;
    int tmo_ctr_max = 2000;                                             // Maximum 2 seconds
    while (tx_mem && tmo_ctr ++ < tmo_ctr_max / sleep_ms) {
      if (bfm_rx_len) {
        if (bfm_rx_len > 0 && bfm_rx_len < MAX_HCI) {
          hci_recv_buf [0] = 0;
          hci_recv_buf [1] = 1;
          memcpy (& hci_recv_buf [2], bfm_rx_buf, bfm_rx_len);
        }
        int rret1 = bfm_rx_len - 6;
        int rret2 = bfm_rx_buf [1] + 4;
        if (rret1 != rret2)
          loge ("shim_hci_xact rret1: %d  rret2: %d", rret1, rret2);
        return (rret1);                                                 // Done return size of response
      }
      quiet_ms_sleep (sleep_ms);
    }

    loge ("shim_hci_xact timeout error");

    return (-1);                                                        // Timeout
  }


  //#define MAX_HCI  264   // 8 prepended bytes + 255 max bytes HCI data/parameters + 1 trailing byte to align

                                                                        // Handle SHIM HCI server command and formulate response
  int gen_server_loop_func (unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max ) {
    if (cmd_len == 1 && cmd_buf [0] == 0x73) {
      logd ("gen_server_loop_func got ready inquiry/start");
      res_buf [0] = cmd_buf [0];                                        // 0x73
      return (1);
    }
    else if (cmd_len == 1 && cmd_buf [0] == 0x7f) {                     // Not used at present
      logd ("gen_server_loop_func got stop");
      res_buf [0] = cmd_buf [0];                                        // 0x7f
      gen_server_exiting = 1;
      return (1);
    }

    int hx_ret = shim_hci_xact (cmd_buf, cmd_len);                      // Do Shim HCI transaction
    if (hx_ret < 8 || hx_ret >= MAX_HCI) {
      hci_recv_buf [0] = 0xff;                                          // [0] = Error
      return (8);
    }
    //hex_dump ("aaaa", 32, hci_recv_buf, hx_ret);
    hci_recv_buf [0] = 0;                                               // [0] = Success
    memcpy (res_buf, hci_recv_buf, hx_ret);
  
    if (res_buf [7])
      loge ("gen_server_loop_func hci err: %d %s", res_buf [7], hci_err_get (res_buf [7]));
  
    return (hx_ret);
  }

