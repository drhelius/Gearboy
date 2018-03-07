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

#include "Processor.h"
#include "opcode_timing.h"
#include "opcode_names.h"

Processor::Processor(Memory* pMemory)
{
    m_pMemory = pMemory;
    m_pMemory->SetProcessor(this);
    InitOPCodeFunctors();
    m_bIME = false;
    m_bHalt = false;
    m_bCGBSpeed = false;
    m_iSpeedMultiplier = 0;
    m_bBranchTaken = false;
    m_bSkipPCBug = false;
    m_iCurrentClockCycles = 0;
    m_iDIVCycles = 0;
    m_iTIMACycles = 0;
    m_iIMECycles = 0;
    m_iSerialBit = 0;
    m_iSerialCycles = 0;
    m_bCGB = false;
    m_iUnhaltCycles = 0;
    for (int i = 0; i < 5; i++)
        m_InterruptDelayCycles[i] = 0;
    m_iAccurateOPCodeState = 0;
    m_iReadCache = 0;
}

Processor::~Processor()
{
}

void Processor::Init()
{
    Reset(false);
}

void Processor::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_bIME = false;
    m_bHalt = false;
    m_bCGBSpeed = false;
    m_iSpeedMultiplier = 0;
    m_bBranchTaken = false;
    m_bSkipPCBug = false;
    m_iCurrentClockCycles = 0;
    m_iDIVCycles = 0;
    m_iTIMACycles = 0;
    m_iIMECycles = 0;
    m_iSerialBit = 0;
    m_iSerialCycles = 0;
    m_iUnhaltCycles = 0;
    PC.SetValue(0x100);
    SP.SetValue(0xFFFE);
    if (m_bCGB)
        AF.SetValue(0x11B0);
    else
        AF.SetValue(0x01B0);
    BC.SetValue(0x0013);
    DE.SetValue(0x00D8);
    HL.SetValue(0x014D);
    for (int i = 0; i < 5; i++)
        m_InterruptDelayCycles[i] = 0;
    m_iAccurateOPCodeState = 0;
    m_iReadCache = 0;
}

u8 Processor::Tick()
{
    m_iCurrentClockCycles = 0;

    if (m_iAccurateOPCodeState == 0 && m_bHalt)
    {
        m_iCurrentClockCycles += AdjustedCycles(4);

        if (m_iUnhaltCycles > 0)
        {
            m_iUnhaltCycles -= m_iCurrentClockCycles;

            if (m_iUnhaltCycles <= 0)
            {
                m_iUnhaltCycles = 0;
                m_bHalt = false;
            }
        }

        if (m_bHalt && (InterruptPending() != None_Interrupt) && (m_iUnhaltCycles == 0))
        {
            m_iUnhaltCycles = AdjustedCycles(12);
        }
    }

    if (!m_bHalt)
    {
        if (m_iAccurateOPCodeState == 0)
            ServeInterrupt(InterruptPending());

        ExecuteOPCode(FetchOPCode());
    }

    UpdateDelayedInterrupts();
    UpdateTimers();
    UpdateSerial();

    if (m_iAccurateOPCodeState == 0 && m_iIMECycles > 0)
    {
        m_iIMECycles -= m_iCurrentClockCycles;

        if (m_iIMECycles <= 0)
        {
            m_iIMECycles = 0;
            m_bIME = true;
        }
    }

    return m_iCurrentClockCycles;
}

void Processor::RequestInterrupt(Interrupts interrupt)
{
    m_pMemory->Load(0xFF0F, m_pMemory->Retrieve(0xFF0F) | interrupt);

    switch (interrupt)
    {
        case VBlank_Interrupt:
            m_InterruptDelayCycles[0] = (m_bCGBSpeed ? 0 : 4);
            break;
        case LCDSTAT_Interrupt:
            m_InterruptDelayCycles[1] = 0;
            break;
        case Timer_Interrupt:
            m_InterruptDelayCycles[2] = 0;
            break;
        case Serial_Interrupt:
            m_InterruptDelayCycles[3] = 0;
            break;
        case Joypad_Interrupt:
            m_InterruptDelayCycles[4] = 0;
            break;
        case None_Interrupt:
            break;
    }
}

void Processor::ResetTIMACycles()
{
    m_iTIMACycles = 0;
    m_pMemory->Load(0xFF05, m_pMemory->Retrieve(0xFF06));
}

void Processor::ResetDIVCycles()
{
    m_iDIVCycles = 0;
    m_pMemory->Load(0xFF04, 0x00);
}

bool Processor::Halted() const
{
    return m_bHalt;
}

bool Processor::CGBSpeed() const
{
    return m_bCGBSpeed;
}

void Processor::AddCycles(unsigned int cycles)
{
    m_iCurrentClockCycles += cycles;
}

u8 Processor::FetchOPCode()
{
    u8 opcode = m_pMemory->Read(PC.GetValue());
    PC.Increment();

    if (m_bSkipPCBug)
    {
        m_bSkipPCBug = false;
        PC.Decrement();
    }
    return opcode;
}

void Processor::ExecuteOPCode(u8 opcode)
{
    const u8* accurateOPcodes;
    const u8* machineCycles;
    OPCptr* opcodeTable;
    bool isCB = (opcode == 0xCB);

    if (isCB)
    {
        accurateOPcodes = kOPCodeCBAccurate;
        machineCycles = kOPCodeCBMachineCycles;
        opcodeTable = m_OPCodesCB;
        opcode = FetchOPCode();
    }
    else
    {
        accurateOPcodes = kOPCodeAccurate;
        machineCycles = kOPCodeMachineCycles;
        opcodeTable = m_OPCodes;
    }

    #ifdef DEBUG_GEARBOY
        u16 opcode_address = PC.GetValue() - 1;
        if (!m_pMemory->IsDisassembled(opcode_address))
        {
            m_pMemory->Disassemble(opcode_address, isCB ? kOPCodeCBNames[opcode] : kOPCodeNames[opcode]);
        }
    #endif

    if ((accurateOPcodes[opcode] != 0) && (m_iAccurateOPCodeState == 0))
    {
        int left_cycles = (accurateOPcodes[opcode] < 3 ? 2 : 3);
        m_iCurrentClockCycles += (machineCycles[opcode] - left_cycles) * AdjustedCycles(4);
        m_iAccurateOPCodeState = 1;
        PC.Decrement();
        if (isCB)
            PC.Decrement();
        return;
    }

    (this->*opcodeTable[opcode])();

    if (m_bBranchTaken)
    {
        m_bBranchTaken = false;
        m_iCurrentClockCycles += kOPCodeBranchMachineCycles[opcode] * AdjustedCycles(4);
    }
    else
    {
        switch (m_iAccurateOPCodeState)
        {
        case 0:
            m_iCurrentClockCycles += machineCycles[opcode] * AdjustedCycles(4);
            break;
        case 1:
            if (accurateOPcodes[opcode] == 3)
            {
                m_iCurrentClockCycles += 1 * AdjustedCycles(4);
                m_iAccurateOPCodeState = 2;
                PC.Decrement();
                if (isCB)
                    PC.Decrement();
            }
            else
            {
                m_iCurrentClockCycles += 2 * AdjustedCycles(4);
                m_iAccurateOPCodeState = 0;
            }
            break;
        case 2:
            m_iCurrentClockCycles += 2 * AdjustedCycles(4);
            m_iAccurateOPCodeState = 0;
            break;
        }
    }
}

