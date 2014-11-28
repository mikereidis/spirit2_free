
/* See also :

alsa-driver-1.0.24/alsa-kernel/Documentation/ControlNames.txt
alsa-driver-1.0.24/alsa-kernel/core/control.c
alsa-utils-1.0.24.2/amixer/amixer.c
control_hw.c


*/
#ifndef __user
#define __user
#endif
#ifndef __force
#define __force
#endif
#include "asound.h"

/*
    struct snd_ctl_elem_info cei = {0};
    memset (& cei, 0, sizeof (cei));

cei.id.name
cei.id.numid
        struct snd_ctl_elem_value val = {0};
        memset (& val, 0, sizeof (val));

        memcpy (& val.id, & cei.id, sizeof (cei.id));

        ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_READ, & val);
        if (ret < 0)

        if (cei.id.name [0]) {
          //if (! name) logd ("name: \"%48.48s\"  numid: %d  ", cei.id.name, cei.id.numid);
          if (! name)
            logd ("name: %48.48s  numid: %3.3d  ", cei.id.name, cei.id.numid);
          else {
            if (! strcmp (name, cei.id.name))
              index_ret = cei.id.numid;

            if (index_ret >= 0)     // Leave here early and avoid rest
              return (index_ret);
          }
        }
*/

   int alsa_info_id (int fd, char * name, int val_idx) {
    int ret = -1, index_ret = -1, version = 0;

    if (! name)
      logd ("alsa_info_id fd: %d  name: %s  val_idx: %d\n", fd, name, val_idx);
    


    ret = ioctl (fd, SNDRV_CTL_IOCTL_PVERSION, & version);
    if (ret < 0)
      return (ret);
    if (! name)
      logd ("Version: 0x%x\n", version);

    struct snd_ctl_card_info info = {0};
    memset (& info, 0, sizeof (info));
    ret = ioctl (fd, SNDRV_CTL_IOCTL_CARD_INFO, & info);
    if (ret < 0)
      return (ret);
    if (! name) {
      logd ("card card:       %d\n", info.card);
      logd ("card pad:        %d\n", info.pad);
      logd ("card id:         %s\n", info.id);
      logd ("card driver:     %s\n", info.driver);
      logd ("card name:       %s\n", info.name);
      logd ("card longname:   %s\n", info.longname);
      logd ("card reserved_:  %s\n", info.reserved_);
      logd ("card mixername:  %s\n", info.mixername);
      logd ("card components: %s\n", info.components);
      logd ("\n");
    }

    struct snd_ctl_elem_list list = {0};
    memset (&list, 0, sizeof (list));
    //struct snd_ctl_elem_id id = {0};
    //memset (&id, 0, sizeof (id));
    struct snd_ctl_elem_info cei = {0};
    memset (& cei, 0, sizeof (cei));

//Big loop:
    int count = 1, ctr = 0;
    for (ctr = 0; ctr < count; ctr++) {                               // For all controls...
      list.offset = ctr;
      list.space = 1;                                                 // Do 1 each time through loop
      list.pids = & cei.id;//& id;
      ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_LIST, & list);
      if (ret < 0)
        break;

      if (ctr == 0)
        if (! name)
          logd ("ctll count:      %d\n", list.count);               // Count for loop
      if (list.count > 0 && list.count <= 4096) // !! Limit
        count = list.count;

      //if (! name) {
        if (list.offset + 1 != cei.id.numid)
          loge ("!!! ctll offset:     %d\n", list.offset);
        if (list.space != 1)
          loge ("!!! ctll space:      %d\n", list.space);
        if (list.used != 1)
          loge ("!!! ctll used:       %d\n", list.used);
        //logd ("------------------------------------\n");
        //logd ("\n");
      //}

      int cei_count = 1, cei_ctr = 0;
      //for (cei_ctr = 0;cei_ctr < cei_count; cei_ctr ++) {
      for (cei_ctr = 0; cei_ctr < 1; cei_ctr ++) {               // Only 1 loop now
        ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_INFO, & cei);
        if (ret < 0)
          break;

        if (cei_ctr == 0) {
          if (! name)
            logd ("cei  count:      %d\n", cei.count);               // Count for loop = number instances of control (eg 2 for stereo controls)
          if (cei.count > 0 && cei.count <= 4096)   // Identical anyway ! (Element info, but vals ?)        Limit !!
            cei_count = cei.count;
          else
            cei_count = 1;
        }

        //cei.id.index = cei_ctr;

        struct snd_ctl_elem_value val = {0};
        memset (& val, 0, sizeof (val));

        memcpy (& val.id, & cei.id, sizeof (cei.id));

        ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_READ, & val);
        if (ret < 0)
          break;

        if (! name) {
          if (val.indirect)
            logd ("val  indirect:   %d\n", val.indirect);       // Never saw this
          //logd ("val  tstamp tvsec: %ld\n", val.tstamp.tv_sec); // Or this ??
          //logd ("val  tstamp tvnsec:%ld\n", val.tstamp.tv_nsec);// Or this ??
          //if (cei_ctr!=0)
          //  continue;
          //logd ("elid numid:      %d\n", cei.id.numid);
          //if (cei.id.iface != 2)
          //  logd ("elid iface:      %d\n", cei.id.iface);
          if (cei.id.device)
            logd ("elid device:     %d\n", cei.id.device);
          if (cei.id.subdevice)
            logd ("elid subdevice:  %d\n", cei.id.subdevice);   // ?? Uncommon ?
        }

        if (cei.id.name [0]) {
          //if (! name) logd ("name: \"%48.48s\"  numid: %d  ", cei.id.name, cei.id.numid);
          //logd ("name: %48.48s  numid: %3.3d  ", cei.id.name, cei.id.numid);
          if (! name) {
            //logd ("name: %48.48s  numid: %3.3d  ", cei.id.name, cei.id.numid);
          }
          else if (val_idx < 0) {
            if (! strcmp (name, cei.id.name)) {
              index_ret = cei.id.numid;
              //logd ("index_ret: %d", index_ret);
              if (index_ret >= 0)     // Leave here early and avoid rest
                return (index_ret);
            }
          }
          else if (! strcmp (name, cei.id.name)) {
            //if (! strcmp (name, cei.id.name)) {
            long value_ret = -1;
            if (cei.type == SNDRV_CTL_ELEM_TYPE_INTEGER)
              value_ret = val.value.integer.value [val_idx];
            else if (cei.type == SNDRV_CTL_ELEM_TYPE_INTEGER64)
              value_ret = val.value.integer64.value [val_idx];
            else if (cei.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED)
              value_ret = val.value.enumerated.item [val_idx];
            else if (cei.type == SNDRV_CTL_ELEM_TYPE_BOOLEAN)
              value_ret = val.value.integer.value [val_idx];

            //logd ("value_ret: %d", value_ret);
            return (value_ret);
          }
        }

        //if (cei.id.index != cei_ctr)
        //  if (! name) logd ("elid index:      %d\n", cei.id.index);   // Not interesting/useful

