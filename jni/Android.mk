LOCAL_PATH := $(call my-dir)

srcdir ?= $(LOCAL_PATH)/../src
incdir ?= $(LOCAL_PATH)/../include
utilsdir ?= $(LOCAL_PATH)/../utils

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(incdir)
LOCAL_SRC_FILES:= $(srcdir)/mixer.c $(srcdir)/pcm.c
LOCAL_MODULE := libtinyalsa
LOCAL_SHARED_LIBRARIES:= libcutils libutils
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(incdir)
LOCAL_SRC_FILES:= $(utilsdir)/tinyplay.c
LOCAL_MODULE := tinyplay
LOCAL_SHARED_LIBRARIES:= libcutils libutils libtinyalsa
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(incdir)
LOCAL_SRC_FILES:= $(utilsdir)/tinycap.c
LOCAL_MODULE := tinycap
LOCAL_SHARED_LIBRARIES:= libcutils libutils libtinyalsa
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(incdir)
LOCAL_SRC_FILES:= $(utilsdir)/tinymix.c
LOCAL_MODULE := tinymix
LOCAL_SHARED_LIBRARIES:= libcutils libutils libtinyalsa
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(incdir)
LOCAL_SRC_FILES:= $(utilsdir)/tinypcminfo.c
LOCAL_MODULE := tinypcminfo
LOCAL_SHARED_LIBRARIES:= libcutils libutils libtinyalsa
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

