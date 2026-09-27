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

#ifndef BARCODE_BOY_H
#define BARCODE_BOY_H

#include "link_cable.h"
#include <iosfwd>

enum GB_BarcodeBoyMode
{
    GB_BarcodeBoyMode_Auto,
    GB_BarcodeBoyMode_Disabled,
    GB_BarcodeBoyMode_Enabled
};

enum GB_BarcodeBoyResult
{
    GB_BarcodeBoyResult_Accepted,
    GB_BarcodeBoyResult_Invalid,
    GB_BarcodeBoyResult_Busy,
    GB_BarcodeBoyResult_Unavailable
};

enum GB_BarcodeBoyStatus
{
    GB_BarcodeBoyStatus_Disabled,
    GB_BarcodeBoyStatus_Handshake,
    GB_BarcodeBoyStatus_Ready,
    GB_BarcodeBoyStatus_Queued,
    GB_BarcodeBoyStatus_Sending
};

class BarcodeBoy
{
public:
    static const int k_state_size = 24;
    static const u32 k_bit_cycles = 512;

    BarcodeBoy();
    void Reset();
    GB_BarcodeBoyResult ScanBarcode(const char* barcode);
    GB_BarcodeBoyStatus GetStatus() const;
    void SaveState(std::ostream& stream) const;
    bool LoadState(std::istream& stream);
    static void StateCallback(u64 cycle, u8 sb, u8 sc, GB_SerialEvent event, void* user_data);
    static void StartCallback(u64 request_cycle, u64 first_shift_cycle, u32 bit_cycles, u8 outgoing_byte,
        u32 transfer_id, u8* incoming_byte, void* user_data);
    static bool PollCallback(u64 cycle, GB_LinkCableTransfer* transfer, void* user_data);

private:
    char m_szBarcode[14];
    u8 m_iHandshakeStep;
    u8 m_iByteIndex;
    u8 m_iPendingTransfer;
    u8 m_iPendingHandshakeStep;
    u8 m_iSerialData;
    bool m_bScanQueued;
    bool m_bSendingBarcode;
    u32 m_iTransferId;
};

#endif /* BARCODE_BOY_H */
