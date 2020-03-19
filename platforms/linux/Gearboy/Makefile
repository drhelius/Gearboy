
CXX = g++
#CXX = clang++

EXE = gearboy

EMULATOR_SRC=../../../src
EMULATOR_AUDIO_SRC=../../../src/audio
EMULATOR_AUDIO_SHARED_SRC=../../audio-shared
IMGUI_SRC=../../imgui-shared

SOURCES = $(IMGUI_SRC)/main.cpp $(IMGUI_SRC)/imgui_impl_sdl.cpp $(IMGUI_SRC)/imgui_impl_opengl2.cpp $(IMGUI_SRC)/imgui.cpp .$(IMGUI_SRC)/imgui_demo.cpp $(IMGUI_SRC)/imgui_draw.cpp $(IMGUI_SRC)/imgui_widgets.cpp $(IMGUI_SRC)/Emulator.cpp 

SOURCES += $(EMULATOR_SRC)/Audio.cpp $(EMULATOR_SRC)/Cartridge.cpp $(EMULATOR_SRC)/CommonMemoryRule.cpp $(EMULATOR_SRC)/GearboyCore.cpp $(EMULATOR_SRC)/Input.cpp $(EMULATOR_SRC)/IORegistersMemoryRule.cpp $(EMULATOR_SRC)/MBC1MemoryRule.cpp $(EMULATOR_SRC)/MBC2MemoryRule.cpp $(EMULATOR_SRC)/MBC3MemoryRule.cpp $(EMULATOR_SRC)/MBC5MemoryRule.cpp $(EMULATOR_SRC)/Memory.cpp $(EMULATOR_SRC)/MemoryRule.cpp $(EMULATOR_SRC)/MultiMBC1MemoryRule.cpp $(EMULATOR_SRC)/opcodes.cpp $(EMULATOR_SRC)/opcodes_cb.cpp $(EMULATOR_SRC)/Processor.cpp $(EMULATOR_SRC)/RomOnlyMemoryRule.cpp $(EMULATOR_SRC)/Video.cpp

SOURCES += $(EMULATOR_AUDIO_SRC)/Blip_Buffer.cpp $(EMULATOR_AUDIO_SRC)/Effects_Buffer.cpp $(EMULATOR_AUDIO_SRC)/Gb_Apu.cpp $(EMULATOR_AUDIO_SRC)/Gb_Apu_State.cpp $(EMULATOR_AUDIO_SRC)/Gb_Oscs.cpp $(EMULATOR_AUDIO_SRC)/Multi_Buffer.cpp

SOURCES += $(EMULATOR_AUDIO_SHARED_SRC)/Sound_Queue.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

UNAME_S := $(shell uname -s)

CXXFLAGS = -I../ -I../../
CXXFLAGS += -g -Wall -Wformat

LIBS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += -lGL -ldl `sdl2-config --libs`

	CXXFLAGS += `sdl2-config --cflags`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LIBS += -L/usr/local/lib -L/opt/local/lib

	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

	CXXFLAGS += `pkg-config --cflags sdl2`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:$(IMGUI_SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(EMULATOR_SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(EMULATOR_AUDIO_SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(EMULATOR_AUDIO_SHARED_SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
