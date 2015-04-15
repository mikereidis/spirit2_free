
  // #include "aud_all.c"                                                  // Audio functions for all supported audio architectures. To be replaced with audio plugins, similar to tuner plugins.

// Does it work ? : S1 LG2_ALSA uses ssd_run_var ("5 8 44100 2 /dev/snd/pcmC0D5p  /dev/snd/pcmC0D17c &", 0);
// Does it work ? : S1 XZ2_ALSA uses ssd_run_var ("5 8 44100 2 /dev/snd/pcmC0D25p /dev/snd/pcmC0D25c &", 1);
//   Also uses same for digital on AOSP. Stock uses: setDeviceConnectionState (0x80000, DEVICE_STATE_AVAILABLE, "");
// Sony Z QC Intent for stock also             uses: setDeviceConnectionState (0x80000, DEVICE_STATE_AVAILABLE, "");        (and Intent "qualcomm.intent.action.FM"/"state" and setFmReceiverOn())


/*


03-01 04:49:30.158 D/FMService(24599): Audio source set it as headset
03-01 04:49:30.160 D/audio_hw_primary(  237): adev_set_parameters: enter: connect=1048576
03-01 04:49:30.160 E/audio_a2dp_hw(  237): adev_set_parameters: ERROR: set param called even when stream out is null

03-01 04:49:30.167 D/audio_hw_primary(  237): out_set_parameters: enter: usecase(0: deep-buffer-playback) kvpairs: handle_fm=1048580
03-01 04:49:30.167 D/audio_hw_fm(  237): audio_extn_fm_set_parameters: FM usecase
03-01 04:49:30.167 D/audio_hw_fm(  237): fm_start: enter
03-01 04:49:30.167 D/audio_hw_primary(  237): select_devices: out_snd_device(4: headphones) in_snd_device(0: )
03-01 04:49:30.167 W/msm8974_platform(  237): Codec backend bitwidth 16, samplerate 48000
03-01 04:49:30.167 D/hardware_info(  237): hw_info_append_hw_type : device_name = headphones-lite
03-01 04:49:30.170 D/audio_hw_primary(  237): select_devices: done
03-01 04:49:30.179 D/audio_hw_fm(  237): fm_set_volume: (0.010000)
03-01 04:49:30.179 D/audio_hw_fm(  237): fm_set_volume: Setting FM volume to 82 
03-01 04:49:30.180 D/audio_hw_fm(  237): fm_start: exit: status(0)
03-01 04:49:30.190 D/audio_hw_primary(  237): out_set_parameters: enter: usecase(0: deep-buffer-playback) kvpairs: routing=4
03-01 04:49:30.195 D/audio_hw_primary(  237): out_set_parameters: enter: usecase(1: low-latency-playback) kvpairs: routing=0


ro.board.platform:

s5pc110:
gs1

exynos4:
gs2
gs3
no1
no2

msm8960:
oxl
xz0
om7

msm8226:
mog

msm8974:
om8
xz1
xz2
lg2



sudo update-alternatives --config java
sudo update-alternatives --config javac

declare -x ANDROID_JAVA_TOOLCHAIN="/usr/lib/jvm/java-1.7.0-openjdk-amd64/bin"
declare -x JAVA_HOME="/usr/lib/jvm/java-1.7.0-openjdk-amd64"

export ANDROID_JAVA_TOOLCHAIN="/usr/lib/jvm/java-1.7.0-openjdk-amd64/bin"
export JAVA_HOME="/usr/lib/jvm/java-1.7.0-openjdk-amd64"


Exclude Seconds:
cd ~/android/system
  3:    time grep -RIsl FM ~/android/system/ --exclude-dir=out --exclude-dir=prebuilts --exclude-dir=external --exclude-dir=frameworks --exclude-dir=kernel | wc -l
 11:    time grep -RIsl FM ~/android/system/ --exclude-dir=out --exclude-dir=prebuilts --exclude-dir=external --exclude-dir=frameworks                      | wc -l
 12:    time grep -RIsl FM ~/android/system/ --exclude-dir=out --exclude-dir=prebuilts --exclude-dir=external                                               | wc -l
141:    time grep -RIsl FM ~/android/system/ --exclude-dir=out --exclude-dir=prebuilts                                                                      | wc -l

 1133
 7363
 7512
12203

======================================================================================================================================================

frameworks/base/media/java/android/media/MediaRecorder.java
  public static final int FM_RX      = 10;
  public static final int FM_RX_A2DP = 11;
   public static final int getAudioSourceMax() {
        return AudioSource.FM_RX_A2DP;
    }


frameworks/base/media/java/android/media/MediaRecorder.java
    protected static final int HOTWORD = 1999;

frameworks/base/media/java/android/media/AudioAttributes.java
       public Builder setCapturePreset(int preset) {
            switch (preset) {
                case MediaRecorder.AudioSource.DEFAULT:
                case MediaRecorder.AudioSource.MIC:
                case MediaRecorder.AudioSource.CAMCORDER:
                case MediaRecorder.AudioSource.VOICE_RECOGNITION:
                case MediaRecorder.AudioSource.VOICE_COMMUNICATION:
                    mSource = preset;
                    break;
                default:
                    Log.e(TAG, "Invalid capture preset " + preset + " for AudioAttributes");
            }
            return this;
        }

        public Builder setInternalCapturePreset(int preset) {
            if ((preset == MediaRecorder.AudioSource.HOTWORD)
                    || (preset == MediaRecorder.AudioSource.REMOTE_SUBMIX)) {
                mSource = preset;
            } else {
                setCapturePreset(preset);
            }
            return this;
        }
    };
======================================================================================================================================================


cd ~/android/system
grep -RIs FM  device | el

device/motorola/msm8226-common/BoardConfigCommon.mk
# Audio
AUDIO_FEATURE_ENABLED_FM := true
# FM
TARGET_QCOM_NO_FM_FIRMWARE := true

device/motorola/msm8226-common/msm8226.mk
# FM
PRODUCT_PACKAGES += \
    FM2 \
    FMRecord \
    libqcomfm_jni \
    qcom.fmradio

device/motorola/msm8226-common/configs/audio_policy.conf    global_conf:        attached_input_devices...                               AUDIO_DEVICE_IN_FM_RX   AUDIO_DEVICE_IN_FM_RX_A2DP
device/motorola/msm8226-common/configs/audio_policy.conf:   primary:            devices...                  AUDIO_DEVICE_OUT_FM     AUDIO_DEVICE_OUT_FM_TX
device/motorola/msm8226-common/configs/audio_policy.conf:   low_latency:                                    AUDIO_DEVICE_OUT_FM     AUDIO_DEVICE_OUT_FM_TX
device/motorola/msm8226-common/configs/audio_policy.conf:   compress_offload:   devices...                                          AUDIO_DEVICE_OUT_FM_TX
device/motorola/msm8226-common/configs/audio_policy.conf:   inputs primary:     devices...                                              AUDIO_DEVICE_IN_FM_RX   AUDIO_DEVICE_IN_FM_RX_A2DP

device/sony/rhine-common/audio/audio_policy.conf:       SAME as above but no global_conf


device/motorola/falcon/configs/mixer_paths.xml
  <mixer> <!-- These are the initial mixer settings -->
  <!-- fm -->
    <ctl name="SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX" value="0" />
    <ctl name="SLIMBUS_DL_HL Switch" value="0" />
    <ctl name="MultiMedia1 Mixer INTERNAL_FM_TX" value="0" />
    <ctl name="MultiMedia2 Mixer INTERNAL_FM_TX" value="0" />
    <ctl name="INTERNAL_FM_RX Audio Mixer MultiMedia1" value="0" />
    <ctl name="INTERNAL_FM_RX Audio Mixer MultiMedia5" value="0" />
  <!-- fm end -->
  <path name="audio-record capture-fm">
    <ctl name="MultiMedia1 Mixer INTERNAL_FM_TX" value="1" />
  </path>
  <path name="fm-virtual-record capture-fm">
    <ctl name="MultiMedia2 Mixer INTERNAL_FM_TX" value="1" />
  </path>
  <path name="play-fm">
    <ctl name="Internal FM RX Volume" value="1" />
    <ctl name="SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX" value="1" />
    <ctl name="SLIMBUS_DL_HL Switch" value="1" />
  </path>
  <path name="capture-fm">
  </path>

device/sony/honami/audio/mixer_paths.xml
Above, plus:
  <mixer> <!-- These are the initial mixer settings -->
    <ctl name="INTERNAL_FM_RX Audio Mixer MultiMedia4" value="0" />
    <path name="deep-buffer-playback transmission-fm">
        <ctl name="INTERNAL_FM_RX Audio Mixer MultiMedia1" value="1" />
    </path>
    <path name="low-latency-playback transmission-fm">
        <ctl name="INTERNAL_FM_RX Audio Mixer MultiMedia5" value="1" />
    </path>
    <path name="compress-offload-playback transmission-fm">
        <ctl name="INTERNAL_FM_RX Audio Mixer MultiMedia4" value="1" />
    </path>
    <path name="play-fm usb-headphones">
        <ctl name="Internal FM RX Volume" value="1" />
        <ctl name="AFE_PCM_RX Port Mixer INTERNAL_FM_TX" value="1" />
        <ctl name="SLIMBUS_DL_HL Switch" value="1" />
    </path>
    <path name="transmission-fm">
    </path>





device/sony/rhine-common/rhine.mk
# FM Radio
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/init.qcom.fm.sh:system/etc/init.qcom.fm.sh

device/sony/rhine-common/rootdir/system/etc/init.qcom.fm.sh
...

device/sony/rhine-common/proprietary-files.txt
# FM
bin/fmconfig
bin/fm_qsoc_patches
lib/libfmradio.so
lib/libfmradio.sony-iris.so



======================================================================================================================================================
frameworks/av/services/audiopolicy/AudioPolicyManager.cpp

#ifdef AUDIO_EXTN_FM_ENABLED
    case AUDIO_SOURCE_FM_RX:
        device = AUDIO_DEVICE_IN_FM_RX;
        break;
    case AUDIO_SOURCE_FM_RX_A2DP:
        device = AUDIO_DEVICE_IN_FM_RX_A2DP;
        break;
#endif

time grep -RIsl ~/android/system/ --exclude-dir=out --exclude-dir=prebuilts --exclude-dir=external -e AUDIO_SOURCE_FM_RX


!! Vs 9 / 10 !!
/home/m/android/system/system/core/include/system/audio.h:    AUDIO_SOURCE_FM_RX               = 10,
/home/m/android/system/system/core/include/system/audio.h:    AUDIO_SOURCE_FM_RX_A2DP          = 11,

/home/m/android/system/hardware/qcom/audio-caf/apq8084/hal/audio_extn/audio_extn.h:#define AUDIO_SOURCE_FM_RX 9
/home/m/android/system/hardware/qcom/audio-caf/apq8084/hal/audio_extn/audio_extn.h:#define AUDIO_SOURCE_FM_RX_A2DP 10


#ifdef AUDIO_EXTN_FM_ENABLED
        if(device == AUDIO_DEVICE_OUT_FM) {
            if (state == AUDIO_POLICY_DEVICE_STATE_AVAILABLE) {
                mOutputs.valueFor(mPrimaryOutput)->changeRefCount(AUDIO_STREAM_MUSIC, 1);
                newDevice = (audio_devices_t)(getNewOutputDevice(mPrimaryOutput, false) | AUDIO_DEVICE_OUT_FM);
            } else {
                mOutputs.valueFor(mPrimaryOutput)->changeRefCount(AUDIO_STREAM_MUSIC, -1);
            }

            AudioParameter param = AudioParameter();
            param.addInt(String8("handle_fm"), (int)newDevice);
            ALOGV("setDeviceConnectionState() setParameters handle_fm");
            mpClientInterface->setParameters(mPrimaryOutput, param.toString());
        }
#endif


~/android/system/hardware/qcom/audio-caf/msm8974 $ el hal/audio_extn/audio_extn.h

#ifndef FM_ENABLED
#define AUDIO_DEVICE_OUT_FM 0x80000
#define AUDIO_DEVICE_OUT_FM_TX 0x100000
#define AUDIO_SOURCE_FM_RX 9
#define AUDIO_SOURCE_FM_RX_A2DP 10
#define AUDIO_DEVICE_IN_FM_RX (AUDIO_DEVICE_BIT_IN | 0x8000)
#define AUDIO_DEVICE_IN_FM_RX_A2DP (AUDIO_DEVICE_BIT_IN | 0x10000)
#endif

======================================================================================================================================================
system/core/include/system/audio.h :

#ifdef QCOM_HARDWARE
    AUDIO_SOURCE_FM_RX               = 10,
    AUDIO_SOURCE_FM_RX_A2DP          = 11,
#endif

    AUDIO_DEVICE_OUT_FM                        = 0x100000,      // FM transmitter out ?
#ifdef QCOM_HARDWARE
    AUDIO_DEVICE_OUT_FM_TX                     = 0x1000000,
    AUDIO_DEVICE_OUT_PROXY                     = 0x2000000,
#endif

    AUDIO_DEVICE_BIT_IN                        = 0x80000000,
    AUDIO_DEVICE_IN_FM_TUNER              = AUDIO_DEVICE_BIT_IN | 0x2000,           // FM tuner input
#ifdef QCOM_HARDWARE
    AUDIO_DEVICE_IN_PROXY                 = AUDIO_DEVICE_BIT_IN | 0x100000,
    AUDIO_DEVICE_IN_FM_RX                 = AUDIO_DEVICE_BIT_IN | 0x200000,
    AUDIO_DEVICE_IN_FM_RX_A2DP            = AUDIO_DEVICE_BIT_IN | 0x400000,
#endif




hardware/qcom/audio-caf/msm8974/hal/audio_extn/audio_extn.h
hardware/qcom/audio-caf/apq8084/hal/audio_extn/audio_extn.h
hardware/qcom/audio-caf/msm8916/hal/audio_extn/audio_extn.h
#ifndef FM_ENABLED
#define AUDIO_DEVICE_OUT_FM 0x80000
#define AUDIO_DEVICE_OUT_FM_TX 0x100000
#define AUDIO_SOURCE_FM_RX 9
#define AUDIO_SOURCE_FM_RX_A2DP 10
#define AUDIO_DEVICE_IN_FM_RX (AUDIO_DEVICE_BIT_IN | 0x8000)
#define AUDIO_DEVICE_IN_FM_RX_A2DP (AUDIO_DEVICE_BIT_IN | 0x10000)
#endif





qall shell "getprop ro.modversion ; grep -l -e AUDIO_DEVICE_OUT_FM -e AUDIO_DEVICE_IN_FM -e fm_volume -e handle_fm -e FmVolume -e HandleFm system/lib/libaudio*"

12-20150226-NIGHTLY-evita
system/lib/libaudiopolicymanagerdefault.so

12-20150226-NIGHTLY-falcon
system/lib/libaudiopolicymanagerdefault.so

11-20150223-NIGHTLY-yuga
system/lib/libaudioflinger.so
system/lib/libaudioparameter.so

12-20150226-NIGHTLY-honami
system/lib/libaudiopolicymanagerdefault.so

12-20150226-NIGHTLY-m8
system/lib/libaudiopolicymanagerdefault.so

qall shell "getprop ro.modversion ; strings system/lib/libaudio*|grep -e AUDIO_DEVICE_OUT_FM -e AUDIO_DEVICE_IN_FM -e fm_volume -e handle_fm -e FmVolume -e HandleFm"

12-20150226-NIGHTLY-evita                       Only most basic symbols, so no FM support
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_IN_FM_TUNER

12-20150226-NIGHTLY-falcon
fm_volume
handle_fm
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_OUT_FM_TX
AUDIO_DEVICE_IN_FM_TUNER
AUDIO_DEVICE_IN_FM_RX
AUDIO_DEVICE_IN_FM_RX_A2DP

11-20150223-NIGHTLY-yuga                        CM11, so different, but few symbols and little FM support ?
_ZN7android14AudioParameter11keyFmVolumeE
_ZN7android14AudioParameter11keyHandleFmE
fm_volume
handle_fm
_ZN7android14AudioParameter11keyFmVolumeE
_ZN7android14AudioParameter11keyHandleFmE
fm_volume
handle_fm

12-20150226-NIGHTLY-honami
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_IN_FM_TUNER

12-20150226-NIGHTLY-m8
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_IN_FM_TUNER



MOG:    a strings system/lib/hw/audio.primary.msm8226.so|grep -i fm|sort|uniq|wc -l
21
audio_extn_fm_set_parameters
audio_hw_fm
capture-fm
fm_set_volume
fm_start
fm_stop
fm-virtual-record
fm_volume
handle_fm
Internal FM RX Volume
play-fm
%s: error in retrieving fm volume
%s: FM usecase
SND_DEVICE_IN_CAPTURE_FM
SND_DEVICE_OUT_TRANSMISSION_FM
sound card is OFFLINE, stop FM
sound card is ONLINE, restart FM
%s: Problem in FM start: status(%d)
%s: set_fm_volume usecase
%s: Setting FM volume to %d 
transmission-fm


OM8:    a strings system/lib/hw/audio.primary.msm8974.so|grep -i fm|wc
      6       6     115
XZ1:    a strings system/lib/hw/audio.primary.msm8974.so|grep -i fm|wc
      6       6     115
play-fm
fm-virtual-record
capture-fm
transmission-fm
SND_DEVICE_OUT_TRANSMISSION_FM
SND_DEVICE_IN_CAPTURE_FM


XZ0:    a strings system/lib/hw/audio.primary.msm8960.so|grep -i fm|sort|uniq|wc -l
20 these 3 more than OXL
handle_fm
_ZN7android14AudioParameter11keyFmVolumeE
_ZN7android14AudioParameter11keyHandleFmE

OXL:    a strings system/lib/hw/audio.primary.msm8960.so|grep -i fm|sort|uniq|wc -l
17
Both:
Capture A2DP FM
Capture FM
FM A2DP REC
FM Digital Radio
FM REC
fm_volume
Internal FM RX Volume
Play FM
set Fm Volume(%f) over 1.0, assuming 1.0
set Fm Volume(%f) under 0.0, assuming 0.0
startFm: could not open PCM device
startFm: pcm_prepare failed
startFm: setHardwareParams failed
startFm: setSoftwareParams failed
startFm: SNDRV_PCM_IOCTL_START failed
_ZN20android_audio_legacy10ALSADevice11setFmVolumeEi
_ZN20android_audio_legacy10ALSADevice7startFmEPNS_13alsa_handle_tE



time grep -RIsl AUDIO_FEATURE_ENABLED_FM ~/android/system/ --exclude-dir=out --exclude-dir=prebuilts --exclude-dir=external

frameworks/av/services/audiopolicy/Android.mk           -> AUDIO_EXTN_FM_ENABLED
hardware/qcom/audio-caf/msm8974/policy_hal/Android.mk
hardware/qcom/audio-caf/apq8084/policy_hal/Android.mk
hardware/qcom/audio-caf/msm8226/policy_hal/Android.mk
hardware/qcom/audio-caf/msm8916/policy_hal/Android.mk

hardware/qcom/audio-caf/msm8974/hal/Android.mk          -> FM_ENABLED       LOCAL_SRC_FILES += audio_extn/fm.c
....


!!
hardware/qcom/audio-caf/msm8226         ->              hardware/qcom/audio-caf/msm8974

??
cd ~/android/system
grep -RIs QCOM_FM_ENABLED hardware/|grep -v ifdef
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/Android.mk:ifeq ($(strip $(QCOM_FM_ENABLED)),true)
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/Android.mk:    common_cflags += -DQCOM_FM_ENABLED



time grep -RIs FM_ENABLE frameworks/ kernel/ hardware/

x12:frameworks/av/services/audiopolicy/AudioPolicyManager.cpp:                  #ifdef AUDIO_EXTN_FM_ENABLED        defines handle_fm, fm_volume and others (MOG & OM8 only)

    frameworks/av/services/audiopolicy/Android.mk:common_cflags += -DAUDIO_EXTN_FM_ENABLED

    hardware/qcom/audio-caf/msm8974/legacy/alsa_sound/ALSAStreamOps.cpp:        #ifdef QCOM_FM_ENABLED
x2: hardware/qcom/audio-caf/msm8974/legacy/alsa_sound/AudioStreamInALSA.cpp:    #ifdef QCOM_FM_ENABLED
x2: hardware/qcom/audio-caf/msm8974/legacy/alsa_sound/alsa_default.cpp:         #ifdef QCOM_FM_ENABLED
x4: hardware/qcom/audio-caf/msm8974/legacy/alsa_sound/audio_hw_hal.cpp:         #ifdef QCOM_FM_ENABLED
x2: hardware/qcom/audio-caf/msm8974/legacy/alsa_sound/AudioHardwareALSA.h:      #ifdef QCOM_FM_ENABLED
x8: hardware/qcom/audio-caf/msm8974/legacy/alsa_sound/AudioHardwareALSA.cpp:    #ifdef QCOM_FM_ENABLED
x8: hardware/qcom/audio-caf/msm8974/policy_hal/AudioPolicyManager.cpp:          #ifdef AUDIO_EXTN_FM_ENABLED
x6: hardware/qcom/audio-caf/msm8974/hal/msm8974/platform.c:                     #ifdef FM_ENABLED
x2: hardware/qcom/audio-caf/msm8974/hal/audio_extn/audio_extn.c:                #ifndef FM_ENABLED
    hardware/qcom/audio-caf/msm8974/hal/audio_extn/fm.c:                        #ifdef FM_ENABLED
hardware/qcom/audio-caf/msm8974/policy_hal/Android.mk:LOCAL_CFLAGS += -DAUDIO_EXTN_FM_ENABLED
hardware/qcom/audio-caf/msm8974/hal/Android.mk:    LOCAL_CFLAGS += -DFM_ENABLED

And similar for:

    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/ALSAStreamOps.cpp:        #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/AudioStreamInALSA.cpp:    #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/ALSADevice.cpp:           #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/audio_hw_hal.cpp:         #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/AudioHardwareALSA.h:      #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/AudioHardwareALSA.cpp:    #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/AudioPolicyManagerALSA.cpp:#ifdef QCOM_FM_ENABLED
hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/Android.mk:ifeq ($(strip $(QCOM_FM_ENABLED)),true)
hardware/qcom/audio-caf/msm8960/legacy/alsa_sound/Android.mk:    common_cflags += -DQCOM_FM_ENABLED

    hardware/qcom/audio-caf/msm8226/legacy/alsa_sound/ALSAStreamOps.cpp:        #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8226/legacy/alsa_sound/AudioStreamInALSA.cpp:    #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8226/legacy/alsa_sound/alsa_default.cpp:         #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8226/legacy/alsa_sound/audio_hw_hal.cpp:         #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8226/legacy/alsa_sound/AudioHardwareALSA.h:      #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8226/legacy/alsa_sound/AudioHardwareALSA.cpp:    #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio-caf/msm8226/policy_hal/AudioPolicyManager.cpp:          #ifdef AUDIO_EXTN_FM_ENABLED
    hardware/qcom/audio-caf/msm8226/hal/msm8974/platform.c:                     #ifdef FM_ENABLED
    hardware/qcom/audio-caf/msm8226/hal/audio_extn/audio_extn.c:                #ifndef FM_ENABLED
    hardware/qcom/audio-caf/msm8226/hal/audio_extn/audio_extn.h:                #ifndef FM_ENABLED
    hardware/qcom/audio-caf/msm8226/hal/audio_extn/fm.c:                        #ifdef FM_ENABLED
hardware/qcom/audio-caf/msm8226/policy_hal/Android.mk:LOCAL_CFLAGS += -DAUDIO_EXTN_FM_ENABLED
hardware/qcom/audio-caf/msm8226/hal/Android.mk:    LOCAL_CFLAGS += -DFM_ENABLED

    hardware/qcom/audio-caf/msm8916/policy_hal/AudioPolicyManager.cpp:          #ifdef AUDIO_EXTN_FM_ENABLED
    hardware/qcom/audio-caf/msm8916/hal/audio_extn/audio_extn.c:                #ifndef FM_ENABLED
    hardware/qcom/audio-caf/msm8916/hal/audio_extn/audio_extn.h:                #ifndef FM_ENABLED
    hardware/qcom/audio-caf/msm8916/hal/audio_extn/fm.c:                        #ifdef FM_ENABLED
hardware/qcom/audio-caf/msm8916/policy_hal/Android.mk:LOCAL_CFLAGS += -DAUDIO_EXTN_FM_ENABLED
hardware/qcom/audio-caf/msm8916/hal/Android.mk:    LOCAL_CFLAGS += -DFM_ENABLED

    hardware/qcom/audio/default/legacy/alsa_sound/ALSAStreamOps.cpp:            #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio/default/legacy/alsa_sound/AudioStreamInALSA.cpp:        #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio/default/legacy/alsa_sound/alsa_default.cpp:             #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio/default/legacy/alsa_sound/audio_hw_hal.cpp:             #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio/default/legacy/alsa_sound/AudioHardwareALSA.h:          #ifdef QCOM_FM_ENABLED
    hardware/qcom/audio/default/legacy/alsa_sound/AudioHardwareALSA.cpp:        #ifdef QCOM_FM_ENABLED




qall shell "getprop ro.product.device ; grep -R -l -e AUDIO_DEVICE_OUT_FM -e AUDIO_DEVICE_IN_FM -e fm_volume -e handle_fm -e FmVolume -e HandleFm ??????/lib/"
evita
    system/lib/hw/audio.primary.msm8960.so
    system/lib/libaudiopolicymanagerdefault.so
falcon_umts
    system/lib/hw/audio.primary.msm8226.so
    system/lib/libaudiopolicymanagerdefault.so
honami                                                  # honami Z1 only has: AUDIO_DEVICE_OUT_FM   AUDIO_DEVICE_IN_FM
    system/lib/libaudiopolicymanagerdefault.so
htc_m8
    system/lib/hw/audio.primary.msm8974.so
    system/lib/libaudiopolicymanagerdefault.so


Same for AUDIO_DEVICE_IN_FM:
qall shell "getprop ro.product.device ; grep -R -l -e AUDIO_DEVICE_OUT_FM ??????/lib/"
evita
    system/lib/libaudiopolicymanagerdefault.so
falcon_umts
    system/lib/libaudiopolicymanagerdefault.so
honami
    system/lib/libaudiopolicymanagerdefault.so
htc_m8
    system/lib/libaudiopolicymanagerdefault.so

qall shell "getprop ro.product.device ; grep -R -l -e fm_volume ??????/lib/"
evita
    system/lib/hw/audio.primary.msm8960.so
falcon_umts
    system/lib/hw/audio.primary.msm8226.so
    system/lib/libaudiopolicymanagerdefault.so
honami
htc_m8
    system/lib/hw/audio.primary.msm8974.so
    system/lib/libaudiopolicymanagerdefault.so

qall shell "getprop ro.product.device ; grep -R -l -e handle_fm ??????/lib/"
evita
falcon_umts
    system/lib/hw/audio.primary.msm8226.so
    system/lib/libaudiopolicymanagerdefault.so
honami
htc_m8
    system/lib/hw/audio.primary.msm8974.so
    system/lib/libaudiopolicymanagerdefault.so

qall shell "getprop ro.product.device ; grep -R -l -e FmVolume ??????/lib/"
evita
    system/lib/hw/audio.primary.msm8960.so
falcon_umts
honami
htc_m8

qall shell "getprop ro.product.device ; grep -R -l -e HandleFm ??????/lib/"
evita
falcon_umts
honami
htc_m8




qall shell " echo ; getprop ro.product.device ; strings -n9 system/lib/libaud* | grep -i fm | grep -v -e Fmke -e Fmt"

evita
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_IN_FM_TUNER

falcon_umts
fm_volume
handle_fm
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_OUT_FM_TX
AUDIO_DEVICE_IN_FM_TUNER
AUDIO_DEVICE_IN_FM_RX
AUDIO_DEVICE_IN_FM_RX_A2DP

honami
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_IN_FM_TUNER

htc_m8
fm_volume
handle_fm
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_OUT_FM_TX
AUDIO_DEVICE_IN_FM_TUNER
AUDIO_DEVICE_IN_FM_RX
AUDIO_DEVICE_IN_FM_RX_A2DP


qall shell " echo ; getprop ro.product.device ; grep -i -e fm_ -e _fm -e handlefm -e fmvolume  system/lib/libaud*"

evita
Binary file system/lib/libaudiopolicymanagerdefault.so matches

falcon_umts
Binary file system/lib/libaudiopolicymanagerdefault.so matches

honami
Binary file system/lib/libaudiopolicymanagerdefault.so matches

htc_m8
Binary file system/lib/libaudiopolicymanagerdefault.so matches


qall shell " echo ; getprop ro.product.device ; strings  system/lib/hw/* | grep -i fm | grep -i -v -e cfm -e fmt"

grep -i -e "-fm" -e "fm-" -e _fm -e fm_ -e startFm -e "FM volume" -e "FM RX Volume" -e "FM start" - "stop FM" -e "restart FM" -e "FM usecase" -e FmVolume -e "FM Digital" -e "Play FM" -e "FM REC" -e "Capture FM" -e "FM A2DP REC" -e "Capture A2DP FM" -e "Internal FM RX Volume"

#qall shell echo && getprop ro.product.device && grep -Ri system/lib/hw/* -e "-fm" -e "fm-" -e _fm -e fm_ -e startFm -e "FM volume" -e "FM RX Volume" -e "FM start" - "stop FM" -e "restart FM" -e "FM usecase" -e FmVolume -e "FM Digital" -e "Play FM" -e "FM REC" -e "Capture FM" -e "FM A2DP REC" -e "Capture A2DP FM" -e "Internal FM RX Volume"

grep -Ri /system/lib/hw/* -e "-fm" -e "fm-" -e _fm -e fm_ -e startFm -e 'FM volume' -e 'FM RX Volume' -e "FM start" -e "stop FM" -e "restart FM" -e "FM usecase" -e FmVolume -e "FM Digital" -e "Play FM" -e "FM REC" -e "Capture FM" -e "FM A2DP REC" -e "Capture A2DP FM" -e "Internal FM RX Volume"

a strings /system/lib/hw/* | grep -i -e "-fm" -e "fm-" -e _fm -e fm_ -e startFm -e 'FM volume' -e 'FM RX Volume' -e "FM start" -e "stop FM" -e "restart FM" -e "FM usecase" -e FmVolume -e "FM Digital" -e "Play FM" -e "FM REC" -e "Capture FM" -e "FM A2DP REC" -e "Capture A2DP FM" -e "Internal FM RX Volume"

    audio_extn_fm_set_parameters
    play-fm
    fm-virtual-record
    capture-fm
    transmission-fm
    audio_hw_fm
    %s: Setting FM volume to %d 
    Internal FM RX Volume
    %s: Problem in FM start: status(%d)
    sound card is OFFLINE, stop FM
    sound card is ONLINE, restart FM
    handle_fm
    %s: FM usecase
    fm_volume
    %s: error in retrieving fm volume
    %s: set_fm_volume usecase
    fm_set_volume
    fm_stop
    audio_extn_fm_set_parameters
    fm_start
    SND_DEVICE_OUT_TRANSMISSION_FM
    SND_DEVICE_IN_CAPTURE_FM

    _ZN20android_audio_legacy10ALSADevice11setFmVolumeEi
    _ZN20android_audio_legacy10ALSADevice7startFmEPNS_13alsa_handle_tE
    FM Digital Radio
    Play FM
    FM REC
    Capture FM
    fm_volume
    set Fm Volume(%f) under 0.0, assuming 0.0
    set Fm Volume(%f) over 1.0, assuming 1.0
    FM A2DP REC
    Capture A2DP FM
    Internal FM RX Volume
    startFm: could not open PCM device
    startFm: setHardwareParams failed
    startFm: setSoftwareParams failed
    startFm: SNDRV_PCM_IOCTL_START failed
    startFm: pcm_prepare failed



a strings /system/lib/libaud* | grep -i -e "-fm" -e "fm-" -e _fm -e fm_ -e startFm -e 'FM volume' -e 'FM RX Volume' -e "FM start" -e "stop FM" -e "restart FM" -e "FM usecase" -e FmVolume -e "FM Digital" -e "Play FM" -e "FM REC" -e "Capture FM" -e "FM A2DP REC" -e "Capture A2DP FM" -e "Internal FM RX Volume" | grep -i -v -e cfm_ -e fmul -e fmt

xz1 / oxl:
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_IN_FM_TUNER

mog / om8:
fm_volume
handle_fm
AUDIO_DEVICE_OUT_FM
AUDIO_DEVICE_OUT_FM_TX
AUDIO_DEVICE_IN_FM_TUNER
AUDIO_DEVICE_IN_FM_RX
AUDIO_DEVICE_IN_FM_RX_A2DP


a strings /system/lib/hw/audio.primary.* | grep -i -e "-fm" -e "fm-" -e _fm -e fm_ -e startFm -e 'FM volume' -e 'FM RX Volume' -e "FM start" -e "stop FM" -e "restart FM" -e "FM usecase" -e FmVolume -e "FM Digital" -e "Play FM" -e "FM REC" -e "Capture FM" -e "FM A2DP REC" -e "Capture A2DP FM" -e "Internal FM RX Volume" | grep -i -v -e cfm_ -e fmul -e fmt

xz1
play-fm
fm-virtual-record
capture-fm
transmission-fm
SND_DEVICE_OUT_TRANSMISSION_FM
SND_DEVICE_IN_CAPTURE_FM

oxl
_ZN20android_audio_legacy10ALSADevice11setFmVolumeEi
_ZN20android_audio_legacy10ALSADevice7startFmEPNS_13alsa_handle_tE
FM Digital Radio
Play FM
FM REC
Capture FM
fm_volume
set Fm Volume(%f) under 0.0, assuming 0.0
set Fm Volume(%f) over 1.0, assuming 1.0
FM A2DP REC
Capture A2DP FM
Internal FM RX Volume
startFm: could not open PCM device
startFm: setHardwareParams failed
startFm: setSoftwareParams failed
startFm: SNDRV_PCM_IOCTL_START failed
startFm: pcm_prepare failed

mog / om8
audio_extn_fm_set_parameters
play-fm
fm-virtual-record
capture-fm
transmission-fm
audio_hw_fm
%s: Setting FM volume to %d 
Internal FM RX Volume
%s: Problem in FM start: status(%d)
sound card is OFFLINE, stop FM
sound card is ONLINE, restart FM
handle_fm
%s: FM usecase
fm_volume
%s: error in retrieving fm volume
%s: set_fm_volume usecase
fm_set_volume
fm_stop
audio_extn_fm_set_parameters
fm_start
SND_DEVICE_OUT_TRANSMISSION_FM
SND_DEVICE_IN_CAPTURE_FM

*/
  int analog_output_mode = 0;
  int analog_and_digital = 0;//1;                                       // Not working on GS1 and GS2/NO1
  int hostless_samplerate = 48000;//44100;
  int hostless_channels = 2;

  int audio_ana_vol_set (int vol) {                                     // Volume for analog mode, via ALSA or codec_reg
    logd ("audio_ana_vol_set vol: %d  curr_chass_plug_aud: %s", vol, curr_chass_plug_aud);
    int ret = -1;
    switch (curr_chass_plug_aud_int) {
      case PLUG_AUD_CUS: if (1) ret = cus_ana_vol_set (vol);break;
      case PLUG_AUD_GS1: if (1) ret = gs1_ana_vol_set (vol);break;
      case PLUG_AUD_GS2: if (1) ret = gs2_ana_vol_set (vol);break;
      case PLUG_AUD_GS3: if (1) ret = gs3_ana_vol_set (vol);break;
      case PLUG_AUD_QCV: if (1) ret = qcv_ana_vol_set (vol);break;
      case PLUG_AUD_OM7: if (1) ret = om7_ana_vol_set (vol);break;
      case PLUG_AUD_LG2: if (1) ret = lg2_ana_vol_set (vol);break;
      case PLUG_AUD_XZ2: if (1) ret = xz2_ana_vol_set (vol);break;
    }
    return (ret);
  }