/* Always "2 Virtual Mixer" for controlCx:
          switch (cei.id.iface) {
            case SNDRV_CTL_ELEM_IFACE_CARD:
              if (! name) logd ("cei  iface:      %d Global\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_HWDEP:
              if (! name) logd ("cei  iface:      %d Hardware Dependent\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_MIXER:
              if (! name) logd ("cei  iface:      %d Virtual Mixer\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_PCM:
              if (! name) logd ("cei  iface:      %d PCM\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_RAWMIDI:
              if (! name) logd ("cei  iface:      %d Raw MIDI\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_TIMER:
              if (! name) logd ("cei  iface:      %d Timer\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_SEQUENCER:
              if (! name) logd ("cei  iface:      %d Sequencer\n", cei.id.iface);
              break;
            default:
              if (! name) logd ("cei  iface:      %d\n", cei.id.iface);
              break;
          }
*/
        if (! name) {
          logd ("access: ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_READ)
            logd ("rd");
          else 
            logd (",  ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_WRITE) {
            logd (",wr");
          }
          else logd (",  ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_VOLATILE)
            logd (",vo");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TIMESTAMP)
            logd (",ts");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_READ) {
            logd (",tr");
          }
          else logd (",  ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_WRITE)
            logd (",tw");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND)
            logd (",tc");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_INACTIVE)
            logd (",ia");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_LOCK)
            logd (",lo");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_OWNER)
            logd (",ow");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK)
            logd (",tb");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_USER)
            logd (",us");
          logd (" (0x%2.2x)  ", cei.access);

          //logd ("cei  count:      %d\n", cei.count);
          int cd_ctr = 0;
          //logd ("cei  owner:      %d\n", cei.owner);    // Not useful = -1
          if (cei.type == SNDRV_CTL_ELEM_TYPE_INTEGER) {
            logd ("type: Integer (%d)  ", cei.type);
            logd ("min: %ld  ", cei.value.integer.min);
            logd ("max: %2.2ld  ", cei.value.integer.max);
            //logd ("cei  step:       %ld\n", cei.value.integer.step);     // !!?? Never populated (almost ?) Always seems to be ascii
            logd ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              logd (" %ld", val.value.integer.value [cd_ctr]);
            logd ("\n");
            //logd ("val  value_ptr:  %p\n", val.value.integer.value_ptr);
          }
          else if (cei.type == SNDRV_CTL_ELEM_TYPE_INTEGER64) {
            logd ("type: Integer64 (%d)  ", cei.type);
            logd ("min: %lld  ", cei.value.integer64.min);
            logd ("max: %lld  ", cei.value.integer64.max);
            //logd ("cei  step:       %lld\n", cei.value.integer64.step);     // !!?? Never populated (almost ?) Always seems to be ascii
            logd ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              logd (" %lld", val.value.integer64.value [cd_ctr]);
            logd ("\n");
            //logd ("val  value_ptr:  %p\n", val.value.integer64.value_ptr);
          }
          else if (cei.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED) {
            logd ("type: Enumerated (%d)                ", cei.type);

            logd ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              logd (" %d", val.value.enumerated.item [cd_ctr]);

            logd ("  items: %d  ", cei.value.enumerated.items);
            logd ("names:");
            int item_ctr = 0;
            for (item_ctr = 0; item_ctr < cei.value.enumerated.items; item_ctr ++) {
              cei.value.enumerated.item = item_ctr;
              ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_INFO, & cei);
              if (ret < 0)
                break;
              logd (" '%s'", cei.value.enumerated.name);
            }
