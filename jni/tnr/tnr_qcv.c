
    // Spirit2 Tuner Plugin for "Qualcomm V4L" API:

    // See https://android.googlesource.com/kernel/msm/+/android-msm-dory-3.10-lollipop-wear-release/drivers/media/radio/radio-iris.c   radio-iris-transport.c 

  #define LOGTAG "sftnrqcv"

  #include <stdio.h>
  #include <errno.h>
  #include <sys/stat.h>
  #include <fcntl.h>

  #include <sys/ioctl.h>
  #include <linux/videodev2.h>

  #include "tnr_tnr.c"

  int dev_hndl = -1;

    // Internal functions:


    // V4L support:

  const char * cid_iris_private [] = {
    "SRCHMODE",
    "SCANDWELL",
    "SRCHON",
    "STATE",
    "TRANSMIT_MODE",
    "RDSGROUP_MASK",
    "REGION",
    "SIGNAL_TH",
    "SRCH_PTY",
    "SRCH_PI",
    "SRCH_CNT",
    "EMPHASIS",
    "RDS_STD",
    "SPACING",
    "RDSON",
    "RDSGROUP_PROC",
    "LP_MODE",
    "ANTENNA",
    "RDSD_BUF",
    "PSALL",

    "TX_SETPSREPEATCOUNT",
    "STOP_RDS_TX_PS_NAME",
    "STOP_RDS_TX_RT",
    "IOVERC",
    "INTDET",
    "MPX_DCC",
    "AF_JUMP",
    "RSSI_DELTA",
    "HLSI",             // 0x800001d

        /*Diagnostic commands*/
    "SOFT_MUTE",
    "RIVA_ACCS_ADDR",
    "RIVA_ACCS_LEN",
    "RIVA_PEEK",
    "RIVA_POKE",
    "SSBI_ACCS_ADDR",
    "SSBI_PEEK",
    "SSBI_POKE",
    "TX_TONE",
    "RDS_GRP_COUNTERS",
    "SET_NOTCH_FILTER", // 0x8000028
    "SET_AUDIO_PATH",   // TAVARUA specific command
    "DO_CALIBRATION",
    "SRCH_ALGORITHM",   // TAVARUA specific command
    "GET_SINR",
    "INTF_LOW_THRESHOLD",
    "INTF_HIGH_THRESHOLD",
    "SINR_THRESHOLD",
    "SINR_SAMPLES",          // 0x8000030


   };





  enum v4l2_cid_iris_private_iris_t {
        V4L2_CID_PRIVATE_IRIS_SRCHMODE = (0x08000000 + 1),                      // = 0
        V4L2_CID_PRIVATE_IRIS_SCANDWELL,
        V4L2_CID_PRIVATE_IRIS_SRCHON,           // = 1 ? Stuck searching ?
        V4L2_CID_PRIVATE_IRIS_STATE,            // = 1
        V4L2_CID_PRIVATE_IRIS_TRANSMIT_MODE,    // 0x08000005 Tx only ??
        V4L2_CID_PRIVATE_IRIS_RDSGROUP_MASK,
        V4L2_CID_PRIVATE_IRIS_REGION,

        V4L2_CID_PRIVATE_IRIS_SIGNAL_TH,        // 0x08000008
        V4L2_CID_PRIVATE_IRIS_SRCH_PTY,
        V4L2_CID_PRIVATE_IRIS_SRCH_PI,
        V4L2_CID_PRIVATE_IRIS_SRCH_CNT,
        V4L2_CID_PRIVATE_IRIS_EMPHASIS,                                         // 0
        V4L2_CID_PRIVATE_IRIS_RDS_STD,          // 0x0800000d = 1
        V4L2_CID_PRIVATE_IRIS_SPACING,
        V4L2_CID_PRIVATE_IRIS_RDSON,            // 0x0800000f = 1
        V4L2_CID_PRIVATE_IRIS_RDSGROUP_PROC,    // 0x08000010 = 56 = 0x38 = 0x07 << RDS_CONFIG_OFFSET (3)
        V4L2_CID_PRIVATE_IRIS_LP_MODE,
        V4L2_CID_PRIVATE_IRIS_ANTENNA,          // = 1
        V4L2_CID_PRIVATE_IRIS_RDSD_BUF,
        V4L2_CID_PRIVATE_IRIS_PSALL,            // 0x8000014 = 56, ? Bug, copied RDSGROUP_PROC instead of boolean "pass all ps strings"
        V4L2_CID_PRIVATE_IRIS_TX_SETPSREPEATCOUNT,                      // START TX controls:
        V4L2_CID_PRIVATE_IRIS_STOP_RDS_TX_PS_NAME,
        V4L2_CID_PRIVATE_IRIS_STOP_RDS_TX_RT,
        V4L2_CID_PRIVATE_IRIS_IOVERC,
        V4L2_CID_PRIVATE_IRIS_INTDET,
        V4L2_CID_PRIVATE_IRIS_MPX_DCC,
        V4L2_CID_PRIVATE_IRIS_AF_JUMP,
        V4L2_CID_PRIVATE_IRIS_RSSI_DELTA,
        V4L2_CID_PRIVATE_IRIS_HLSI,             // 0x800001d
        V4L2_CID_PRIVATE_IRIS_SOFT_MUTE,        // 0x800001e            // START Diagnostic commands:
        V4L2_CID_PRIVATE_IRIS_RIVA_ACCS_ADDR,
        V4L2_CID_PRIVATE_IRIS_RIVA_ACCS_LEN,
        V4L2_CID_PRIVATE_IRIS_RIVA_PEEK,
        V4L2_CID_PRIVATE_IRIS_RIVA_POKE,
        V4L2_CID_PRIVATE_IRIS_SSBI_ACCS_ADDR,
        V4L2_CID_PRIVATE_IRIS_SSBI_PEEK,
        V4L2_CID_PRIVATE_IRIS_SSBI_POKE,
        V4L2_CID_PRIVATE_IRIS_TX_TONE,
        V4L2_CID_PRIVATE_IRIS_RDS_GRP_COUNTERS,
        V4L2_CID_PRIVATE_IRIS_SET_NOTCH_FILTER, // 0x8000028
        V4L2_CID_PRIVATE_IRIS_SET_AUDIO_PATH,   // TAVARUA specific command
        V4L2_CID_PRIVATE_IRIS_DO_CALIBRATION,
        V4L2_CID_PRIVATE_IRIS_SRCH_ALGORITHM,   // TAVARUA specific command
        V4L2_CID_PRIVATE_IRIS_GET_SINR,
        V4L2_CID_PRIVATE_INTF_LOW_THRESHOLD,
        V4L2_CID_PRIVATE_INTF_HIGH_THRESHOLD,
        V4L2_CID_PRIVATE_SINR_THRESHOLD,
        V4L2_CID_PRIVATE_SINR_SAMPLES,          // 0x8000030

        /*using private CIDs under userclass*/
        V4L2_CID_PRIVATE_IRIS_READ_DEFAULT = 0x00980928,
        V4L2_CID_PRIVATE_IRIS_WRITE_DEFAULT,
        V4L2_CID_PRIVATE_IRIS_SET_CALIBRATION,
  };


    // Not in /home/m/bin/android-ndk-r10/platforms/android-19/arch-arm/usr/include/linux/videodev2.h or videodev.h
  #define V4L2_CID_RDS_TX_PI                      (V4L2_CID_FM_TX_CLASS_BASE + 2)
  #define V4L2_CID_RDS_TX_PTY                     (V4L2_CID_FM_TX_CLASS_BASE + 3)
  #define V4L2_CID_RDS_TX_PS_NAME                 (V4L2_CID_FM_TX_CLASS_BASE + 5)
  #define V4L2_CID_RDS_TX_RADIO_TEXT              (V4L2_CID_FM_TX_CLASS_BASE + 6)
 
