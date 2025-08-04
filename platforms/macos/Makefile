include ../desktop-shared/Makefile.sources

SOURCES_MM += $(DESKTOP_SRC_DIR)/nfd/nfd_cocoa.mm
SOURCES_MM += $(DESKTOP_SRC_DIR)/macos/fullscreen.mm
OBJECTS += $(SOURCES_MM:.mm=.o)
LDFLAGS += -framework AppKit -framework UniformTypeIdentifiers

include ../desktop-shared/Makefile.common

APP_NAME=Gearboy

bundle:
	rm -rf $(APP_NAME).app
	mkdir -p $(APP_NAME).app/Contents/{MacOS,Resources,Frameworks}
	sed -e "s/@version@/$(GIT_VERSION)/g" Info.plist > $(APP_NAME).app/Contents/Info.plist
	cp iconfile.icns $(APP_NAME).app/Contents/Resources/
	cp $(TARGET) $(APP_NAME).app/Contents/MacOS/
	cp run.sh $(APP_NAME).app/Contents/MacOS/
	cp ../gamecontrollerdb.txt $(APP_NAME).app/Contents/MacOS/

dist: clean all bundle
	@echo Success!!
