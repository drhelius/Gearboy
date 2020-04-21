include ../desktop-shared/Makefile.common

DYLIB_PATH=/usr/local/opt/sdl2/lib
SDL_DYLIB=libSDL2-2.0.0.dylib
APP_NAME=Gearboy

bundle:
	rm -rf $(APP_NAME).app
	mkdir -p $(APP_NAME).app/Contents/{MacOS,Resources}
	cp Info.plist $(APP_NAME).app/Contents/
	cp $(DYLIB_PATH)/$(SDL_DYLIB) $(APP_NAME).app/Contents/MacOS/
	cp iconfile.icns $(APP_NAME).app/Contents/Resources/
	cp $(EXE) $(APP_NAME).app/Contents/MacOS/
	cp run.sh $(APP_NAME).app/Contents/MacOS/
	cp gamecontrollerdb.txt $(APP_NAME).app/Contents/MacOS/
	install_name_tool -change $(DYLIB_PATH)/$(SDL_DYLIB) @$(APP_NAME).app/Contents/MacOS/$(SDL_DYLIB) $(APP_NAME).app/Contents/MacOS/$(APP_NAME)

dist: clean all bundle
	@echo Success!!