/*
        .vidioc_querycap              = iris_vidioc_querycap,
        .vidioc_queryctrl             = iris_vidioc_queryctrl,
        .vidioc_g_ctrl                = iris_vidioc_g_ctrl,
        .vidioc_s_ctrl                = iris_vidioc_s_ctrl,
        .vidioc_g_tuner               = iris_vidioc_g_tuner,
        .vidioc_s_tuner               = iris_vidioc_s_tuner,
        .vidioc_g_frequency           = iris_vidioc_g_frequency,
        .vidioc_s_frequency           = iris_vidioc_s_frequency,
        .vidioc_s_hw_freq_seek        = iris_vidioc_s_hw_freq_seek,
        .vidioc_dqbuf                 = iris_vidioc_dqbuf,
        .vidioc_g_fmt_type_private    = iris_vidioc_g_fmt_type_private,
        .vidioc_s_ext_ctrls           = iris_vidioc_s_ext_ctrls,
        .vidioc_g_ext_ctrls           = iris_vidioc_g_ext_ctrls,
*/

  // V4l2 structures:
  struct v4l2_capability    v4l_cap     = {0};
  struct v4l2_tuner         v4l_tuner   = {0};
  struct v4l2_control       v4l_ctrl    = {0};
  struct v4l2_frequency     v4l_freq    = {0};

  //#define V4L2_CAP_HW_FREQ_SEEK           0x00000400  /* Can do hardware frequency seek  */

  #ifndef v4l2_hw_freq_seek
  struct v4l2_hw_freq_seek {
        __u32                 tuner;
        enum v4l2_tuner_type  type;
        __u32                 seek_upward;
        __u32                 wrap_around;
        //__u32                 reserved[8];
        __u32                 spacing;
        __u32                 reserved[7];
  };
  #endif

  struct v4l2_hw_freq_seek  v4l_seek;//    = {0};

  //int tuner_cap_low = 0;  // Don't need for Qualcomm chip, hi only

  #ifndef DEF_BUF
    #define DEF_BUF 512                                                 // Raised from 256 so we can add headers to 255-256 byte buffers
  #endif

  #ifndef   VIDIOC_S_HW_FREQ_SEEK
    #define VIDIOC_S_HW_FREQ_SEEK    _IOW('V', 82, struct v4l2_hw_freq_seek)
  #endif

  char * req_get (int req) {
    switch (req) {
      case VIDIOC_DQBUF:            return ("VIDIOC_DQBUF");
      case VIDIOC_G_CTRL:           return ("VIDIOC_G_CTRL");
      case VIDIOC_G_FREQUENCY:      return ("VIDIOC_G_FREQUENCY");
      case VIDIOC_G_TUNER:          return ("VIDIOC_G_TUNER");
      case VIDIOC_QUERYCAP:         return ("VIDIOC_QUERYCAP");
      case VIDIOC_S_CTRL:           return ("VIDIOC_S_CTRL");
      case VIDIOC_S_EXT_CTRLS:      return ("VIDIOC_S_EXT_CTRLS");
      case VIDIOC_S_FREQUENCY:      return ("VIDIOC_S_FREQUENCY");
      case VIDIOC_S_HW_FREQ_SEEK:   return ("VIDIOC_S_HW_FREQ_SEEK");
      case VIDIOC_S_TUNER:          return ("VIDIOC_S_TUNER");
    }
    return ("Unknown !");
  }

  int qcv_ioctl_par (int fd, int req, void * par) {                         // (int d, int request, ...);
    int ret = ioctl (fd, req, par);
    if (ena_log_chip_access) {
      if (req == VIDIOC_G_TUNER)
        logd ("qcv_ioctl_par req: %s", req_get (req));
      else
        logd ("qcv_ioctl_par req: %s", req_get (req));
    }
    return (ret);
  }

  int buf_display (char * buf, int size) {
    int ctr = 0;
    char asc [4 * DEF_BUF] = "evt: ";
    char byte_asc [4] = " xx";
    for (ctr = 0; ctr < size && ctr < DEF_BUF; ctr ++) {
      byte_asc [1] = '0' + ((0xF0 & buf [ctr]) >> 4);
      if (byte_asc [1] > '9') {
        byte_asc [1] -= '0';
        byte_asc [1] += 'A';
        byte_asc [1] -= 10;
      }
      byte_asc [2] = '0' + ((0x0F & buf [ctr]) >>0);
      if (byte_asc [2] > '9') {
        byte_asc [2] -= '0';
        byte_asc [2] += 'A';
        byte_asc [2] -= 10;
      }
      strlcat (asc, byte_asc, sizeof (asc));
    }
    logd ("%s", asc);
    return (0);
  }


  int iris_buf_get (char * buf, int buf_len, int type) {
    int ret = 0;

    //noblock_set (dev_hndl);

    struct v4l2_requestbuffers  reqbuf;
    memset (& reqbuf, 0, sizeof (reqbuf));
    reqbuf.type = V4L2_BUF_TYPE_PRIVATE;
    reqbuf.memory = V4L2_MEMORY_USERPTR;

    struct v4l2_buffer          v4l2_buf;
    memset (& v4l2_buf, 0, sizeof (v4l2_buf));
    v4l2_buf.index = type;
    v4l2_buf.type = reqbuf.type;
    v4l2_buf.length = buf_len;//128;
    v4l2_buf.m.userptr = (unsigned long) buf;

    ret = qcv_ioctl_par (dev_hndl, VIDIOC_DQBUF, & v4l2_buf);
    if (ret < 0) {
      return (-1);
    }

    return (v4l2_buf.bytesused);
  }


    #define MISC_BUF_SIZE    128

    // Types:
    #define BUF_SRCH_LIST   0  // stationList
    #define BUF_EVENTS      1
    #define BUF_RT_RDS      2
    #define BUF_PS_RDS      3
    #define BUF_RAW_RDS     4
    #define BUF_AF_LIST     5
    #define BUF_MAX         6

    // From radio-iris.h:
  enum iris_buf_t {
        IRIS_BUF_SRCH_LIST,
        IRIS_BUF_EVENTS,
        IRIS_BUF_RT_RDS,
        IRIS_BUF_PS_RDS,
        IRIS_BUF_RAW_RDS,
        IRIS_BUF_AF_LIST,
        IRIS_BUF_PEEK,
        IRIS_BUF_SSBI_PEEK,
        IRIS_BUF_RDS_CNTRS,
        IRIS_BUF_RD_DEFAULT,
        IRIS_BUF_CAL_DATA,
        IRIS_BUF_RT_PLUS,
        IRIS_BUF_ERT,
        IRIS_BUF_MAX,
  };


  const char * id_get (int id) {
    if (id >= V4L2_CID_PRIVATE_IRIS_SRCHMODE && id <= V4L2_CID_PRIVATE_SINR_SAMPLES) {          // 0x8000030
      int idx = id - V4L2_CID_PRIVATE_IRIS_SRCHMODE;
      //logd ("id: %d  idx: %d", id, idx);
      const char * id_asc = cid_iris_private [idx];
      //logd ("id_asc: %p", id_asc);
      //logd ("id_asc s: %s", id_asc);
      return (id_asc);
    }
    else
      return ("Unk");
  }

  int chip_ctrl_get (int id) {
    v4l_ctrl.id = id;

    errno = 0;
    int ret = qcv_ioctl_par (dev_hndl, VIDIOC_G_CTRL, & v4l_ctrl);
    int value = v4l_ctrl.value;
    if (ret < 0) {
      loge ("chip_ctrl_get VIDIOC_G_CTRL id: %s (0x%x)  errno: %d (%s)", id_get (id), id, errno, strerror (errno));
      return (-1);
    }
    else {
      logd ("chip_ctrl_get VIDIOC_G_CTRL OK id: %s (0x%x)  value: %d (0x%x)", id_get (id), id, value, value);
    }
    return (value);
  }

  int chip_ctrl_set (int id, int value) {
    v4l_ctrl.value = value;
    v4l_ctrl.id = id;

    errno = 0;
    int ret = qcv_ioctl_par (dev_hndl, VIDIOC_S_CTRL, & v4l_ctrl);
    if (ret < 0)
      loge ("chip_ctrl_set VIDIOC_S_CTRL Error id: 0x%x  value: %d  errno: %d (%s)", id, value, errno, strerror (errno));
    else
      logd ("chip_ctrl_set VIDIOC_S_CTRL OK    id: 0x%x  value: %d", id, value);
    return (ret);
  }

    #define V4L2_CTRL_CLASS_FM_TX           0x009b0000                      // FM Modulator controls
    #define V4L2_CID_FM_TX_CLASS_BASE               (V4L2_CTRL_CLASS_FM_TX | 0x900)
        //#define V4L2_CID_FM_TX_CLASS                    (V4L2_CTRL_CLASS_FM_TX | 1)
    #define V4L2_CID_TUNE_POWER_LEVEL               (V4L2_CID_FM_TX_CLASS_BASE + 113)
        //#define V4L2_CID_TUNE_ANTENNA_CAPACITOR         (V4L2_CID_FM_TX_CLASS_BASE + 114)

  int pwr_off () {
    int ret = 0;
    logd ("pwr_off");
    chip_imp_mute_sg (1);                                               // Mute

    logd ("pwr_off chip_imp_mute_sg done");

    if (curr_rds_state) {
//    if (need_at_least_one_of_these_delays_or_tx_on_may_fail)
//      ms_sleep (digital_delay);
      if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_RDSON, 0) < 0)           // 0 = OFF, 1 = ON
        loge ("pwr_off PRIVATE_IRIS_RDSON 0 error");
      else
        logd ("pwr_off PRIVATE_IRIS_RDSON 0 success");