/*
  void tuner_mute_set (int mute) {                                      // Mute tuner; here to avoid noise
    if (tnr_funcs == NULL)
      return;
    if (mute == 0) {
      curr_tuner_mute_int = tnr_funcs->tnr_tuner_mute_sg (curr_tuner_mute_int = 0);                           // UnMute
      if (curr_tuner_mute_int == 0)
        strlcpy (curr_tuner_mute, "Unmute",  sizeof (curr_tuner_mute));
    }
    else {
      curr_tuner_mute_int = tnr_funcs->tnr_tuner_mute_sg (curr_tuner_mute_int = 1);                           // Mute
      if (curr_tuner_mute_int == 1)
        strlcpy (curr_tuner_mute, "Mute",  sizeof (curr_tuner_mute));
    }
  }
*/
  int audio_state_start () {
    int ret = -1;
    //tuner_mute_set (1);                                                 // Mute to avoid noise
    switch (curr_chass_plug_aud_int) {
      case PLUG_AUD_CUS: if (analog_output_mode) ret = cus_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = cus_dig_audio_state_start ();  break;
      case PLUG_AUD_GS1: if (analog_output_mode) ret = gs1_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = gs1_dig_audio_state_start ();  break;
      case PLUG_AUD_GS2: if (analog_output_mode) ret = gs2_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = gs2_dig_audio_state_start ();  break;
      case PLUG_AUD_GS3: if (analog_output_mode) ret = gs3_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = gs3_dig_audio_state_start ();  break;
      case PLUG_AUD_QCV: if (analog_output_mode) ret = qcv_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = qcv_dig_audio_state_start ();  break;
      case PLUG_AUD_OM7: if (analog_output_mode) ret = om7_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = om7_dig_audio_state_start ();  break;
      case PLUG_AUD_LG2: if (analog_output_mode) ret = lg2_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = lg2_dig_audio_state_start ();  break;
      case PLUG_AUD_XZ2: if (analog_output_mode) ret = xz2_ana_audio_state_start ();  if (! analog_output_mode || analog_and_digital) ret = xz2_dig_audio_state_start ();  break;
    }
    //tuner_mute_set (0);                                                 // Unmute to play audio
    return (ret);
  }

  int audio_state_stop () {
    int ret = -1;
    //tuner_mute_set (1);                                                 // Mute to avoid noise (Audio stopping so leave muted; don't need to unmute at end)
    switch (curr_chass_plug_aud_int) {
      case PLUG_AUD_CUS: if (analog_output_mode) ret = cus_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = cus_dig_audio_state_stop ();  break;
      case PLUG_AUD_GS1: if (analog_output_mode) ret = gs1_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = gs1_dig_audio_state_stop ();  break;
      case PLUG_AUD_GS2: if (analog_output_mode) ret = gs2_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = gs2_dig_audio_state_stop ();  break;
      case PLUG_AUD_GS3: if (analog_output_mode) ret = gs3_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = gs3_dig_audio_state_stop ();  break;
      case PLUG_AUD_QCV: if (analog_output_mode) ret = qcv_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = qcv_dig_audio_state_stop ();  break;
      case PLUG_AUD_OM7: if (analog_output_mode) ret = om7_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = om7_dig_audio_state_stop ();  break;
      case PLUG_AUD_LG2: if (analog_output_mode) ret = lg2_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = lg2_dig_audio_state_stop ();  break;
      case PLUG_AUD_XZ2: if (analog_output_mode) ret = xz2_ana_audio_state_stop ();  if (! analog_output_mode || analog_and_digital) ret = xz2_dig_audio_state_stop ();  break;
    }
    return (ret);
  }

  char * audio_state_set (char * new_audio_state) {
    logd ("audio_state_set new_audio_state: %d  curr_chass_plug_aud: %s", new_audio_state, curr_chass_plug_aud);
    if (! strcmp (new_audio_state, "Start"))                            // Start
      logd ("audio_state Start: %d", audio_state_start ());
    else if (! strcmp (new_audio_state, "Stop"))                        // Stop and Pause are the same
      logd ("audio_state Stop: %d", audio_state_stop ());
    else if (! strcmp (new_audio_state, "Pause"))
      logd ("audio_state Stop: %d", audio_state_stop ());
    else
      logd ("Unknown new_audio_state: %s", new_audio_state);

    strlcpy (curr_audio_state, new_audio_state, sizeof (curr_audio_state));
    return (curr_audio_state);
  }


    //

  char * audio_mode_set (char * new_audio_mode) {
    logd ("audio_mode_set new_audio_mode: %d  curr_chass_plug_aud: %s", new_audio_mode, curr_chass_plug_aud);
    int new_analog_output_mode = -1;
    if (! strcmp (new_audio_mode, "Digital")) {
      logd ("audio_mode_set Digital curr_audio_state: %s", curr_audio_state);
      new_analog_output_mode = 0;
    }
    else if (! strcmp (new_audio_mode, "Analog")) {
      logd ("audio_mode_set Analog curr_audio_state: %s", curr_audio_state);
      new_analog_output_mode = 1;
    }
    else {
      logd ("Unknown audio_mode_set: %s", new_audio_mode);
      return (curr_audio_mode);
    }

    if (! strcmp (curr_audio_state, "Start"))
      audio_state_stop ();                                              // Turn off current

    analog_output_mode = new_analog_output_mode;                        // Set new mode

    if (! strcmp (curr_audio_state, "Start"))
      audio_state_start ();                                             // Turn on new

    strlcpy (curr_audio_mode, new_audio_mode, sizeof (curr_audio_mode));
    return (curr_audio_mode);
  }


    // Custom:

  int cus_dig_audio_state_start () {

    return (0);
  }

  int cus_dig_audio_state_stop () {

    return (0);
  }

  int cus_ana_audio_state_start () {

    return (0);
  }

  int cus_ana_audio_state_stop () {

    return (0);
  }

  int cus_ana_vol_set (int vol) {

    return (0);
  }


    // GS1:

    //RADIO_L -> IN2LN / DMICDAT1
    //RADIO_R -> IN2RN / DMICDAT2

  char * gs1_cdc = "/sys/kernel/debug/asoc/smdkc110/wm8994-samsung-codec.4-001a/codec_reg";
  int gs1_cdc_flags = O_RDWR;

  int gs1_dig_audio_state_start () {
    int fd = -1;                                                        // New file open

    file_write_many (gs1_cdc, & fd, "0300 4010", 9, gs1_cdc_flags);     // Default: C010    ?? Magic value

                                                                                            // ?? Digital volume at 0400 and 0401       == 8 = 3db ?
                                                                                            // 0400: 00c0
                                                                                            // 0401: 00c0
                                                                                            // 440/450 -> 0 to disable dynamic range control
                                                                                            // Digital record (AIFx ADC output path) volume defaults = 0 dB
    file_write_many (gs1_cdc, & fd, "0410 0000", 9, gs1_cdc_flags);     // Default: 2800    // 0410 = 1800 to remove dc offset w/ hifi, cf = 4 hz at 44K, 3.7 @ 44.1

    file_write_many (gs1_cdc, & fd, "0002 63a0", 9, gs1_cdc_flags);     //                  // Power Management (2) = TSHUT_ENA    | TSHUT _OPDIS | MIXINL_ENA    | MIXINR_ENA   | IN2L_ENA | IN2R_ENA

    file_write_many (gs1_cdc, & fd, "0004 0303", 9, gs1_cdc_flags);     // Default: 3003    // Power Management (4) =                               AIF1ADC1L_ENA | AIF1ADCR_ENA | ADCL_ENA | ADCR_ENA
  //file_write_many (gs1_cdc, & fd, "0004 3303", 9, gs1_cdc_flags);     //                  // Power Management (4) = AIF2ADCL_ENA | AIF2ADCR_ENA | AIF1ADC1L_ENA | AIF1ADCR_ENA | ADCL_ENA | ADCR_ENA
                                                                        // 0301 -> 4001 for loopback

    file_write_many (gs1_cdc, & fd, "0018 0080", 9, gs1_cdc_flags);     // Default: 008b    // Left  Line Input 1&2 Volume = IN1L_MUTE                  80 = mute
    file_write_many (gs1_cdc, & fd, "0019 014d", 9, gs1_cdc_flags);     // Default: 004b    // Left  Line Input 3&4 Volume = IN2_VU | IN2L_ZC | 0x0d    Raise volume
    file_write_many (gs1_cdc, & fd, "001a 0080", 9, gs1_cdc_flags);     // Default: 008b    // Right Line Input 1&2 Volume = IN1R_MUTE                  80 = mute
    file_write_many (gs1_cdc, & fd, "001b 014d", 9, gs1_cdc_flags);     // Default: 004b    // Right Line Input 3&4 Volume = IN2_VU | IN2R_ZC | 0x0d    Raise volume

    file_write_many (gs1_cdc, & fd, "0028 0044", 9, gs1_cdc_flags);
    file_write_many (gs1_cdc, & fd, "0029 0100", 9, gs1_cdc_flags);
    file_write_many (gs1_cdc, & fd, "002a 0100", 9, gs1_cdc_flags);

    file_write_many (gs1_cdc, & fd, "0606 0002", 9, gs1_cdc_flags);     // Default: 0000    // AIF1 ADC1 Left  Mixer Routing = ADC1L_TO_AIF1ADC1L
    file_write_many (gs1_cdc, & fd, "0607 0002", 9, gs1_cdc_flags);     // Default: 0000    // AIF1 ADC1 Right Mixer Routing = ADC1R_TO_AIF1ADC1R

    close (fd);                                                         // Close codec_reg file

    return (0);
  }

  int gs1_dig_audio_state_stop () {                                     // Don't need to do anything; Audio HALs will take care of reprogramming.

    return (0);
  }


  //String hex_get ()

  int gs1_ana_vol_set (int vol) {
    //alsa_long_set ("Playback Volume", vol / 1040); // Range 0 - 63    // Doesn't work
    //return (vol);

    int alsa_vol = vol / 4369; // Range 0 - 15)

    char itoa_ret [MAX_ITOA_SIZE] = {0};
    itoa (alsa_vol, itoa_ret, 16);

    char echo1 [DEF_BUF] = "0019 014";
    char echo2 [DEF_BUF] = "001b 014";
    strlcat (echo1, itoa_ret, sizeof (echo1));
    strlcat (echo2, itoa_ret, sizeof (echo1));

    logd ("Setting codec_reg alsa_vol: %d  echo1: %s  echo2: %s", alsa_vol, echo1, echo2);

    int fd = -1;                                                        // New file open
    file_write_many (gs1_cdc, & fd, echo1, strlen (echo1), gs1_cdc_flags);
    file_write_many (gs1_cdc, & fd, echo2, strlen (echo1), gs1_cdc_flags);

    //file_write_many (gs1_cdc, & fd, "0019 014d", 9, gs1_cdc_flags);   // Default: 004b    // Left  Line Input 3&4 Volume = IN2_VU | IN2L_ZC | 0x0d    Raise volume
    //file_write_many (gs1_cdc, & fd, "001b 014d", 9, gs1_cdc_flags);   // Default: 004b    // Right Line Input 3&4 Volume = IN2_VU | IN2R_ZC | 0x0d    Raise volume

    close (fd);                                                         // Close codec_reg file
  }

  int gs1_ana_audio_state_start () {
    alsa_enum_set ("Codec Status", 2);                                  // 2 = "FMR_FLAG_CLEAR"
    alsa_enum_set ("FM Radio Path", 2);                                 // 2 = "FMR_HP"
    alsa_enum_set ("Playback Path", 3);                                 // 3 = "HP"
    return (0);
  }

  int gs1_ana_audio_state_stop () {
    alsa_enum_set ("Codec Status", 2);                                  // 2 = "FMR_FLAG_CLEAR"
    return (0);
  }

    // GS2 / Note1:
    // C1YMU823 / MC-1N2
    // /sys/kernel/debug/asoc/U1-YMU823/mc1n2.6-003a/codec_reg

  int gs2_dig_audio_state_start () {

        // Mic2 off:
    alsa_bool_set ("ADCL MIXER Mic2 Switch", 1);    // !!  Must first switch ON, then OFF !!
    alsa_bool_set ("ADCR MIXER Mic2 Switch", 1);
    alsa_bool_set ("ADCL MIXER Mic2 Switch", 0);
    alsa_bool_set ("ADCR MIXER Mic2 Switch", 0);

        // Mic1 off:
    alsa_bool_set ("ADCL MIXER Mic1 Switch", 1);    // !!  Must first switch ON, then OFF for Mic1 when using CAMCORDER !!
    alsa_bool_set ("ADCR MIXER Mic1 Switch", 1);
    alsa_bool_set ("ADCL MIXER Mic1 Switch", 0);
    alsa_bool_set ("ADCR MIXER Mic1 Switch", 0);

        // Line on:
    alsa_bool_set ("ADCL MIXER Line Switch", 0);    // !!  Must first switch OFF, then ON !!
    alsa_bool_set ("ADCR MIXER Line Switch", 0);
    alsa_bool_set ("ADCL MIXER Line Switch", 1);
    alsa_bool_set ("ADCR MIXER Line Switch", 1);

        // Volume
    alsa_long_set ("AD Analog Volume", 22);
    return (0);
  }

  int gs2_dig_audio_state_stop () {

        // Line off:
    alsa_bool_set ("ADCL MIXER Line Switch", 0);
    alsa_bool_set ("ADCR MIXER Line Switch", 0);
    return (0);
  }

  int gs2_ana_vol_set (int vol) {
    alsa_long_set ("Line Bypass Playback Volume", vol / 2114); // Range 0 - 31
    return (vol);
  }


  int gs2_ana_audio_state_start () {
//alsa_long_set ("Line Bypass Playback Volume", 20);//25);
//alsa_long_set ("Headphone Playback Volume", 10);//15);
    alsa_bool_set ("Line Bypass Playback Switch", 1);
alsa_bool_set ("Headphone Playback Switch", 1);
    //alsa_bool_set ("Speaker Playback Switch", 0);
    //alsa_bool_set ("SPL MIXER Line Switch", 0);
    alsa_bool_set ("HPR MIXER LineR Switch", 1);
    alsa_bool_set ("HPL MIXER Line Switch", 1);
    return (0);
  }
  int gs2_ana_audio_state_stop () {
    //alsa_long_set ("Line Bypass Playback Volume", 0);
    alsa_bool_set ("Line Bypass Playback Switch", 0);
    //alsa_bool_set ("SPL MIXER Line Switch", 0);
    alsa_bool_set ("HPR MIXER LineR Switch", 0);
    alsa_bool_set ("HPL MIXER Line Switch", 0);
    return (0);
  }


    // GT-I9300 Galaxy S3 & GT-N7100 Note2:

  #define NOTE2_MIXINR_IN2R_Switch_HACK
  #ifdef NOTE2_MIXINR_IN2R_Switch_HACK

  void NOTE2_MIXINR_IN2R_Switch_hack () {
    // PROBLEM 2:
    // Note2 problem is that FM Right (IN2RN) is very low, compared to FM Left:
    // FM Left     IN2RP       OK
    // FM Right    IN2RN       Very Low
    char * codec_reg_omni     = "/sys/kernel/debug/asoc/Midas_WM1811/wm8994-codec/codec_reg";   // GS3 CM12, Omni Kitkat.  OmniROM 9300 also has codec_reg_sa44, but read-only.
    char * codec_reg_cm11     = "/sys/kernel/debug/asoc/T0_WM1811/wm8994-codec/codec_reg";      // NO2 CM12, GS3/NO2 CM11
    char * codec_reg_sa44     = "/sys/devices/platform/soc-audio/WM8994 AIF1/codec_reg";        // Only access on stock, additional on CM11 unofficial for N7100    // Read-only on CM12
    char * codec_reg_sa44_esc = "/sys/devices/platform/soc-audio/WM8994\\ AIF1/codec_reg";      // Same, Space Escaped

    //if (! no2_get ())
    //  return (0);

    int gs3_cdc_flags = O_RDWR;//O_WRONLY;
    char gs3_cdc [DEF_BUF] = "";        // = "/sys/kernel/debug/asoc/T0_WM1811/wm8994-codec/codec_reg";    // Space Escaped
    if (file_get (codec_reg_omni))
      strlcpy (gs3_cdc, codec_reg_omni,       sizeof (gs3_cdc));
    else if (file_get (codec_reg_cm11))
      strlcpy (gs3_cdc, codec_reg_cm11,       sizeof (gs3_cdc));
    else if (file_get (codec_reg_sa44))
      strlcpy (gs3_cdc, codec_reg_sa44_esc,   sizeof (gs3_cdc));

    int fd = -1;                                                        // New file open

    // This substitutes for:    // alsa_bool_set ("MIXINR IN2R Switch", 1);
    file_write_many (gs3_cdc, & fd, "0002 2320", 9, gs3_cdc_flags);     // Power Management:    b               b               b               b
                                                                            //                  -               0:TSHUT_ENA     1:TSHUT_OPDIS   -
                                                                            //                  0:OPCLK_ENA     -               1:MIXINL_ENA    1:MIXINR_ENA
                                                                            //                  0:IN2L_ENA      0:IN1L_ENA      1:IN2R_ENA      0:IN1R_ENA
                                                                            //                  -               -               -               -
/*
    file_write_many (gs3_cdc, & fd, "0018 0116", 9, gs3_cdc_flags);     // Left Line Input 1 Volume:            b               b               b
                                                                            //                  -               -               -               -
                                                                            //                  -               -               -               1:IN1_VU
                                                                            //                  0:IN1L_MUTE     0:IN1L_ZC       -               1:IN1L_VOL4
                                                                            //                  0:IN1L_VOL3     1:IN1L_VOL2     1:IN1L_VOL1     0:IN1L_VOL0
*/
    close (fd);                                                         // Close codec_reg file

  }
  #endif    // #ifdef NOTE2_MIXINR_IN2R_Switch_HACK


    // GS3 Analog: GS3 schematic:
        /*
    U605:   WM1811ECS

    FM Left     IN2RP
    FM Right    IN2RN

    IN1LP   ->  MAIN_MIC_N                                              IN!L = Main microphone mono differential
    IN1LN   ->  MAIN_MIC_P
    IN1RP   ->  SUB_MIC_N                                               IN1R = Sub  microphone mono differential
    IN1RN   ->  SUB_MIC_P

    MICBIAS1    NC
    MICBIAS2->  EAR_MICBIAS_LDO_2.8V
    MICDET  ->  EAR_ADC_3.5 

    IN2LP   ->  EAR_MIC_N                                               IN2L = External wired headset Ear microphone mono differential
    IN2LN   ->  EAR_MIC_P
    IN2RP   ->  IN2RP                   (1811 pin 38) FM Left   C649    IN2R = FM stereo single sided
    IN2RN   ->  IN2RN                   (1811 pin 37) FM Right  C650

    2*SPKOUTLN tied to 2*SPKOUTRN   -> SPK_OUT_L_N  = Main Speaker N    SPKOUT = Back main ring/music speaker mono differential
    2*SPKOUTLP tied to 2*SPKOUTRP   -> SPK_OUT_L_P  = Main Speaker P

    HPOUT1R -> EAROUT_R                                                 HPOUT1 = External wired headset stereo single sided
    HPOUT1L -> EAROUT_L
    HPOUT1FB-> EAROUT_FB = Gnd

    HPOUT2N ->  RVC_P                                                   HPOUT2 = Phone receiver mono differential
    HPOUT2P ->  RVC_N

    LINEOUT1N   NC              ?? For Digital Dock ?
    LINEOUT1P   NC              ?? For Digital Dock ?

    LINEOUT2N   -> VPS_L        ?? Analog Dock Left
    LINEOUT2P   -> VPS_R        ?? Analog Dock Right
    LINEOUTFB   Cap Ground                                              */



    // PROBLEM 1:
        // We have to jump through some hoops on GS3/Note2 to get both Left and Right digitized; Have to go through "RXVOICE", LOL. Despite name of "voice", audio quality seems fine.

    // Thus MIXINL      = FM Left               = FM Left  channel
        // FM Left IN2RP -> MIXINL via RXVOICE:
        // RXVOICE = IN2RP (FM Left) - IN2LP (EAR_MIC_N)
        // -> MIXINL / MIXINR:     6 steps:    -12, -9, -6, -3, 0, +3, +6

    // Thus MIXINR      = (0 - FM Right)        = FM Right channel, but negative, or 180 degrees out of phase
        // FM Right IN2RN -> MIXINR via IN2R PGA:
        // IN2R = -16.5 to +30 * ( 0/IN2RP (FM Left) - 0/IN2RN (FM Right) )
        // -> MIXINR:  0, +30      = -16.5 to +30 or  13.5 to +60                  */

  int gs3_dig_audio_state_start () {
    // MIXIN Right:
    alsa_bool_set ("MIXINR IN2R Switch", 1);                            // NEED Hack to substitute for this for some NO2 ROMs !!     Source is IN2RP                    (FM Left) - IN2RN (FM Right)
    alsa_bool_set ("IN2R PGA IN2RP Switch", 0);                         // Disable IN2RP (FM Left) or would be FM Left -         FM Right instead of desired    (0 - FM Right)
    //alsa_bool_set ("IN2R PGA IN2RN Switch", 1);                       // On for -FM Right
    //alsa_bool_set ("IN2R Switch", 1);                                 // Added because was off on stock XXEMA2, but same as power on default.
    alsa_long_set ("MIXINR IN2R Volume", 0);                             // +0         Default = 1
    alsa_long_set ("IN2R Volume", 15);                                   // 0-31 : +6        15 is actually max, not 31  Total right: 0 + 6 = +6 db      // NOTE2 has STUCK low bug  Default = 11

    alsa_bool_set ("MIXINR IN1R Switch", 0);                            // Disable SUB_MIC_N / SUB_MIC_P Camcorder microphone

    // MIXIN Left:
    alsa_long_set ("MIXINL Direct Voice Volume", 7);                     // Source is "Direct/RX Voice" output = IN2RP (FM Left) - IN2LP (0)    Highest (+1) = +6 db on left ?? 7 is louder than alleged "Max" of 6 ?

    alsa_bool_set ("MIXINL IN1L Switch", 0);                            // Disable MAIN_MIC_N / MAIN_MIC_P Main microphone
    alsa_bool_set ("MIXINL IN2L Switch", 0);                            // Disable EAR_MIC_N / EAR_MIC_P Wired headset microphone
    //alsa_long_set ("MIXINL IN2L Volume", 0);                           // Volume irrelevant as we disabled by switch

    // AIF1 ADC 1:
    alsa_bool_set ("AIF1ADC1 HPF Switch", 0);                           // "AIF1ADC1 HPF Mode" = 0 = HiFi
    alsa_enum_set ("AIF1ADCL Source", 0);                               // Left = Left
    alsa_enum_set ("AIF1ADCR Source", 1);                               // Right= Right     !! Set to Left (0) on recent KK/LOL ROMs January 2015, perhaps due to stereo recording of mono microphone
    //alsa_long_set ("AIF1ADC1 Volume", 96);//104);         // Raise ADC gain from 96

  #ifdef NOTE2_MIXINR_IN2R_Switch_HACK
    NOTE2_MIXINR_IN2R_Switch_hack ();
  #endif    // #ifdef NOTE2_MIXINR_IN2R_Switch_HACK

    return (0);
  }

  int gs3_dig_audio_state_stop () {                                     // Set back to assumed defaults
    //alsa_bool_set ("MIXINR IN2R Switch", 0);                          // DISABLED: Need this to support digital to analog switch: ??
    return (0);
  }

  int gs3_ana_vol_set (int vol) {
    alsa_long_set ("Headphone Volume", vol / 1200);//1040); // Range 0 - 63
    return (vol);
  }

  int gs3_ana_audio_state_start () {
    alsa_bool_set ("HP Switch", 1);                                     // HP Switch                            = true      Enable Headphone output
    alsa_bool_set ("MIXINL IN2L Switch", 0);                            // !!
        //    FM Left     IN2RP     // !! Backwards !!
        //    FM Right    IN2RN     // !! Backwards !!
    if (analog_and_digital) {
      alsa_bool_set ( "Left Output Mixer IN2LP Switch", 0);
      alsa_bool_set ( "Left Output Mixer IN2LN Switch", 0);
      alsa_bool_set ( "Left Output Mixer IN2RN Switch", 1);                                              //  Left Output Mixer IN2RN Switch      = true      FMLeft /IN2RN enabled on Analog output mixer
      alsa_bool_set ("Right Output Mixer IN2LN Switch", 0);
      alsa_bool_set ("Right Output Mixer IN2RN Switch", 0);
      alsa_bool_set ("Right Output Mixer IN2RP Switch", 1);                                             // Right Output Mixer IN2RP Switch      = true      FMRight/IN2RP enabled on Analog output mixer
      return (0);
    }
    alsa_bool_set ("Left Output Mixer IN2RN Switch", 1);                                              //  Left Output Mixer IN2RN Switch      = true      FMLeft /IN2RN enabled on Analog output mixer
    alsa_bool_set ("Right Output Mixer IN2RN Switch", 1);
    alsa_bool_set ("Right Output Mixer IN2RP Switch", 0);           // !!!! FALSE                                   // Right Output Mixer IN2RP Switch      = true      FMRight/IN2RP enabled on Analog output mixer
    return (0);
  }
  int gs3_ana_audio_state_stop () {
    if (analog_and_digital) {
      alsa_bool_set ( "Left Output Mixer IN2RN Switch", 0);
      alsa_bool_set ("Right Output Mixer IN2RP Switch", 0);
      return (0);
    }
    alsa_bool_set ("Left Output Mixer IN2RN Switch", 0);                                            //  Left Output Mixer IN2RN Switch      = false     FMLeft /IN2RN disabled on Analog output mixer
    alsa_bool_set ("Right Output Mixer IN2RN Switch", 0);
    //alsa_bool_set ("Right Output Mixer IN2RP Switch", 0);                                            // Right Output Mixer IN2RP Switch      = false     FMRight/IN2RP disabled on Analog output mixer
    return (0);
  }


    // Qualcomm:

    #include "acdb.c"

  int qcv_dig_audio_state_start () {
    if (curr_tuner_mode_int) {                                          // If Transmit...
      alsa_bool_set ("SLIMBUS_0_RX Audio Mixer MultiMedia1", 0);        // Wired headset audio off
      alsa_bool_set ("INTERNAL_FM_RX Audio Mixer MultiMedia1", 1);      // Transmit input on MM 1:  <path name=     "deep-buffer-playback transmission-fm">
      alsa_bool_set ("INTERNAL_FM_RX Audio Mixer MultiMedia4", 1);      // Transmit input on MM 4:  <path name="compress-offload-playback transmission-fm">
      alsa_bool_set ("INTERNAL_FM_RX Audio Mixer MultiMedia5", 1);      // Transmit input on MM 5:  <path name=     "low-latency-playback transmission-fm">
      return (0);
    }

    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 0);                   // Turn off microphone path to MM 1
    //alsa_enum_set ("SLIM_0_TX Channels", 1);                          // 2        Set SLIMBus TX channels     to "2"
    //alsa_enum_set ("SLIM_0_TX Channels", 0);
    //alsa_enum_set ("SLIM_0_TX Channels", 1);                          // 2        Set SLIMBus TX channels     to "2"

                // MotoG, Z1 re-enable camcorder microphone sometimes !
    alsa_long_set ("ADC1 Volume", 0);
    alsa_long_set ("ADC2 Volume", 0);
    alsa_long_set ("ADC3 Volume", 0);
    //alsa_long_set ("ADC4 Volume", 0);                                 // ADC4 not used on MOG
    alsa_long_set ("DEC1 Volume", 0);
    alsa_long_set ("DEC2 Volume", 0);                                   // Xperia Z1
    if (! msm8226_get ()) {                                             // If not MotoG msm8226 chipset...
        alsa_long_set ("ADC4 Volume", 0);                               // Xperia Z1
        alsa_long_set ("ADC5 Volume", 0);                               // Xperia Z1
        //alsa_long_set ("ADC6 Volume", 0);
        alsa_long_set ("DEC3 Volume", 0);                               // Xperia Z1
        //alsa_long_set ("DEC4 Volume", 0);
        alsa_long_set ("DEC5 Volume", 0);
        //alsa_long_set ("DEC6 Volume", 0);
        //alsa_long_set ("DEC7 Volume", 0);
        //alsa_long_set ("DEC8 Volume", 0);
        //alsa_long_set ("DEC9 Volume", 0);
      }


    alsa_bool_set ("MultiMedia1 Mixer INTERNAL_FM_TX", 1);              // Internal FM audio source to MM 1
