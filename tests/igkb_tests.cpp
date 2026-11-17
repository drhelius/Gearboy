#include "IGKeyboard.h"
#include <cstdio>
#include <cstdlib>
#include <climits>
static void check(bool value, const char* message) { if (!value) { fprintf(stderr,"FAIL: %s\n",message); exit(1); } }
static u8 read(IGKeyboard& device) { u8 byte=255; IGKeyboard::StartCallback(0,0,512,0,1,&byte,&device); return byte; }
int main() {
    IGKeyboard keyboard;
    check(read(keyboard)==0,"idle adapter returns zero");
    check(keyboard.QueueKey(4,false) && read(keyboard)=='a' && read(keyboard)==0,"manufacturer lowercase encoding");
    check(keyboard.QueueKey(4,true) && read(keyboard)=='A' && read(keyboard)==0,"manufacturer shift encoding");
    check(keyboard.QueueKey(0x28,false) && read(keyboard)==133 && read(keyboard)==0,"enter uses HID offset, not ASCII");
    check(!keyboard.QueueKey(0,false) && !keyboard.QueueKey(0xE1,false),"release and modifier-only reports are silent");
    IGKeyboard mouse;
    check(mouse.QueueMouse(-127,127,1),"mouse report accepted");
    check(read(mouse)==0x61 && read(mouse)==255 && read(mouse)==255,"signed movement uses flag bits and nonzero magnitudes");
    check(mouse.QueueMouse(0,0,0),"button release without movement accepted");
    check(read(mouse)==0x70 && read(mouse)==0x80 && read(mouse)==0x80,"zero movement stays distinct from idle");
    check(mouse.QueueMouse(200,-128,2),"large movement splits into complete reports");
    check(read(mouse)==0x52 && read(mouse)==255 && read(mouse)==255,"first split report");
    check(read(mouse)==0x52 && read(mouse)==201 && read(mouse)==129,"remaining movement is retained");
    check(!mouse.QueueMouse(INT_MIN,0,0),"reject out of range movement without overflow");
    for (int i=0;i<85;++i) check(mouse.QueueMouse(0,0,i&1),"fill report queue");
    check(!mouse.QueueMouse(1,0,1),"queue rejects a whole report when full");
    for (int i=0;i<85;++i) check(read(mouse)==(0x70 | (i&1)) && read(mouse)==0x80 && read(mouse)==0x80,"queued reports stay aligned");
    check(read(mouse)==0,"no partial report after overflow");
    check(mouse.QueueMouse(10,20,0) && mouse.QueueMouse(5,-5,0),"coalesce pending hover motion");
    check(read(mouse)==0x70 && read(mouse)==143 && read(mouse)==143 && read(mouse)==0,"coalescing preserves movement and drains promptly");
    check(mouse.QueueMouse(1,2,0) && mouse.QueueKey(4,false) && mouse.QueueMouse(3,4,0),"one adapter queues keyboard and mouse together");
    check(read(mouse)==0x70 && read(mouse)==129 && read(mouse)==130 && read(mouse)=='a',"a keyboard byte separates mouse reports");
    check(read(mouse)==0x70 && read(mouse)==131 && read(mouse)==132 && read(mouse)==0,"coalescing does not cross keyboard data");
    puts("iGKB keyboard encoding, mouse signs, zero movement, splitting and atomic queue tests passed");
}
