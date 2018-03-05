LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libretro

ifeq ($(TARGET_ARCH),arm)
LOCAL_CXXFLAGS += -DANDROID_ARM
LOCAL_ARM_MODE := arm
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CXXFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CXXFLAGS += -DANDROID_MIPS
endif

CORE_DIR := $(realpath ../)
SOURCE_DIR := $(realpath ../../../src)

include ../Makefile.common

LOCAL_SRC_FILES := $(SOURCES_CXX) $(SOURCES_C)
LOCAL_CXXFLAGS += -DINLINE=inline -DHAVE_STDINT_H -DHAVE_INTTYPES_H -D__LIBRETRO__ -DNDEBUG

include $(BUILD_SHARED_LIBRARY)
