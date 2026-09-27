/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#include "BarcodeBoy.h"
#include <cstring>
#include <istream>
#include <ostream>

BarcodeBoy::BarcodeBoy()
{
    Reset();
}

void BarcodeBoy::Reset()
{
    memset(m_szBarcode, '0', 13);
    m_szBarcode[13] = 0;

    m_iHandshakeStep = 0;
    m_iByteIndex = 0;
    m_iPendingTransfer = 0;
    m_iPendingHandshakeStep = 0;
    m_iSerialData = 0xFF;

    m_bScanQueued = false;
    m_bSendingBarcode = false;
    m_iTransferId = 0;
}

GB_BarcodeBoyResult BarcodeBoy::ScanBarcode(const char* barcode)
{
    if (!barcode)
        return GB_BarcodeBoyResult_Invalid;

    for (int i = 0; i < 13; i++)
    {
        if (barcode[i] < '0' || barcode[i] > '9')
            return GB_BarcodeBoyResult_Invalid;
    }

    if (barcode[13] != 0)
        return GB_BarcodeBoyResult_Invalid;

    if (m_bSendingBarcode)
        return GB_BarcodeBoyResult_Busy;

    memcpy(m_szBarcode, barcode, sizeof(m_szBarcode));
    m_bScanQueued = true;

    return GB_BarcodeBoyResult_Accepted;
}

GB_BarcodeBoyStatus BarcodeBoy::GetStatus() const
{
    if (m_bSendingBarcode)
        return GB_BarcodeBoyStatus_Sending;

    if (m_bScanQueued)
        return GB_BarcodeBoyStatus_Queued;

    return m_iHandshakeStep == 4 ? GB_BarcodeBoyStatus_Ready : GB_BarcodeBoyStatus_Handshake;
}

void BarcodeBoy::StateCallback(u64 cycle, u8 sb, u8 sc, GB_SerialEvent event, void* user_data)
{
    UNUSED(cycle);
    UNUSED(sc);

    BarcodeBoy* pBarcodeBoy = (BarcodeBoy*)user_data;
    pBarcodeBoy->m_iSerialData = sb;

    if (event == GB_SerialEvent_ControlWrite)
    {
        // A cancelled or restarted byte has not reached the peripheral
        pBarcodeBoy->m_iPendingTransfer = 0;
    }
    else if (event == GB_SerialEvent_Complete)
    {
        if (pBarcodeBoy->m_iPendingTransfer == 1)
            pBarcodeBoy->m_iHandshakeStep = pBarcodeBoy->m_iPendingHandshakeStep;
        else if (pBarcodeBoy->m_iPendingTransfer == 2)
        {
            pBarcodeBoy->m_iByteIndex++;

            if (pBarcodeBoy->m_iByteIndex == 30)
            {
                pBarcodeBoy->m_iByteIndex = 0;
                pBarcodeBoy->m_iHandshakeStep = 0;
                pBarcodeBoy->m_bScanQueued = false;
                pBarcodeBoy->m_bSendingBarcode = false;
            }
        }

        pBarcodeBoy->m_iPendingTransfer = 0;
    }
}

void BarcodeBoy::StartCallback(u64 request_cycle, u64 first_shift_cycle, u32 bit_cycles, u8 outgoing_byte,
    u32 transfer_id, u8* incoming_byte, void* user_data)
{
    UNUSED(request_cycle);
    UNUSED(first_shift_cycle);
    UNUSED(bit_cycles);
    UNUSED(transfer_id);

    BarcodeBoy* pBarcodeBoy = (BarcodeBoy*)user_data;
    static const u8 kHandshakeSequence[] = { 0x10, 0x07, 0x10, 0x07 };

    *incoming_byte = 0xFF;
    pBarcodeBoy->m_iPendingTransfer = 1;
    pBarcodeBoy->m_iPendingHandshakeStep = pBarcodeBoy->m_iHandshakeStep;

    if (pBarcodeBoy->m_iHandshakeStep == 4)
        return;

    if (outgoing_byte == kHandshakeSequence[pBarcodeBoy->m_iHandshakeStep])
    {
        if (pBarcodeBoy->m_iHandshakeStep >= 2)
            *incoming_byte = outgoing_byte;

        pBarcodeBoy->m_iPendingHandshakeStep++;
    }
    else
        pBarcodeBoy->m_iPendingHandshakeStep = outgoing_byte == 0x10 ? 1 : 0;
}

bool BarcodeBoy::PollCallback(u64 cycle, GB_LinkCableTransfer* transfer, void* user_data)
{
    BarcodeBoy* pBarcodeBoy = (BarcodeBoy*)user_data;

    if (pBarcodeBoy->m_iHandshakeStep != 4 || !pBarcodeBoy->m_bScanQueued || pBarcodeBoy->m_iPendingTransfer != 0)
        return false;

    u8 byte_index = pBarcodeBoy->m_iByteIndex % 15;

    transfer->incoming_byte = byte_index == 0 ? 0x02 : (byte_index == 14 ? 0x03 : pBarcodeBoy->m_szBarcode[byte_index - 1]);
    transfer->local_byte = pBarcodeBoy->m_iSerialData;
    transfer->request_cycle = cycle;
    transfer->first_shift_cycle = cycle + k_bit_cycles;
    transfer->bit_cycles = k_bit_cycles;
    transfer->transfer_id = ++pBarcodeBoy->m_iTransferId;

    pBarcodeBoy->m_iPendingTransfer = 2;
    pBarcodeBoy->m_bSendingBarcode = true;

    return true;
}

void BarcodeBoy::SaveState(std::ostream& stream) const
{
    u8 state[k_state_size] = {};

    memcpy(state, m_szBarcode, 14);
    state[14] = m_iHandshakeStep;
    state[15] = m_iByteIndex;
    state[16] = m_iPendingTransfer;
    state[17] = m_iPendingHandshakeStep;
    state[18] = m_iSerialData;
    state[19] = (m_bScanQueued ? 1 : 0) | (m_bSendingBarcode ? 2 : 0);

    for (int i = 0; i < 4; i++)
        state[20 + i] = (u8)(m_iTransferId >> (i * 8));

    stream.write((const char*)state, sizeof(state));
}

bool BarcodeBoy::LoadState(std::istream& stream)
{
    u8 state[k_state_size] = {};
    stream.read((char*)state, sizeof(state));

    if (!stream.good() || state[13] != 0 || state[14] > 4 || state[15] >= 30 || state[16] > 2 ||
        state[17] > 4 || state[19] > 3 || state[19] == 2)
        return false;

    for (int i = 0; i < 13; i++)
    {
        if (state[i] < '0' || state[i] > '9')
            return false;
    }

    if ((state[15] != 0 || state[16] == 2 || (state[19] & 2)) && (state[14] != 4 || state[19] != 3))
        return false;

    memcpy(m_szBarcode, state, 14);

    m_iHandshakeStep = state[14];
    m_iByteIndex = state[15];
    m_iPendingTransfer = state[16];
    m_iPendingHandshakeStep = state[17];
    m_iSerialData = state[18];

    m_bScanQueued = (state[19] & 1) != 0;
    m_bSendingBarcode = (state[19] & 2) != 0;
    m_iTransferId = 0;

    for (int i = 0; i < 4; i++)
        m_iTransferId |= (u32)state[20 + i] << (i * 8);

    return true;
}
