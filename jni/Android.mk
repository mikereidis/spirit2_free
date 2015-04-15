LOCAL_PATH:= $(call my-dir)

    # s2d daemon:
include $(CLEAR_VARS)
LOCAL_MODULE    := s2d
#LOCAL_MODULE_FILENAME  := libs2d
LOCAL_SRC_FILES := s2d/s2d.c
LOCAL_LDLIBS := -llog
#LOCAL_LDLIBS := -llog --output /home/m/dev/s2/obj/local/armeabi/mys2d.so
include $(BUILD_EXECUTABLE)
#include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)

#mv ../libs/armeabi/libs2d ../libs/armeabi/libs2d.so

#$(NDK_APP_LIBS_OUT)/%/libs2d.so: $(NDK_APP_LIBS_OUT)/%/s2d
#all: $(foreach _abi,$(APP_ABI),$(NDK_APP_LIBS_OUT)/$(_abi)/libs2d.so)
#all: $(call host-mv, $<, $@)

    # 5 Tuner Plugins:

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_ssl.c
LOCAL_MODULE:= libs2t_ssl
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_qcv.c
LOCAL_MODULE:= libs2t_qcv
LOCAL_LDLIBS := -llog
#LOCAL_SHARED_LIBRARIES := libnativehelper libcutils
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_bch.c
LOCAL_MODULE:= libs2t_bch
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= tnr/tnr_cus.c
LOCAL_MODULE:= libs2t_cus
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

    # Bluetooth libbt-vendor shim:
include $(CLEAR_VARS)
LOCAL_MODULE    := libbt-vendor
LOCAL_SRC_FILES := bts/bt-ven.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

    # Bluetooth libbt-hci shim:
include $(CLEAR_VARS)
LOCAL_MODULE    := libbt-hci
LOCAL_SRC_FILES := bts/bt-hci.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

    # Disable TEST:
#include $(CLEAR_VARS)
#LOCAL_MODULE:= libbt-hci
#LOCAL_SRC_FILES := libbt-hci.so
#include $(PREBUILT_SHARED_LIBRARY)

    # JNI utilities
include $(CLEAR_VARS)
LOCAL_MODULE:= jut
LOCAL_SRC_FILES:= jut/jut.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