bool Processor::InterruptIsAboutToRaise()
{
    u8 ie_reg = m_pMemory->Retrieve(0xFFFF);
    u8 if_reg = m_pMemory->Retrieve(0xFF0F);

    return (if_reg & ie_reg & 0x1F) != 0;
}

Processor::Interrupts Processor::InterruptPending()
{
    u8 ie_reg = m_pMemory->Retrieve(0xFFFF);
    u8 if_reg = m_pMemory->Retrieve(0xFF0F);
    u8 ie_if = if_reg & ie_reg;

    if ((ie_if & 0x01) && (m_InterruptDelayCycles[0] <= 0))
    {
        return VBlank_Interrupt;
    }
    else if ((ie_if & 0x02) && (m_InterruptDelayCycles[1] <= 0))
    {
        return LCDSTAT_Interrupt;
    }
    else if ((ie_if & 0x04) && (m_InterruptDelayCycles[2] <= 0))
    {
        return Timer_Interrupt;
    }
    else if ((ie_if & 0x08) && (m_InterruptDelayCycles[3] <= 0))
    {
        return Serial_Interrupt;
    }
    else if ((ie_if & 0x10) && (m_InterruptDelayCycles[4] <= 0))
    {
        return Joypad_Interrupt;
    }

    return None_Interrupt;
}

void Processor::ServeInterrupt(Interrupts interrupt)
{
    if (m_bIME)
    {
        u8 if_reg = m_pMemory->Retrieve(0xFF0F);
        switch (interrupt)
        {
            case VBlank_Interrupt:
                m_InterruptDelayCycles[0] = 0;
                m_pMemory->Load(0xFF0F, if_reg & 0xFE);
                m_bIME = false;
                StackPush(&PC);
                PC.SetValue(0x0040);
                m_iCurrentClockCycles += AdjustedCycles(20);
                break;
            case LCDSTAT_Interrupt:
                m_InterruptDelayCycles[1] = 0;
                m_pMemory->Load(0xFF0F, if_reg & 0xFD);
                m_bIME = false;
                StackPush(&PC);
                PC.SetValue(0x0048);
                m_iCurrentClockCycles += AdjustedCycles(20);
                break;
            case Timer_Interrupt:
                m_InterruptDelayCycles[2] = 0;
                m_pMemory->Load(0xFF0F, if_reg & 0xFB);
                m_bIME = false;
                StackPush(&PC);
                PC.SetValue(0x0050);
                m_iCurrentClockCycles += AdjustedCycles(20);
                break;
            case Serial_Interrupt:
                m_InterruptDelayCycles[3] = 0;
                m_pMemory->Load(0xFF0F, if_reg & 0xF7);
                m_bIME = false;
                StackPush(&PC);
                PC.SetValue(0x0058);
                m_iCurrentClockCycles += AdjustedCycles(20);
                break;
            case Joypad_Interrupt:
                m_InterruptDelayCycles[4] = 0;
                m_pMemory->Load(0xFF0F, if_reg & 0xEF);
                m_bIME = false;
                StackPush(&PC);
                PC.SetValue(0x0060);
                m_iCurrentClockCycles += AdjustedCycles(20);
                break;
            case None_Interrupt:
                break;
        }
    }
}

void Processor::UpdateTimers()
{
    m_iDIVCycles += m_iCurrentClockCycles;

    unsigned int div_cycles = AdjustedCycles(256);

    while (m_iDIVCycles >= div_cycles)
    {
        m_iDIVCycles -= div_cycles;
        u8 div = m_pMemory->Retrieve(0xFF04);
        div++;
        m_pMemory->Load(0xFF04, div);
    }

    u8 tac = m_pMemory->Retrieve(0xFF07);

    // if tima is running
    if (tac & 0x04)
    {
        m_iTIMACycles += m_iCurrentClockCycles;

        unsigned int freq = 0;

        switch (tac & 0x03)
        {
            case 0:
                freq = AdjustedCycles(1024);
                break;
            case 1:
                freq = AdjustedCycles(16);
                break;
            case 2:
                freq = AdjustedCycles(64);
                break;
            case 3:
                freq = AdjustedCycles(256);
                break;
        }

        while (m_iTIMACycles >= freq)
        {
            m_iTIMACycles -= freq;
            u8 tima = m_pMemory->Retrieve(0xFF05);

            if (tima == 0xFF)
            {
                tima = m_pMemory->Retrieve(0xFF06);
                RequestInterrupt(Timer_Interrupt);
            }
            else
                tima++;

            m_pMemory->Load(0xFF05, tima);
        }
    }
}

void Processor::UpdateSerial()
{
    u8 sc = m_pMemory->Retrieve(0xFF02);

    if (IsSetBit(sc, 7) && IsSetBit(sc, 0))
    {
        m_iSerialCycles += m_iCurrentClockCycles;

        if (m_iSerialBit < 0)
        {
            m_iSerialBit = 0;
            m_iSerialCycles = 0;
            return;
        }

        int serial_cycles = AdjustedCycles(512);

        if (m_iSerialCycles >= serial_cycles)
        {
            if (m_iSerialBit > 7)
            {
                m_pMemory->Load(0xFF02, sc & 0x7F);
                RequestInterrupt(Serial_Interrupt);
                m_iSerialBit = -1;

                return;
            }

            u8 sb = m_pMemory->Retrieve(0xFF01);
            sb <<= 1;
            sb |= 0x01;
            m_pMemory->Load(0xFF01, sb);

            m_iSerialCycles -= serial_cycles;
            m_iSerialBit++;
        }
    }
}

void Processor::UpdateDelayedInterrupts()
{
    for (int i = 0; i < 5; i++)
    {
        if (m_InterruptDelayCycles[i] > 0)
        {
            m_InterruptDelayCycles[i] -= m_iCurrentClockCycles;
        }
    }
}

void Processor::SaveState(std::stringstream& stream)
{
    using namespace std;

    u16 af = AF.GetValue();
    u16 bc = BC.GetValue();
    u16 de = DE.GetValue();
    u16 hl = HL.GetValue();
    u16 sp = SP.GetValue();
    u16 pc = PC.GetValue();

    stream.write(reinterpret_cast<const char*> (&af), sizeof(af));
    stream.write(reinterpret_cast<const char*> (&bc), sizeof(bc));
    stream.write(reinterpret_cast<const char*> (&de), sizeof(de));
    stream.write(reinterpret_cast<const char*> (&hl), sizeof(hl));
    stream.write(reinterpret_cast<const char*> (&sp), sizeof(sp));
    stream.write(reinterpret_cast<const char*> (&pc), sizeof(pc));

    stream.write(reinterpret_cast<const char*> (&m_bIME), sizeof(m_bIME));
    stream.write(reinterpret_cast<const char*> (&m_bHalt), sizeof(m_bHalt));
    stream.write(reinterpret_cast<const char*> (&m_bBranchTaken), sizeof(m_bBranchTaken));
    stream.write(reinterpret_cast<const char*> (&m_bSkipPCBug), sizeof(m_bSkipPCBug));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentClockCycles), sizeof(m_iCurrentClockCycles));
    stream.write(reinterpret_cast<const char*> (&m_iDIVCycles), sizeof(m_iDIVCycles));
    stream.write(reinterpret_cast<const char*> (&m_iTIMACycles), sizeof(m_iTIMACycles));
    stream.write(reinterpret_cast<const char*> (&m_iSerialBit), sizeof(m_iSerialBit));
    stream.write(reinterpret_cast<const char*> (&m_iSerialCycles), sizeof(m_iSerialCycles));
    stream.write(reinterpret_cast<const char*> (&m_iIMECycles), sizeof(m_iIMECycles));
    stream.write(reinterpret_cast<const char*> (&m_iUnhaltCycles), sizeof(m_iUnhaltCycles));
    stream.write(reinterpret_cast<const char*> (m_InterruptDelayCycles), sizeof(m_InterruptDelayCycles));
    stream.write(reinterpret_cast<const char*> (&m_bCGBSpeed), sizeof(m_bCGBSpeed));
    stream.write(reinterpret_cast<const char*> (&m_iSpeedMultiplier), sizeof(m_iSpeedMultiplier));
    stream.write(reinterpret_cast<const char*> (&m_iAccurateOPCodeState), sizeof(m_iAccurateOPCodeState));
    stream.write(reinterpret_cast<const char*> (&m_iReadCache), sizeof(m_iReadCache));
}