//    if (need_at_least_one_of_these_delays_or_tx_on_may_fail)
//      ms_sleep (digital_delay);
    }
    logd ("pwr_off RDS off done");

        // We must set IRIS state to 0 because the handle close () does not power down the chip. Keeps running on AT&T OneX(L), and mute will prevent noticing
    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_STATE, 0) < 0)              // 0 = FM_OFF, 1 = FM_RECV, 2 = FM_TRANS, 3 = FM_RESET
      loge ("pwr_off PRIVATE_IRIS_STATE 0 error");
    else
      logd ("pwr_off PRIVATE_IRIS_STATE 0 success");
    logd ("pwr_off done");

    curr_state = 0;
    return (curr_state);
  }

  int vol_get () {
    int vol = curr_vol;
    logd ("chip_imp_vol_get: %d", vol);
    return (vol);
  }

  int freq_get () {//freq_sg
    int ret = 0;
    int freq = 88500;
    if (dev_hndl <= 0) {
      loge ("dev_hndl <= 0");
      //return (-1);
      return (freq);
    }
    v4l_freq.tuner = 0;                                                   // Tuner index = 0
    v4l_freq.type = V4L2_TUNER_RADIO;
    memset (v4l_freq.reserved, 0, sizeof (v4l_freq.reserved));
    errno = 0;
    ret = qcv_ioctl_par (dev_hndl, VIDIOC_G_FREQUENCY, & v4l_freq);
    if (ret < 0) {
      loge ("freq_get VIDIOC_G_FREQUENCY errno: %d (%s)", errno, strerror (errno));
      return (-1);
    }
    freq = v4l_freq.frequency / 16;
    curr_freq_int = freq;
    if (ena_log_tnr_extra)
      logd ("freq_get VIDIOC_G_FREQUENCY success: %d", freq);
    return (freq);
  }

    // Seek:

  enum search_t {
	SEEK,
	SCAN,
	SCAN_FOR_STRONG,
	SCAN_FOR_WEAK,
	RDS_SEEK_PTY,
	RDS_SCAN_PTY,
	RDS_SEEK_PI,
	RDS_AF_JUMP,
  };

  int seek_stop () {
    logd ("seek_stop");

    curr_seek_state = 0;
    return (curr_seek_state);
  }


    // Band stuff:

/*              .id            = V4L2_CID_PRIVATE_IRIS_REGION,
                .type          = V4L2_CTRL_TYPE_INTEGER,
                .name          = "radio standard",
                .minimum       = 0,
                .maximum       = 2,
                .step          = 1,
                .default_value = 0,
enum iris_region_t {
	IRIS_REGION_US,
	IRIS_REGION_EU,
	IRIS_REGION_JAPAN,
	IRIS_REGION_JAPAN_WIDE,
	IRIS_REGION_OTHER   };  */

  int band_set (int low , int high, int band) {                 // ? Do we need to stop/restart RDS power in reg 0x00 ? Or rbds_set to flush ?
    logd ("band_set low: %d  high: %d  band: %d", low, high, band);
    int ret = 0;
    if (dev_hndl <= 0) {
      loge ("dev_hndl <= 0");
      return (-1);
    }
    #define TUNE_MULT  16
    v4l_tuner.index = 0;                                                // Tuner index = 0
    v4l_tuner.type = V4L2_TUNER_RADIO;
    v4l_tuner.rangelow = (low * TUNE_MULT);
    v4l_tuner.rangehigh = (high * TUNE_MULT);
    memset (v4l_tuner.reserved, 0, sizeof (v4l_tuner.reserved));
    if (curr_stereo)
      v4l_tuner.audmode = V4L2_TUNER_MODE_STEREO;
    else
      v4l_tuner.audmode = V4L2_TUNER_MODE_MONO;
    errno = 0;
    ret = qcv_ioctl_par (dev_hndl, VIDIOC_S_TUNER, & v4l_tuner);
    if (ret < 0)
      loge ("band_set VIDIOC_S_TUNER errno: %d (%s)", errno, strerror (errno));
    else
      logd ("band_set VIDIOC_S_TUNER success");

    int v4reg = 1;    // EU
    if (low < 87500 || band == 2)
      v4reg = 0;//2;//4;//3;    // Japan Wide       2;      // Japan
    else if (band == 1)
      v4reg = 0;      // US
    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_REGION, v4reg) < 0) {
      loge ("band_set PRIVATE_IRIS_REGION error band: %d", band);
      return (-1);
    }
    else
      logd ("band_set PRIVATE_IRIS_REGION success band: %d", band);
    return (band);
  }

  int freq_inc_set (int inc) {
    logd ("freq_inc_set inc: %d", inc);
    int ret = 0;
    if (dev_hndl <= 0) {
      loge ("dev_hndl <= 0");
      return (-1);
    }
    int v4spac = 0;                                                     // Must be 0, 1 or 2
    if (inc <= 50)
      v4spac = 2;
    else if (inc <= 100)
      v4spac = 1;
    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_SPACING, v4spac) < 0) {
      loge ("freq_inc_set PRIVATE_IRIS_SPACING error inc: %d", inc);
      return (-1);
    }
    else
      logd ("freq_inc_set PRIVATE_IRIS_SPACING success inc: %d", inc);
    return (inc);
  }

/*                .id            = V4L2_CID_PRIVATE_IRIS_EMPHASIS,
                .type          = V4L2_CTRL_TYPE_BOOLEAN,
                .name          = "Emphasis",
                .minimum       = 0,
                .maximum       = 1,
                .default_value = 0, */

  int emph75_set (int band) {
    logd ("emph75_set band: %d", band);
    if (dev_hndl <= 0) {
      loge ("dev_hndl <= 0");
      return (-1);
    }
    int v4emph = 0;
    if (band == 1)                                                      // If US...
      v4emph = 1;
    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_EMPHASIS, v4emph) < 0) {
      loge ("chip_emph75_set PRIVATE_IRIS_EMPHASIS error band: %d", band);
      return (-1);
    }
    else
      logd ("chip_emph75_set PRIVATE_IRIS_EMPHASIS success band: %d", band);
    return (band);
  }

  int rbds_set (int band) {
    logd ("rbds_set band: %d", band);
    return (band);
  }

    // Disabled internal functions:

  int rds_pi_set (int pi) {
    if (chip_ctrl_set (V4L2_CID_RDS_TX_PI, pi) < 0)
      loge ("rds_pi_set RDS_TX_PI error pi: %d", pi);
    else
      logd ("rds_pi_set RDS_TX_PI success pi: %d", pi);

    curr_rds_pi = cand_rds_pi = conf_rds_pi = pi;
    need_pi_chngd = 1;
    return (0);
  }

  int rds_pt_set (int pt) {
    if (chip_ctrl_set (V4L2_CID_RDS_TX_PTY, pt) < 0)
      loge ("rds_pt_set RDS_TX_PTY error pt: %d", pt);
    else
      logd ("rds_pt_set RDS_TX_PTY success pt: %d", pt);
    curr_rds_pt = cand_rds_pt = conf_rds_pt = pt;
    need_pt_chngd = 1;
    return (0);
  }

  struct mr_v4l2_ext_control {
    __u32 id;
    __u32 size;
    __u32 reserved2 [1];
    union {
      __s32 value;
      __s64 value64;
      char * string;
    };
  } __attribute__ ((packed));
 
  struct mr_v4l2_ext_controls {
    __u32 ctrl_class;
    __u32 count;
    __u32 error_idx;
    __u32 reserved [2];
    struct mr_v4l2_ext_control * controls;
  };
 
/*
   public static final int FM_TX_MAX_PS_LEN           =  (96+1);
   public static final int FM_TX_MAX_RT_LEN           =  (64-1); // One space to include NULL

   private static final int MAX_PS_CHARS = 97;
   private static final int MAX_PS_REP_COUNT = 15;
   private static final int MAX_RDS_GROUP_BUF_SIZE = 62;

#define MASK_TXREPCOUNT            (0x0000000F)
*/


      //                       v-0x00                          v-0x20                          v-0x40                          v-0x60                        v-0x7e            v-0x90          v-0xa0                          v-0xc0                          v-0xd0          v-0xe0          v-0xff //
        char rds_xlat [389] = "áàéèíìóòúùÑÇŞß¡Ĳâäêëîïôöûüñçşǧıĳªα©‰Ǧěňőπ~£€←↑→↓º¹²³±İńűµ¿÷°¼½¾§ÁÀÉÈÍÌÓÒÚÙŘČŠŽÐĿÂÄÊËÎÏÔÖÛÜřčšžđŀÃÅÆŒŷÝÕØÞŊŔĆŚŹŦðãåæœŵýõøþŋŕćśźŧ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~~"; // last ~ @ 0x100
      //char rds_xlat [3] = "á";//àéèíìóò";
      // ąƇȉʋ̍ΏБғԕؙ֗ڛܝޟ
