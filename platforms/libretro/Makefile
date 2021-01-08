STATIC_LINKING := 0
AR             ?= ar

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
	EXE_EXT = .exe
	system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
	system_platform = osx
	arch = intel
ifeq ($(shell uname -p),powerpc)
	arch = ppc
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
	system_platform = win
endif

export DEPSDIR := $(CURDIR)/
CORE_DIR    += .
SOURCE_DIR  += ../../src
TARGET_NAME := gearboy
LIBM		    = -lm

ifeq ($(ARCHFLAGS),)
ifeq ($(archs),ppc)
   ARCHFLAGS = -arch ppc -arch ppc64
else
   ARCHFLAGS = -arch i386 -arch x86_64
endif
endif

ifeq ($(platform), osx)
ifndef ($(NOUNIVERSAL))
   CXXFLAGS += $(ARCHFLAGS)
   LFLAGS += $(ARCHFLAGS)
endif
endif

ifeq ($(STATIC_LINKING), 1)
EXT := a
endif

# Unix
ifneq (,$(findstring unix,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,-version-script=$(CORE_DIR)/link.T
   ifneq (,$(findstring Haiku,$(shell uname -s)))
   LDFLAGS += -lnetwork -lroot
   endif

   prefix := /usr
   libdir := $(prefix)/lib
   LIBRETRO_INSTALL_DIR := libretro

   # Raspberry Pi
   ifneq (,$(findstring rpi,$(platform)))
      CFLAGS += -fomit-frame-pointer -ffast-math -marm
      CXXFLAGS += $(CFLAGS)
      ifneq (,$(findstring rpi1,$(platform)))
         CFLAGS += -march=armv6j -mfpu=vfp -mfloat-abi=hard
      else ifneq (,$(findstring rpi2,$(platform)))
         CFLAGS += -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
      else ifneq (,$(findstring rpi3,$(platform)))
         CFLAGS += -mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard
      endif
   endif
   
# (armv7 a7, hard point, neon based) ### 
# NESC, SNESC, C64 mini 
else ifeq ($(platform), classic_armv7_a7)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC
  SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
	CFLAGS += -Ofast \
	-flto=4 -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -Wl,--gc-sections \
	-fno-stack-protector -fno-ident -fomit-frame-pointer \
	-falign-functions=1 -falign-jumps=1 -falign-loops=1 \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops \
	-fmerge-all-constants -fno-math-errno \
	-marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
	CXXFLAGS += $(CFLAGS)
	CPPFLAGS += $(CFLAGS)
	ASFLAGS += $(CFLAGS)
	HAVE_NEON = 1
	ARCH = arm
	BUILTIN_GPU = neon
	USE_DYNAREC = 1
	ifeq ($(shell echo `$(CC) -dumpversion` "< 4.9" | bc -l), 1)
	  CFLAGS += -march=armv7-a
	else
	  CFLAGS += -march=armv7ve
	  # If gcc is 5.0 or later
	  ifeq ($(shell echo `$(CC) -dumpversion` ">= 5" | bc -l), 1)
	    LDFLAGS += -static-libgcc -static-libstdc++
	  endif
	endif
#######################################

# Nintendo Switch (libtransistor)
else ifeq ($(platform), switch)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   include $(LIBTRANSISTOR_HOME)/libtransistor.mk
   CXXFLAGS += -DMINIZ_NO_TIME -Wall
   STATIC_LINKING=1
# Nintendo Switch (libnx)
else ifeq ($(platform), libnx)
   include $(DEVKITPRO)/libnx/switch_rules
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   DEFINES := -D__SWITCH__ -DHAVE_LIBNX
   CFLAGS := $(DEFINES) -fPIE -I$(LIBNX)/include/ -ffunction-sections -fdata-sections -ftls-model=local-exec -specs=$(LIBNX)/switch.specs
   CFLAGS += -march=armv8-a -mtune=cortex-a57 -mtp=soft -mcpu=cortex-a57+crc+fp+simd -ffast-math
   CXXFLAGS := $(ASFLAGS) $(CFLAGS) -DMINIZ_NO_TIME -Wall
   STATIC_LINKING=1
# Nintendo Game Cube / Wii / WiiU
else ifneq (,$(filter $(platform), ngc wii wiiu))
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CXXFLAGS += -DMINIZ_NO_TIME -DGEKKO -mcpu=750 -meabi -mhard-float -D__ppc__ -DMSB_FIRST -I$(DEVKITPRO)/libogc/include
   CXXFLAGS += -U__INT32_TYPE__ -U __UINT32_TYPE__ -D__INT32_TYPE__=int
   STATIC_LINKING = 1
   ifneq (,$(findstring wiiu,$(platform)))
      CXXFLAGS += -DWIIU -DHW_RVL
   else ifneq (,$(findstring wii,$(platform)))
      CXXFLAGS += -DHW_RVL -mrvl
   else ifneq (,$(findstring ngc,$(platform)))
      CXXFLAGS += -DHW_DOL -mrvl
   endif
# PS3
else ifeq ($(platform), ps3)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CXXFLAGS += -DMINIZ_NO_TIME -DMINIZ_NO_STDIO -Wall -D__ppc__ -DMSB_FIRST
   STATIC_LINKING = 1
# OSX
else ifneq (,$(findstring osx,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
# iOS
else ifneq (,$(findstring ios,$(platform)))
   TARGET := $(TARGET_NAME)_libretro_ios.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
   ifeq ($(IOSSDK),)
      IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
   endif
   ifeq ($(platform),ios-arm64)
      CC = cc -arch arm64 -isysroot $(IOSSDK)
      CXX = c++ -arch arm64 -isysroot $(IOSSDK)
   else
      CC = cc -arch armv7 -isysroot $(IOSSDK)
      CXX = c++ -arch armv7 -isysroot $(IOSSDK)
   endif
   ifeq ($(platform),$(filter $(platform),ios9 ios-arm64))
      CC += -miphoneos-version-min=8.0
      CXX += -miphoneos-version-min=8.0
      PLATFORM_DEFINES := -miphoneos-version-min=8.0
   else
      CC += -miphoneos-version-min=5.0
      CXX += -miphoneos-version-min=5.0
      PLATFORM_DEFINES := -miphoneos-version-min=5.0
   endif
# tvOS
else ifeq ($(platform), tvos-arm64)
   TARGET := $(TARGET_NAME)_libretro_tvos.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
   ifeq ($(IOSSDK),)
      IOSSDK := $(shell xcodebuild -version -sdk appletvos Path)
   endif
# QNX
else ifneq (,$(findstring qnx,$(platform)))
	TARGET := $(TARGET_NAME)_libretro_qnx.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/link.T -Wl,--no-undefined
# emscripten
else ifeq ($(platform), emscripten)
   TARGET := $(TARGET_NAME)_libretro_emscripten.bc
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/link.T -Wl,--no-undefined
# PSP
else ifeq ($(platform), psp1)
   TARGET := $(TARGET_NAME)_libretro_psp1.a
   CC = psp-gcc
   CXX = psp-g++
   AR = psp-ar
   CXXFLAGS += -Wall -DMINIZ_NO_TIME -ffast-math -fsingle-precision-constant
	STATIC_LINKING = 1
# Vita
else ifeq ($(platform), vita)
   TARGET := $(TARGET_NAME)_libretro_vita.a
   CC = arm-vita-eabi-gcc
   CXX = arm-vita-eabi-g++
   AR = arm-vita-eabi-ar
   CXXFLAGS += -Wl,-q -Wall -DMINIZ_NO_TIME -mword-relocations -fno-optimize-sibling-calls
	STATIC_LINKING = 1
# CTR(3DS)
else ifeq ($(platform), ctr)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(DEVKITARM)/bin/arm-none-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITARM)/bin/arm-none-eabi-g++$(EXE_EXT)
   AR = $(DEVKITARM)/bin/arm-none-eabi-ar$(EXE_EXT)
   PLATFORM_DEFINES := -DARM11 -D_3DS -DCC_RESAMPLER -DCC_RESAMPLER_NO_HIGHPASS
   CFLAGS += -march=armv6k -mtune=mpcore -mfloat-abi=hard
   CFLAGS += -Wall -mword-relocations
   CFLAGS += -fomit-frame-pointer -ffast-math
   CXXFLAGS += $(CFLAGS)
   STATIC_LINKING = 1
else
   CC ?= gcc
   CXX ?= g++
   TARGET := $(TARGET_NAME)_libretro.dll
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=$(CORE_DIR)/link.T -Wl,--no-undefined
endif

LDFLAGS += $(LIBM)

ifeq ($(DEBUG), 1)
   CXXFLAGS += -O0 -g
else
   CXXFLAGS += -O3
endif

include Makefile.common

OBJECTS := $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o)

INCLUDES += -I$(SOURCE_DIR)

CFLAGS   += -DGEARBOY_DISABLE_DISASSEMBLER -Wall -D__LIBRETRO__ $(fpic)
CXXFLAGS += -DGEARBOY_DISABLE_DISASSEMBLER -Wall -D__LIBRETRO__ $(fpic)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "** BUILDING $(TARGET) FOR PLATFORM $(platform) **"
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	$(CXX) $(fpic) $(SHARED) $(INCLUDES) -o $@ $(OBJECTS) $(LDFLAGS)
endif
	@echo "** BUILD SUCCESSFUL! **"
  
%.o: %.c
	$(CXX) $(CXXFLAGS) $(fpic) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

install:
	install -D -m 755 $(TARGET) $(DESTDIR)$(libdir)/$(LIBRETRO_INSTALL_DIR)/$(TARGET)
	install -D -m 755 $(TARGET_NAME).libretro $(DESTDIR)$(libdir)/$(LIBRETRO_INSTALL_DIR)/$(TARGET_NAME).libretro

uninstall:
	rm $(DESTDIR)$(libdir)/$(LIBRETRO_INSTALL_DIR)/$(TARGET)
	rm $(DESTDIR)$(libdir)/$(LIBRETRO_INSTALL_DIR)/$(TARGET_NAME).libretro

.PHONY: clean install uninstall
