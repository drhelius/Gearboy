//
//  PS2Keyboard.cpp
//  by Nolan Baker (Q*Bert Reynolds)
//

#include "PS2Keyboard.h"
#include <SDL3/SDL.h>
#include <cstring>

u8 gearboy_ps2_map_sdl_scancode(int scancode)
{
    switch (scancode)
    {
        case SDL_SCANCODE_A: return 0x1C;
        case SDL_SCANCODE_B: return 0x32;
        case SDL_SCANCODE_C: return 0x21;
        case SDL_SCANCODE_D: return 0x23;
        case SDL_SCANCODE_E: return 0x24;
        case SDL_SCANCODE_F: return 0x2B;
        case SDL_SCANCODE_G: return 0x34;
        case SDL_SCANCODE_H: return 0x33;
        case SDL_SCANCODE_I: return 0x43;
        case SDL_SCANCODE_J: return 0x3B;
        case SDL_SCANCODE_K: return 0x42;
        case SDL_SCANCODE_L: return 0x4B;
        case SDL_SCANCODE_M: return 0x3A;
        case SDL_SCANCODE_N: return 0x31;
        case SDL_SCANCODE_O: return 0x44;
        case SDL_SCANCODE_P: return 0x4D;
        case SDL_SCANCODE_Q: return 0x15;
        case SDL_SCANCODE_R: return 0x2D;
        case SDL_SCANCODE_S: return 0x1B;
        case SDL_SCANCODE_T: return 0x2C;
        case SDL_SCANCODE_U: return 0x3C;
        case SDL_SCANCODE_V: return 0x2A;
        case SDL_SCANCODE_W: return 0x1D;
        case SDL_SCANCODE_X: return 0x22;
        case SDL_SCANCODE_Y: return 0x35;
        case SDL_SCANCODE_Z: return 0x1A;
        case SDL_SCANCODE_1: return 0x16;
        case SDL_SCANCODE_2: return 0x1E;
        case SDL_SCANCODE_3: return 0x26;
        case SDL_SCANCODE_4: return 0x25;
        case SDL_SCANCODE_5: return 0x2E;
        case SDL_SCANCODE_6: return 0x36;
        case SDL_SCANCODE_7: return 0x3D;
        case SDL_SCANCODE_8: return 0x3E;
        case SDL_SCANCODE_9: return 0x46;
        case SDL_SCANCODE_0: return 0x45;
        case SDL_SCANCODE_ESCAPE: return 0x76;
        case SDL_SCANCODE_GRAVE: return 0x0E;
        case SDL_SCANCODE_MINUS: return 0x4E;
        case SDL_SCANCODE_EQUALS: return 0x55;
        case SDL_SCANCODE_BACKSPACE: return 0x66;
        case SDL_SCANCODE_TAB: return 0x0D;
        case SDL_SCANCODE_LEFTBRACKET: return 0x54;
        case SDL_SCANCODE_RIGHTBRACKET: return 0x5B;
        case SDL_SCANCODE_BACKSLASH: return 0x5D;
        case SDL_SCANCODE_SEMICOLON: return 0x4C;
        case SDL_SCANCODE_APOSTROPHE: return 0x52;
        case SDL_SCANCODE_RETURN: return 0x5A;
        case SDL_SCANCODE_LSHIFT: return 0x12;
        case SDL_SCANCODE_RSHIFT: return 0x59;
        case SDL_SCANCODE_LCTRL: return 0x14;
        case SDL_SCANCODE_LALT: return 0x11;
        case SDL_SCANCODE_SPACE: return 0x29;
        case SDL_SCANCODE_COMMA: return 0x41;
        case SDL_SCANCODE_PERIOD: return 0x49;
        case SDL_SCANCODE_SLASH: return 0x4A;
        case SDL_SCANCODE_F1: return 0x05;
        case SDL_SCANCODE_F2: return 0x06;
        case SDL_SCANCODE_F3: return 0x04;
        case SDL_SCANCODE_F4: return 0x0C;
        case SDL_SCANCODE_F5: return 0x03;
        case SDL_SCANCODE_F6: return 0x0B;
        case SDL_SCANCODE_F7: return 0x83;
        case SDL_SCANCODE_F8: return 0x0A;
        case SDL_SCANCODE_F9: return 0x01;
        case SDL_SCANCODE_F10: return 0x09;
        case SDL_SCANCODE_F11: return 0x78;
        case SDL_SCANCODE_F12: return 0x07;
        case SDL_SCANCODE_CAPSLOCK: return 0x58;
        case SDL_SCANCODE_SCROLLLOCK: return 0x7E;
        case SDL_SCANCODE_NUMLOCKCLEAR: return 0x77;
        case SDL_SCANCODE_UP: return 0x75;
        case SDL_SCANCODE_DOWN: return 0x72;
        case SDL_SCANCODE_LEFT: return 0x6B;
        case SDL_SCANCODE_RIGHT: return 0x74;
        case SDL_SCANCODE_KP_0: return 0x70;
        case SDL_SCANCODE_KP_1: return 0x69;
        case SDL_SCANCODE_KP_2: return 0x72;
        case SDL_SCANCODE_KP_3: return 0x7A;
        case SDL_SCANCODE_KP_4: return 0x6B;
        case SDL_SCANCODE_KP_5: return 0x73;
        case SDL_SCANCODE_KP_6: return 0x74;
        case SDL_SCANCODE_KP_7: return 0x6C;
        case SDL_SCANCODE_KP_8: return 0x75;
        case SDL_SCANCODE_KP_9: return 0x7D;
        case SDL_SCANCODE_KP_PERIOD: return 0x71;
        case SDL_SCANCODE_KP_DIVIDE: return 0x4A;
        case SDL_SCANCODE_KP_MULTIPLY: return 0x7C;
        case SDL_SCANCODE_KP_MINUS: return 0x7B;
        case SDL_SCANCODE_KP_PLUS: return 0x79;
        case SDL_SCANCODE_KP_ENTER: return 0x5A;
        default: return 0;
    }
}