/*

wc
áàéè      0       1       8

hd
áàéè00000000  c3 a1 c3 a0 c3 a9 c3 a8                           |........|
00000008

*/
  int rds_ps_set (const char * ps) {       // chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_STOP_RDS_TX_PS_NAME, 0);

    chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_TX_SETPSREPEATCOUNT, 15);

    struct mr_v4l2_ext_control ext_ctl;
    struct mr_v4l2_ext_controls v4l2_ctls;

    ext_ctl.id     = V4L2_CID_RDS_TX_PS_NAME;
    //ext_ctl.string = rds_xlat;
    ext_ctl.string = (char *) ps;
    ext_ctl.size   = strlen (ext_ctl.string) + 1;//8;//9;//!! Must be 9 !! ?? //strlen (ps) + 1;//8;//9;//strlen (ps) + 1;

    // Ctrls data struct:
    v4l2_ctls.ctrl_class = V4L2_CTRL_CLASS_FM_TX;
    v4l2_ctls.count      = 1;//15;//1;//15;//
    v4l2_ctls.controls   = & ext_ctl;

    errno = 0;
    int ret = qcv_ioctl_par (dev_hndl, VIDIOC_S_EXT_CTRLS, & v4l2_ctls);
    if (ret < 0)
      loge ("rds_ps_set error ps: %s  ret: %d  errno: %d (%s)", ps, ret, errno, strerror (errno));
    else
      logd ("rds_ps_set success ps: %s  ret: %d", ps, ret);
    return (0);
  }
  int rds_rt_set (const char * rt) {   // chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_STOP_RDS_TX_RT, 0);
    struct mr_v4l2_ext_control ext_ctl;
    struct mr_v4l2_ext_controls v4l2_ctls;

    ext_ctl.id     = V4L2_CID_RDS_TX_RADIO_TEXT;
    //ext_ctl.string = rds_xlat;
    ext_ctl.string = (char *) rt;

    // Ctrls data struct:
    v4l2_ctls.ctrl_class = V4L2_CTRL_CLASS_FM_TX;
    v4l2_ctls.count      = 1;//15;//
    v4l2_ctls.controls   = & ext_ctl;

    errno = 0;
    int ret = qcv_ioctl_par (dev_hndl, VIDIOC_S_EXT_CTRLS, & v4l2_ctls);
    if (ret < 0)
      loge ("rds_rt_set error rt: %s  ret: %d  errno: %d (%s)", rt, ret, errno, strerror (errno));
    else
      logd ("rds_rt_set success rt: %s  ret: %d", rt, ret);
    return (0);
  }


    /* Private Control range: 1 - 48 decimal or 0x01 - 0x30
        V4L2_CID_PRIVATE_IRIS_SRCHMODE = (0x08000000 + 1),  // = 0
        V4L2_CID_PRIVATE_SINR_SAMPLES,          // 0x8000030
    */

  void qc_test () {
    int argc = 0;
    char ** argv = NULL;

    //int src_mo = 0;
    int sft_mu = 0;
    int sig_th = 0;
    int snr_th = 0;
    int ihi_th = 0;
    int ilo_th = 0;
    //src_mo = chip_ctrl_get (V4L2_CID_PRIVATE_IRIS_SRCHMODE);
    sft_mu = chip_ctrl_get (V4L2_CID_PRIVATE_IRIS_SOFT_MUTE);
    sig_th = chip_ctrl_get (V4L2_CID_PRIVATE_IRIS_SIGNAL_TH);
    snr_th = chip_ctrl_get (V4L2_CID_PRIVATE_SINR_THRESHOLD);
    ihi_th = chip_ctrl_get (V4L2_CID_PRIVATE_INTF_HIGH_THRESHOLD);
    ilo_th = chip_ctrl_get (V4L2_CID_PRIVATE_INTF_LOW_THRESHOLD);
    //D/v4l chip_ctrl_get VIDIOC_G_CTRL OK:         id:                                SOFT_MUTE 30 (0x800001e)  value:   1 (0x01)  0/1     BAD: 2
    //D/v4l chip_ctrl_get VIDIOC_G_CTRL OK:         id:                                SIGNAL_TH  8 (0x8000008)  value:   0 (0x00)  0
    //D/v4l chip_ctrl_get VIDIOC_G_CTRL OK:         id:                           SINR_THRESHOLD 47 (0x800002f)  value:   7 (0x07)  3/7
    //D/v4l chip_ctrl_get VIDIOC_G_CTRL OK:         id:                      INTF_HIGH_THRESHOLD 46 (0x800002e)  value: 115 (0x73)  115
    //D/v4l chip_ctrl_get VIDIOC_G_CTRL OK:         id:                       INTF_LOW_THRESHOLD 45 (0x800002d)  value: 109 (0x6d)  109

    //D/v4l chip_ctrl_set VIDIOC_S_CTRL OK:         id:                                SOFT_MUTE 30 (0x800001e)  value:   1 (0x01)

    if (argc > 1)
      sft_mu = atoi (argv [1]);
    if (argc > 2)
      sig_th = atoi (argv [2]);
    if (argc > 3)
      snr_th = atoi (argv [3]);
    if (argc > 4)
      ihi_th = atoi (argv [4]);
    if (argc > 5)
      ilo_th = atoi (argv [5]);

    logd ("sft_mu: %d", sft_mu);

    //src_mo = 1;
    //if (argc > 1)
    //  chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_SRCHMODE, src_mo);

    if (sft_mu != -1000000 && argc > 1)
//    if (sft_mu != -1000000)
      chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_SOFT_MUTE, sft_mu);


    if (sig_th != -1000000 && argc > 2)
      chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_SIGNAL_TH, sig_th);
    if (snr_th != -1000000 && argc > 3)
      chip_ctrl_set (V4L2_CID_PRIVATE_SINR_THRESHOLD, snr_th);
    if (ihi_th != -1000000 && argc > 4)
      chip_ctrl_set (V4L2_CID_PRIVATE_INTF_HIGH_THRESHOLD, ihi_th);
    if (ilo_th != -1000000 && argc > 5)
      chip_ctrl_set (V4L2_CID_PRIVATE_INTF_LOW_THRESHOLD, ilo_th);

  }

    // From radio-iris-commands.h :

  enum iris_evt_t {
        IRIS_EVT_RADIO_READY,
        IRIS_EVT_TUNE_SUCC,
        IRIS_EVT_SEEK_COMPLETE,
        IRIS_EVT_SCAN_NEXT,
        IRIS_EVT_NEW_RAW_RDS,
        IRIS_EVT_NEW_RT_RDS,
        IRIS_EVT_NEW_PS_RDS,
        IRIS_EVT_ERROR,
        IRIS_EVT_BELOW_TH,
        IRIS_EVT_ABOVE_TH,
        IRIS_EVT_STEREO,
        IRIS_EVT_MONO,
        IRIS_EVT_RDS_AVAIL,
        IRIS_EVT_RDS_NOT_AVAIL,
        IRIS_EVT_NEW_SRCH_LIST,
        IRIS_EVT_NEW_AF_LIST,
        IRIS_EVT_TXRDSDAT,
        IRIS_EVT_TXRDSDONE,
        IRIS_EVT_RADIO_DISABLED,
        IRIS_EVT_NEW_ODA,
        IRIS_EVT_NEW_RT_PLUS,
        IRIS_EVT_NEW_ERT,
  };

    // Chip API:
                                                                        // Polling function called every event_sg_ms milliseconds. Not used remotely but could be in future.
//#define  SUPPORT_RDS
#ifdef  SUPPORT_RDS
  #include "rds_qcv.c"
#else
  int rds_poll (unsigned char * rds_grpd) {return (EVT_GET_NONE);}
#endif

/*
  int event_sg_count = 0;
  void qcv_pwr_hack () {
    event_sg_count ++;
    if (event_sg_count % 600 == 0) { //1800 == 0) {                                   // Every 180 seconds, re set the stereo setting to keep chip audio running (better alternative to blank checking) 
                                                                            // Was OK at 600 seconds (15 minute power timeout ?) but MotoG w/ CM12 was seen to blank after 5 minutes
                                                                        // ?? Is there some power control register to do this ? See /sys/devices/qcom,iris-fm.66/power/
      //logd ("qcv_pwr_hack chip_imp_stereo_sg (curr_stereo): %d", chip_imp_stereo_sg (curr_stereo));
      logd ("qcv_pwr_hack TX_SETPSREPEATCOUNT: %d", chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_TX_SETPSREPEATCOUNT, 15));
    }
  }
*/

  int chip_imp_event_sg (unsigned char * rds_grpd) {
//    qcv_pwr_hack ();
    int ret = 0;
    ret = rds_poll (rds_grpd);
    return (ret);
  }

  int chip_imp_api_mode_sg (int api_mode) {                             // No API modes for QCV like UART/SHIM for BCH (But could in theory go to lower levels than V4L, like HCI or SMD)
    if (api_mode == GET)
      return (curr_api_mode);
    curr_api_mode = api_mode;
    logd ("chip_imp_api_mode_sg curr_api_mode: %d", curr_api_mode);
    return (curr_api_mode);
  }

