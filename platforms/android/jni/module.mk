LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := difont
LOCAL_SRC_FILES := ../obj/local/$(TARGET_ARCH_ABI)/libdifont.a
include $(PREBUILT_STATIC_LIBRARY)