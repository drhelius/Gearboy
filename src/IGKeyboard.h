#ifndef IG_KEYBOARD_H
#define IG_KEYBOARD_H
#include "link_cable.h"
#include <deque>

// insideGadgets CH559/AVR adapter: the GB clocks one queued byte per transfer.
// Keyboard: manufacturer v1.0 encoding. Mouse: bbbbbr's three-byte firmware.
class IGKeyboard {
public:
    IGKeyboard() : m_hover_tail(false) {}
    void Reset() { m_queue.clear(); m_hover_tail = false; }
    bool QueueKey(u8 hid, bool shift);
    bool QueueMouse(int dx, int dy, u8 buttons);
    static void StartCallback(u64, u64, u32, u8, u32, u8*, void*);
private:
    bool m_hover_tail; // the queue ends with a complete, coalescible mouse report
    std::deque<u8> m_queue;
};
#endif
