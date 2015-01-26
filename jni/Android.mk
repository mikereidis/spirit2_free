LOCAL_PATH:= $(call my-dir)

    # Bluetooth libbt-vendor shim:
include $(CLEAR_VARS)
LOCAL_MODULE    := libbt-vendor
LOCAL_SRC_FILES := bts/bt-ven.c
include $(BUILD_SHARED_LIBRARY)

    # Bluetooth libbt-hci shim:
include $(CLEAR_VARS)
LOCAL_MODULE    := libbt-hci
LOCAL_SRC_FILES := bts/bt-hci.c
include $(BUILD_SHARED_LIBRARY)

    # s2d daemon:
include $(CLEAR_VARS)
LOCAL_MODULE    := libs2d
LOCAL_SRC_FILES := s2d/s2d.c
include $(BUILD_EXECUTABLE)

    # JNI utilities
#include $(CLEAR_VARS)
#LOCAL_MODULE:= jut
#LOCAL_SRC_FILES:= jut/jut.c
##LOCAL_LDLIBS += -l OpenSLES
#include $(BUILD_SHARED_LIBRARY)


    # 5 Tuner Plugins:

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_ssl.c
LOCAL_MODULE:= libs2t_ssl
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_qcv.c
LOCAL_MODULE:= libs2t_qcv
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_bch.c
LOCAL_MODULE:= libs2t_bch
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_gen.c
LOCAL_MODULE:= libs2t_gen
include $(BUILD_SHARED_LIBRARY)

# Disable TEST:

#include $(CLEAR_VARS)
#LOCAL_MODULE:= libbt-hci
#LOCAL_SRC_FILES := libbt-hci.so
#include $(PREBUILT_SHARED_LIBRARY)