/* /proc/kallsyms only on XZ1 with radio_iris_transport directly in kernel

> 00000000 t iris_vidioc_g_fmt_type_private
> 00000000 t iris_q_event
> 00000000 t iris_vidioc_s_frequency
> 00000000 t iris_vidioc_g_frequency
> 00000000 t iris_search
> 00000000 t iris_vidioc_g_tuner
> 00000000 t iris_fops_release
> 00000000 t iris_vidioc_dqbuf
> 00000000 t iris_vidioc_queryctrl
> 00000000 t iris_q_evt_data
> 00000000 t iris_vidioc_s_hw_freq_seek
> 00000000 t iris_vidioc_querycap
> 00000000 t iris_vidioc_s_tuner
> 00000000 t iris_vidioc_g_ext_ctrls
> 00000000 t iris_vidioc_g_ctrl
> 00000000 t iris_vidioc_s_ext_ctrls
> 00000000 t iris_vidioc_s_ctrl
> 00000000 t iris_remove
> 00000000 r iris_fm_match
> 00000000 r iris_fops
> 00000000 r iris_ioctl_ops
> 00000000 t iris_radio_init
> 00000000 t iris_probe
> 00000000 t iris_radio_exit
> 00000000 d iris_fm
> 00000000 d iris_driver
> 00000000 d iris_viddev_template
> 00000000 d iris_v4l2_queryctrl

> 00000000 T radio_hci_register_dev
> 00000000 T radio_hci_unregister_dev
> 00000000 t radio_hci_cmd_task
> 00000000 T radio_hci_send_cmd
> 00000000 t radio_hci_req_complete
> 00000000 T radio_hci_event_packet
> 00000000 t radio_hci_rx_task
> 00000000 T radio_hci_recv_frame
> 00000000 t radio_hci_smd_destruct
> 00000000 t radio_hci_smd_send_frame
> 00000000 t radio_hci_smd_recv_event
> 00000000 t radio_hci_smd_notify_cmd

> 00000000 t hci_set_fm_recv_conf
> 00000000 t hci_set_fm_trans_conf
> 00000000 t hci_fm_rds_grps_process
> 00000000 t hci_fm_do_calibration_req
> 00000000 t hci_fm_get_ch_det_th
> 00000000 t hci_get_fm_trans_conf_req
> 00000000 t hci_fm_disable_trans_req
> 00000000 t hci_fm_enable_trans_req
> 00000000 t hci_fm_get_station_dbg_param_req
> 00000000 t hci_fm_get_feature_lists_req
> 00000000 t hci_fm_reset_req
> 00000000 t hci_fm_cancel_search_req
> 00000000 t hci_fm_get_af_list_req
> 00000000 t hci_fm_get_radio_text_req
> 00000000 t hci_fm_get_program_service_req
> 00000000 t hci_fm_get_sig_threshold_req
> 00000000 t hci_fm_get_station_param_req
> 00000000 t hci_get_fm_recv_conf_req
> 00000000 t hci_fm_disable_recv_req
> 00000000 t hci_fm_enable_recv_req
> 00000000 t hci_fm_tune_station_req
> 00000000 t hci_fm_set_cal_req_proc
> 00000000 t hci_fm_tone_generator
> 00000000 t hci_fm_set_event_mask
> 00000000 t hci_fm_set_sig_threshold_req
> 00000000 t hci_fm_rds_grp_process_req
> 00000000 t hci_fm_set_antenna_req
> 00000000 t hci_fm_do_cal_req
> 00000000 t hci_fm_srch_station_list_req
> 00000000 t hci_fm_srch_rds_stations_req
> 00000000 t hci_fm_search_stations_req
> 00000000 t hci_set_fm_trans_conf_req
> 00000000 t hci_set_fm_stereo_mode_req
> 00000000 t hci_fm_rds_grp_mask_req
> 00000000 t hci_set_fm_mute_mode_req
> 00000000 t hci_set_fm_recv_conf_req
> 00000000 t hci_fm_set_ch_det_th
> 00000000 T hci_fm_do_calibration
> 00000000 T hci_fm_smd_register
> 00000000 T hci_fm_smd_deregister

> 00000000 t hci_read_grp_counters_req
> 00000000 t hci_set_notch_filter_req
> 00000000 t hci_def_data_read_req
> 00000000 t hci_def_data_write_req
> 00000000 t hci_trans_rt_req
> 00000000 t hci_trans_ps_req

> 00000000 t update_spur_table
> 00000000 t send_disable_event
> 00000000 d rds_buf
> 00000000 d sig_blend
> 00000000 d ert_carrier
> 00000000 d rt_plus_carrier
*/

  int sys_run (char * cmd) {
    int ret = system (cmd);                                             // !! Binaries like ssd that write to stdout cause C system() to crash !
    logd ("sys_run ret: %d  cmd: \"%s\"", ret, cmd);
    return (ret);
  }

/*
OM8:    sys/devices/qcom,iris-fm.68/power
MOG:    sys/devices/qcom,iris-fm.64/power
XZ1:    sys/devices/qcom,iris-fm.66/power
XZ0/OXL:sys/devices/platform/iris_fm/power

OM8:    sys/devices/fe02c000.sound/INT_FM_TX/power
MOG:    sys/devices/sound-9302.43/INT_FM_TX/power
XZ1:    sys/devices/fe02b000.sound/INT_FM_TX/power
XZ0/OXL:sys/devices/platform/soc-audio.0/INT_FM_TX/power
*/

  int power_control_set (int on) {

logd ("power_control_set() DISABLED; doesn't seem to help");
return (0);

    if (file_get ("/sys/devices/platform/soc-audio.0/INT_FM_TX/power/control")) {       // OXL/XZ0
      if (on)
        sys_run ("echo   on > /sys/devices/platform/soc-audio.0/INT_FM_TX/power/control");
      else
        sys_run ("echo auto > /sys/devices/platform/soc-audio.0/INT_FM_TX/power/control");
    }
    else if (file_get ("/sys/devices/sound-9302.43/INT_FM_TX/power/control")) {         // MOG
      if (on)
        sys_run ("echo   on > /sys/devices/sound-9302.43/INT_FM_TX/power/control");
      else
        sys_run ("echo auto > /sys/devices/sound-9302.43/INT_FM_TX/power/control");
    }
    else if (file_get ("/sys/devices/fe02b000.sound/INT_FM_TX/power/control")) {        // XZ1
      if (on)
        sys_run ("echo   on > /sys/devices/fe02b000.sound/INT_FM_TX/power/control");
      else
        sys_run ("echo auto > /sys/devices/fe02b000.sound/INT_FM_TX/power/control");
    }
    else if (file_get ("/sys/devices/fe02c000.sound/INT_FM_TX/power/control")) {        // OM8
      if (on)
        sys_run ("echo   on > /sys/devices/fe02c000.sound/INT_FM_TX/power/control");
      else
        sys_run ("echo auto > /sys/devices/fe02c000.sound/INT_FM_TX/power/control");
    }
    else {
      loge ("No power control");
    }


    if (file_get ("/sys/devices/platform/iris_fm/power/control")) {                     // OXL/XZ0
      if (on)
        sys_run ("echo   on > /sys/devices/platform/iris_fm/power/control");
      else
        sys_run ("echo auto > /sys/devices/platform/iris_fm/power/control");
    }
    else if (file_get ("/sys/devices/qcom,iris-fm.64/power/control")) {                 // MOG
      if (on)
        sys_run ("echo   on > /sys/devices/qcom,iris-fm.64/power/control");
      else
        sys_run ("echo auto > /sys/devices/qcom,iris-fm.64/power/control");
    }
    else if (file_get ("/sys/devices/qcom,iris-fm.66/power/control")) {                 // XZ1
      if (on)
        sys_run ("echo   on > /sys/devices/qcom,iris-fm.66/power/control");
      else
        sys_run ("echo auto > /sys/devices/qcom,iris-fm.66/power/control");
    }
    else if (file_get ("/sys/devices/qcom,iris-fm.68/power/control")) {                 // OM8
      if (on)
        sys_run ("echo   on > /sys/devices/qcom,iris-fm.68/power/control");
      else
        sys_run ("echo auto > /sys/devices/qcom,iris-fm.68/power/control");
    }
    else {
      loge ("No power control");
    }
    return (0);
  }

  // #define V4L2_CAP_RDS_CAPTURE            0x00000100  /* RDS data capture */
      // #define V4L2_CAP_VIDEO_OUTPUT_OVERLAY   0x00000200  /* Can do video output overlay */
  // #define V4L2_CAP_HW_FREQ_SEEK           0x00000400  /* Can do hardware frequency seek  */
  // #define V4L2_CAP_RDS_OUTPUT             0x00000800  /* Is an RDS encoder */
  
  // #define V4L2_CAP_TUNER                  0x00010000  /* has a tuner */
  // #define V4L2_CAP_AUDIO                  0x00020000  /* has audio support */
  // #define V4L2_CAP_RADIO                  0x00040000  /* is a radio device */
  // #define V4L2_CAP_MODULATOR              0x00080000  /* has a modulator */
  
  // #define V4L2_CAP_READWRITE              0x01000000  /* read/write systemcalls */
  // #define V4L2_CAP_ASYNCIO                0x02000000  /* async I/O */
  // #define V4L2_CAP_STREAMING              0x04000000  /* streaming I/O ioctl's */
  
  // #define V4L2_CAP_DEVICE_CAPS            0x80000000  /* sets device capabilities field */
  
   int chip_version_get () {

    errno = 0;  
    int ret = qcv_ioctl_par (dev_hndl, VIDIOC_QUERYCAP, & v4l_cap);
    if (ret < 0) {
      loge ("chip_get VIDIOC_QUERYCAP error: %d", ret);
      //close (dev_hndl);
      return (-1);
    }
    logd ("chip_get VIDIOC_QUERYCAP ret: %d  cap: 0x%x  drv: %s  card: %s  bus: %s  ver: 0x%x", ret, v4l_cap.capabilities, v4l_cap.driver, v4l_cap.card, v4l_cap.bus_info, v4l_cap.version);
        // chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x00050000  drv: radio-tavarua  card: Qualcomm FM Radio Transceiver  bus: I2C       ver: 0x0
        // chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x00050000  drv: radio-tavarua  card: Qualcomm FM Radio Transceiver  bus: I2C       ver: 0x2010204
        // chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x010d0d00  drv: CG2900 Driver  card: CG2900 FM Radio                bus: platform  ver: 0x10100

        // M8 w/ CM11:
        //v4_get VIDIOC_QUERYCAP ret: 0  cap: 0x50000  drv: radio-iris  card: Qualcomm FM Radio Transceiver  bus:   ver: 0x30400

    if ( ! (v4l_cap.capabilities & V4L2_CAP_TUNER) )
      logd ("chip_get no V4L2_CAP_TUNER !!!!");

      return (v4l_cap.version);
    }
