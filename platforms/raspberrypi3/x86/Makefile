GEARBOY_SRC=../../../src
GEARBOY_AUDIO_SRC=../../audio-shared
GEARBOY_RPI_SRC=../../raspberrypi

include $(GEARBOY_RPI_SRC)/Makefile.common

CFLAGS+=-Wall -O3 -march=armv7-a -mtune=cortex-a53 -mfpu=neon-vfpv4 -mfloat-abi=hard -DGEARBOY_DISABLE_DISASSEMBLER

LDFLAGS+=-L$(SDKSTAGE)/opt/vc/lib/ -lbrcmEGL -lbrcmGLESv2 -lopenmaxil -lvcos -lvchiq_arm -lm -lrt -lconfig++ -lbcm_host `sdl2-config --libs`

INCLUDES+=-I$(SDKSTAGE)/opt/vc/include/ -I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux  -I$(GEARBOY_SRC) -I./

.SECONDARY: $(OBJS)

all: $(BIN) $(LIB)

%.o: %.cpp
	@rm -f $@
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) -o $@ -Wl,--whole-archive $(OBJS) $(LDFLAGS) -lstdc++ -Wl,--no-whole-archive -rdynamic

clean:
	for i in $(OBJS); do (if test -e "$$i"; then ( rm $$i ); fi ); done
	@rm -f $(BIN) $(LIB)