//            logd ("\n");

//            logd ("Values:");
//            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
//              logd (" %d", val.value.enumerated.item [cd_ctr]);
            logd ("\n");
            //logd ("val  item_ptr:   %p\n", val.value.enumerated.item_ptr);
          }
          else if (cei.type == SNDRV_CTL_ELEM_TYPE_BOOLEAN) {
            logd ("type: Boolean (%d)                   ", cei.type);
            logd ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              logd (" %ld", val.value.integer.value [cd_ctr]);
            logd ("\n");
            //logd ("val  value_ptr:  %p\n", val.value.integer.value_ptr);
          }
          else if (cei.type == SNDRV_CTL_ELEM_TYPE_BYTES) {
            logd ("type: Bytes (%d)  ", cei.type);
            logd ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              logd (" %d", val.value.bytes.data [cd_ctr]);
            logd ("\n");
            //logd ("val  data_ptr:   %p\n", val.value.bytes.data_ptr);
          }
          else if (cei.type == SNDRV_CTL_ELEM_TYPE_IEC958) {
            logd ("type: IEC958 (S/PDIF) (%d)\n", cei.type);
          }
          else {
            logd ("type: %d\n", cei.type);
          }
          if (cei.dimen.d [0] || cei.dimen.d [1] || cei.dimen.d [2] || cei.dimen.d [3]) // ?? Never used ??
            logd ("cei dimen: %d %d %d %d\n", cei.dimen.d [0], cei.dimen.d [1], cei.dimen.d [2], cei.dimen.d [3]);
        }
    //#define USE_TLV
#ifdef  USE_TLV
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_READ) {
            //struct snd_ctl_tlv tlv = {0};

            //struct snd_ctl_tlv tlvi;
            //struct snd_ctl_tlv *tlvp = &tlvi;

            int tlvi[1024] = {0};//alsa-driver-1.0.24/alsa-kernel/Documentation/ControlNames.txt

            struct snd_ctl_tlv *tlvp = (struct snd_ctl_tlv *)tlvi;
            
            memset (tlvp,0,sizeof (*tlvp));
            tlvp->length = 64;
            tlvp->numid = cei.id.numid;
            ret = ioctl (fd, SNDRV_CTL_IOCTL_TLV_READ, tlvp);
            if (ret<0) {
              if (! name) logd ("SNDRV_CTL_IOCTL_TLV_READ ioctl errno: %d\n", errno);
              break;
            }
            int tlv_ctr=0;
            if (! name) logd ("tlv  length:    %d\n", tlvp->length);
            for (tlv_ctr=0;tlv_ctr<tlvp->length/4;tlv_ctr++)
              if (! name) logd (" %x", tlvp->tlv[tlv_ctr]);
            if (! name) logd ("\n");
          }
#endif

        //if (cei_ctr + 1 <cei_count)
          //if (! name) logd ("\n");
      }
    }
    if (! name) logd ("------------------------------------\n");
    //if (! name) logd ("\n");

    if (index_ret >= 0)
      return (index_ret);

    return (ret);
  }

int alsa_cmd (int fd, int type, char * id, long value) {
  int ret = -1, int_id = 1;
  //char * endptr = NULL;

  switch (type) {
    case 0:                                                           // 0 = display, other = set
      ret = alsa_info_id (fd, NULL, -1);                              // Display info
      if (ret < 0)
        loge ("alsa_cmd alsa_info_id error ret: %d  errno: %d  type: %d  id: %s  value: %d", ret, errno, type, id, value);
      break;

    default:
      int_id = alsa_info_id (fd, id, -1);                               // Get integer ID from ALSA control name
      if (int_id > 0) {                                                 // If this is a valid control number...
        ret = alsa_set (fd, int_id, value, type);                       // Write ALSA value
        if (ret < 0)
          loge ("alsa_cmd alsa_set error ret: %d  errno: %d  type: %d  id: %s  value: %d", ret, errno, type, id, value);
#ifdef  CHECK_ALSA
        ret = alsa_info_id (fd, id, 0);                                 // Return value for index 0
        if (ret != value) {
          loge ("Error write: %5.5d  read: %5.5d  id: \'%s\'", value, ret, id);
        }
        //else
        //  logd ("Good  write: %5.5d  read: %5.5d  id: \"%48.48s\"", value, ret, id);
#endif
      }
      else {
        loge ("alsa_cmd alsa_info_id control not found error ret: %d  errno: %d  type: %d  id: %s  value: %d", ret, errno, type, id, value);
      }

      break;
  }
  //if (ret < 0)
  //  loge ("alsa_cmd ioctl error type: %d  ret: %d  errno: %d\n", type, ret, errno);
  //else
  //  logd ("alsa_cmd ioctl success type: %d\n", type);
  return (ret);
}