//    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 0);                   // Turn off microphone path to MM 1

    //acdb_disable ();
    return (0);
  }
/* Z1 regular mic:
Input mic:
    ADC2 Volume     = 8     (default 19 ?)
    DEC5 Volume     = 99    (default 84 ?)

Cam mic:
    ADC5 Volume     = 12
    DEC2 Volume     = ??    (default 84 ?)

Output:
    RX1 Digital Volume
    RX2 Digital Volume

a /data/local/tmp/ssd 4 0 | grep -e "ADC2 Volume" -e "DEC5 Volume" -e "RX[12] Digital Volume"

*/
  int qcv_dig_audio_state_stop () {
    if (curr_tuner_mode_int) {                                          // If Transmit...
      alsa_bool_set ("SLIMBUS_0_RX Audio Mixer MultiMedia1", 1);        // Restore Wired headset audio to on, since we turned it off before
      return (0);
    }

    alsa_long_set ("ADC1 Volume", 19);
    alsa_long_set ("ADC2 Volume", 19);
    alsa_long_set ("ADC3 Volume", 19);
    alsa_long_set ("DEC1 Volume", 84);
    alsa_long_set ("DEC2 Volume", 84);
    if (! msm8226_get ()) {
      alsa_long_set ("ADC4 Volume", 19);
      alsa_long_set ("ADC5 Volume", 19);
      alsa_long_set ("DEC3 Volume", 84);
      alsa_long_set ("DEC5 Volume", 84);
    }

    alsa_bool_set ("MultiMedia1 Mixer INTERNAL_FM_TX", 0);              // Internal FM audio source off
    return (0);
  }

  int qcv_ana_vol_set (int vol) {
    //alsa_long_set ("HPHL Volume", vol / 3276); // Range 0 - 20        // 'HPH? Volume' doesn't work so must use Internal FM RX Volume
    //alsa_long_set ("HPHR Volume", vol / 3276); // Range 0 - 20

    int alsa_vol = vol / 48;//32;//16;//8;
    logd ("Setting Internal FM RX Volume to alsa_vol: %d", alsa_vol);
    alsa_long_set ("Internal FM RX Volume", alsa_vol);

    return (vol);
  }

  int qcv_ana_audio_state_start () {                                    // Qualcomm FM has pure digital audio, so this just sets up hostless transfer
    alsa_bool_set ("SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX", 1);        // FM to Slimbus 0
    alsa_bool_set ("SLIMBUS_DL_HL Switch", 1);                          // Enable hostless transfer
                                                                        // Start hostless transfer
    hostless_transfer_start (hostless_samplerate, hostless_channels, "/dev/snd/pcmC0D6c", NULL);
    return (0);
  }

  int qcv_ana_audio_state_stop () {
    hostless_transfer_stop ();

    alsa_bool_set ("SLIMBUS_0_RX Port Mixer INTERNAL_FM_TX", 0);
    alsa_bool_set ("SLIMBUS_DL_HL Switch", 0);

    return (0);
  }


    // HTC One M7:

  int om7_dig_audio_state_start () {
    alsa_bool_set ("MultiMedia1 Mixer PRI_TX", 1);                      // FM Path = MultiMedia1
    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 0);                   // Turn off microphone path
    return (-1);
  }
  int om7_dig_audio_state_stop () {
    alsa_bool_set ("MultiMedia1 Mixer PRI_TX", 0);                      // FM Path = ! MultiMedia1
    return (-1);
  }

  int om7_ana_vol_set (int vol) {
    //alsa_long_set ("Headphone Volume", vol / 1040);                   //DISABLED: Doesn't work        WAS: Range 0 - 63

    int alsa_vol = vol / 1500;   // 0 - 40
    if (alsa_vol <= 0)
      alsa_vol = 0;
    else
      alsa_vol += 40;                                                    // RX3 Digital Volume and RX5 Digital Volume is logarithmic instead of the desired linear
    alsa_long_set ("RX3 Digital Volume", alsa_vol);
    alsa_long_set ("RX5 Digital Volume", alsa_vol);

/*
    int alsa_vol = vol / 8192;//5461; // Range 0 - 12)
    alsa_long_set ("LINEOUT1 Volume", alsa_vol);                     // ?? We could get 24 settings by designating one as low order bit
    alsa_long_set ("LINEOUT3 Volume", alsa_vol);

    alsa_long_set ("LINEOUT2 Volume", alsa_vol);
    alsa_long_set ("LINEOUT4 Volume", alsa_vol);
*/
    return (vol);
  }

  int om7_ana_audio_state_start () {                                    // HTC One M7 has pure digital audio, so this just sets up hostless transfer
//    alsa_long_set ("RX3 Digital Volume", 70);//83);                   // Def: 84/124
    //alsa_long_set ("RX4 Digital Volume", 83);
//    alsa_long_set ("RX5 Digital Volume", 70);//83);
    //alsa_long_set ("RX6 Digital Volume", 83);
    alsa_bool_set ("SLIMBUS_0_RX Port Mixer PRI_TX", 1);                 // Def: 0/1

    hostless_transfer_start (hostless_samplerate, hostless_channels, "/dev/snd/pcmC0D6c", NULL);
    return (0);
  }

  int om7_ana_audio_state_stop () {
    hostless_transfer_stop ();

    alsa_bool_set ("SLIMBUS_0_RX Port Mixer PRI_TX", 0);

    return (0);
  }



    // Xperia Z2/Z3+:

  int xz2_dig_audio_state_start () {                                                 // ??  <path name="capture-fm">
    alsa_bool_set ("AIF1_CAP Mixer SLIM TX7", 1);                       //          Connect Analog Interface 1  to SLIMBus TX 7
    alsa_enum_set ("SLIM TX7 MUX", 9);                                  // DEC2     Connect SLIMBus TX 7 Mux    to 'Decimator 2'
    alsa_enum_set ("DEC2 MUX", 2);                                      // ADC5     Connect Decimator 2         to 'ADC 5'
    alsa_long_set ("DEC2 Volume", 80);//83);                            // 80       Set     Decimator 2 Volume  to 80               !! default 84 distorts on CM11
    alsa_long_set ("ADC5 Volume", 4);                                   // 4        Set     ADC 5 Volume        to  4

    alsa_bool_set ("AIF1_CAP Mixer SLIM TX8", 1);                       //          Connect Analog Interface 1  to SLIMBus TX 8
    alsa_enum_set ("SLIM TX8 MUX", 8);                                  // DEC1     Connect SLIMBus TX 8 Mux    to 'Decimator 1'
    alsa_enum_set ("DEC1 MUX", 2);                                      // ADC6     Connect Decimator 1         to 'ADC 6'
    alsa_long_set ("DEC1 Volume", 80);//83);                            // 80       Set     Decimator 1 Volume  to 80               !! default 84 distorts on CM11
    alsa_long_set ("ADC6 Volume", 4);                                   // 4        Set     ADC 6 Volume        to  4

    alsa_enum_set ("SLIM_0_TX Channels", 1);                            // 2        Set SLIMBus TX channels     to "2"

    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 1);                   // MultiMedia1 enable  SLIMBus TX channel 0:    On, off, on needed to reset here (for channels) because audio was already playing
    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 0);
    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 1);
    return (0);
  }

  int xz2_dig_audio_state_stop () {
    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 0);                   // MultiMedia1 disable SLIMBus TX channel 0:    Off
    return (0);
  }

  int xz2_ana_vol_set (int vol) {
    alsa_long_set ("HPHL Volume", vol / 4369);// 3276); // Range 0 - 20, but use only 0-15        // Also works, unlike QCV
    alsa_long_set ("HPHR Volume", vol / 4369);// 3276); // Range 0 - 20, but use only 0-15
    return (vol);
  }

