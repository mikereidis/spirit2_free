
    // ALSA sound device code

    // See also:// control_hw.c
                // alsa-driver-1.0.24/alsa-kernel/Documentation/ControlNames.txt
                // alsa-driver-1.0.24/alsa-kernel/core/control.c
                // alsa-utils-1.0.24.2/amixer/amixer.c
    
  #include "alsa.h"

  int pcm_control_fd = -1;                                              // File descriptor   for /dev/snd/controlC0
  int pcm_device1_fd = -1;                                              // File descriptor 1 for /dev/snd/pcmC0D0c, /dev/snd/pcmC0D0p etc 
  int pcm_device2_fd = -1;                                              // File descriptor 2 for /dev/snd/pcmC0D0c, /dev/snd/pcmC0D0p etc 

    // alsa_control_id_get can be used to:
        // Get the integer ID of the named control and log the search
        // Log all controls (ena_log_alsa_verbo = true) by passing an unused control name (w/ basic info) or NULL (with extra info)

    // Could cache control IDs if speed becomes needed

  int alsa_control_id_get (char * name) {
    if (pcm_control_open () < 0)
      return (-1);

    int ret = -1;
    int index_ret = -1;
    int alsa_version = 0;

    if (! name) if (ena_log_alsa_verbo) logd ("alsa_control_id_get: name: %s", name);
	  ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_PVERSION, & alsa_version);
    if (ret < 0)
      return (ret);
    if (! name) if (ena_log_alsa_verbo) logd ("alsa_control_id_get: alsa_version: 0x%x", alsa_version);

    struct snd_ctl_card_info info = {0};
    memset (&info, 0, sizeof (info));
    ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_CARD_INFO, & info);
    if (ret < 0)
      return (ret);
    if (! name) if (ena_log_alsa_verbo) logd ("card card:       %d", info.card);
    if (! name) if (ena_log_alsa_verbo) logd ("card pad:        %d", info.pad);
    if (! name) if (ena_log_alsa_verbo) logd ("card id:         %s", info.id);
    if (! name) if (ena_log_alsa_verbo) logd ("card driver:     %s", info.driver);
    if (! name) if (ena_log_alsa_verbo) logd ("card name:       %s", info.name);
    if (! name) if (ena_log_alsa_verbo) logd ("card longname:   %s", info.longname);
    if (! name) if (ena_log_alsa_verbo) logd ("card reserved_:  %s", info.reserved_);
    if (! name) if (ena_log_alsa_verbo) logd ("card mixername:  %s", info.mixername);
    if (! name) if (ena_log_alsa_verbo) logd ("card components: %s", info.components);
    if (! name) if (ena_log_alsa_verbo) logd ("");

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
	    ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_ELEM_LIST, & list);
      if (ret < 0) {
        if (ena_log_alsa_verbo) logd ("alsa_control_id_get SNDRV_CTL_IOCTL_ELEM_LIST ret: %d  errno: %d", ret, errno);
        continue;//break;
      }

      if (ctr == 0)
        if (! name) if (ena_log_alsa_verbo) logd ("ctll count:      %d", list.count);     // Count for loop
      if (list.count > 0 && list.count <= 4096)                       // Limit !!
        count = list.count;

      if (list.offset + 1 != cei.id.numid)
        if (! name) if (ena_log_alsa_verbo) logd ("!!! ctll offset:     %d", list.offset);
      if (list.space != 1)
        if (! name) if (ena_log_alsa_verbo) logd ("!!! ctll space:      %d", list.space);
      if (list.used != 1)
        if (! name) if (ena_log_alsa_verbo) logd ("!!! ctll used:       %d", list.used);

      //if (! name) if (ena_log_alsa_verbo) logd ("------------------------------------");
      //if (! name) if (ena_log_alsa_verbo) logd ("");

      int cei_count = 1, cei_ctr = 0;
      //for (cei_ctr = 0;cei_ctr < cei_count; cei_ctr ++) {
      for (cei_ctr = 0; cei_ctr < 1; cei_ctr ++) {               // Only 1 loop now
        ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_ELEM_INFO, & cei);
        if (ret < 0) {
          if (ena_log_alsa_verbo) logd ("alsa_control_id_get SNDRV_CTL_IOCTL_ELEM_INFO ret: %d  errno: %d", ret, errno);
          continue;//break;
        }

        if (cei_ctr == 0) {
          //if (! name) if (ena_log_alsa_verbo) logd ("cei  count:      %d", cei.count);               // Count for loop = number instances of control (eg 2 for stereo controls)
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
          //if (ena_log_alsa_verbo) logd ("alsa_control_id_get SNDRV_CTL_IOCTL_ELEM_READ hack for Stock MotoG 4.4.4 !!!! !!!!   cei.id.name: %s", cei.id.name);
          if (ena_log_alsa_verbo) logd ("alsa_control_id_get SNDRV_CTL_IOCTL_ELEM_READ hack for 'Get RMS' !!!! !!!!");//   cei.id.name: %s", cei.id.name);
          ret = 11111;
        }
        else
          ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_ELEM_READ, & val);
        if (ret < 0) {
          if (ena_log_alsa_verbo) loge ("alsa_control_id_get SNDRV_CTL_IOCTL_ELEM_READ ret: %d  errno: %d  ctr: %d", ret, errno, ctr);  // !! These errors are common; many ALSA devices have unused control IDs
          continue;//break;
        }

        if (val.indirect)
          if (! name) if (ena_log_alsa_verbo) logd ("val  indirect:   %d", val.indirect);       // Never saw this
        //if (! name) if (ena_log_alsa_verbo) logd ("val  tstamp tvsec: %ld", val.tstamp.tv_sec); // Or this ??
        //if (! name) if (ena_log_alsa_verbo) logd ("val  tstamp tvnsec:%ld", val.tstamp.tv_nsec);// Or this ??
        //if (cei_ctr!=0)
        //  continue;
        //if (! name) if (ena_log_alsa_verbo) logd ("elid numid:      %d", cei.id.numid);
        //if (cei.id.iface != 2)
        //  if (! name) if (ena_log_alsa_verbo) logd ("elid iface:      %d", cei.id.iface);
        if (cei.id.device)
          if (! name) if (ena_log_alsa_verbo) logd ("elid device:     %d", cei.id.device);
        if (cei.id.subdevice)
          if (! name) if (ena_log_alsa_verbo) logd ("elid subdevice:  %d", cei.id.subdevice);
        if (cei.id.name [0]) {
          //if (! name) if (ena_log_alsa_verbo) logd ("name: \"%s\"  numid: %d  ", cei.id.name, cei.id.numid);
          if (! name) if (ena_log_alsa_verbo) logd ("name: %s  numid: %d  ", cei.id.name, cei.id.numid);
          if (name != NULL) {
            if (! strcmp (name, cei.id.name)) {
              index_ret = cei.id.numid;
              return (index_ret); // Shortcut Aug 18, 2014
            }
          }
        }
        //if (cei.id.index != cei_ctr)
        //  if (! name) if (ena_log_alsa_verbo) logd ("elid index:      %d", cei.id.index);   // Not interesting/useful

  /* Always "2 Virtual Mixer" for controlCx:
        switch (cei.id.iface) {
          case SNDRV_CTL_ELEM_IFACE_CARD:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d Global", cei.id.iface);
            break;
          case SNDRV_CTL_ELEM_IFACE_HWDEP:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d Hardware Dependent", cei.id.iface);
            break;
          case SNDRV_CTL_ELEM_IFACE_MIXER:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d Virtual Mixer", cei.id.iface);
            break;
          case SNDRV_CTL_ELEM_IFACE_PCM:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d PCM", cei.id.iface);
            break;
          case SNDRV_CTL_ELEM_IFACE_RAWMIDI:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d Raw MIDI", cei.id.iface);
            break;
          case SNDRV_CTL_ELEM_IFACE_TIMER:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d Timer", cei.id.iface);
            break;
          case SNDRV_CTL_ELEM_IFACE_SEQUENCER:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d Sequencer", cei.id.iface);
            break;
          default:
            if (! name) if (ena_log_alsa_verbo) logd ("cei  iface:      %d", cei.id.iface);
            break;
        }
  */
        if (! name) if (ena_log_alsa_verbo) logd ("access: ");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_READ) {
          if (! name) if (ena_log_alsa_verbo) logd ("rd");
        }
        else if (! name) if (ena_log_alsa_verbo) logd (",  ");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_WRITE) {
          if (! name) if (ena_log_alsa_verbo) logd (",wr");
        }
        else if (! name) if (ena_log_alsa_verbo) logd (",  ");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_VOLATILE)
          if (! name) if (ena_log_alsa_verbo) logd (",vo");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_TIMESTAMP)
          if (! name) if (ena_log_alsa_verbo) logd (",ts");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_READ) {
          if (! name) if (ena_log_alsa_verbo) logd (",tr");
        }
        else if (! name) if (ena_log_alsa_verbo) logd (",  ");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_WRITE)
          if (! name) if (ena_log_alsa_verbo) logd (",tw");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND)
          if (! name) if (ena_log_alsa_verbo) logd (",tc");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_INACTIVE)
          if (! name) if (ena_log_alsa_verbo) logd (",ia");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_LOCK)
          if (! name) if (ena_log_alsa_verbo) logd (",lo");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_OWNER)
          if (! name) if (ena_log_alsa_verbo) logd (",ow");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK)
          if (! name) if (ena_log_alsa_verbo) logd (",tb");
        if (cei.access & SNDRV_CTL_ELEM_ACCESS_USER)
          if (! name) if (ena_log_alsa_verbo) logd (",us");
        if (! name) if (ena_log_alsa_verbo) logd (" (0x%x)  ", cei.access);

        //if (! name) if (ena_log_alsa_verbo) logd ("cei  count:      %d", cei.count);
        int cd_ctr = 0;
        //if (! name) if (ena_log_alsa_verbo) logd ("cei  owner:      %d", cei.owner);    // Not useful = -1
        if (cei.type==SNDRV_CTL_ELEM_TYPE_INTEGER) {
          if (! name) if (ena_log_alsa_verbo) logd ("type: Integer (%d)  ", cei.type);
          if (! name) if (ena_log_alsa_verbo) logd ("min: %ld  ", cei.value.integer.min);
          if (! name) if (ena_log_alsa_verbo) logd ("max: %ld  ", cei.value.integer.max);
          //if (! name) if (ena_log_alsa_verbo) logd ("cei  step:       %ld", cei.value.integer.step);     // !!?? Never populated (almost ?) Always seems to be ascii
          if (! name) if (ena_log_alsa_verbo) logd ("Values:");
          for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
            if (! name) if (ena_log_alsa_verbo) logd (" %ld", val.value.integer.value [cd_ctr]);
          if (! name) if (ena_log_alsa_verbo) logd ("");
          //if (! name) if (ena_log_alsa_verbo) logd ("val  value_ptr:  %p", val.value.integer.value_ptr);
        }
        else if (cei.type==SNDRV_CTL_ELEM_TYPE_INTEGER64) {
          if (! name) if (ena_log_alsa_verbo) logd ("type: Integer64 (%d)  ", cei.type);
          if (! name) if (ena_log_alsa_verbo) logd ("min: %lld  ", cei.value.integer64.min);
          if (! name) if (ena_log_alsa_verbo) logd ("max: %lld  ", cei.value.integer64.max);
          //if (! name) if (ena_log_alsa_verbo) logd ("cei  step:       %lld", cei.value.integer64.step);     // !!?? Never populated (almost ?) Always seems to be ascii
          if (! name) if (ena_log_alsa_verbo) logd ("Values:");
          for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
            if (! name) if (ena_log_alsa_verbo) logd (" %lld", val.value.integer64.value [cd_ctr]);
          if (! name) if (ena_log_alsa_verbo) logd ("");
          //if (! name) if (ena_log_alsa_verbo) logd ("val  value_ptr:  %p", val.value.integer64.value_ptr);
        }
        else if (cei.type==SNDRV_CTL_ELEM_TYPE_ENUMERATED) {
          if (! name) if (ena_log_alsa_verbo) logd ("type: Enumerated (%d)                ", cei.type);

          if (! name) if (ena_log_alsa_verbo) logd ("Values:");
          for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
            if (! name) if (ena_log_alsa_verbo) logd (" %d", val.value.enumerated.item [cd_ctr]);

          if (! name) if (ena_log_alsa_verbo) logd ("  items: %d  ", cei.value.enumerated.items);
          if (! name) if (ena_log_alsa_verbo) logd ("names:");
          int item_ctr = 0;
          for (item_ctr = 0; item_ctr < cei.value.enumerated.items; item_ctr ++) {
            cei.value.enumerated.item = item_ctr;
            ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_ELEM_INFO, & cei);
            if (ret < 0) {
              if (ena_log_alsa_verbo) logd ("alsa_control_id_get SNDRV_CTL_IOCTL_ELEM_INFO 2 ret: %d  errno: %d", ret, errno);
              continue;//break;
            }
            if (! name) if (ena_log_alsa_verbo) logd (" '%s'", cei.value.enumerated.name);
          }
          //if (! name) if (ena_log_alsa_verbo) logd ("");

          //if (! name) if (ena_log_alsa_verbo) logd ("Values:");
          //for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
          //  if (! name) if (ena_log_alsa_verbo) logd (" %d", val.value.enumerated.item [cd_ctr]);
          if (! name) if (ena_log_alsa_verbo) logd ("");
          //if (! name) if (ena_log_alsa_verbo) logd ("val  item_ptr:   %p", val.value.enumerated.item_ptr);
        }
        else if (cei.type==SNDRV_CTL_ELEM_TYPE_BOOLEAN) {
          if (! name) if (ena_log_alsa_verbo) logd ("type: Boolean (%d)                   ", cei.type);
          if (! name) if (ena_log_alsa_verbo) logd ("Values:");
          for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
            if (! name) if (ena_log_alsa_verbo) logd (" %ld", val.value.integer.value [cd_ctr]);
          if (! name) if (ena_log_alsa_verbo) logd ("");
          //if (! name) if (ena_log_alsa_verbo) logd ("val  value_ptr:  %p", val.value.integer.value_ptr);
        }
        else if (cei.type==SNDRV_CTL_ELEM_TYPE_BYTES) {
          if (! name) if (ena_log_alsa_verbo) logd ("type: Bytes (%d)  ", cei.type);
          if (! name) if (ena_log_alsa_verbo) logd ("Values:");
          for (cd_ctr = 0; cd_ctr < cei_count; cd_ctr ++)
            if (! name) if (ena_log_alsa_verbo) logd (" %d", val.value.bytes.data [cd_ctr]);
          if (! name) if (ena_log_alsa_verbo) logd ("");
          //if (! name) if (ena_log_alsa_verbo) logd ("val  data_ptr:   %p", val.value.bytes.data_ptr);
        }
        else if (cei.type==SNDRV_CTL_ELEM_TYPE_IEC958) {
          if (! name) if (ena_log_alsa_verbo) logd ("type: IEC958 (S/PDIF) (%d)", cei.type);
        }
        else {
          if (! name) if (ena_log_alsa_verbo) logd ("type: %d", cei.type);
        }
        if (cei.dimen.d [0] || cei.dimen.d [1] || cei.dimen.d [2] || cei.dimen.d [3]) // ?? Never used ??
          if (! name) if (ena_log_alsa_verbo) logd ("cei dimen: %d %d %d %d", cei.dimen.d [0], cei.dimen.d [1], cei.dimen.d [2], cei.dimen.d [3]);

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
          ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_TLV_READ, tlvp);
          if (ret < 0) {
            //if (! name)
            if (ena_log_alsa_verbo) logd ("alsa_control_id_get SNDRV_CTL_IOCTL_TLV_READ ret: %d  errno: %d", ret, errno);
            continue;//break;
          }

          int tlv_ctr=0;
          if (! name) if (ena_log_alsa_verbo) logd ("tlv  length:    %d", tlvp->length);
          for (tlv_ctr=0;tlv_ctr<tlvp->length/4;tlv_ctr++)
            if (! name) if (ena_log_alsa_verbo) logd (" %x", tlvp->tlv[tlv_ctr]);
          if (! name) if (ena_log_alsa_verbo) logd ("");
        }
      #endif

        //if (cei_ctr + 1 <cei_count)
          //if (! name) if (ena_log_alsa_verbo) logd ("");
      }
    }
    if (! name) if (ena_log_alsa_verbo) logd ("------------------------------------");
    //if (! name) if (ena_log_alsa_verbo) logd ("");

    if (index_ret >= 0)
      return (index_ret);

    return (ret);
  }


  int pcm_control_open () {
    if (pcm_control_fd < 0) {                                           // If   control device not open yet...
      pcm_control_fd = open ("/dev/snd/controlC0", O_NONBLOCK | O_RDWR);// Open control device
      if (pcm_control_fd < 0) {
        loge ("pcm_control_open error opening /dev/snd/controlC0 errno: %s (%d)", strerror (errno), errno);
        return (pcm_control_fd);
      }
      logd ("pcm_control_open pcm_control_fd: %d", pcm_control_fd);
    }
    return (pcm_control_fd);
  }

  long value_set (int type, int id, long value, struct snd_ctl_elem_value * val) {
    val->id.numid = id;
    switch (type) {
      case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
      case SNDRV_CTL_ELEM_TYPE_INTEGER:
      case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
        //if (ena_log_alsa_verbo) logd ("value_set Integer %ld", value);
        val->value.integer.value [0] = value;                            // Set as 4 instances for cases where 2-4 instances are needed (such as for stereo controls)
        val->value.integer.value [1] = value;
        val->value.integer.value [2] = value;
        val->value.integer.value [3] = value;
        break;    
      case SNDRV_CTL_ELEM_TYPE_INTEGER64:                               // 64 bit integer never seen on Android yet
        //if (ena_log_alsa_verbo) logd ("value_set Integer64 %ld", value);
        val->value.integer64.value [0] = value;                          // Set as 4 instances
        val->value.integer64.value [1] = value;
        val->value.integer64.value [2] = value;
        val->value.integer64.value [3] = value;
        break;    
      //case SNDRV_CTL_ELEM_TYPE_BYTES:                                 // Bytes not supported
      //  val->value.bytes.data[0] = value;
      //  break;    
    }
    return (value);
  }

  int value_get (int type, int id, long value, struct snd_ctl_elem_value * val) {
    switch (type) {
      case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
      case SNDRV_CTL_ELEM_TYPE_INTEGER:
      case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
        //if (ena_log_alsa_verbo) logd ("value_get Integer %ld", value);
        value = val->value.integer.value [0];                           // !! First instance only !!
        break;    
    }
    return (value);
  }

  int alsa_control_log (int verbose) {
    if (pcm_control_open () < 0)
      return (-1);

    int id = -1;
    if (verbose)
      id = alsa_control_id_get (NULL);                              // Long controls info verbose
    else
      id = alsa_control_id_get ("Fake Control");                    // Long controls info terse: Get integer ID from fake ALSA control name

    logd ("id: %d", id);
    return (id);
  }

  long alsa_control_set_get (int is_set, int type, char * idname, long value) {
    if (pcm_control_open () < 0)
      return (-1);

    int id = alsa_control_id_get (idname);                          // Get integer ID from ALSA control name
    if (id < 0) {                                                   // If this is not a valid control number... ?? 0 is never valid ??
      loge ("alsa_control_set_get can't get control id from alsa_control_id_get() idname: %s", idname);
      return (-2);
    }

    int ret = -1;
    struct snd_ctl_elem_value val = {0};
    memset (& val, 0, sizeof (val));

    if (is_set) {
      value_set (type, id, value, & val);
      ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_ELEM_WRITE, & val);    // Write ALSA value
      if (ret < 0)
        if (ena_log_alsa_error) loge ("alsa_control_set_get SNDRV_CTL_IOCTL_ELEM_WRITE ioctl errno: %d  idname: %s", errno, idname);
      else
        if (ena_log_alsa_verbo) logd ("alsa_control_set_get SNDRV_CTL_IOCTL_ELEM_WRITE ioctl OK  idname: %s", idname);
    }
    else {
      ret = ioctl (pcm_control_fd, SNDRV_CTL_IOCTL_ELEM_READ,  & val);    // Read  ALSA value
      if (ret < 0)
        if (ena_log_alsa_error) loge ("alsa_control_set_get SNDRV_CTL_IOCTL_ELEM_READ ioctl errno: %d  idname: %s", errno, idname);
      else
        if (ena_log_alsa_verbo) logd ("alsa_control_set_get SNDRV_CTL_IOCTL_ELEM_READ ioctl OK  idname: %s", idname);
      ret = value_get (type, id, -1, & val);
    }

    return (ret);
  }

  long alsa_bool_set (char * idname, long value) {
    long ret = alsa_control_set_get (1, SNDRV_CTL_ELEM_TYPE_BOOLEAN, idname, value);
    return (ret);
  }
  long alsa_long_set (char * idname, long value) {
    long ret = alsa_control_set_get (1, SNDRV_CTL_ELEM_TYPE_INTEGER, idname, value);
    return (ret);
  }
  long alsa_enum_set (char * idname, long value) {
    long ret = alsa_control_set_get (1, SNDRV_CTL_ELEM_TYPE_ENUMERATED, idname, value);
    return (ret);
  }

  long alsa_bool_get (char * idname, long value) {
    long ret = alsa_control_set_get (0, SNDRV_CTL_ELEM_TYPE_BOOLEAN, idname, value);
    return (ret);
  }
  long alsa_long_get (char * idname, long value) {
    long ret = alsa_control_set_get (0, SNDRV_CTL_ELEM_TYPE_INTEGER, idname, value);
    return (ret);
  }
  long alsa_enum_get (char * idname, long value) {
    long ret = alsa_control_set_get (0, SNDRV_CTL_ELEM_TYPE_ENUMERATED, idname, value);
    return (ret);
  }


    // Hostless transfer:

  void alsa_pcm_info_log (int pcm_fd) {
    struct snd_pcm_info info;
    struct snd_pcm_info * i = & info;
  
    int ret = ioctl (pcm_fd, SNDRV_PCM_IOCTL_INFO, & info);   //-2128592639 0x81204101  size: 288 // 2166374657     //logd ("alsa_pcm_info_log info: %d %x  size: %d", SNDRV_PCM_IOCTL_INFO, SNDRV_PCM_IOCTL_INFO, sizeof (info));
    if (ret) {
      logd ("alsa_pcm_info_log info error ret: %d  errno: %d", ret, errno);
      return;
    }
  
    logd ("alsa_pcm_info_log device: %d  subdevice: %d  stream: %d  card: %d  id: %s  name: %s  subname: %s", i->device, i->subdevice, i->stream, i->card, i->id, i->name, i->subname);
    logd ("alsa_pcm_info_log dev_class: %d  dev_subclass: %d  subdevices_count: %d  subdevices_avail: %d", i->dev_class, i->dev_subclass, i->subdevices_count, i->subdevices_avail);
    /*int ctr = 0;
    logd ("alsa_pcm_info_log sync:");
    for (ctr = 0; ctr < 16; ctr ++)
      logd (" %x", i->sync.id [ctr]);*/
  }


  static void alsa_pcm_mask_set (struct snd_pcm_hw_params * p, int n, unsigned bit) {
    if (bit >= SNDRV_MASK_MAX)
      return;
    if (n >= SNDRV_PCM_HW_PARAM_FIRST_MASK && n <= SNDRV_PCM_HW_PARAM_LAST_MASK) {
      struct snd_mask * m = & (p->masks [n - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
      m->bits [0] = 0;
      m->bits [1] = 0;
      m->bits [bit >> 5] |= (1 << (bit & 31));
    }
  }
  
  static void alsa_pcm_interval_set (struct snd_pcm_hw_params * p, int n, unsigned val) {
    if (n >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL && n <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL) {
      struct snd_interval * i = & (p->intervals [n - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
      i->min = val;
      i->max = val;
      i->integer = 1;
    }
  }

  int alsa_pcm_init (int pcm_fd, int samplerate, int channels) {
    int ret = 0;

    ret = fcntl (pcm_fd, F_SETFL, O_RDWR);
    alsa_pcm_info_log (pcm_fd);

    struct snd_pcm_hw_params params;
    struct snd_pcm_hw_params * p = & params;
    int n;
    memset (p, 0, sizeof (* p));
    for (n = SNDRV_PCM_HW_PARAM_FIRST_MASK; n <= SNDRV_PCM_HW_PARAM_LAST_MASK; n ++) {
      struct snd_mask * m = & (p->masks [n - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
      m->bits[0] = ~0;
      m->bits[1] = ~0;
    }
    for (n = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL; n <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; n ++) {
      struct snd_interval * i = & (p->intervals [n - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
      i->min = 0;
      i->max = ~0;
    }
  
    int sample_bits = 16;
  
    alsa_pcm_mask_set (& params, SNDRV_PCM_HW_PARAM_ACCESS,      SNDRV_PCM_ACCESS_RW_INTERLEAVED);
    alsa_pcm_mask_set (& params, SNDRV_PCM_HW_PARAM_FORMAT,      SNDRV_PCM_FORMAT_S16_LE);
    alsa_pcm_mask_set (& params, SNDRV_PCM_HW_PARAM_SUBFORMAT,   SNDRV_PCM_SUBFORMAT_STD);
  
    alsa_pcm_interval_set  (& params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, sample_bits);
    alsa_pcm_interval_set  (& params, SNDRV_PCM_HW_PARAM_FRAME_BITS,  sample_bits * channels);
    alsa_pcm_interval_set  (& params, SNDRV_PCM_HW_PARAM_CHANNELS,    channels);
    alsa_pcm_interval_set  (& params, SNDRV_PCM_HW_PARAM_RATE,        samplerate);
  
    ret = ioctl (pcm_fd, SNDRV_PCM_IOCTL_HW_PARAMS, & params);
    if (ret) {
      logd ("alsa_pcm_init set hw params error ret: %d  errno: %d", ret, errno);
      return (-1);
    }

    struct snd_pcm_sw_params sparams;                                   // Don't need to set software params anymore ??
    memset (& sparams, 0, sizeof (sparams));
  
    sparams.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;//ENABLE;
    sparams.period_step =         1;
    sparams.avail_min =           80;         //1;                                                // Act when a minimum of 1 frame is ready
    sparams.start_threshold =     1;          //1024*8/2;//period_cnt * period_sz / 2;                 // 2048
    sparams.stop_threshold =      2147483647; //1024*8*10;//period_cnt * period_sz * 10;                 // 40960
    sparams.xfer_align =          1;//80;         //512;//period_sz / 2;                                   // 512      needed for old kernels
    sparams.silence_size =        0;
    sparams.silence_threshold =   0;
  
    ret = ioctl (pcm_fd, SNDRV_PCM_IOCTL_SW_PARAMS, & sparams);
    if (ret)
      logd ("alsa_pcm_init set sw params error ret: %d  errno: %d", ret, errno);
    else
      logd ("alsa_pcm_init set sw params OK");

    ret = ioctl (pcm_fd, SNDRV_PCM_IOCTL_PREPARE);                     // Prepare for transfer
    if (ret) {
      logd ("alsa_pcm_init prepare error ret: %d  errno: %d", ret, errno);
      return (-1);
    }
    //else
    //  logd ("alsa_pcm_init 0x4140, 0x7 OK");
  
    return (0);
  }


    // QCV: hostless_transfer_start (hostless_samplerate, 2, "/dev/snd/pcmC0D6c", "");
    // ONE: hostless_transfer_start (hostless_samplerate, 2, "/dev/snd/pcmC0D6c", "");
    // LG2: hostless_transfer_start (hostless_samplerate, 2, "/dev/snd/pcmC0D17c", "/dev/snd/pcmC0D5p");
    // XZ2: hostless_transfer_start (hostless_samplerate, 2, "/dev/snd/pcmC0D25c", "/dev/snd/pcmC0D25p");

  int hostless_transfer_start (int samplerate, int channels, char * pcm_capture_name, char * pcm_playback_name) {
    int ret = 0;
    char * pcm_1_name = pcm_capture_name;
    char * pcm_2_name = NULL;

    if (pcm_capture_name == NULL) {                                          // Done if no capture PCM
      loge ("hostless_transfer_start samplerate: %d  channels: %d  pcm_capture_name: %p  pcm_playback_name: %p", samplerate, channels, pcm_capture_name, pcm_playback_name);
      return (-1);
    }
    else if (pcm_playback_name) {
      logd ("hostless_transfer_start samplerate: %d  channels: %d  pcm_capture_name: %s  pcm_playback_name: %s", samplerate, channels, pcm_capture_name, pcm_playback_name);
      pcm_1_name = pcm_playback_name;
      pcm_2_name = pcm_capture_name;
    }
    else
      logd ("hostless_transfer_start samplerate: %d  channels: %d  pcm_capture_name: %s", samplerate, channels, pcm_capture_name);

    hostless_transfer_stop ();                                          // Ensure file descriptors are closed from a previous start

    logd ("hostless_transfer_start opening pcm_1_name: %s", pcm_1_name);
    pcm_device1_fd = open (pcm_1_name, O_NONBLOCK | O_RDWR );                  // Open pcm1
    if (pcm_device1_fd < 0) {
      loge ("hostless_transfer_start error opening pcm_1_name: %s  errno: %s (%d)", pcm_1_name, strerror (errno), errno);
      return (-2);
    }
    logd ("hostless_transfer_start pcm_device1_fd: %d", pcm_device1_fd);
    if (alsa_pcm_init (pcm_device1_fd, samplerate, channels))
      return (-3);
    ret = ioctl (pcm_device1_fd, SNDRV_PCM_IOCTL_START);                       // Start Hostless Transfer on pcm1
    if (ret) {
      loge ("hostless_transfer_start start error pcm1 ret: %d  errno: %d", ret, errno);
      return (-4);
    }
    logd ("hostless_transfer_start start done success pcm1");

    if (pcm_playback_name == NULL)                                      // Done if no playback PCM
      return (ret);
  
    logd ("hostless_transfer_start opening pcm_2_name: %s", pcm_2_name);
    pcm_device2_fd = open (pcm_playback_name, O_NONBLOCK | O_RDWR);            // Open pcm2
    if (pcm_device2_fd < 0) {
      loge ("hostless_transfer_start error opening pcm_2_name: %s  errno: %s (%d)", pcm_1_name, strerror (errno), errno);
      return (-5);
    }
    logd ("hostless_transfer_start pcm_device2_fd: %d", pcm_device2_fd);
    if (alsa_pcm_init (pcm_device2_fd, samplerate, channels))
      return (-6);

    ret = ioctl (pcm_device2_fd, SNDRV_PCM_IOCTL_START);                       // Start Hostless Transfer on pcm2
    if (ret) {
      loge ("hostless_transfer_start start error pcm2 ret: %d  errno: %d", ret, errno);
      return (-7);
    }
    logd ("hostless_transfer_start start done success pcm2");

    return (ret);
  }

  int hostless_transfer_stop () {                                       // Closing file descriptors stops as well as sending Ioctl ??
    int ret = 0;
    logd ("hostless_transfer_stop pcm_device1_fd: %d  pcm_device2_fd: %d", pcm_device1_fd, pcm_device2_fd);
    if (pcm_device1_fd >= 0)
      close (pcm_device1_fd);
    pcm_device1_fd = -1;
    if (pcm_device2_fd >= 0)
      close (pcm_device2_fd);
    pcm_device2_fd = -1;
    return (ret);
  }

