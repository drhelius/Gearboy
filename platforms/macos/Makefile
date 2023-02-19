include ../desktop-shared/Makefile.common


# Brew use a different path on Apple Silicon as on Intel
UNAME_P := $(shell uname -m)
ifneq ($(filter arm64%,$(UNAME_P)),)
	DYLIB_PATH=/opt/homebrew/lib/
else
	DYLIB_PATH=/usr/local/opt/sdl2/lib
endif


SDL_DYLIB=libSDL2-2.0.0.dylib
APP_NAME=Gearboy

bundle:
	rm -rf $(APP_NAME).app
	mkdir -p $(APP_NAME).app/Contents/{MacOS,Resources,Frameworks}
	cp Info.plist $(APP_NAME).app/Contents/
	cp $(DYLIB_PATH)/$(SDL_DYLIB) $(APP_NAME).app/Contents/Frameworks/
	cp iconfile.icns $(APP_NAME).app/Contents/Resources/
	cp $(EXE) $(APP_NAME).app/Contents/MacOS/
	cp run.sh $(APP_NAME).app/Contents/MacOS/
	cp gamecontrollerdb.txt $(APP_NAME).app/Contents/MacOS/
	install_name_tool -change $(DYLIB_PATH)/$(SDL_DYLIB) @$(APP_NAME).app/Contents/MacOS/$(SDL_DYLIB) $(APP_NAME).app/Contents/MacOS/$(APP_NAME)

dist: clean all bundle
	@echo Success!!