void Processor::LoadState(std::stringstream& stream)
{
    using namespace std;

    u16 af;
    u16 bc;
    u16 de;
    u16 hl;
    u16 sp;
    u16 pc;

    stream.read(reinterpret_cast<char*> (&af), sizeof(af));
    stream.read(reinterpret_cast<char*> (&bc), sizeof(bc));
    stream.read(reinterpret_cast<char*> (&de), sizeof(de));
    stream.read(reinterpret_cast<char*> (&hl), sizeof(hl));
    stream.read(reinterpret_cast<char*> (&sp), sizeof(sp));
    stream.read(reinterpret_cast<char*> (&pc), sizeof(pc));

    AF.SetValue(af);
    BC.SetValue(bc);
    DE.SetValue(de);
    HL.SetValue(hl);
    SP.SetValue(sp);
    PC.SetValue(pc);

    stream.read(reinterpret_cast<char*> (&m_bIME), sizeof(m_bIME));
    stream.read(reinterpret_cast<char*> (&m_bHalt), sizeof(m_bHalt));
    stream.read(reinterpret_cast<char*> (&m_bBranchTaken), sizeof(m_bBranchTaken));
    stream.read(reinterpret_cast<char*> (&m_bSkipPCBug), sizeof(m_bSkipPCBug));
    stream.read(reinterpret_cast<char*> (&m_iCurrentClockCycles), sizeof(m_iCurrentClockCycles));
    stream.read(reinterpret_cast<char*> (&m_iDIVCycles), sizeof(m_iDIVCycles));
    stream.read(reinterpret_cast<char*> (&m_iTIMACycles), sizeof(m_iTIMACycles));
    stream.read(reinterpret_cast<char*> (&m_iSerialBit), sizeof(m_iSerialBit));
    stream.read(reinterpret_cast<char*> (&m_iSerialCycles), sizeof(m_iSerialCycles));
    stream.read(reinterpret_cast<char*> (&m_iIMECycles), sizeof(m_iIMECycles));
    stream.read(reinterpret_cast<char*> (&m_iUnhaltCycles), sizeof(m_iUnhaltCycles));
    stream.read(reinterpret_cast<char*> (m_InterruptDelayCycles), sizeof(m_InterruptDelayCycles));
    stream.read(reinterpret_cast<char*> (&m_bCGBSpeed), sizeof(m_bCGBSpeed));
    stream.read(reinterpret_cast<char*> (&m_iSpeedMultiplier), sizeof(m_iSpeedMultiplier));
    stream.read(reinterpret_cast<char*> (&m_iAccurateOPCodeState), sizeof(m_iAccurateOPCodeState));
    stream.read(reinterpret_cast<char*> (&m_iReadCache), sizeof(m_iReadCache));
}