/*all shell "getprop dev ; logcat -d -v time | grep -e chip_get" 
oxl
03-13 03:36:45.185 D/s2tnrqcv(23676): chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x50000  drv: radio-iris  card: Qualcomm FM Radio Transceiver  bus:   ver: 0x0
xz0
03-13 03:36:44.602 D/s2tnrqcv(11948): chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x50000  drv: radio-iris  card: Qualcomm FM Radio Transceiver  bus:   ver: 0x30400
mog
03-13 03:36:45.123 D/s2tnrqcv(11680): chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x50000  drv: radio-iris  card: Qualcomm FM Radio Transceiver  bus:   ver: 0x3042a
xz1
03-13 03:36:44.803 D/s2tnrqcv(14422): chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x50000  drv: radio-iris  card: Qualcomm FM Radio Transceiver  bus:   ver: 0x30400
om8
03-13 03:36:44.736 D/s2tnrqcv(11619): chip_get VIDIOC_QUERYCAP ret: 0  cap: 0x50000  drv: radio-iris  card: Qualcomm FM Radio Transceiver  bus:   ver: 0x30400

qall shell "getprop dev ; getprop | grep fm ; lsmod ; ls -l /system/etc/init.qcom.fm.sh" 
oxl
[hw.fm.init]: [1]
[hw.fm.mode]: [normal]
[hw.fm.version]: [0]
[init.svc.fm_dl]: [stopped]
cat: /proc/modules: No such file or directory
-rw-r--r-- root     root         3140 2015-03-12 01:09 init.qcom.fm.sh
xz0
[hw.fm.init]: [1]
[hw.fm.mode]: [normal]
[hw.fm.version]: [197632]
[init.svc.fm_dl]: [stopped]
radio_iris_transport 3606 0 - Live 0x00000000
wlan 2708287 0 - Live 0x00000000 (C)
-rw-r--r-- root     root         3073 2008-08-01 08:00 init.qcom.fm.sh
mog
[hw.fm.mode]: [normal]
[hw.fm.version]: [197674]
cat: /proc/modules: No such file or directory
/system/etc/init.qcom.fm.sh: No such file or directory
xz1
[hw.fm.init]: [1]
[hw.fm.mode]: [normal]
[hw.fm.version]: [197632]
[init.svc.fm_dl]: [stopped]
-rw-r--r-- root     root         3073 2015-03-12 04:46 init.qcom.fm.sh
om8
[hw.fm.init]: [1]
[hw.fm.mode]: [normal]
[hw.fm.version]: [197632]
[init.svc.fm_dl]: [stopped]
cat: /proc/modules: No such file or directory
/system/etc/init.qcom.fm.sh: No such file or directory
*/  

  int chip_imp_api_state_sg (int state) {
    logd ("chip_imp_api_state_sg state: %d", state);
    if (state == GET)
      return (curr_api_state);

    if (state == 0) {
      power_control_set (0);
      if (dev_hndl >= 0) {
        close (dev_hndl);
      }
      curr_api_state = 0;
      return (curr_api_state);
    }

    errno = 0;
    dev_hndl = open ("/dev/radio0", O_RDWR   | O_NONBLOCK);
    if (dev_hndl < 0) {
      loge ("chip_imp_api_state_sg error opening qualcomm /dev/radio0 errno: %d (%s)", errno, strerror (errno));
      curr_api_state = 0;
      return (curr_api_state);
    }
    logd ("chip_imp_api_state_sg qualcomm /dev/radio0: %d", dev_hndl);

int disable_official_qualcomm = 1;

if (disable_official_qualcomm) {
    // Need 5 - 20 ms delay when module is inserted on Xperia Z1. For a safety factor of 5, delay for 100 ms. This avoid 10s timeout and first start failure, sometimes recoverably
    quiet_ms_sleep (100);   // Need delay even when module already loaded ??
}
else {

    // Copy "official" Qualcomm way to load module, in case it helps with power issue: (It did not)

    int ver = chip_version_get ();
    if (ver < 0) {
      loge ("chip_imp_api_state_sg error chip_version_get: %d  errno: %d (%s)", errno, strerror (errno));
      curr_api_state = 0;
      close (dev_hndl);
      dev_hndl = -1;
      return (curr_api_state);
    }

    //char ver_str [40] = {0};
    //sprintf (ver_str, "%d", ver);
    //property_set ("hw.fm.version", ver_str);
    char cmd_str [DEF_BUF] = {0};
    sprintf (cmd_str, "setprop hw.fm.version  %d", ver);
    sys_run (cmd_str);

    //Set the mode for soc downloader
    //property_set ("hw.fm.mode", "normal");
    //property_set ("ctl.start", "fm_dl");
    sys_run ("setprop hw.fm.mode normal");
    sys_run ("setprop ctl.start fm_dl");

    //sys_run ("setprop hw.fm.mode normal ; setprop hw.fm.version 0 ; start fm_dl",);
    //sys_run ("setprop hw.fm.mode normal ; setprop hw.fm.version 0 ; setprop ctl.start fm_dl");

    //sched_yield();
    quiet_ms_sleep (100);

    char value [4] = "z";//'z';//0;    //char prop_buf [DEF_BUF] = "";
    int retries = 0;
    int init_success = 0;
    for (retries = 0; retries < 20; retries ++) {                       // Wait up to 2 seconds
      __system_property_get ("hw.fm.init", value);
      if (value [0] == '1') {
        init_success = 1;
        break;
      }
      else {
        quiet_ms_sleep (100);
      }
    }
    if (init_success == 1)
      logd ("chip_imp_api_state_sg init success after retries: %d", retries);
    else
      logw ("chip_imp_api_state_sg error %c after retries: %d", value [4], retries);
}

    // If all else fails, do it manually:
    if (file_get ("/system/lib/modules/radio-iris-transport.ko"))
      util_insmod ("/system/lib/modules/radio-iris-transport.ko");

    curr_api_state = 1;

    if (! file_get ("/sys/bus/platform/drivers/iris_fm/") && ! file_get ("/sys/bus/platform/drivers/iris_fm/uevent")) {
      loge ("No /sys/bus/platform/drivers/iris_fm/ or /sys/bus/platform/drivers/iris_fm/uevent");
      curr_api_state = 0;
    }


    if (! file_get ("/sys/module/radio_iris/") && ! file_get ("/sys/module/radio_iris/parameters/sig_blend") &&
        ! file_get ("/sys/module/radio_iris_transport/") && ! file_get ("/sys/module/radio_iris_transport/uevent")) {   // Xperia Z w/ CM11 only has these
      loge ("No /sys/module/radio_iris/ or /sys/module/radio_iris/parameters/sig_blend");
      curr_api_state = 0;
    }

    power_control_set (1);

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
    int new_state = 1;                                                    // Rx
    if (curr_mode)                                                        // Else if Transmit mode...
      new_state = 2;                                                      // Tx

    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_STATE, new_state) < 0)              // 0 = FM_OFF, 1 = FM_RECV, 2 = FM_TRANS, 3 = FM_RESET
      loge ("chip_imp_state_sg PRIVATE_IRIS_STATE 1 error for: %d", new_state);
    else
      logd ("chip_imp_state_sg PRIVATE_IRIS_STATE 1 success for: %d", new_state);

    chip_ctrl_set (V4L2_CID_TUNE_POWER_LEVEL, 7);                       // Maximum power for transmit

    v4l_tuner.index = 0;                                                // Tuner index = 0
    v4l_tuner.type = V4L2_TUNER_RADIO;
    memset (v4l_tuner.reserved, 0, sizeof (v4l_tuner.reserved));

    errno = 0;
    ret = qcv_ioctl_par (dev_hndl, VIDIOC_G_TUNER, & v4l_tuner);
    if (ret < 0) {
      loge ("chip_imp_state_sg VIDIOC_G_TUNER errno: %d (%s)", errno, strerror (errno));
      if (errno == EINVAL) {
        curr_state = 0;
        loge ("chip_imp_state_sg EINVAL curr_state: %d", curr_state);
        return (curr_state);
      }
    }
    else {
      logd ("chip_imp_state_sg VIDIOC_G_TUNER success name: %s  type: %d  cap: 0x%x  lo: %d  hi: %d  sc: %d  am: %d  sig: %d  afc: %d", v4l_tuner.name,
          v4l_tuner.type, v4l_tuner.capability, v4l_tuner.rangelow , v4l_tuner.rangehigh, v4l_tuner.rxsubchans, v4l_tuner.audmode, v4l_tuner.signal, v4l_tuner.afc);
    }

    chip_imp_mute_sg (1);                                               // Mute for now

    ret = chip_ctrl_get (V4L2_CID_PRIVATE_IRIS_ANTENNA);                // Log current setting

    curr_antenna = qcv_need_internal_antenna_get ();

    ret = chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_ANTENNA, curr_antenna);  // 0 = External/headset antenna for OneS/XL/Evo 4G/All others, 1 = internal for Xperia T - Z1
    logd ("chip_imp_state_sg PRIVATE_IRIS_ANTENNA ret: %d  curr_antenna: %d", ret, curr_antenna);

    if (curr_rds_state) {
      if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_RDSON, 1) < 0)           // 0 = OFF, 1 = ON
        loge ("chip_imp_state_sg PRIVATE_IRIS_RDSON 1 error");
      else
        logd ("chip_imp_state_sg PRIVATE_IRIS_RDSON 1 success");

      if (curr_mode) {                                                  // If Transmit
        rds_pi_set (34357);                                             // WSTX
        rds_pt_set (9);                                                 // Varied/Top 40
        rds_ps_set ("SpiritTx");
        rds_rt_set ("rt34567890123456rt34567890123456rt34567890123456rt34567890123456");
      }
      else {                                                            // Else if receive...
        chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_RDSGROUP_PROC, -1);        // 3 Sets:
        chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_PSALL, -1);
        chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_RDSD_BUF, 128);
            // Test ??
        chip_ctrl_get (V4L2_CID_PRIVATE_IRIS_RDSGROUP_PROC);            // 3 Gets to log:   // 0x08000010 = 56 = 0x38 = 0x07 << RDS_CONFIG_OFFSET (3)
        chip_ctrl_get (V4L2_CID_PRIVATE_IRIS_PSALL);                                        // 0x8000014 = 56, ? Bug, copied RDSGROUP_PROC instead of boolean "pass all ps strings"
        chip_ctrl_get (V4L2_CID_PRIVATE_IRIS_RDSD_BUF);
      }
    }

    //chip_imp_band_sg (0);

    chip_imp_vol_sg (65535);

    curr_state = 1;
    logd ("chip_imp_state_sg curr_state: %d", curr_state);
    return (curr_state);
  }

  int chip_imp_antenna_sg (int antenna) {
    if (antenna == GET)
      return (curr_antenna);

    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_ANTENNA, antenna) < 0)     // 0 = common external, 1 = Sony Z/Z1 internal
      loge ("chip_imp_antenna_sg ANTENNA error");
    else
      logd ("chip_imp_antenna_sg ANTENNA success");

    curr_antenna = antenna;
    logd ("chip_imp_antenna_sg curr_antenna: %d", curr_antenna);
    return (curr_antenna);
  }

  int chip_imp_band_sg (int band) {                                     //  0:EU    1:US    2:UU
    if (band == GET)
      return (curr_band);

    logd ("chip_imp_band_sg band: %d", band);

    curr_band = band;

    curr_freq_hi = 108000;

    curr_freq_lo = 87500;
    if (band == 2)                                                      // If Wide
      curr_freq_lo = 76000;//65000;

    curr_freq_inc = 100;
    if (band == 1)                                                      // If US
      curr_freq_inc = 200;

    band_set (curr_freq_lo, curr_freq_hi, band);

    freq_inc_set (curr_freq_inc);

    emph75_set (band);

    rbds_set (band);

    return (band);
  }

  int chip_imp_freq_sg (int freq) {        // 10 KHz resolution    76 MHz = 7600, 108 MHz = 10800
    if (freq == GET)
      return (freq_get ());

    logd ("chip_imp_freq_sg: %d", freq);
    int ret = 0;
    v4l_freq.tuner = 0;                                                   // Tuner index = 0
    v4l_freq.type = V4L2_TUNER_RADIO;
    v4l_freq.frequency = freq * 16;                                     // in units of 62.5 Hz ALWAYS for FM tuner. 62.5 KHz is too coarse for FM -> 250 KHz, Need 50 KHz
    memset (v4l_freq.reserved, 0, sizeof (v4l_freq.reserved));

    errno = 0;
    ret = qcv_ioctl_par (dev_hndl, VIDIOC_S_FREQUENCY, & v4l_freq);
    if (ret < 0) {
      loge ("chip_imp_freq_sg VIDIOC_S_FREQUENCY errno: %d (%s)  freq: %d  v4l_freq.frequency: %d", errno, strerror (errno), freq, v4l_freq.frequency);
      return (curr_freq_int);
    }
    curr_freq_int = freq;
    logd ("chip_imp_freq_sg VIDIOC_S_FREQUENCY success");
    rds_init ();
    return (curr_freq_int);
  }

    //#include "alsa.c"
  int chip_imp_vol_sg (int vol) {
    if (vol == GET)
      return (curr_vol);

    //utils_qcv_ana_vol_set (vol);

    //char cmd [256] = "ssd 4 2 \"Internal FM RX Volume\" ";
    //strlcat (cmd, itoa (vol / 8), sizeof (cmd));             // 0-65535 -> 8191 (max 8K or more seems dangerous ? Can go to 2,000,000 before volume stops rising)
    //ret = internal_sh_run (cmd);
    //logd ("Internal FM RX Volume set: %d %s", ret, cmd);

    curr_vol = vol;
    logd ("chip_imp_vol_sg curr_vol: %d", curr_vol);
    return (curr_vol);
  }

    //../fm/qcom/fmradio/FmReceiver.java:   private static final int FM_RX_RSSI_LEVEL_VERY_WEAK   = -105;
    //../fm/qcom/fmradio/FmReceiver.java:   private static final int FM_RX_RSSI_LEVEL_WEAK        = -100;
    //../fm/qcom/fmradio/FmReceiver.java:   private static final int FM_RX_RSSI_LEVEL_STRONG      = -96;
    //../fm/qcom/fmradio/FmReceiver.java:   private static final int FM_RX_RSSI_LEVEL_VERY_STRONG = -90;
  int chip_imp_thresh_sg (int thresh) {
    if (thresh == GET)
      return (curr_thresh);

    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_SIGNAL_TH, thresh - 105) < 0)
      loge ("chip_imp_mute_sg SIGNAL_TH error");
    else
      logd ("chip_imp_mute_sg SIGNAL_TH success");

    curr_thresh = thresh;
    logd ("chip_imp_thresh_sg curr_thresh: %d", curr_thresh);
    return (curr_thresh);
  }

  int chip_imp_mute_sg (int mute) {
    if (mute == GET)
      return (curr_mute);

    if (chip_ctrl_set (V4L2_CID_AUDIO_MUTE, mute) < 0)                  // 0 = unmute, 1 = mute
      loge ("chip_imp_mute_sg MUTE error");
    else
      logd ("chip_imp_mute_sg MUTE success");

    curr_mute = mute;
    logd ("chip_imp_mute_sg curr_mute: %d", curr_mute);
    return (curr_mute);
  }

  int chip_imp_softmute_sg (int softmute) {
    if (softmute == GET)
      return (curr_softmute);

    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_SOFT_MUTE, softmute) < 0)  // 0 = softmute disable, 1 = default softmute enable
      loge ("chip_imp_softmute_sg SOFT_MUTE error");
    else
      logd ("chip_imp_softmute_sg SOFT_MUTE success");

    curr_softmute = softmute;
    logd ("chip_imp_softmute_sg curr_softmute: %d", curr_softmute);
    return (curr_softmute);
  }

  int chip_imp_stereo_sg (int stereo) {                                 //
    if (stereo == GET)
      return (curr_stereo);

    int ret = 0;
    v4l_tuner.index = 0;                                                // Tuner index = 0
    v4l_tuner.type = V4L2_TUNER_RADIO;
    v4l_tuner.rangelow  = curr_freq_lo * TUNE_MULT;
    v4l_tuner.rangehigh = curr_freq_hi * TUNE_MULT;
    memset (v4l_tuner.reserved, 0, sizeof (v4l_tuner.reserved));
    if (stereo)
      v4l_tuner.audmode = V4L2_TUNER_MODE_STEREO;
    else
      v4l_tuner.audmode = V4L2_TUNER_MODE_MONO;
    errno = 0;
    ret = qcv_ioctl_par (dev_hndl, VIDIOC_S_TUNER, & v4l_tuner);
    if (ret < 0)
      loge ("chip_imp_stereo_sg VIDIOC_S_TUNER errno: %d (%s)", errno, strerror (errno));
    else
      logd ("chip_imp_stereo_sg VIDIOC_S_TUNER success");

    curr_stereo = stereo;
    logd ("chip_imp_stereo_sg curr_stereo: %d", curr_stereo);
    return (curr_stereo);
  }