PS2Keyboard::PS2Keyboard()
{
    Reset();
}

void PS2Keyboard::Reset()
{
    m_queue_head = 0;
    m_queue_tail = 0;
    m_last_serial_data = 0xFF;
    m_transfer_id = 0;
    memset(m_queue, 0, sizeof(m_queue));
}

u8 PS2Keyboard::ComputeParity(u8 scan_code)
{
    u8 ones = 0;

    for (int bit = 0; bit < 8; bit++)
    {
        if ((scan_code >> bit) & 0x01)
            ones++;
    }

    return (ones & 0x01) ? 0 : 1;
}

void PS2Keyboard::BuildFrame(u8 scan_code, u8 frame[2])
{
    const u8 parity = ComputeParity(scan_code);

    frame[0] = 0; // start bit
    for (int bit = 0; bit < 7; bit++)
        frame[0] |= ((scan_code >> bit) & 1) << (6-bit);

    frame[1] = (scan_code & 0x80) | (parity << 6) | (1 << 5);
}

bool PS2Keyboard::QueueScanCode(u8 scan_code)
{
    u8 frame[2];
    BuildFrame(scan_code, frame);

    for (int i = 0; i < 2; i++)
    {
        const u8 next_tail = (m_queue_tail + 1) % k_queue_size;
        if (next_tail == m_queue_head)
            return false;

        m_queue[m_queue_tail] = frame[i];
        m_queue_tail = next_tail;
    }

    return true;
}

void PS2Keyboard::StateCallback(u64 cycle, u8 sb, u8 sc, GB_SerialEvent event, void* user_data)
{
    UNUSED(cycle);
    UNUSED(sc);
    UNUSED(event);

    PS2Keyboard* keyboard = (PS2Keyboard*)user_data;
    if (keyboard)
        keyboard->m_last_serial_data = sb;
}

void PS2Keyboard::StartCallback(u64 request_cycle, u64 first_shift_cycle, u32 bit_cycles, u8 outgoing_byte,
    u32 transfer_id, u8* incoming_byte, void* user_data)
{
    UNUSED(request_cycle);
    UNUSED(first_shift_cycle);
    UNUSED(bit_cycles);
    UNUSED(outgoing_byte);
    UNUSED(transfer_id);

    PS2Keyboard* keyboard = (PS2Keyboard*)user_data;
    if (!keyboard || !incoming_byte) return;

    const u8 head = keyboard->m_queue_head;
    if (head == keyboard->m_queue_tail) return;

    const u8 next_head = (head + 1) % k_queue_size;
    *incoming_byte = keyboard->m_queue[head];
    keyboard->m_queue_head = next_head;
    keyboard->m_last_serial_data = *incoming_byte;
}

bool PS2Keyboard::PollCallback(u64 cycle, GB_LinkCableTransfer* transfer, void* user_data)
{
    PS2Keyboard* keyboard = (PS2Keyboard*)user_data;
    if (!keyboard || !transfer)
        return false;

    const u8 head = keyboard->m_queue_head;
    if (head == keyboard->m_queue_tail)
        return false;

    const u8 next_head = (head + 1) % k_queue_size;
    transfer->incoming_byte = keyboard->m_queue[head];
    transfer->local_byte = keyboard->m_last_serial_data;
    transfer->request_cycle = cycle;
    transfer->first_shift_cycle = cycle + k_bit_cycles;
    transfer->bit_cycles = k_bit_cycles;
    transfer->transfer_id = ++keyboard->m_transfer_id;

    keyboard->m_queue_head = next_head;
    return true;
}
