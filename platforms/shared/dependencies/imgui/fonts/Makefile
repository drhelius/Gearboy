SOURCES_C :=

SOURCES_CXX := \
    ./binary_to_compressed_c.cpp \

TARGET_NAME = binary_to_compressed_c
UNAME_S := $(shell uname -s)
PLATFORM = "undefined"

OBJECTS += $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o)

USE_CLANG ?= 0
ifeq ($(USE_CLANG), 1)
    CXX = clang++
    CC = clang
else
    CXX = g++
    CC = gcc
endif

CPPFLAGS += -I../ -I../../
CPPFLAGS += -Wall -Wextra -Wformat -Wno-unused-but-set-variable
CXXFLAGS += -std=c++11
CFLAGS += -std=c99

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CPPFLAGS +=-DDEBUG -g3
else
    CPPFLAGS +=-DNDEBUG -O2 -flto=auto
    LDFLAGS += -O2 -flto=auto
endif

SANITIZE ?= 0
ifeq ($(SANITIZE), 1)
    CPPFLAGS +=-fsanitize=address
    LDFLAGS += -lasan
endif

ifeq ($(UNAME_S), Linux)
    PLATFORM = "Linux"
    LDFLAGS += -ldl
    TARGET := $(TARGET_NAME)
else ifeq ($(UNAME_S), Darwin)
    PLATFORM = "macOS"
    LDFLAGS += -L/usr/local/lib
    CPPFLAGS += -I/usr/local/include -I/opt/local/include
    TARGET := $(TARGET_NAME)
else
    PLATFORM = "Generic Unix-like/BSD"
    CXXFLAGS += -std=gnu++11
    TARGET := $(TARGET_NAME)
endif

all: $(TARGET)
	@echo Build complete for $(PLATFORM)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.mm
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