int alsa_set (int fd, int id, long value, int type) {
  int ret = 0;
  struct snd_ctl_elem_value val = {0};
  memset (& val, 0, sizeof (val));
  val.id.numid = id;

  switch (type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
    case SNDRV_CTL_ELEM_TYPE_INTEGER:
    case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
      //logd ("Integer %ld\n", value);

      val.value.integer.value [0] = value;                               // Set as 4 instances
      val.value.integer.value [1] = value;
      val.value.integer.value [2] = value;
      val.value.integer.value [3] = value;
      break;    
    case SNDRV_CTL_ELEM_TYPE_INTEGER64:
      //logd ("Integer64 %ld\n", value);
      val.value.integer64.value [0] = value;                             // Set as 4 instances
      val.value.integer64.value [1] = value;
      val.value.integer64.value [2] = value;
      val.value.integer64.value [3] = value;
      break;    
    //case SNDRV_CTL_ELEM_TYPE_BYTES:
    //  val.value.bytes.data[0] = value;
    //  break;    
  }
  ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_WRITE, & val);
  if (ret < 0)
    loge ("SNDRV_CTL_IOCTL_ELEM_WRITE ioctl errno: %d  fd: %d  id: %d  value: %d  type: %d", errno, fd, id, value, type);
  //else
  //  logd ("SNDRV_CTL_IOCTL_ELEM_WRITE ioctl OK\n");
  return (0);
}

#define PCM_ERROR_MAX 128

struct pcm {
    int fd;
    unsigned flags;
    int running:1;
    int underruns;
    unsigned buffer_size;
    char error[PCM_ERROR_MAX];
};

unsigned pcm_buffer_size(struct pcm *pcm)
{
    return pcm->buffer_size;
}

const char* pcm_error(struct pcm *pcm)
{
    return pcm->error;
}

static struct pcm bad_pcm = {
    .fd = -1,
};

#define AUDIO_HW_OUT_PERIOD_MULT 8 // (8 * 128 = 1024 frames)
#define AUDIO_HW_OUT_PERIOD_CNT 4

#define PCM_OUT        0x00000000
#define PCM_IN         0x10000000

#define PCM_STEREO     0x00000000
#define PCM_MONO       0x01000000

#define PCM_44100HZ    0x00000000
#define PCM_48000HZ    0x00100000
#define PCM_8000HZ     0x00200000
#define PCM_RATE_MASK  0x00F00000

#define PCM_PERIOD_CNT_MIN 2
#define PCM_PERIOD_CNT_SHIFT 16
#define PCM_PERIOD_CNT_MASK (0xF << PCM_PERIOD_CNT_SHIFT)
#define PCM_PERIOD_SZ_MIN 128
#define PCM_PERIOD_SZ_SHIFT 12
#define PCM_PERIOD_SZ_MASK (0xF << PCM_PERIOD_SZ_SHIFT)

/* Acquire/release a pcm channel.
 * Returns non-zero on error
 */
struct pcm *pcm_open (unsigned flags);
int pcm_close (struct pcm *pcm);
int pcm_ready (struct pcm *pcm);

/* Returns a human readable reason for the last error. */
const char *pcm_error(struct pcm *pcm);


#define PARAM_MAX SNDRV_PCM_HW_PARAM_LAST_INTERVAL

static inline int param_is_mask(int p)
{
    return (p >= SNDRV_PCM_HW_PARAM_FIRST_MASK) &&
        (p <= SNDRV_PCM_HW_PARAM_LAST_MASK);
}

static inline int param_is_interval(int p)
{
    return (p >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL) &&
        (p <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL);
}

