LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
# Here we give our module name and source file(s)
LOCAL_MODULE    := image_proc
LOCAL_SRC_FILES := image_proc.c
LOCAL_CFLAGS := -g
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
 
include $(BUILD_SHARED_LIBRARY)
