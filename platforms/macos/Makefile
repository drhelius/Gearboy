include ../shared/makefiles/Makefile.sources

SOURCES_MM += macos.mm
OBJECTS += $(SOURCES_MM:.mm=.o)
LDFLAGS += -framework AppKit

include ../shared/makefiles/Makefile.common

APP_NAME=Gearboy

bundle:
	rm -rf $(APP_NAME).app
	mkdir -p $(APP_NAME).app/Contents/{MacOS,Resources,Frameworks}
	sed -e "s/@version@/$(GIT_VERSION)/g" Info.plist > $(APP_NAME).app/Contents/Info.plist
	cp iconfile.icns $(APP_NAME).app/Contents/Resources/
	cp $(TARGET) $(APP_NAME).app/Contents/MacOS/
	cp run.sh $(APP_NAME).app/Contents/MacOS/
	$(eval SDL3_DYLIB := $(shell otool -L $(TARGET) | grep libSDL3 | awk '{print $$1}'))
	cp $(SDL3_DYLIB) $(APP_NAME).app/Contents/Frameworks/
	chmod 755 $(APP_NAME).app/Contents/Frameworks/libSDL3.0.dylib
	install_name_tool -change $(SDL3_DYLIB) @executable_path/../Frameworks/libSDL3.0.dylib $(APP_NAME).app/Contents/MacOS/$(TARGET)
	mkdir -p $(APP_NAME).app/Contents/Resources/mcp
	cp ../shared/gamecontrollerdb.txt $(APP_NAME).app/Contents/Resources/
	cp -r ../shared/desktop/mcp/resources $(APP_NAME).app/Contents/Resources/mcp/

dist: clean all bundle
	@echo Success!!