static inline struct snd_interval *param_to_interval(struct snd_pcm_hw_params *p, int n)
{
    return &(p->intervals[n - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
}

static inline struct snd_mask *param_to_mask(struct snd_pcm_hw_params *p, int n)
{
    return &(p->masks[n - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
}

static void param_set_mask(struct snd_pcm_hw_params *p, int n, unsigned bit)
{
    if (bit >= SNDRV_MASK_MAX)
        return;
    if (param_is_mask(n)) {
        struct snd_mask *m = param_to_mask(p, n);
        m->bits[0] = 0;
        m->bits[1] = 0;
        m->bits[bit >> 5] |= (1 << (bit & 31));
    }
}

static void param_set_min(struct snd_pcm_hw_params *p, int n, unsigned val)
{
    if (param_is_interval(n)) {
        struct snd_interval *i = param_to_interval(p, n);
        i->min = val;
    }
}

static void param_set_max(struct snd_pcm_hw_params *p, int n, unsigned val)
{
    if (param_is_interval(n)) {
        struct snd_interval *i = param_to_interval(p, n);
        i->max = val;
    }
}

static void param_set_int(struct snd_pcm_hw_params *p, int n, unsigned val)
{
    if (param_is_interval(n)) {
        struct snd_interval *i = param_to_interval(p, n);
        i->min = val;
        i->max = val;
        i->integer = 1;
    }
}

static void param_init(struct snd_pcm_hw_params *p)
{
    int n;
    memset(p, 0, sizeof (*p));
    for (n = SNDRV_PCM_HW_PARAM_FIRST_MASK;
         n <= SNDRV_PCM_HW_PARAM_LAST_MASK; n++) {
            struct snd_mask *m = param_to_mask(p, n);
            m->bits[0] = ~0;
            m->bits[1] = ~0;
    }
    for (n = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL;
         n <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; n++) {
            struct snd_interval *i = param_to_interval(p, n);
            i->min = 0;
            i->max = ~0;
    }
}

int pcm_close (struct pcm *pcm)
{
    if (pcm == &bad_pcm)
        return 0;

    if (pcm->fd >= 0)
        close (pcm->fd);
    pcm->running = 0;
    pcm->buffer_size = 0;
    pcm->fd = -1;
    return 0;
}

  struct pcm * pcm_open (unsigned flags) {
    const char *dname;
    struct pcm *pcm;
    struct snd_pcm_info info;
    struct snd_pcm_hw_params params;
    struct snd_pcm_sw_params sparams;
    unsigned period_sz;
    unsigned period_cnt;

    //logd ("pcm_open\n");
    logd ("pcm_open(0x%08x)\n", flags);

    pcm = calloc(1, sizeof (struct pcm));
    if (!pcm)
        return &bad_pcm;

    if (flags & PCM_IN) {
        dname = "/dev/snd/pcmC0D0c";
    } else {
        dname = "/dev/snd/pcmC0D0p";
    }
logd ("pcm_open %s\n", dname);
    logd("pcm_open() period sz multiplier %d\n", ((flags & PCM_PERIOD_SZ_MASK) >> PCM_PERIOD_SZ_SHIFT) + 1);

    period_sz = 128 * (((flags & PCM_PERIOD_SZ_MASK) >> PCM_PERIOD_SZ_SHIFT) + 1);

    logd("pcm_open() period cnt %d\n", ((flags & PCM_PERIOD_CNT_MASK) >> PCM_PERIOD_CNT_SHIFT) + PCM_PERIOD_CNT_MIN);
         
    period_cnt = ((flags & PCM_PERIOD_CNT_MASK) >> PCM_PERIOD_CNT_SHIFT) + PCM_PERIOD_CNT_MIN;

    pcm->flags = flags;
    pcm->fd = open (dname, O_RDWR);//O_NONBLOCK | O_RDONLY);
    if (pcm->fd < 0) {
        loge ("cannot open device errno: %d\n", errno);
        return pcm;
    }

    if (ioctl (pcm->fd, SNDRV_PCM_IOCTL_INFO, &info)) {
        loge ("cannot get info errno: %d\n", errno);
        goto fail;
    }
    //info_dump(&info);

    logd("pcm_open() period_cnt %d period_sz %d channels %d\n", period_cnt, period_sz, (flags & PCM_MONO) ? 1 : 2);

    param_init(&params);
    param_set_mask(&params, SNDRV_PCM_HW_PARAM_ACCESS,
                   SNDRV_PCM_ACCESS_RW_INTERLEAVED);
    param_set_mask(&params, SNDRV_PCM_HW_PARAM_FORMAT,
                   SNDRV_PCM_FORMAT_S16_LE);
    param_set_mask(&params, SNDRV_PCM_HW_PARAM_SUBFORMAT,
                   SNDRV_PCM_SUBFORMAT_STD);
    param_set_min(&params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, period_sz);
    param_set_int(&params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, 16);
    param_set_int(&params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                  (flags & PCM_MONO) ? 16 : 32);
    param_set_int(&params, SNDRV_PCM_HW_PARAM_CHANNELS,
                  (flags & PCM_MONO) ? 1 : 2);
    param_set_int(&params, SNDRV_PCM_HW_PARAM_PERIODS, period_cnt);
    param_set_int(&params, SNDRV_PCM_HW_PARAM_RATE, 44100);

    if (ioctl (pcm->fd, SNDRV_PCM_IOCTL_HW_PARAMS, &params)) {
        loge ("cannot set hw params errno: %d\n", errno);

        goto fail;
    }
    //param_dump(&params);

    memset(&sparams, 0, sizeof (sparams));
    sparams.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    sparams.period_step = 1;
    sparams.avail_min = 1;
    sparams.start_threshold = period_cnt * period_sz;
    sparams.stop_threshold = period_cnt * period_sz;
    sparams.xfer_align = period_sz / 2;                                 // needed for old kernels
    sparams.silence_size = 0;
    sparams.silence_threshold = 0;

    if (ioctl(pcm->fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sparams)) {
        loge ("cannot set sw params errno: %d\n", errno);
        goto fail;
    }

    logd ("pcm_open OK\n");
    pcm->buffer_size = period_cnt * period_sz;
    pcm->underruns = 0;
    return pcm;

fail:
    loge ("pcm_open fail\n");
    close (pcm->fd);
    pcm->fd = -1;
    return pcm;
}

int pcm_ready(struct pcm *pcm)
{
    return pcm->fd >= 0;
}


// CONVERT to new alsa_info_id or whatever !!!! From alsao.c or alsa.c from Spirit1 UL:

int m4_do_info (int fd, char *name) {
  int ret = -1, index_ret = -1, version = 0;
      if (! name) printf ("m4_do info\n");
	  ret = ioctl (fd, SNDRV_CTL_IOCTL_PVERSION, & version);
      if (ret < 0)
        return (ret);
      if (! name) printf ("Version: 0x%x\n", version);

      struct snd_ctl_card_info info = {0};
      memset (&info, 0, sizeof (info));
      ret = ioctl (fd, SNDRV_CTL_IOCTL_CARD_INFO, & info);
      if (ret < 0)
        return (ret);
      if (! name) printf ("card card:       %d\n", info.card);
      if (! name) printf ("card pad:        %d\n", info.pad);
      if (! name) printf ("card id:         %s\n", info.id);
      if (! name) printf ("card driver:     %s\n", info.driver);
      if (! name) printf ("card name:       %s\n", info.name);
      if (! name) printf ("card longname:   %s\n", info.longname);
      if (! name) printf ("card reserved_:  %s\n", info.reserved_);
      if (! name) printf ("card mixername:  %s\n", info.mixername);
      if (! name) printf ("card components: %s\n", info.components);
      if (! name) printf ("\n");

      struct snd_ctl_elem_list list = {0};
      memset (&list, 0, sizeof (list));
      //struct snd_ctl_elem_id id = {0};
      //memset (&id, 0, sizeof (id));
      struct snd_ctl_elem_info cei = {0};
      memset (& cei, 0, sizeof (cei));

      int count = 1, ctr = 0;
      for (ctr = 0; ctr < count; ctr ++) {                              // For all controls...
        list.offset = ctr;
        list.space = 1;                                                 // Do 1 each time through loop
        list.pids = & cei.id;//& id;
	    ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_LIST, & list);
        if (ret < 0) {
          printf ("m4_do_info SNDRV_CTL_IOCTL_ELEM_LIST ret: %d  errno: %d\n", ret, errno);
          continue;//break;
        }

        if (ctr == 0)
          if (! name) printf ("ctll count:      %d\n", list.count);     // Count for loop
        if (list.count > 0 && list.count <= 4096)                       // Limit !!
          count = list.count;

        if (list.offset + 1 != cei.id.numid)
          if (! name) printf ("!!! ctll offset:     %d\n", list.offset);
        if (list.space != 1)
          if (! name) printf ("!!! ctll space:      %d\n", list.space);
        if (list.used != 1)
          if (! name) printf ("!!! ctll used:       %d\n", list.used);

//        if (! name) printf ("------------------------------------\n");
        //if (! name) printf ("\n");

        int cei_count = 1, cei_ctr = 0;
        //for (cei_ctr = 0;cei_ctr < cei_count; cei_ctr ++) {
        for (cei_ctr = 0; cei_ctr < 1; cei_ctr ++) {               // Only 1 loop now
          ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_INFO, & cei);
          if (ret < 0) {
            printf ("m4_do_info SNDRV_CTL_IOCTL_ELEM_INFO ret: %d  errno: %d\n", ret, errno);
            continue;//break;
          }

          if (cei_ctr == 0) {
            //if (! name) printf ("cei  count:      %d\n", cei.count);               // Count for loop = number instances of control (eg 2 for stereo controls)
            if (cei.count > 0 && cei.count <= 4096)   // Identical anyway ! (Element info, but vals ?)   // !! Limit
              cei_count = cei.count;
            else
              cei_count = 1;
          }

          //cei.id.index = cei_ctr;

          struct snd_ctl_elem_value val = {0};
          memset (& val, 0, sizeof (val));

          memcpy (& val.id, & cei.id, sizeof (cei.id));

          if (! strcmp (cei.id.name, "Get RMS")  /*&& cei.id.numid == 398 && file_get2 ("/system/framework/protobufs-2.3.0.jar")*/) {
            //printf ("m4_do_info SNDRV_CTL_IOCTL_ELEM_READ hack for Stock MotoG 4.4.4 !!!! !!!!   cei.id.name: %s\n", cei.id.name);
            printf ("m4_do_info SNDRV_CTL_IOCTL_ELEM_READ hack for 'Get RMS' !!!! !!!!");//   cei.id.name: %s\n", cei.id.name);
            ret = 11111;
          }
          else
            ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_READ, & val);
          if (ret < 0) {
            printf ("m4_do_info SNDRV_CTL_IOCTL_ELEM_READ ret: %d  errno: %d\n", ret, errno);
            continue;//break;
          }

          if (val.indirect)
            if (! name) printf ("val  indirect:   %d\n", val.indirect);       // Never saw this
          //if (! name) printf ("val  tstamp tvsec: %ld\n", val.tstamp.tv_sec); // Or this ??
          //if (! name) printf ("val  tstamp tvnsec:%ld\n", val.tstamp.tv_nsec);// Or this ??
          //if (cei_ctr!=0)
          //  continue;
          //if (! name) printf ("elid numid:      %d\n", cei.id.numid);
          //if (cei.id.iface != 2)
          //  if (! name) printf ("elid iface:      %d\n", cei.id.iface);
          if (cei.id.device)
            if (! name) printf ("elid device:     %d\n", cei.id.device);
          if (cei.id.subdevice)
            if (! name) printf ("elid subdevice:  %d\n", cei.id.subdevice);
          if (cei.id.name [0]) {
            //if (! name) printf ("name: \"%48.48s\"  numid: %d  ", cei.id.name, cei.id.numid);
            if (! name) printf ("name: %48.48s  numid: %3.3d  ", cei.id.name, cei.id.numid);
            if (name != NULL) {
              if (! strcmp (name, cei.id.name)) {
                index_ret = cei.id.numid;
                return (index_ret); // Shortcut Aug 18, 2014
              }
            }
          }
          //if (cei.id.index != cei_ctr)
          //  if (! name) printf ("elid index:      %d\n", cei.id.index);   // Not interesting/useful

/* Always "2 Virtual Mixer" for controlCx:
          switch (cei.id.iface) {
            case SNDRV_CTL_ELEM_IFACE_CARD:
              if (! name) printf ("cei  iface:      %d Global\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_HWDEP:
              if (! name) printf ("cei  iface:      %d Hardware Dependent\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_MIXER:
              if (! name) printf ("cei  iface:      %d Virtual Mixer\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_PCM:
              if (! name) printf ("cei  iface:      %d PCM\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_RAWMIDI:
              if (! name) printf ("cei  iface:      %d Raw MIDI\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_TIMER:
              if (! name) printf ("cei  iface:      %d Timer\n", cei.id.iface);
              break;
            case SNDRV_CTL_ELEM_IFACE_SEQUENCER:
              if (! name) printf ("cei  iface:      %d Sequencer\n", cei.id.iface);
              break;
            default:
              if (! name) printf ("cei  iface:      %d\n", cei.id.iface);
              break;
          }
*/
          if (! name) printf ("access: ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_READ) {
            if (! name) printf ("rd");
          }
          else if (! name) printf (",  ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_WRITE) {
            if (! name) printf (",wr");
          }
          else if (! name) printf (",  ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_VOLATILE)
            if (! name) printf (",vo");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TIMESTAMP)
            if (! name) printf (",ts");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_READ) {
            if (! name) printf (",tr");
          }
          else if (! name) printf (",  ");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_WRITE)
            if (! name) printf (",tw");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND)
            if (! name) printf (",tc");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_INACTIVE)
            if (! name) printf (",ia");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_LOCK)
            if (! name) printf (",lo");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_OWNER)
            if (! name) printf (",ow");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK)
            if (! name) printf (",tb");
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_USER)
            if (! name) printf (",us");
          if (! name) printf (" (0x%2.2x)  ", cei.access);

          //if (! name) printf ("cei  count:      %d\n", cei.count);
          int cd_ctr = 0;
          //if (! name) printf ("cei  owner:      %d\n", cei.owner);    // Not useful = -1
          if (cei.type==SNDRV_CTL_ELEM_TYPE_INTEGER) {
            if (! name) printf ("type: Integer (%d)  ", cei.type);
            if (! name) printf ("min: %ld  ", cei.value.integer.min);
            if (! name) printf ("max: %2.2ld  ", cei.value.integer.max);
            //if (! name) printf ("cei  step:       %ld\n", cei.value.integer.step);     // !!?? Never populated (almost ?) Always seems to be ascii
            if (! name) printf ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              if (! name) printf (" %ld", val.value.integer.value [cd_ctr]);
            if (! name) printf ("\n");
            //if (! name) printf ("val  value_ptr:  %p\n", val.value.integer.value_ptr);
          }
          else if (cei.type==SNDRV_CTL_ELEM_TYPE_INTEGER64) {
            if (! name) printf ("type: Integer64 (%d)  ", cei.type);
            if (! name) printf ("min: %lld  ", cei.value.integer64.min);
            if (! name) printf ("max: %lld  ", cei.value.integer64.max);
            //if (! name) printf ("cei  step:       %lld\n", cei.value.integer64.step);     // !!?? Never populated (almost ?) Always seems to be ascii
            if (! name) printf ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              if (! name) printf (" %lld", val.value.integer64.value [cd_ctr]);
            if (! name) printf ("\n");
            //if (! name) printf ("val  value_ptr:  %p\n", val.value.integer64.value_ptr);
          }
          else if (cei.type==SNDRV_CTL_ELEM_TYPE_ENUMERATED) {
            if (! name) printf ("type: Enumerated (%d)                ", cei.type);

            if (! name) printf ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              if (! name) printf (" %d", val.value.enumerated.item [cd_ctr]);

            if (! name) printf ("  items: %d  ", cei.value.enumerated.items);
            if (! name) printf ("names:");
            int item_ctr = 0;
            for (item_ctr = 0; item_ctr < cei.value.enumerated.items; item_ctr ++) {
              cei.value.enumerated.item = item_ctr;
              ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_INFO, & cei);
              if (ret < 0) {
                printf ("m4_do_info SNDRV_CTL_IOCTL_ELEM_INFO 2 ret: %d  errno: %d\n", ret, errno);
                continue;//break;
              }
              if (! name) printf (" '%s'", cei.value.enumerated.name);
            }