void Processor::InitOPCodeFunctors()
{
    m_OPCodes[0x00] = &Processor::OPCode0x00;
    m_OPCodes[0x01] = &Processor::OPCode0x01;
    m_OPCodes[0x02] = &Processor::OPCode0x02;
    m_OPCodes[0x03] = &Processor::OPCode0x03;
    m_OPCodes[0x04] = &Processor::OPCode0x04;
    m_OPCodes[0x05] = &Processor::OPCode0x05;
    m_OPCodes[0x06] = &Processor::OPCode0x06;
    m_OPCodes[0x07] = &Processor::OPCode0x07;
    m_OPCodes[0x08] = &Processor::OPCode0x08;
    m_OPCodes[0x09] = &Processor::OPCode0x09;
    m_OPCodes[0x0A] = &Processor::OPCode0x0A;
    m_OPCodes[0x0B] = &Processor::OPCode0x0B;
    m_OPCodes[0x0C] = &Processor::OPCode0x0C;
    m_OPCodes[0x0D] = &Processor::OPCode0x0D;
    m_OPCodes[0x0E] = &Processor::OPCode0x0E;
    m_OPCodes[0x0F] = &Processor::OPCode0x0F;

    m_OPCodes[0x10] = &Processor::OPCode0x10;
    m_OPCodes[0x11] = &Processor::OPCode0x11;
    m_OPCodes[0x12] = &Processor::OPCode0x12;
    m_OPCodes[0x13] = &Processor::OPCode0x13;
    m_OPCodes[0x14] = &Processor::OPCode0x14;
    m_OPCodes[0x15] = &Processor::OPCode0x15;
    m_OPCodes[0x16] = &Processor::OPCode0x16;
    m_OPCodes[0x17] = &Processor::OPCode0x17;
    m_OPCodes[0x18] = &Processor::OPCode0x18;
    m_OPCodes[0x19] = &Processor::OPCode0x19;
    m_OPCodes[0x1A] = &Processor::OPCode0x1A;
    m_OPCodes[0x1B] = &Processor::OPCode0x1B;
    m_OPCodes[0x1C] = &Processor::OPCode0x1C;
    m_OPCodes[0x1D] = &Processor::OPCode0x1D;
    m_OPCodes[0x1E] = &Processor::OPCode0x1E;
    m_OPCodes[0x1F] = &Processor::OPCode0x1F;

    m_OPCodes[0x20] = &Processor::OPCode0x20;
    m_OPCodes[0x21] = &Processor::OPCode0x21;
    m_OPCodes[0x22] = &Processor::OPCode0x22;
    m_OPCodes[0x23] = &Processor::OPCode0x23;
    m_OPCodes[0x24] = &Processor::OPCode0x24;
    m_OPCodes[0x25] = &Processor::OPCode0x25;
    m_OPCodes[0x26] = &Processor::OPCode0x26;
    m_OPCodes[0x27] = &Processor::OPCode0x27;
    m_OPCodes[0x28] = &Processor::OPCode0x28;
    m_OPCodes[0x29] = &Processor::OPCode0x29;
    m_OPCodes[0x2A] = &Processor::OPCode0x2A;
    m_OPCodes[0x2B] = &Processor::OPCode0x2B;
    m_OPCodes[0x2C] = &Processor::OPCode0x2C;
    m_OPCodes[0x2D] = &Processor::OPCode0x2D;
    m_OPCodes[0x2E] = &Processor::OPCode0x2E;
    m_OPCodes[0x2F] = &Processor::OPCode0x2F;

    m_OPCodes[0x30] = &Processor::OPCode0x30;
    m_OPCodes[0x31] = &Processor::OPCode0x31;
    m_OPCodes[0x32] = &Processor::OPCode0x32;
    m_OPCodes[0x33] = &Processor::OPCode0x33;
    m_OPCodes[0x34] = &Processor::OPCode0x34;
    m_OPCodes[0x35] = &Processor::OPCode0x35;
    m_OPCodes[0x36] = &Processor::OPCode0x36;
    m_OPCodes[0x37] = &Processor::OPCode0x37;
    m_OPCodes[0x38] = &Processor::OPCode0x38;
    m_OPCodes[0x39] = &Processor::OPCode0x39;
    m_OPCodes[0x3A] = &Processor::OPCode0x3A;
    m_OPCodes[0x3B] = &Processor::OPCode0x3B;
    m_OPCodes[0x3C] = &Processor::OPCode0x3C;
    m_OPCodes[0x3D] = &Processor::OPCode0x3D;
    m_OPCodes[0x3E] = &Processor::OPCode0x3E;
    m_OPCodes[0x3F] = &Processor::OPCode0x3F;

    m_OPCodes[0x40] = &Processor::OPCode0x40;
    m_OPCodes[0x41] = &Processor::OPCode0x41;
    m_OPCodes[0x42] = &Processor::OPCode0x42;
    m_OPCodes[0x43] = &Processor::OPCode0x43;
    m_OPCodes[0x44] = &Processor::OPCode0x44;
    m_OPCodes[0x45] = &Processor::OPCode0x45;
    m_OPCodes[0x46] = &Processor::OPCode0x46;
    m_OPCodes[0x47] = &Processor::OPCode0x47;
    m_OPCodes[0x48] = &Processor::OPCode0x48;
    m_OPCodes[0x49] = &Processor::OPCode0x49;
    m_OPCodes[0x4A] = &Processor::OPCode0x4A;
    m_OPCodes[0x4B] = &Processor::OPCode0x4B;
    m_OPCodes[0x4C] = &Processor::OPCode0x4C;
    m_OPCodes[0x4D] = &Processor::OPCode0x4D;
    m_OPCodes[0x4E] = &Processor::OPCode0x4E;
    m_OPCodes[0x4F] = &Processor::OPCode0x4F;

    m_OPCodes[0x50] = &Processor::OPCode0x50;
    m_OPCodes[0x51] = &Processor::OPCode0x51;
    m_OPCodes[0x52] = &Processor::OPCode0x52;
    m_OPCodes[0x53] = &Processor::OPCode0x53;
    m_OPCodes[0x54] = &Processor::OPCode0x54;
    m_OPCodes[0x55] = &Processor::OPCode0x55;
    m_OPCodes[0x56] = &Processor::OPCode0x56;
    m_OPCodes[0x57] = &Processor::OPCode0x57;
    m_OPCodes[0x58] = &Processor::OPCode0x58;
    m_OPCodes[0x59] = &Processor::OPCode0x59;
    m_OPCodes[0x5A] = &Processor::OPCode0x5A;
    m_OPCodes[0x5B] = &Processor::OPCode0x5B;
    m_OPCodes[0x5C] = &Processor::OPCode0x5C;
    m_OPCodes[0x5D] = &Processor::OPCode0x5D;
    m_OPCodes[0x5E] = &Processor::OPCode0x5E;
    m_OPCodes[0x5F] = &Processor::OPCode0x5F;

    m_OPCodes[0x60] = &Processor::OPCode0x60;
    m_OPCodes[0x61] = &Processor::OPCode0x61;
    m_OPCodes[0x62] = &Processor::OPCode0x62;
    m_OPCodes[0x63] = &Processor::OPCode0x63;
    m_OPCodes[0x64] = &Processor::OPCode0x64;
    m_OPCodes[0x65] = &Processor::OPCode0x65;
    m_OPCodes[0x66] = &Processor::OPCode0x66;
    m_OPCodes[0x67] = &Processor::OPCode0x67;
    m_OPCodes[0x68] = &Processor::OPCode0x68;
    m_OPCodes[0x69] = &Processor::OPCode0x69;
    m_OPCodes[0x6A] = &Processor::OPCode0x6A;
    m_OPCodes[0x6B] = &Processor::OPCode0x6B;
    m_OPCodes[0x6C] = &Processor::OPCode0x6C;
    m_OPCodes[0x6D] = &Processor::OPCode0x6D;
    m_OPCodes[0x6E] = &Processor::OPCode0x6E;
    m_OPCodes[0x6F] = &Processor::OPCode0x6F;

    m_OPCodes[0x70] = &Processor::OPCode0x70;
    m_OPCodes[0x71] = &Processor::OPCode0x71;
    m_OPCodes[0x72] = &Processor::OPCode0x72;
    m_OPCodes[0x73] = &Processor::OPCode0x73;
    m_OPCodes[0x74] = &Processor::OPCode0x74;
    m_OPCodes[0x75] = &Processor::OPCode0x75;
    m_OPCodes[0x76] = &Processor::OPCode0x76;
    m_OPCodes[0x77] = &Processor::OPCode0x77;
    m_OPCodes[0x78] = &Processor::OPCode0x78;
    m_OPCodes[0x79] = &Processor::OPCode0x79;
    m_OPCodes[0x7A] = &Processor::OPCode0x7A;
    m_OPCodes[0x7B] = &Processor::OPCode0x7B;
    m_OPCodes[0x7C] = &Processor::OPCode0x7C;
    m_OPCodes[0x7D] = &Processor::OPCode0x7D;
    m_OPCodes[0x7E] = &Processor::OPCode0x7E;
    m_OPCodes[0x7F] = &Processor::OPCode0x7F;

    m_OPCodes[0x80] = &Processor::OPCode0x80;
    m_OPCodes[0x81] = &Processor::OPCode0x81;
    m_OPCodes[0x82] = &Processor::OPCode0x82;
    m_OPCodes[0x83] = &Processor::OPCode0x83;
    m_OPCodes[0x84] = &Processor::OPCode0x84;
    m_OPCodes[0x85] = &Processor::OPCode0x85;
    m_OPCodes[0x86] = &Processor::OPCode0x86;
    m_OPCodes[0x87] = &Processor::OPCode0x87;
    m_OPCodes[0x88] = &Processor::OPCode0x88;
    m_OPCodes[0x89] = &Processor::OPCode0x89;
    m_OPCodes[0x8A] = &Processor::OPCode0x8A;
    m_OPCodes[0x8B] = &Processor::OPCode0x8B;
    m_OPCodes[0x8C] = &Processor::OPCode0x8C;
    m_OPCodes[0x8D] = &Processor::OPCode0x8D;
    m_OPCodes[0x8E] = &Processor::OPCode0x8E;
    m_OPCodes[0x8F] = &Processor::OPCode0x8F;

    m_OPCodes[0x90] = &Processor::OPCode0x90;
    m_OPCodes[0x91] = &Processor::OPCode0x91;
    m_OPCodes[0x92] = &Processor::OPCode0x92;
    m_OPCodes[0x93] = &Processor::OPCode0x93;
    m_OPCodes[0x94] = &Processor::OPCode0x94;
    m_OPCodes[0x95] = &Processor::OPCode0x95;
    m_OPCodes[0x96] = &Processor::OPCode0x96;
    m_OPCodes[0x97] = &Processor::OPCode0x97;
    m_OPCodes[0x98] = &Processor::OPCode0x98;
    m_OPCodes[0x99] = &Processor::OPCode0x99;
    m_OPCodes[0x9A] = &Processor::OPCode0x9A;
    m_OPCodes[0x9B] = &Processor::OPCode0x9B;
    m_OPCodes[0x9C] = &Processor::OPCode0x9C;
    m_OPCodes[0x9D] = &Processor::OPCode0x9D;
    m_OPCodes[0x9E] = &Processor::OPCode0x9E;
    m_OPCodes[0x9F] = &Processor::OPCode0x9F;

    m_OPCodes[0xA0] = &Processor::OPCode0xA0;
    m_OPCodes[0xA1] = &Processor::OPCode0xA1;
    m_OPCodes[0xA2] = &Processor::OPCode0xA2;
    m_OPCodes[0xA3] = &Processor::OPCode0xA3;
    m_OPCodes[0xA4] = &Processor::OPCode0xA4;
    m_OPCodes[0xA5] = &Processor::OPCode0xA5;
    m_OPCodes[0xA6] = &Processor::OPCode0xA6;
    m_OPCodes[0xA7] = &Processor::OPCode0xA7;
    m_OPCodes[0xA8] = &Processor::OPCode0xA8;
    m_OPCodes[0xA9] = &Processor::OPCode0xA9;
    m_OPCodes[0xAA] = &Processor::OPCode0xAA;
    m_OPCodes[0xAB] = &Processor::OPCode0xAB;
    m_OPCodes[0xAC] = &Processor::OPCode0xAC;
    m_OPCodes[0xAD] = &Processor::OPCode0xAD;
    m_OPCodes[0xAE] = &Processor::OPCode0xAE;
    m_OPCodes[0xAF] = &Processor::OPCode0xAF;

    m_OPCodes[0xB0] = &Processor::OPCode0xB0;
    m_OPCodes[0xB1] = &Processor::OPCode0xB1;
    m_OPCodes[0xB2] = &Processor::OPCode0xB2;
    m_OPCodes[0xB3] = &Processor::OPCode0xB3;
    m_OPCodes[0xB4] = &Processor::OPCode0xB4;
    m_OPCodes[0xB5] = &Processor::OPCode0xB5;
    m_OPCodes[0xB6] = &Processor::OPCode0xB6;
    m_OPCodes[0xB7] = &Processor::OPCode0xB7;
    m_OPCodes[0xB8] = &Processor::OPCode0xB8;
    m_OPCodes[0xB9] = &Processor::OPCode0xB9;
    m_OPCodes[0xBA] = &Processor::OPCode0xBA;
    m_OPCodes[0xBB] = &Processor::OPCode0xBB;
    m_OPCodes[0xBC] = &Processor::OPCode0xBC;
    m_OPCodes[0xBD] = &Processor::OPCode0xBD;
    m_OPCodes[0xBE] = &Processor::OPCode0xBE;
    m_OPCodes[0xBF] = &Processor::OPCode0xBF;

    m_OPCodes[0xC0] = &Processor::OPCode0xC0;
    m_OPCodes[0xC1] = &Processor::OPCode0xC1;
    m_OPCodes[0xC2] = &Processor::OPCode0xC2;
    m_OPCodes[0xC3] = &Processor::OPCode0xC3;
    m_OPCodes[0xC4] = &Processor::OPCode0xC4;
    m_OPCodes[0xC5] = &Processor::OPCode0xC5;
    m_OPCodes[0xC6] = &Processor::OPCode0xC6;
    m_OPCodes[0xC7] = &Processor::OPCode0xC7;
    m_OPCodes[0xC8] = &Processor::OPCode0xC8;
    m_OPCodes[0xC9] = &Processor::OPCode0xC9;
    m_OPCodes[0xCA] = &Processor::OPCode0xCA;
    m_OPCodes[0xCB] = &Processor::OPCode0xCB;
    m_OPCodes[0xCC] = &Processor::OPCode0xCC;
    m_OPCodes[0xCD] = &Processor::OPCode0xCD;
    m_OPCodes[0xCE] = &Processor::OPCode0xCE;
    m_OPCodes[0xCF] = &Processor::OPCode0xCF;

    m_OPCodes[0xD0] = &Processor::OPCode0xD0;
    m_OPCodes[0xD1] = &Processor::OPCode0xD1;
    m_OPCodes[0xD2] = &Processor::OPCode0xD2;
    m_OPCodes[0xD3] = &Processor::OPCode0xD3;
    m_OPCodes[0xD4] = &Processor::OPCode0xD4;
    m_OPCodes[0xD5] = &Processor::OPCode0xD5;
    m_OPCodes[0xD6] = &Processor::OPCode0xD6;
    m_OPCodes[0xD7] = &Processor::OPCode0xD7;
    m_OPCodes[0xD8] = &Processor::OPCode0xD8;
    m_OPCodes[0xD9] = &Processor::OPCode0xD9;
    m_OPCodes[0xDA] = &Processor::OPCode0xDA;
    m_OPCodes[0xDB] = &Processor::OPCode0xDB;
    m_OPCodes[0xDC] = &Processor::OPCode0xDC;
    m_OPCodes[0xDD] = &Processor::OPCode0xDD;
    m_OPCodes[0xDE] = &Processor::OPCode0xDE;
    m_OPCodes[0xDF] = &Processor::OPCode0xDF;

    m_OPCodes[0xE0] = &Processor::OPCode0xE0;
    m_OPCodes[0xE1] = &Processor::OPCode0xE1;
    m_OPCodes[0xE2] = &Processor::OPCode0xE2;
    m_OPCodes[0xE3] = &Processor::OPCode0xE3;
    m_OPCodes[0xE4] = &Processor::OPCode0xE4;
    m_OPCodes[0xE5] = &Processor::OPCode0xE5;
    m_OPCodes[0xE6] = &Processor::OPCode0xE6;
    m_OPCodes[0xE7] = &Processor::OPCode0xE7;
    m_OPCodes[0xE8] = &Processor::OPCode0xE8;
    m_OPCodes[0xE9] = &Processor::OPCode0xE9;
    m_OPCodes[0xEA] = &Processor::OPCode0xEA;
    m_OPCodes[0xEB] = &Processor::OPCode0xEB;
    m_OPCodes[0xEC] = &Processor::OPCode0xEC;
    m_OPCodes[0xED] = &Processor::OPCode0xED;
    m_OPCodes[0xEE] = &Processor::OPCode0xEE;
    m_OPCodes[0xEF] = &Processor::OPCode0xEF;

    m_OPCodes[0xF0] = &Processor::OPCode0xF0;
    m_OPCodes[0xF1] = &Processor::OPCode0xF1;
    m_OPCodes[0xF2] = &Processor::OPCode0xF2;
    m_OPCodes[0xF3] = &Processor::OPCode0xF3;
    m_OPCodes[0xF4] = &Processor::OPCode0xF4;
    m_OPCodes[0xF5] = &Processor::OPCode0xF5;
    m_OPCodes[0xF6] = &Processor::OPCode0xF6;
    m_OPCodes[0xF7] = &Processor::OPCode0xF7;
    m_OPCodes[0xF8] = &Processor::OPCode0xF8;
    m_OPCodes[0xF9] = &Processor::OPCode0xF9;
    m_OPCodes[0xFA] = &Processor::OPCode0xFA;
    m_OPCodes[0xFB] = &Processor::OPCode0xFB;
    m_OPCodes[0xFC] = &Processor::OPCode0xFC;
    m_OPCodes[0xFD] = &Processor::OPCode0xFD;
    m_OPCodes[0xFE] = &Processor::OPCode0xFE;
    m_OPCodes[0xFF] = &Processor::OPCode0xFF;


    m_OPCodesCB[0x00] = &Processor::OPCodeCB0x00;
    m_OPCodesCB[0x01] = &Processor::OPCodeCB0x01;
    m_OPCodesCB[0x02] = &Processor::OPCodeCB0x02;
    m_OPCodesCB[0x03] = &Processor::OPCodeCB0x03;
    m_OPCodesCB[0x04] = &Processor::OPCodeCB0x04;
    m_OPCodesCB[0x05] = &Processor::OPCodeCB0x05;
    m_OPCodesCB[0x06] = &Processor::OPCodeCB0x06;
    m_OPCodesCB[0x07] = &Processor::OPCodeCB0x07;
    m_OPCodesCB[0x08] = &Processor::OPCodeCB0x08;
    m_OPCodesCB[0x09] = &Processor::OPCodeCB0x09;
    m_OPCodesCB[0x0A] = &Processor::OPCodeCB0x0A;
    m_OPCodesCB[0x0B] = &Processor::OPCodeCB0x0B;
    m_OPCodesCB[0x0C] = &Processor::OPCodeCB0x0C;
    m_OPCodesCB[0x0D] = &Processor::OPCodeCB0x0D;
    m_OPCodesCB[0x0E] = &Processor::OPCodeCB0x0E;
    m_OPCodesCB[0x0F] = &Processor::OPCodeCB0x0F;

    m_OPCodesCB[0x10] = &Processor::OPCodeCB0x10;
    m_OPCodesCB[0x11] = &Processor::OPCodeCB0x11;
    m_OPCodesCB[0x12] = &Processor::OPCodeCB0x12;
    m_OPCodesCB[0x13] = &Processor::OPCodeCB0x13;
    m_OPCodesCB[0x14] = &Processor::OPCodeCB0x14;
    m_OPCodesCB[0x15] = &Processor::OPCodeCB0x15;
    m_OPCodesCB[0x16] = &Processor::OPCodeCB0x16;
    m_OPCodesCB[0x17] = &Processor::OPCodeCB0x17;
    m_OPCodesCB[0x18] = &Processor::OPCodeCB0x18;
    m_OPCodesCB[0x19] = &Processor::OPCodeCB0x19;
    m_OPCodesCB[0x1A] = &Processor::OPCodeCB0x1A;
    m_OPCodesCB[0x1B] = &Processor::OPCodeCB0x1B;
    m_OPCodesCB[0x1C] = &Processor::OPCodeCB0x1C;
    m_OPCodesCB[0x1D] = &Processor::OPCodeCB0x1D;
    m_OPCodesCB[0x1E] = &Processor::OPCodeCB0x1E;
    m_OPCodesCB[0x1F] = &Processor::OPCodeCB0x1F;

    m_OPCodesCB[0x20] = &Processor::OPCodeCB0x20;
    m_OPCodesCB[0x21] = &Processor::OPCodeCB0x21;
    m_OPCodesCB[0x22] = &Processor::OPCodeCB0x22;
    m_OPCodesCB[0x23] = &Processor::OPCodeCB0x23;
    m_OPCodesCB[0x24] = &Processor::OPCodeCB0x24;
    m_OPCodesCB[0x25] = &Processor::OPCodeCB0x25;
    m_OPCodesCB[0x26] = &Processor::OPCodeCB0x26;
    m_OPCodesCB[0x27] = &Processor::OPCodeCB0x27;
    m_OPCodesCB[0x28] = &Processor::OPCodeCB0x28;
    m_OPCodesCB[0x29] = &Processor::OPCodeCB0x29;
    m_OPCodesCB[0x2A] = &Processor::OPCodeCB0x2A;
    m_OPCodesCB[0x2B] = &Processor::OPCodeCB0x2B;
    m_OPCodesCB[0x2C] = &Processor::OPCodeCB0x2C;
    m_OPCodesCB[0x2D] = &Processor::OPCodeCB0x2D;
    m_OPCodesCB[0x2E] = &Processor::OPCodeCB0x2E;
    m_OPCodesCB[0x2F] = &Processor::OPCodeCB0x2F;

    m_OPCodesCB[0x30] = &Processor::OPCodeCB0x30;
    m_OPCodesCB[0x31] = &Processor::OPCodeCB0x31;
    m_OPCodesCB[0x32] = &Processor::OPCodeCB0x32;
    m_OPCodesCB[0x33] = &Processor::OPCodeCB0x33;
    m_OPCodesCB[0x34] = &Processor::OPCodeCB0x34;
    m_OPCodesCB[0x35] = &Processor::OPCodeCB0x35;
    m_OPCodesCB[0x36] = &Processor::OPCodeCB0x36;
    m_OPCodesCB[0x37] = &Processor::OPCodeCB0x37;
    m_OPCodesCB[0x38] = &Processor::OPCodeCB0x38;
    m_OPCodesCB[0x39] = &Processor::OPCodeCB0x39;
    m_OPCodesCB[0x3A] = &Processor::OPCodeCB0x3A;
    m_OPCodesCB[0x3B] = &Processor::OPCodeCB0x3B;
    m_OPCodesCB[0x3C] = &Processor::OPCodeCB0x3C;
    m_OPCodesCB[0x3D] = &Processor::OPCodeCB0x3D;
    m_OPCodesCB[0x3E] = &Processor::OPCodeCB0x3E;
    m_OPCodesCB[0x3F] = &Processor::OPCodeCB0x3F;

    m_OPCodesCB[0x40] = &Processor::OPCodeCB0x40;
    m_OPCodesCB[0x41] = &Processor::OPCodeCB0x41;
    m_OPCodesCB[0x42] = &Processor::OPCodeCB0x42;
    m_OPCodesCB[0x43] = &Processor::OPCodeCB0x43;
    m_OPCodesCB[0x44] = &Processor::OPCodeCB0x44;
    m_OPCodesCB[0x45] = &Processor::OPCodeCB0x45;
    m_OPCodesCB[0x46] = &Processor::OPCodeCB0x46;
    m_OPCodesCB[0x47] = &Processor::OPCodeCB0x47;
    m_OPCodesCB[0x48] = &Processor::OPCodeCB0x48;
    m_OPCodesCB[0x49] = &Processor::OPCodeCB0x49;
    m_OPCodesCB[0x4A] = &Processor::OPCodeCB0x4A;
    m_OPCodesCB[0x4B] = &Processor::OPCodeCB0x4B;
    m_OPCodesCB[0x4C] = &Processor::OPCodeCB0x4C;
    m_OPCodesCB[0x4D] = &Processor::OPCodeCB0x4D;
    m_OPCodesCB[0x4E] = &Processor::OPCodeCB0x4E;
    m_OPCodesCB[0x4F] = &Processor::OPCodeCB0x4F;

    m_OPCodesCB[0x50] = &Processor::OPCodeCB0x50;
    m_OPCodesCB[0x51] = &Processor::OPCodeCB0x51;
    m_OPCodesCB[0x52] = &Processor::OPCodeCB0x52;
    m_OPCodesCB[0x53] = &Processor::OPCodeCB0x53;
    m_OPCodesCB[0x54] = &Processor::OPCodeCB0x54;
    m_OPCodesCB[0x55] = &Processor::OPCodeCB0x55;
    m_OPCodesCB[0x56] = &Processor::OPCodeCB0x56;
    m_OPCodesCB[0x57] = &Processor::OPCodeCB0x57;
    m_OPCodesCB[0x58] = &Processor::OPCodeCB0x58;
    m_OPCodesCB[0x59] = &Processor::OPCodeCB0x59;
    m_OPCodesCB[0x5A] = &Processor::OPCodeCB0x5A;
    m_OPCodesCB[0x5B] = &Processor::OPCodeCB0x5B;
    m_OPCodesCB[0x5C] = &Processor::OPCodeCB0x5C;
    m_OPCodesCB[0x5D] = &Processor::OPCodeCB0x5D;
    m_OPCodesCB[0x5E] = &Processor::OPCodeCB0x5E;
    m_OPCodesCB[0x5F] = &Processor::OPCodeCB0x5F;

    m_OPCodesCB[0x60] = &Processor::OPCodeCB0x60;
    m_OPCodesCB[0x61] = &Processor::OPCodeCB0x61;
    m_OPCodesCB[0x62] = &Processor::OPCodeCB0x62;
    m_OPCodesCB[0x63] = &Processor::OPCodeCB0x63;
    m_OPCodesCB[0x64] = &Processor::OPCodeCB0x64;
    m_OPCodesCB[0x65] = &Processor::OPCodeCB0x65;
    m_OPCodesCB[0x66] = &Processor::OPCodeCB0x66;
    m_OPCodesCB[0x67] = &Processor::OPCodeCB0x67;
    m_OPCodesCB[0x68] = &Processor::OPCodeCB0x68;
    m_OPCodesCB[0x69] = &Processor::OPCodeCB0x69;
    m_OPCodesCB[0x6A] = &Processor::OPCodeCB0x6A;
    m_OPCodesCB[0x6B] = &Processor::OPCodeCB0x6B;
    m_OPCodesCB[0x6C] = &Processor::OPCodeCB0x6C;
    m_OPCodesCB[0x6D] = &Processor::OPCodeCB0x6D;
    m_OPCodesCB[0x6E] = &Processor::OPCodeCB0x6E;
    m_OPCodesCB[0x6F] = &Processor::OPCodeCB0x6F;

    m_OPCodesCB[0x70] = &Processor::OPCodeCB0x70;
    m_OPCodesCB[0x71] = &Processor::OPCodeCB0x71;
    m_OPCodesCB[0x72] = &Processor::OPCodeCB0x72;
    m_OPCodesCB[0x73] = &Processor::OPCodeCB0x73;
    m_OPCodesCB[0x74] = &Processor::OPCodeCB0x74;
    m_OPCodesCB[0x75] = &Processor::OPCodeCB0x75;
    m_OPCodesCB[0x76] = &Processor::OPCodeCB0x76;
    m_OPCodesCB[0x77] = &Processor::OPCodeCB0x77;
    m_OPCodesCB[0x78] = &Processor::OPCodeCB0x78;
    m_OPCodesCB[0x79] = &Processor::OPCodeCB0x79;
    m_OPCodesCB[0x7A] = &Processor::OPCodeCB0x7A;
    m_OPCodesCB[0x7B] = &Processor::OPCodeCB0x7B;
    m_OPCodesCB[0x7C] = &Processor::OPCodeCB0x7C;
    m_OPCodesCB[0x7D] = &Processor::OPCodeCB0x7D;
    m_OPCodesCB[0x7E] = &Processor::OPCodeCB0x7E;
    m_OPCodesCB[0x7F] = &Processor::OPCodeCB0x7F;

    m_OPCodesCB[0x80] = &Processor::OPCodeCB0x80;
    m_OPCodesCB[0x81] = &Processor::OPCodeCB0x81;
    m_OPCodesCB[0x82] = &Processor::OPCodeCB0x82;
    m_OPCodesCB[0x83] = &Processor::OPCodeCB0x83;
    m_OPCodesCB[0x84] = &Processor::OPCodeCB0x84;
    m_OPCodesCB[0x85] = &Processor::OPCodeCB0x85;
    m_OPCodesCB[0x86] = &Processor::OPCodeCB0x86;
    m_OPCodesCB[0x87] = &Processor::OPCodeCB0x87;
    m_OPCodesCB[0x88] = &Processor::OPCodeCB0x88;
    m_OPCodesCB[0x89] = &Processor::OPCodeCB0x89;
    m_OPCodesCB[0x8A] = &Processor::OPCodeCB0x8A;
    m_OPCodesCB[0x8B] = &Processor::OPCodeCB0x8B;
    m_OPCodesCB[0x8C] = &Processor::OPCodeCB0x8C;
    m_OPCodesCB[0x8D] = &Processor::OPCodeCB0x8D;
    m_OPCodesCB[0x8E] = &Processor::OPCodeCB0x8E;
    m_OPCodesCB[0x8F] = &Processor::OPCodeCB0x8F;

    m_OPCodesCB[0x90] = &Processor::OPCodeCB0x90;
    m_OPCodesCB[0x91] = &Processor::OPCodeCB0x91;
    m_OPCodesCB[0x92] = &Processor::OPCodeCB0x92;
    m_OPCodesCB[0x93] = &Processor::OPCodeCB0x93;
    m_OPCodesCB[0x94] = &Processor::OPCodeCB0x94;
    m_OPCodesCB[0x95] = &Processor::OPCodeCB0x95;
    m_OPCodesCB[0x96] = &Processor::OPCodeCB0x96;
    m_OPCodesCB[0x97] = &Processor::OPCodeCB0x97;
    m_OPCodesCB[0x98] = &Processor::OPCodeCB0x98;
    m_OPCodesCB[0x99] = &Processor::OPCodeCB0x99;
    m_OPCodesCB[0x9A] = &Processor::OPCodeCB0x9A;
    m_OPCodesCB[0x9B] = &Processor::OPCodeCB0x9B;
    m_OPCodesCB[0x9C] = &Processor::OPCodeCB0x9C;
    m_OPCodesCB[0x9D] = &Processor::OPCodeCB0x9D;
    m_OPCodesCB[0x9E] = &Processor::OPCodeCB0x9E;
    m_OPCodesCB[0x9F] = &Processor::OPCodeCB0x9F;

    m_OPCodesCB[0xA0] = &Processor::OPCodeCB0xA0;
    m_OPCodesCB[0xA1] = &Processor::OPCodeCB0xA1;
    m_OPCodesCB[0xA2] = &Processor::OPCodeCB0xA2;
    m_OPCodesCB[0xA3] = &Processor::OPCodeCB0xA3;
    m_OPCodesCB[0xA4] = &Processor::OPCodeCB0xA4;
    m_OPCodesCB[0xA5] = &Processor::OPCodeCB0xA5;
    m_OPCodesCB[0xA6] = &Processor::OPCodeCB0xA6;
    m_OPCodesCB[0xA7] = &Processor::OPCodeCB0xA7;
    m_OPCodesCB[0xA8] = &Processor::OPCodeCB0xA8;
    m_OPCodesCB[0xA9] = &Processor::OPCodeCB0xA9;
    m_OPCodesCB[0xAA] = &Processor::OPCodeCB0xAA;
    m_OPCodesCB[0xAB] = &Processor::OPCodeCB0xAB;
    m_OPCodesCB[0xAC] = &Processor::OPCodeCB0xAC;
    m_OPCodesCB[0xAD] = &Processor::OPCodeCB0xAD;
    m_OPCodesCB[0xAE] = &Processor::OPCodeCB0xAE;
    m_OPCodesCB[0xAF] = &Processor::OPCodeCB0xAF;

    m_OPCodesCB[0xB0] = &Processor::OPCodeCB0xB0;
    m_OPCodesCB[0xB1] = &Processor::OPCodeCB0xB1;
    m_OPCodesCB[0xB2] = &Processor::OPCodeCB0xB2;
    m_OPCodesCB[0xB3] = &Processor::OPCodeCB0xB3;
    m_OPCodesCB[0xB4] = &Processor::OPCodeCB0xB4;
    m_OPCodesCB[0xB5] = &Processor::OPCodeCB0xB5;
    m_OPCodesCB[0xB6] = &Processor::OPCodeCB0xB6;
    m_OPCodesCB[0xB7] = &Processor::OPCodeCB0xB7;
    m_OPCodesCB[0xB8] = &Processor::OPCodeCB0xB8;
    m_OPCodesCB[0xB9] = &Processor::OPCodeCB0xB9;
    m_OPCodesCB[0xBA] = &Processor::OPCodeCB0xBA;
    m_OPCodesCB[0xBB] = &Processor::OPCodeCB0xBB;
    m_OPCodesCB[0xBC] = &Processor::OPCodeCB0xBC;
    m_OPCodesCB[0xBD] = &Processor::OPCodeCB0xBD;
    m_OPCodesCB[0xBE] = &Processor::OPCodeCB0xBE;
    m_OPCodesCB[0xBF] = &Processor::OPCodeCB0xBF;

    m_OPCodesCB[0xC0] = &Processor::OPCodeCB0xC0;
    m_OPCodesCB[0xC1] = &Processor::OPCodeCB0xC1;
    m_OPCodesCB[0xC2] = &Processor::OPCodeCB0xC2;
    m_OPCodesCB[0xC3] = &Processor::OPCodeCB0xC3;
    m_OPCodesCB[0xC4] = &Processor::OPCodeCB0xC4;
    m_OPCodesCB[0xC5] = &Processor::OPCodeCB0xC5;
    m_OPCodesCB[0xC6] = &Processor::OPCodeCB0xC6;
    m_OPCodesCB[0xC7] = &Processor::OPCodeCB0xC7;
    m_OPCodesCB[0xC8] = &Processor::OPCodeCB0xC8;
    m_OPCodesCB[0xC9] = &Processor::OPCodeCB0xC9;
    m_OPCodesCB[0xCA] = &Processor::OPCodeCB0xCA;
    m_OPCodesCB[0xCB] = &Processor::OPCodeCB0xCB;
    m_OPCodesCB[0xCC] = &Processor::OPCodeCB0xCC;
    m_OPCodesCB[0xCD] = &Processor::OPCodeCB0xCD;
    m_OPCodesCB[0xCE] = &Processor::OPCodeCB0xCE;
    m_OPCodesCB[0xCF] = &Processor::OPCodeCB0xCF;

    m_OPCodesCB[0xD0] = &Processor::OPCodeCB0xD0;
    m_OPCodesCB[0xD1] = &Processor::OPCodeCB0xD1;
    m_OPCodesCB[0xD2] = &Processor::OPCodeCB0xD2;
    m_OPCodesCB[0xD3] = &Processor::OPCodeCB0xD3;
    m_OPCodesCB[0xD4] = &Processor::OPCodeCB0xD4;
    m_OPCodesCB[0xD5] = &Processor::OPCodeCB0xD5;
    m_OPCodesCB[0xD6] = &Processor::OPCodeCB0xD6;
    m_OPCodesCB[0xD7] = &Processor::OPCodeCB0xD7;
    m_OPCodesCB[0xD8] = &Processor::OPCodeCB0xD8;
    m_OPCodesCB[0xD9] = &Processor::OPCodeCB0xD9;
    m_OPCodesCB[0xDA] = &Processor::OPCodeCB0xDA;
    m_OPCodesCB[0xDB] = &Processor::OPCodeCB0xDB;
    m_OPCodesCB[0xDC] = &Processor::OPCodeCB0xDC;
    m_OPCodesCB[0xDD] = &Processor::OPCodeCB0xDD;
    m_OPCodesCB[0xDE] = &Processor::OPCodeCB0xDE;
    m_OPCodesCB[0xDF] = &Processor::OPCodeCB0xDF;

    m_OPCodesCB[0xE0] = &Processor::OPCodeCB0xE0;
    m_OPCodesCB[0xE1] = &Processor::OPCodeCB0xE1;
    m_OPCodesCB[0xE2] = &Processor::OPCodeCB0xE2;
    m_OPCodesCB[0xE3] = &Processor::OPCodeCB0xE3;
    m_OPCodesCB[0xE4] = &Processor::OPCodeCB0xE4;
    m_OPCodesCB[0xE5] = &Processor::OPCodeCB0xE5;
    m_OPCodesCB[0xE6] = &Processor::OPCodeCB0xE6;
    m_OPCodesCB[0xE7] = &Processor::OPCodeCB0xE7;
    m_OPCodesCB[0xE8] = &Processor::OPCodeCB0xE8;
    m_OPCodesCB[0xE9] = &Processor::OPCodeCB0xE9;
    m_OPCodesCB[0xEA] = &Processor::OPCodeCB0xEA;
    m_OPCodesCB[0xEB] = &Processor::OPCodeCB0xEB;
    m_OPCodesCB[0xEC] = &Processor::OPCodeCB0xEC;
    m_OPCodesCB[0xED] = &Processor::OPCodeCB0xED;
    m_OPCodesCB[0xEE] = &Processor::OPCodeCB0xEE;
    m_OPCodesCB[0xEF] = &Processor::OPCodeCB0xEF;

    m_OPCodesCB[0xF0] = &Processor::OPCodeCB0xF0;
    m_OPCodesCB[0xF1] = &Processor::OPCodeCB0xF1;
    m_OPCodesCB[0xF2] = &Processor::OPCodeCB0xF2;
    m_OPCodesCB[0xF3] = &Processor::OPCodeCB0xF3;
    m_OPCodesCB[0xF4] = &Processor::OPCodeCB0xF4;
    m_OPCodesCB[0xF5] = &Processor::OPCodeCB0xF5;
    m_OPCodesCB[0xF6] = &Processor::OPCodeCB0xF6;
    m_OPCodesCB[0xF7] = &Processor::OPCodeCB0xF7;
    m_OPCodesCB[0xF8] = &Processor::OPCodeCB0xF8;
    m_OPCodesCB[0xF9] = &Processor::OPCodeCB0xF9;
    m_OPCodesCB[0xFA] = &Processor::OPCodeCB0xFA;
    m_OPCodesCB[0xFB] = &Processor::OPCodeCB0xFB;
    m_OPCodesCB[0xFC] = &Processor::OPCodeCB0xFC;
    m_OPCodesCB[0xFD] = &Processor::OPCodeCB0xFD;
    m_OPCodesCB[0xFE] = &Processor::OPCodeCB0xFE;
    m_OPCodesCB[0xFF] = &Processor::OPCodeCB0xFF;
}