/* For SRCHMODE:
  enum search_t {
	SEEK,
	SCAN,
	SCAN_FOR_STRONG,
	SCAN_FOR_WEAK,
	RDS_SEEK_PTY,
	RDS_SCAN_PTY,
	RDS_SEEK_PI,
	RDS_AF_JUMP,
  };
*/
  int chip_imp_seek_state_sg (int seek_state) {
    if (seek_state == GET)
      return (curr_seek_state);

    if (seek_state == 0)
      return (seek_stop ());

    int ret = 0;
    logd ("chip_imp_seek_state_sg seek_state: %d", seek_state);
    //seek_stop ();
    int seek_up = 1;
    if (seek_state == 2)
      seek_up = 0;

    int last_freq = freq_get ();                                        // Save original frequency as last_freq
    v4l_seek.tuner = 0;                                                 // Tuner index = 0
    v4l_seek.type = V4L2_TUNER_RADIO;
    memset (v4l_seek.reserved, 0, sizeof (v4l_seek.reserved));
    v4l_seek.wrap_around = 1;                                           // ? yes
    v4l_seek.seek_upward = seek_up;                                     // ? always down ? If non-zero, seek upward from the current frequency, else seek downward.
    v4l_seek.spacing = 0;//curr_freq_inc * 1000;                        // ? 0 ok

    ret = qcv_ioctl_par (dev_hndl, VIDIOC_S_HW_FREQ_SEEK, & v4l_seek);  // Start the seek
    if (ret < 0)
      loge ("chip_imp_seek_state_sg VIDIOC_S_HW_FREQ_SEEK error: %d", ret);
    else
      logd ("chip_imp_seek_state_sg VIDIOC_S_HW_FREQ_SEEK success");

    ms_sleep (303);                                                     // Wait a bit to ensure frequency should have changed  (100 ms OK normally, 500 for change end ?)

    int ctr = 0, new_freq = 0;
    for (ctr = 0; ctr < 50 && new_freq != last_freq; ctr ++) {          // For 50 tries / 5 seconds max...
      if (new_freq >= 50000 && new_freq <= 150000)                      // If new_freq seems valid...
        last_freq = new_freq;                                           // Set new last_freq
      ms_sleep (101);
      new_freq = freq_get ();                                           // Get new_freq
      logd ("chip_imp_seek_state_sg ctr: %d  last_freq: %d  new_freq: %d", ctr, last_freq, new_freq);
    }

    curr_freq_int = new_freq;                                           // Save new frequency

    curr_seek_state = 0;
    rds_init ();
    return (curr_freq_int);                                             // More useful to return newly seeked frequency than curr_seek_state which has changed to 0 / Stop now anyway
  }

  int chip_imp_rds_state_sg (int rds_state) {
    if (rds_state == GET)
      return (curr_rds_state);

    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_RDSON, rds_state) < 0)     // 0 = OFF, 1 = ON
      loge ("chip_imp_rds_state_sg PRIVATE_IRIS_RDSON error");
    else
      logd ("chip_imp_rds_state_sg PRIVATE_IRIS_RDSON success");

    curr_rds_state = rds_state;
    logd ("chip_imp_rds_state_sg curr_rds_state: %d", curr_rds_state);
    return (curr_rds_state);
  }

  int chip_imp_rds_af_state_sg (int rds_af_state) {
    if (rds_af_state == GET)
      return (curr_rds_af_state);

    if (chip_ctrl_set (V4L2_CID_PRIVATE_IRIS_AF_JUMP, rds_af_state) < 0)// 0 = OFF, 1 = ON
      loge ("chip_imp_rds_af_state_sg PRIVATE_IRIS_AF_JUMP error");
    else
      logd ("chip_imp_rds_af_state_sg PRIVATE_IRIS_AF_JUMP success");

    curr_rds_af_state = rds_af_state;
    logd ("chip_imp_rds_af_state_sg curr_rds_af_state: %d", curr_rds_af_state);
    return (curr_rds_af_state);
  }

  int chip_imp_rssi_sg (int fake_rssi) {
    int rssi = -7;
    v4l_tuner.index = 0;                                                // Tuner index = 0
    v4l_tuner.type = V4L2_TUNER_RADIO;
    memset (v4l_tuner.reserved, 0, sizeof (v4l_tuner.reserved));

    errno = 0;
    int ret = qcv_ioctl_par (dev_hndl, VIDIOC_G_TUNER, & v4l_tuner);
    if (ret < 0) {                                                      // If error...
      static int times = 0;
      if (times ++ % 1 == 0)
        loge ("chip_imp_rssi_sg VIDIOC_G_TUNER errno: %d (%s)", errno, strerror (errno));
      if (errno == EINVAL) {
        //chip_imp_api_state_sg (0);
        //chip_imp_api_state_sg (1);
      }
      return (curr_rssi);
    }
    static int timesb = 0;
    int factor = 10;
    if (timesb ++ % factor == 0)
      if (ena_log_tnr_extra)
        logd ("chip_imp_rssi_sg VIDIOC_G_TUNER success name: %s  type: %d  cap: 0x%x  lo: %d  hi: %d  sc: %d  am: %d  sig: %d  afc: %d", v4l_tuner.name,
            v4l_tuner.type, v4l_tuner.capability, v4l_tuner.rangelow , v4l_tuner.rangehigh, v4l_tuner.rxsubchans, v4l_tuner.audmode, v4l_tuner.signal, v4l_tuner.afc);
    rssi = v4l_tuner.signal;                                        // 151 seen before proper power up ?      2014: ?  150 (0x96) = -106 to 205 (0xCD) = -51
    rssi -= 150;
    //rssi *= 3;
    //rssi /= 2;                                        // Multiply by 1.5 to scale similar to other chips / Broadcom (-144)
    curr_rssi = rssi;
    return (curr_rssi);
  }

  int chip_imp_pilot_sg (int fake_pilot) {
    v4l_tuner.index = 0;                                                  // Tuner index = 0
    v4l_tuner.type = V4L2_TUNER_RADIO;
    memset (v4l_tuner.reserved, 0, sizeof (v4l_tuner.reserved));

    errno = 0;
    int ret = qcv_ioctl_par (dev_hndl, VIDIOC_G_TUNER, & v4l_tuner);
    if (ret < 0) {
      loge ("chip_imp_pilot_sg VIDIOC_G_TUNER errno: %d (%s)", errno, strerror (errno));
      if (errno == EINVAL) {
        //chip_imp_api_state_sg (0);
        //chip_imp_api_state_sg (1);
      }
      //if (errno == EINVAL) {
        curr_pilot  = 0;
        return (curr_pilot);
      //}
    }
    else if (v4l_tuner.audmode)
      curr_pilot  = 1;
    else
      curr_pilot  = 0;
    return (curr_pilot);
  }

  int chip_imp_rds_pi_sg (int rds_pi) {
    if (rds_pi == GET)
      return (curr_rds_pi);
    int ret = -1;
    ret = rds_pi_set (rds_pi);
    curr_rds_pi = rds_pi;
    return (curr_rds_pi);
  }
  int chip_imp_rds_pt_sg (int rds_pt) {
    if (rds_pt == GET)
      return (curr_rds_pt);
    int ret = -1;
    ret = rds_pt_set (rds_pt);
    curr_rds_pt = rds_pt;
    return (curr_rds_pt);
  }
  char * chip_imp_rds_ps_sg (char * rds_ps) {
    if (rds_ps == GETP)
      return (curr_rds_ps);
    int ret = -1;
    ret = rds_ps_set (rds_ps);
    strlcpy (curr_rds_ps, rds_ps, sizeof (curr_rds_ps));
    return (curr_rds_ps);
  }
  char * chip_imp_rds_rt_sg (char * rds_rt) {
    if (rds_rt == GETP)
      return (curr_rds_rt);
    int ret = -1;
    ret = rds_rt_set (rds_rt);
    strlcpy (curr_rds_rt, rds_rt, sizeof (curr_rds_rt));
    return (curr_rds_rt);
  }

    // Private Control range: 1 - 48 decimal or 0x01 - 0x30:
    //      V4L2_CID_PRIVATE_IRIS_SRCHMODE      = (0x08000000 + 1),
    //      V4L2_CID_PRIVATE_SINR_SAMPLES,      // 0x08000030


    // See gui_gui: // Codes 200 000 - 2xx xxx - 299 999 = get/set private Broadcom/Qualcomm control

  char * chip_imp_extension_sg (char * reg) {
    if (reg == GETP)
      return (curr_extension);

    int ret = -1;
    strlcpy (curr_extension, reg, sizeof (curr_extension));

    int full_val = atoi (reg);                                          // full_val = -2^31 ... +2^31 - 1       = Over 9 digits
    int ctrl = 0;
    if (full_val >= 300000)                                             // If set
      ctrl = (full_val / 1000) - 300;                                   // ctrl = hi 3 digits - 400     (control to write to)
    else
      ctrl = (full_val / 1000) - 200;                                   // ctrl = hi 3 digits - 200     (control to write to)

    int val  = (full_val % 1000);                                       // val  = lo 3 digits           (value to write)

    logd ("chip_imp_extension_sg reg: %s  full_val: %d  ctrl: %d  val: %d", reg, full_val, ctrl, val);

    if (ctrl >= 0 && ctrl <= 99) {
      ctrl += 0x08000000;
      int ret = chip_ctrl_get (ctrl);
      logd ("chip_imp_extension_sg get ret: %d  ctrl: %x", ret, ctrl);

      if (full_val >= 300000) {                                           // If set
        ret = chip_ctrl_set (ctrl, val);
        logd ("chip_imp_extension_sg set ret: %d  ctrl: %x", ret, ctrl);
      }
    }

    return (curr_extension);
  }