//            if (! name) printf ("\n");

//            if (! name) printf ("Values:");
//            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
//              if (! name) printf (" %d", val.value.enumerated.item [cd_ctr]);
            if (! name) printf ("\n");
            //if (! name) printf ("val  item_ptr:   %p\n", val.value.enumerated.item_ptr);
          }
          else if (cei.type==SNDRV_CTL_ELEM_TYPE_BOOLEAN) {
            if (! name) printf ("type: Boolean (%d)                   ", cei.type);
            if (! name) printf ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              if (! name) printf (" %ld", val.value.integer.value [cd_ctr]);
            if (! name) printf ("\n");
            //if (! name) printf ("val  value_ptr:  %p\n", val.value.integer.value_ptr);
          }
          else if (cei.type==SNDRV_CTL_ELEM_TYPE_BYTES) {
            if (! name) printf ("type: Bytes (%d)  ", cei.type);
            if (! name) printf ("Values:");
            for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
              if (! name) printf (" %d", val.value.bytes.data [cd_ctr]);
            if (! name) printf ("\n");
            //if (! name) printf ("val  data_ptr:   %p\n", val.value.bytes.data_ptr);
          }
          else if (cei.type==SNDRV_CTL_ELEM_TYPE_IEC958) {
            if (! name) printf ("type: IEC958 (S/PDIF) (%d)\n", cei.type);
          }
          else {
            if (! name) printf ("type: %d\n", cei.type);
          }
          if (cei.dimen.d [0] || cei.dimen.d [1] || cei.dimen.d [2] || cei.dimen.d [3]) // ?? Never used ??
            if (! name) printf ("cei dimen: %d %d %d %d\n", cei.dimen.d [0], cei.dimen.d [1], cei.dimen.d [2], cei.dimen.d [3]);

    //#define USE_TLV
