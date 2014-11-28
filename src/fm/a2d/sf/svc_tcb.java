
    // Tuner Callbacks

package fm.a2d.sf;

public interface svc_tcb {

  public abstract void cb_tuner_key          (String key, String val);

/*  Callbacks are possible for changes to each of these:
// Essential:
  t_api_state                   //
  tuner_state                   //

// Helpful:
  tuner_freq                    // Helps deal with seek results
  tuner_rssi                    // Just for display & low level AF/scanning
  tuner_qual                    //

// RDS Basics:
  tuner_rds_pi                  //
  tuner_rds_pt                  // PT and/or PTYN
  tuner_rds_ps                  //
  tuner_rds_rt                  //
*/


// RDS special extensions that some APIs don't support:
/*
  tuner_rds_af
  tuner_rds_ms
  tuner_rds_ct
  tuner_rds_tmc
  tuner_rds_tp
  tuner_rds_ta
  tuner_rds_taf

  //tuner_rds_ptyn              // Use pt callback

// API specific:
  tuner_extra_resp  // via tuner_extra_cmdZ
*/
}
