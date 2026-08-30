APP_STL := c++_static
APP_ABI := all
NDK_TOOLCHAIN_VERSION := clang

LTO ?= 1

ifneq ($(APP_OPTIM),debug)
ifneq ($(NDK_DEBUG),1)
APP_CFLAGS += -O3
ifeq ($(LTO),1)
APP_CFLAGS += -flto=thin
APP_LDFLAGS += -O3 -flto=thin
endif
endif
endif
