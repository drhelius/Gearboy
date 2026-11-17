#include "IGKeyboard.h"
#include <algorithm>
#include <cstdlib>

bool IGKeyboard::QueueKey(u8 hid, bool shift) {
    if (!hid || hid >= 0xE0 || m_queue.size() >= 256) return false;
    m_queue.push_back(u8(hid + (shift ? 61 : 93)));
    m_hover_tail = false;
    return true;
}
bool IGKeyboard::QueueMouse(int dx, int dy, u8 buttons) {
    if (dx < -10795 || dx > 10795 || dy < -10795 || dy > 10795) return false;
    // Coalesce hover motion which the GB has not begun clocking yet. Preserve
    // button-down reports and every edge so clicks and drags keep their anchors.
    if (m_hover_tail && !(buttons & 7) && m_queue.size() >= 3) {
        size_t at = m_queue.size() - 3;
        u8 flags = m_queue[at];
        if (!(flags & 7)) {
            int x = (m_queue[at+1] & 127) * (flags & 0x10 ? 1 : -1) + dx;
            int y = (m_queue[at+2] & 127) * (flags & 0x20 ? 1 : -1) + dy;
            if (x >= -127 && x <= 127 && y >= -127 && y <= 127) {
                m_queue[at] = 0x40 | (x >= 0 ? 0x10 : 0) | (y >= 0 ? 0x20 : 0);
                m_queue[at+1] = 0x80 | std::abs(x);
                m_queue[at+2] = 0x80 | std::abs(y);
                return true;
            }
        }
    }
    // Keep each report atomic, including zero movement and button releases.
    int packets = std::max(1, std::max((std::abs(dx)+126)/127, (std::abs(dy)+126)/127));
    if (packets > 85 || m_queue.size() + packets*3 > 256) return false;
    do {
        int x = std::max(-127, std::min(127, dx)), y = std::max(-127, std::min(127, dy));
        m_queue.push_back(0x40 | (buttons & 7) | (x >= 0 ? 0x10 : 0) | (y >= 0 ? 0x20 : 0));
        m_queue.push_back(0x80 | std::abs(x));
        m_queue.push_back(0x80 | std::abs(y));
        dx -= x; dy -= y;
    } while (dx || dy);
    m_hover_tail = !(buttons & 7);
    return true;
}
void IGKeyboard::StartCallback(u64, u64, u32, u8, u32, u8* incoming, void* data) {
    IGKeyboard* device = static_cast<IGKeyboard*>(data);
    if (!incoming || !device) return;
    *incoming = 0; // AVR idle byte, not an unplugged cable's $FF
    if (!device->m_queue.empty()) {
        *incoming = device->m_queue.front();
        device->m_queue.pop_front();
    }
}