#define XZ2_SIMPLE_METHOD
#ifdef  XZ2_SIMPLE_METHOD
  int xz2_ana_audio_state_start () {                                    // Xperia Z2/Z3+ FM audio is actually analog, so this simple method works and avoids ALSA interference issues.
    alsa_bool_set ("HPHR_PA_MIXER AUX_PGA_R Switch", 1);
    alsa_bool_set ("HPHL_PA_MIXER AUX_PGA_L Switch", 1);
    return (0);
  }

  int xz2_ana_audio_state_stop () {
    alsa_bool_set ("HPHR_PA_MIXER AUX_PGA_R Switch", 0);
    alsa_bool_set ("HPHL_PA_MIXER AUX_PGA_L Switch", 0);
    return (0);
  }
#else
  int xz2_ana_audio_state_start () {                                    // This is how the stock app enables FM audio
    //alsa_enum_set ("SLIM_0_RX Channels", 1);
    //alsa_enum_set ("SLIM_0_TX Channels", 1);
    //alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 1);
    alsa_bool_set ("SLIM_0_RX_Voice Mixer Voice Stub", 1);
    alsa_bool_set ("Voice Stub Tx Mixer SLIM_0_TX", 1);
    alsa_bool_set ("SLIMBUS_0_RX Port Mixer SLIM_0_TX", 1);
    //alsa_bool_set ("SLIMBUS_DL_HL Switch", 1);

    hostless_transfer_start (hostless_samplerate, hostless_channels, "/dev/snd/pcmC0D25c", NULL);   // 25c/25p = "Voice Stub"; just a convenient virtual hitching post for hostless ?
    return (0);
  }

  int xz2_ana_audio_state_stop () {
    hostless_transfer_stop ();

    //alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 0);
    alsa_bool_set ("SLIMBUS_0_RX Port Mixer SLIM_0_TX", 0);
    alsa_bool_set ("Voice Stub Tx Mixer SLIM_0_TX", 0);
    alsa_bool_set ("SLIM_0_RX_Voice Mixer Voice Stub", 0);              // Some kind of weird Voice Stub/Mixer connection to FM; should check to see if audio is any better
    //alsa_enum_set ("SLIM_0_RX Channels", 0);
    //alsa_enum_set ("SLIM_0_TX Channels", 0);

    return (0);
  }