#ifdef  USE_TLV
          if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_READ) {
            //struct snd_ctl_tlv tlv = {0};

            //struct snd_ctl_tlv tlvi;
            //struct snd_ctl_tlv *tlvp = &tlvi;

            int tlvi [1024] = {0};      alsa-driver-1.0.24/alsa-kernel/Documentation/ControlNames.txt

            struct snd_ctl_tlv *tlvp = (struct snd_ctl_tlv *)tlvi;
            
            memset (tlvp,0,sizeof (*tlvp));
            tlvp->length = 64;
            tlvp->numid = cei.id.numid;
            ret = ioctl (fd, SNDRV_CTL_IOCTL_TLV_READ, tlvp);
            if (ret < 0) {
              //if (! name)
              printf ("m4_do_info SNDRV_CTL_IOCTL_TLV_READ ret: %d  errno: %d\n", ret, errno);
              continue;//break;
            }

            int tlv_ctr=0;
            if (! name) printf ("tlv  length:    %d\n", tlvp->length);
            for (tlv_ctr=0;tlv_ctr<tlvp->length/4;tlv_ctr++)
              if (! name) printf (" %x", tlvp->tlv[tlv_ctr]);
            if (! name) printf ("\n");
          }
#endif

          //if (cei_ctr + 1 <cei_count)
            //if (! name) printf ("\n");
        }
      }
      if (! name) printf ("------------------------------------\n");
      //if (! name) printf ("\n");

  if (index_ret >= 0)
    return (index_ret);

  return (ret);
}

