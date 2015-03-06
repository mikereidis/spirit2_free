
  #define  NO_RDS                                                  // RDS include line removed from SpiritFree

    // RDS:

  #define RDS_MAX_PS     8                                              // RDS PS   has maximum of  8 characters.
  #define RDS_MAX_RT    64                                              // RDS RT   has maximum of 64 characters.
  #define RDS_MAX_AFS   25                                              // RDS AF   has maximum of 25 alternate frequencies.

  //#define RDS_MAX_PTYN   8                                              // RDS PTYN has maximum of  8 characters. Translated name form of RDS PT integer.
  //#define RDS_MAX_CT    14                                              // RDS CT   has maximum of 14 characters. String from RDS UTC time
  //#define RDS_MAX_TMC    3                                              // RDS TMC  has maximum of  3 entries.

  typedef struct {                                                      // rds__struct_t

    unsigned short  srds_pi;
    short           srds_pt;
    short           srds_af_num;
    short           rsvd        [5];

    char            srds_ps     [RDS_MAX_PS  + 8];
    char            srds_rt     [RDS_MAX_RT  + 8];

    int             srds_af     [RDS_MAX_AFS + 3];
// Above:   bytes: 216

    //short           tp;
    //short           ta;
    //short           ms;
    //char            ct        [RDS_MAX_CT  + 1];
    //char            ptyn      [RDS_MAX_PTYN+ 1];
    //short           tmc       [RDS_MAX_TMC];
    //int             taf;
  } rds_struct_t;

    // Plugin Callbacks structure:

  typedef struct {
    void (* cb_tuner_state)  (int new_state);
    void (* cb_tuner_rssi)   (int new_rssi);
    void (* cb_tuner_pilot)  (int new_pilot);
    void (* cb_tuner_rds)    (rds_struct_t * new_rds);
    void (* cb_tuner_rds_af) (int new_freq);
  } plugin_cbs_t;

    // Plugin Functions structure:

  typedef struct {     // plugin_funcs_t            All implemented in tnr_tnr.c:            As tuner_mode_sg(), tuner_event_sg(), 

        // 21 Remote sget Set/Get:  Implemented in tnr_tnr.c, with 16 calls to similar functions in tnr_abc.c and 5 just a getter/setter for a local variable

    int (* tnr_tuner_event_sg)         (unsigned char * rds_grpd);//int event_sg_ms);             // Connection to chip_imp_event_sg()

    int (* tnr_tuner_api_mode_sg)      (int api_mode);                // Set/get curr_api_mode
    int (* tnr_tuner_api_state_sg)     (int api_state);
    int (* tnr_tuner_mode_sg)          (int mode);                    // Set/get curr_mode
    int (* tnr_tuner_state_sg)         (int state);

    int (* tnr_tuner_antenna_sg)       (int antenna);
    int (* tnr_tuner_band_sg)          (int band);
    int (* tnr_tuner_freq_sg)          (int freq);
    int (* tnr_tuner_vol_sg)           (int vol);
    int (* tnr_tuner_thresh_sg)        (int thresh);                  // Set/get curr_thresh

    int (* tnr_tuner_mute_sg)          (int mute);
    int (* tnr_tuner_softmute_sg)      (int softmute);
    int (* tnr_tuner_stereo_sg)        (int stereo);
    int (* tnr_tuner_seek_state_sg)    (int seek_state);
    int (* tnr_tuner_rds_state_sg)     (int rds_state);               // Set/get curr_pwr_rds
    int (* tnr_tuner_rds_af_state_sg)  (int af_state);                // Set/get curr_rds_af_state

    int (* tnr_tuner_rssi_sg)          (int fake_rssi);
    int (* tnr_tuner_pilot_sg)         (int fake_pilot);

    int (* tnr_tuner_rds_pi_sg)        (int rds_pi);
    int (* tnr_tuner_rds_pt_sg)        (int rds_pt);

    char * (* tnr_tuner_rds_ps_sg)     (char * rds_ps);
    char * (* tnr_tuner_rds_rt_sg)     (char * rds_rt);

    char * (* tnr_tuner_extension_sg)  (char * reg);                   // FM tuner chip low level register access and other extensions

  }  plugin_funcs_t;



  int chip_imp_event_sg        (unsigned char * rds_grpd);//int event_sg_ms);             // Connection to chip_imp_event_sg()

  int chip_imp_api_mode_sg     (int api_mode);                          // Set/get curr_api_mode
  int chip_imp_api_state_sg    (int api_state);
  int chip_imp_mode_sg         (int mode);                              // Set/get curr_mode
  int chip_imp_state_sg        (int state);

  int chip_imp_antenna_sg      (int antenna);
  int chip_imp_band_sg         (int band);
  int chip_imp_freq_sg         (int freq);
  int chip_imp_vol_sg          (int vol);
  int chip_imp_thresh_sg       (int thresh);                            // Set/get curr_thresh

  int chip_imp_mute_sg         (int mute);
  int chip_imp_softmute_sg     (int softmute);
  int chip_imp_stereo_sg       (int stereo);
  int chip_imp_seek_state_sg   (int seek_state);
  int chip_imp_rds_state_sg    (int rds_state);                         // Set/get curr_pwr_rds
  int chip_imp_rds_af_state_sg (int af_state);                          // Set/get curr_rds_af_state

  int chip_imp_rssi_sg         (int fake_rssi);                         // Fake = read-only; set makes no sense for receive
  int chip_imp_pilot_sg        (int fake_pilot);                        // Fake = read-only; set makes no sense for receive

  int chip_imp_rds_pi_sg       (int rds_pi);
  int chip_imp_rds_pt_sg       (int rds_pt);

  char * chip_imp_rds_ps_sg    (char * rds_ps);
  char * chip_imp_rds_rt_sg    (char * rds_rt);

  char * chip_imp_extension_sg (char * reg);                   // FM tuner chip low level register access and other extensions



    // Plugin Registration function typedef implemented by plugin.
    // Returns plugin signature and pointer to functions structure.
    // Expects callback pointers to be returned.

  typedef int (* plugin_reg_t) (unsigned int * sig, plugin_funcs_t * funcs, plugin_cbs_t * cbs);

  #define PLUGIN_SIG 0xABCDEF01                                         // Plugin signature to test