#endif  // #ifdef  XZ2_SIMPLE_METHOD


    // LG G2:

  int lg2_dig_audio_state_start () {
    alsa_enum_set ("FM Radio", 0);
    alsa_bool_set ("MultiMedia1 Mixer TERT_MI2S_TX", 1);
    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 0);                   // Turn off microphone path
    return (-1);
  }

  int lg2_dig_audio_state_stop () {
    alsa_bool_set ("MultiMedia1 Mixer TERT_MI2S_TX", 0);
    alsa_bool_set ("MultiMedia1 Mixer SLIM_0_TX", 1);
    return (-1);
  }

  int lg2_ana_vol_set (int vol) {
    //alsa_long_set ("HPHL Volume", vol / 4369);// 3276); // Range 0 - 20, but use only 0-15        // Also works, unlike QCV
    //alsa_long_set ("HPHR Volume", vol / 4369);// 3276); // Range 0 - 20, but use only 0-15

    alsa_long_set ("RX1 Digital Volume", vol / 1310);// 655); // Range 0 - 100, but use only 0-50
    alsa_long_set ("RX2 Digital Volume", vol / 1310);// 655); // Range 0 - 100, but use only 0-50

    return (vol);
  }

  int lg2_ana_audio_state_start () {                                    // LG G2 has pure digital audio, so this just sets up hostless transfer

                                                                        // Enable FM audio output
    alsa_enum_set ("FM Radio", 0);                                      // ALSA Control Missing on pure AOSP ROMs without the LG ALSA FM extensions

                                                                        // !!!! Testing only !!!!       This is the silly GPIO that "FM Radio" controls should use to select between BT and FM with
                                                                        // that weird analog switch chip where it appears software could have been used instead, since both go to SOC. (BT & FM play together too.)
    //sys_run ("echo 69 > /sys/class/gpio/export ; echo out > /sys/class/gpio/gpio69/direction ; echo 0 > /sys/class/gpio/gpio69/value", 1);

                                                                        // Enable Tertiary MI2S FM digital audio input
    alsa_bool_set ("SLIMBUS_0_RX Port Mixer TERT_MI2S_TX", 1);          // ALSA Control Missing on pure AOSP ROMs without the LG ALSA FM extensions
    alsa_bool_set ("SLIMBUS_DL_HL Switch", 1);

//    alsa_long_set ("RX1 Digital Volume", 50);
//    alsa_long_set ("RX2 Digital Volume", 50);

    hostless_transfer_start (hostless_samplerate, hostless_channels, "/dev/snd/pcmC0D17c", NULL);     // /dev/snd/pcmC0D5p
    return (0);
  }


  int lg2_ana_audio_state_stop () {
    hostless_transfer_stop ();

    //alsa_enum_set ("FM Radio", 1);
    alsa_bool_set ("SLIMBUS_0_RX Port Mixer TERT_MI2S_TX", 0);      // Not on older Mahdi & some other ROMs, after switch to AOSP/Nexus based Audio HAL without LG specific FM extensions
    alsa_bool_set ("SLIMBUS_DL_HL Switch", 0);

    return (0);
  }

