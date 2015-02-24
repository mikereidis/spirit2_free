

  unsigned char hci_recv_buf [1024] = {0};             // Should only need 259 bytes ?

  int gen_server_poll_func (int poll_ms) {
    //loge ("!!!! NOT USED");
  }

  int shim_hci_xact (unsigned char * cmd, int cmd_len) {                // Do bluedroid mode command

    //Tx:
    //if (cmd [4] == 0x01

    uint16_t ocf = cmd [5];                                               // 0x15
    uint16_t ocf_hi = (cmd [6] & 0x03) << 8;
    ocf |= ocf_hi;
    uint16_t ogf = (cmd [6] & 0xfc) >> 2;

    unsigned char * hci_data = & cmd [8];
    int hci_len = cmd_len - 8; // cmd [7]

    //logd ("shim_hci_xact ogf: 0x%x  ocf: 0x%x  hci_data: %p  hci_len: %d", ogf, ocf, hci_data, hci_len);

    bfm_rx_len = 0;

    // NO gv_mem
    //bfm_rx_

    TRANSAC tx_mem = bfm_send (& cmd [5], hci_len + 3);                   // Send command to Bluedroid

    // ff 04 0f 04 00 01 00 fc
    // 00  
///* Doesn't help: ??
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

      loge ("!!!!!!!!!    shim_hci_xact LG G2 BC intercept before ms_sleep(700)");
      ms_sleep (700);
      loge ("!!!!!!!!!    shim_hci_xact LG G2 BC intercept after ms_sleep(700)");

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
          loge ("shim_hci_xact rret1: %d  rret2: %d", rret1, rret2);
        return (rret1);
      }
      //ms_sleep (1);
      quiet_ms_sleep (10);      // !!!! Is it OK that this triggers all the time ??
    }

    if (tmo_ctr >= 2000)
      return (-1);

    return (-2);
  }



  int gen_server_loop_func (unsigned char * cmd_buf, int cmd_len, unsigned char * res_buf, int res_max ) {
    if (cmd_len == 1 && cmd_buf [0] == 0x73) {
      logd ("gen_server_loop_func got ready inquiry/start");
      res_buf [0] = cmd_buf [0];
      return (1);
    }
    else if (cmd_len == 1 && cmd_buf [0] == 0x7f) {                     // Not used at present
      logd ("gen_server_loop_func got stop");
      res_buf [0] = cmd_buf [0];
      gen_server_exiting = 1;
      return (1);
    }

    int hx_ret = shim_hci_xact (cmd_buf, cmd_len);                      // Do Shim HCI transaction
    if (hx_ret < 8 || hx_ret > 270) {
      hci_recv_buf [0] = 0xff; // Error
      return (8);
    }
    hci_recv_buf [0] = 0;
    memcpy (res_buf, hci_recv_buf, hx_ret);
    //hex_dump ("aaaa", 32, hci_recv_buf, hx_ret);
    //hex_dump ("bbbb", 32, res_buf, hx_ret);
  
    if (res_buf [7])
      loge ("gen_server_loop_func hci err: %d %s", res_buf [7], hci_err_get (res_buf [7]));
  
    return (hx_ret);
  }