int m4_do (int fd, int type, char * id, long value) {
  int ret = -1, int_id = 1;
  char * endptr = NULL;

  switch (type) {
    case 0:                                                             // Display
      ret = m4_do_info (fd, NULL);
      break;

    default:
      endptr = id;
      int_id = strtol (id, & endptr, 0);                                // Get integer ID from string: 0 = base 10, base 16 if starts with "0x", base 8 if starts with "0"

      if (endptr != NULL && * endptr != '\0') {                         // If numeric conversion error... then must be a control name
        int_id = m4_do_info (fd, id);                                   // Get integer ID from ALSA control name
      }
      if (int_id >= 0)                                                  // If this is a valid control number...
        ret = m4_write (fd, int_id, value, type);                       // Write ALSA value
      break;
  }
  if (ret < 0)
    printf ("m4_do ioctl error type: %d  ret: %d  errno: %d\n", type, ret, errno);
  else
    printf ("m4_do ioctl success type: %d\n", type);
  return (ret);
}

int m4_write (int fd, int id, long value, int type) {
  int ret = 0;
  struct snd_ctl_elem_value val = {0};
  memset (& val, 0, sizeof (val));
  val.id.numid = id;

  switch (type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
    case SNDRV_CTL_ELEM_TYPE_INTEGER:
    case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
      printf ("Integer %ld\n", value);
      val.value.integer.value [0] = value;                               // Set as 4 instances
      val.value.integer.value [1] = value;
      val.value.integer.value [2] = value;
      val.value.integer.value [3] = value;
      break;    
    case SNDRV_CTL_ELEM_TYPE_INTEGER64:
      printf ("Integer64 %ld\n", value);
      val.value.integer64.value [0] = value;                             // Set as 4 instances
      val.value.integer64.value [1] = value;
      val.value.integer64.value [2] = value;
      val.value.integer64.value [3] = value;
      break;    
    //case SNDRV_CTL_ELEM_TYPE_BYTES:
    //  val.value.bytes.data[0] = value;
    //  break;    
  }
  ret = ioctl (fd, SNDRV_CTL_IOCTL_ELEM_WRITE, & val);
  if (ret < 0)
    printf ("SNDRV_CTL_IOCTL_ELEM_WRITE ioctl errno: %d\n", errno);
  else
    printf ("SNDRV_CTL_IOCTL_ELEM_WRITE ioctl OK\n");
  return (0);
}

