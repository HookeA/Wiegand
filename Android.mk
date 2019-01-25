# -*- mode: makefile -*-

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libwiegand-jni

LOCAL_SRC_FILES := src/main/native/wiegand.c

LOCAL_MODULE_TAGS := optional

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)
