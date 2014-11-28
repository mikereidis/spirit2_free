LOCAL_PATH:= $(call my-dir)

    # ssd alsa utility:
include $(CLEAR_VARS)
LOCAL_MODULE    := libssd
LOCAL_SRC_FILES := ssd.c
include $(BUILD_EXECUTABLE)

    # s2d daemon:
include $(CLEAR_VARS)
LOCAL_MODULE    := libs2d
LOCAL_SRC_FILES := s2d.c
include $(BUILD_EXECUTABLE)

    # JNI utilities
include $(CLEAR_VARS)
LOCAL_MODULE:= jut
LOCAL_SRC_FILES:= jut.c
#LOCAL_LDLIBS += -l OpenSLES
include $(BUILD_SHARED_LIBRARY)

    # Bluetooth shim:
include $(CLEAR_VARS)
LOCAL_MODULE    := libbt-hci
LOCAL_SRC_FILES := bt-hci.c
include $(BUILD_SHARED_LIBRARY)


    # 5 Tuner Plugins:

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= plug/tnr_ssl.c
LOCAL_MODULE:= libs2t_ssl
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= plug/tnr_qcv.c
LOCAL_MODULE:= libs2t_qcv
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= plug/tnr_bch.c
LOCAL_MODULE:= libs2t_bch
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= plug/tnr_sdr.c
LOCAL_MODULE:= libs2t_sdr
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= plug/tnr_gen.c
LOCAL_MODULE:= libs2t_gen
include $(BUILD_SHARED_LIBRARY)

# Disable TEST:

#include $(CLEAR_VARS)
#LOCAL_MODULE:= libbt-hci
#LOCAL_SRC_FILES := libbt-hci.so
#include $(PREBUILT_SHARED_LIBRARY)

