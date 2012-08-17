/* 
 * Gearboy Gameboy Emulator
 * Copyright (C) 2012 Ignacio Sanchez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * The full license is available at http://www.gnu.org/licenses/gpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "Processor.h"

void Processor::OPCodeCB0x00()
{
    // RLC B
    OPCodes_RLC(BC.GetHighRegister());
}

void Processor::OPCodeCB0x01()
{
    // RLC C
    OPCodes_RLC(BC.GetLowRegister());
}

void Processor::OPCodeCB0x02()
{
    // RLC D
    OPCodes_RLC(DE.GetHighRegister());
}

void Processor::OPCodeCB0x03()
{
    // RLC E
    OPCodes_RLC(DE.GetLowRegister());
}

void Processor::OPCodeCB0x04()
{
    // RLC H
    OPCodes_RLC(HL.GetHighRegister());
}

void Processor::OPCodeCB0x05()
{
    // RLC L
    OPCodes_RLC(HL.GetLowRegister());
}

void Processor::OPCodeCB0x06()
{
    // RLC (HL)
    OPCodes_RLC_HL();
}

void Processor::OPCodeCB0x07()
{
    // RLC A
    OPCodes_RLC(AF.GetHighRegister());
}

void Processor::OPCodeCB0x08()
{
    // RRC B
    OPCodes_RRC(BC.GetHighRegister());
}

void Processor::OPCodeCB0x09()
{
    // RRC C
    OPCodes_RRC(BC.GetLowRegister());
}

void Processor::OPCodeCB0x0A()
{
    // RRC D
    OPCodes_RRC(DE.GetHighRegister());
}

void Processor::OPCodeCB0x0B()
{
    // RRC E
    OPCodes_RRC(DE.GetLowRegister());
}

void Processor::OPCodeCB0x0C()
{
    // RRC H
    OPCodes_RRC(HL.GetHighRegister());
}

void Processor::OPCodeCB0x0D()
{
    // RRC L
    OPCodes_RRC(HL.GetLowRegister());
}

void Processor::OPCodeCB0x0E()
{
    // RRC (HL)
    OPCodes_RRC_HL();
}

void Processor::OPCodeCB0x0F()
{
    // RRC A
    OPCodes_RRC(AF.GetHighRegister());
}

void Processor::OPCodeCB0x10()
{
    // RL B
    OPCodes_RL(BC.GetHighRegister());
}

void Processor::OPCodeCB0x11()
{
    // RL C
    OPCodes_RL(BC.GetLowRegister());
}

void Processor::OPCodeCB0x12()
{
    // RL D
    OPCodes_RL(DE.GetHighRegister());
}

void Processor::OPCodeCB0x13()
{
    // RL E
    OPCodes_RL(DE.GetLowRegister());
}

void Processor::OPCodeCB0x14()
{
    // RL H
    OPCodes_RL(HL.GetHighRegister());
}

void Processor::OPCodeCB0x15()
{
    // RL L
    OPCodes_RL(HL.GetLowRegister());
}

void Processor::OPCodeCB0x16()
{
    // RL (HL)
    OPCodes_RL_HL();
}

void Processor::OPCodeCB0x17()
{
    // RL A
    OPCodes_RL(AF.GetHighRegister());
}

void Processor::OPCodeCB0x18()
{
    // RR B
    OPCodes_RR(BC.GetHighRegister());
}

void Processor::OPCodeCB0x19()
{
    // RR C
    OPCodes_RR(BC.GetLowRegister());
}

void Processor::OPCodeCB0x1A()
{
    // RR D
    OPCodes_RR(DE.GetHighRegister());
}

void Processor::OPCodeCB0x1B()
{
    // RR E
    OPCodes_RR(DE.GetLowRegister());
}

void Processor::OPCodeCB0x1C()
{
    // RR H
    OPCodes_RR(HL.GetHighRegister());
}

void Processor::OPCodeCB0x1D()
{
    // RR L
    OPCodes_RR(HL.GetLowRegister());
}

void Processor::OPCodeCB0x1E()
{
    // RR (HL)
    OPCodes_RR_HL();
}

void Processor::OPCodeCB0x1F()
{
    // RR A
    OPCodes_RR(AF.GetHighRegister());
}

void Processor::OPCodeCB0x20()
{
    // SLA B
    OPCodes_SLA(BC.GetHighRegister());
}

void Processor::OPCodeCB0x21()
{
    // SLA C
    OPCodes_SLA(BC.GetLowRegister());
}

void Processor::OPCodeCB0x22()
{
    // SLA D
    OPCodes_SLA(DE.GetHighRegister());
}

void Processor::OPCodeCB0x23()
{
    // SLA E
    OPCodes_SLA(DE.GetLowRegister());
}

void Processor::OPCodeCB0x24()
{
    // SLA H
    OPCodes_SLA(HL.GetHighRegister());
}

void Processor::OPCodeCB0x25()
{
    // SLA L
    OPCodes_SLA(HL.GetLowRegister());
}

void Processor::OPCodeCB0x26()
{
    // SLA (HL)
    OPCodes_SLA_HL();
}

void Processor::OPCodeCB0x27()
{
    // SLA A
    OPCodes_SLA(AF.GetHighRegister());
}

void Processor::OPCodeCB0x28()
{
    // SRA B
    OPCodes_SRA(BC.GetHighRegister());
}

void Processor::OPCodeCB0x29()
{
    // SRA C
    OPCodes_SRA(BC.GetLowRegister());
}

void Processor::OPCodeCB0x2A()
{
    // SRA D
    OPCodes_SRA(DE.GetHighRegister());
}

void Processor::OPCodeCB0x2B()
{
    // SRA E
    OPCodes_SRA(DE.GetLowRegister());
}

void Processor::OPCodeCB0x2C()
{
    // SRA H
    OPCodes_SRA(HL.GetHighRegister());
}

void Processor::OPCodeCB0x2D()
{
    // SRA L
    OPCodes_SRA(HL.GetLowRegister());
}

void Processor::OPCodeCB0x2E()
{
    // SRA (HL)
    OPCodes_SRA_HL();
}

void Processor::OPCodeCB0x2F()
{
    // SRA A
    OPCodes_SRA(AF.GetHighRegister());
}

void Processor::OPCodeCB0x30()
{
    // SWAP B
    OPCodes_SWAP_Register(BC.GetHighRegister());
}

void Processor::OPCodeCB0x31()
{
    // SWAP C
    OPCodes_SWAP_Register(BC.GetLowRegister());
}

void Processor::OPCodeCB0x32()
{
    // SWAP D
    OPCodes_SWAP_Register(DE.GetHighRegister());
}

void Processor::OPCodeCB0x33()
{
    // SWAP E
    OPCodes_SWAP_Register(DE.GetLowRegister());
}

void Processor::OPCodeCB0x34()
{
    // SWAP H
    OPCodes_SWAP_Register(HL.GetHighRegister());
}

void Processor::OPCodeCB0x35()
{
    // SWAP L
    OPCodes_SWAP_Register(HL.GetLowRegister());
}

void Processor::OPCodeCB0x36()
{
    // SWAP (HL)
    OPCodes_SWAP_HL();
}

void Processor::OPCodeCB0x37()
{
    // SWAP A
    OPCodes_SWAP_Register(AF.GetHighRegister());
}

void Processor::OPCodeCB0x38()
{
    // SRL B
    OPCodes_SRL(BC.GetHighRegister());
}

void Processor::OPCodeCB0x39()
{
    // SRL C
    OPCodes_SRL(BC.GetLowRegister());
}

void Processor::OPCodeCB0x3A()
{
    // SRL D
    OPCodes_SRL(DE.GetHighRegister());
}

void Processor::OPCodeCB0x3B()
{
    // SRL E
    OPCodes_SRL(DE.GetLowRegister());
}

void Processor::OPCodeCB0x3C()
{
    // SRL H
    OPCodes_SRL(HL.GetHighRegister());
}

void Processor::OPCodeCB0x3D()
{
    // SRL L
    OPCodes_SRL(HL.GetLowRegister());
}

void Processor::OPCodeCB0x3E()
{
    // SRL (HL)
    OPCodes_SRL_HL();
}

void Processor::OPCodeCB0x3F()
{
    // SRL A
    OPCodes_SRL(AF.GetHighRegister());
}

void Processor::OPCodeCB0x40()
{
    // BIT 0 B
    OPCodes_BIT(BC.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x41()
{
    // BIT 0 C
    OPCodes_BIT(BC.GetLowRegister(), 0);
}

void Processor::OPCodeCB0x42()
{
    // BIT 0 D
    OPCodes_BIT(DE.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x43()
{
    // BIT 0 E
    OPCodes_BIT(DE.GetLowRegister(), 0);
}

void Processor::OPCodeCB0x44()
{
    // BIT 0 H
    OPCodes_BIT(HL.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x45()
{
    // BIT 0 L
    OPCodes_BIT(HL.GetLowRegister(), 0);
}

void Processor::OPCodeCB0x46()
{
    // BIT 0 (HL)
    OPCodes_BIT_HL(0);
}

void Processor::OPCodeCB0x47()
{
    // BIT 0 A
    OPCodes_BIT(AF.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x48()
{
    // BIT 1 B
    OPCodes_BIT(BC.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x49()
{
    // BIT 1 C
    OPCodes_BIT(BC.GetLowRegister(), 1);
}

void Processor::OPCodeCB0x4A()
{
    // BIT 1 D
    OPCodes_BIT(DE.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x4B()
{
    // BIT 1 E
    OPCodes_BIT(DE.GetLowRegister(), 1);
}

void Processor::OPCodeCB0x4C()
{
    // BIT 1 H
    OPCodes_BIT(HL.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x4D()
{
    // BIT 1 L
    OPCodes_BIT(HL.GetLowRegister(), 1);
}

void Processor::OPCodeCB0x4E()
{
    // BIT 1 (HL)
    OPCodes_BIT_HL(1);
}

void Processor::OPCodeCB0x4F()
{
    // BIT 1 A
    OPCodes_BIT(AF.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x50()
{
    // BIT 2 B
    OPCodes_BIT(BC.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x51()
{
    // BIT 2 C
    OPCodes_BIT(BC.GetLowRegister(), 2);
}

void Processor::OPCodeCB0x52()
{
    // BIT 2 D
    OPCodes_BIT(DE.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x53()
{
    // BIT 2 E
    OPCodes_BIT(DE.GetLowRegister(), 2);
}

void Processor::OPCodeCB0x54()
{
    // BIT 2 H
    OPCodes_BIT(HL.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x55()
{
    // BIT 2 L
    OPCodes_BIT(HL.GetLowRegister(), 2);
}

void Processor::OPCodeCB0x56()
{
    // BIT 2 (HL)
    OPCodes_BIT_HL(2);
}

void Processor::OPCodeCB0x57()
{
    // BIT 2 A
    OPCodes_BIT(AF.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x58()
{
    // BIT 3 B
    OPCodes_BIT(BC.GetHighRegister(), 3);
}

void Processor::OPCodeCB0x59()
{
    // BIT 3 C
    OPCodes_BIT(BC.GetLowRegister(), 3);
}

void Processor::OPCodeCB0x5A()
{
    // BIT 3 D
    OPCodes_BIT(DE.GetHighRegister(), 3);
}

void Processor::OPCodeCB0x5B()
{
    // BIT 3 E
    OPCodes_BIT(DE.GetLowRegister(), 3);
}

void Processor::OPCodeCB0x5C()
{
    // BIT 3 H
    OPCodes_BIT(HL.GetHighRegister(), 3);
}

void Processor::OPCodeCB0x5D()
{
    // BIT 3 L
    OPCodes_BIT(HL.GetLowRegister(), 3);
}

void Processor::OPCodeCB0x5E()
{
    // BIT 3 (HL)
    OPCodes_BIT_HL(3);
}

void Processor::OPCodeCB0x5F()
{
    // BIT 3 A
    OPCodes_BIT(AF.GetHighRegister(), 3);
}

void Processor::OPCodeCB0x60()
{
    // BIT 4 B
    OPCodes_BIT(BC.GetHighRegister(), 4);
}

void Processor::OPCodeCB0x61()
{
    // BIT 4 C
    OPCodes_BIT(BC.GetLowRegister(), 4);
}

void Processor::OPCodeCB0x62()
{
    // BIT 4 D
    OPCodes_BIT(DE.GetHighRegister(), 4);
}

void Processor::OPCodeCB0x63()
{
    // BIT 4 E
    OPCodes_BIT(DE.GetLowRegister(), 4);
}

void Processor::OPCodeCB0x64()
{
    // BIT 4 H
    OPCodes_BIT(HL.GetHighRegister(), 4);
}

void Processor::OPCodeCB0x65()
{
    // BIT 4 L
    OPCodes_BIT(HL.GetLowRegister(), 4);
}

void Processor::OPCodeCB0x66()
{
    // BIT 4 (HL)
    OPCodes_BIT_HL(4);
}

void Processor::OPCodeCB0x67()
{
    // BIT 4 A
    OPCodes_BIT(AF.GetHighRegister(), 4);
}

void Processor::OPCodeCB0x68()
{
    // BIT 5 B
    OPCodes_BIT(BC.GetHighRegister(), 5);
}

void Processor::OPCodeCB0x69()
{
    // BIT 5 C
    OPCodes_BIT(BC.GetLowRegister(), 5);
}

void Processor::OPCodeCB0x6A()
{
    // BIT 5 D
    OPCodes_BIT(DE.GetHighRegister(), 5);
}

void Processor::OPCodeCB0x6B()
{
    // BIT 5 E
    OPCodes_BIT(DE.GetLowRegister(), 5);
}

void Processor::OPCodeCB0x6C()
{
    // BIT 5 H
    OPCodes_BIT(HL.GetHighRegister(), 5);
}

void Processor::OPCodeCB0x6D()
{
    // BIT 5 L
    OPCodes_BIT(HL.GetLowRegister(), 5);
}

void Processor::OPCodeCB0x6E()
{
    // BIT 5 (HL)
    OPCodes_BIT_HL(5);
}

void Processor::OPCodeCB0x6F()
{
    // BIT 5 A
    OPCodes_BIT(AF.GetHighRegister(), 5);
}

void Processor::OPCodeCB0x70()
{
    // BIT 6 B
    OPCodes_BIT(BC.GetHighRegister(), 6);
}

void Processor::OPCodeCB0x71()
{
    // BIT 6 C
    OPCodes_BIT(BC.GetLowRegister(), 6);
}

void Processor::OPCodeCB0x72()
{
    // BIT 6 D
    OPCodes_BIT(DE.GetHighRegister(), 6);
}

void Processor::OPCodeCB0x73()
{
    // BIT 6 E
    OPCodes_BIT(DE.GetLowRegister(), 6);
}

void Processor::OPCodeCB0x74()
{
    // BIT 6 H
    OPCodes_BIT(HL.GetHighRegister(), 6);
}

void Processor::OPCodeCB0x75()
{
    // BIT 6 L
    OPCodes_BIT(HL.GetLowRegister(), 6);
}

void Processor::OPCodeCB0x76()
{
    // BIT 6 (HL)
    OPCodes_BIT_HL(6);
}

void Processor::OPCodeCB0x77()
{
    // BIT 6 A
    OPCodes_BIT(AF.GetHighRegister(), 6);
}

void Processor::OPCodeCB0x78()
{
    // BIT 7 B
    OPCodes_BIT(BC.GetHighRegister(), 7);
}

void Processor::OPCodeCB0x79()
{
    // BIT 7 C
    OPCodes_BIT(BC.GetLowRegister(), 7);
}

void Processor::OPCodeCB0x7A()
{
    // BIT 7 D
    OPCodes_BIT(DE.GetHighRegister(), 7);
}

void Processor::OPCodeCB0x7B()
{
    // BIT 7 E
    OPCodes_BIT(DE.GetLowRegister(), 7);
}

void Processor::OPCodeCB0x7C()
{
    // BIT 7 H
    OPCodes_BIT(HL.GetHighRegister(), 7);
}

void Processor::OPCodeCB0x7D()
{
    // BIT 7 L
    OPCodes_BIT(HL.GetLowRegister(), 7);
}

void Processor::OPCodeCB0x7E()
{
    // BIT 7 (HL)
    OPCodes_BIT_HL(7);
}

void Processor::OPCodeCB0x7F()
{
    // BIT 7 A
    OPCodes_BIT(AF.GetHighRegister(), 7);
}

void Processor::OPCodeCB0x80()
{
    // RES 0 B
    OPCodes_RES(BC.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x81()
{
    // RES 0 C
    OPCodes_RES(BC.GetLowRegister(), 0);
}

void Processor::OPCodeCB0x82()
{
    // RES 0 D
    OPCodes_RES(DE.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x83()
{
    // RES 0 E
    OPCodes_RES(DE.GetLowRegister(), 0);
}

void Processor::OPCodeCB0x84()
{
    // RES 0 H
    OPCodes_RES(HL.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x85()
{
    // RES 0 L
    OPCodes_RES(HL.GetLowRegister(), 0);
}

void Processor::OPCodeCB0x86()
{
    // RES 0 (HL)
    OPCodes_RES_HL(0);
}

void Processor::OPCodeCB0x87()
{
    // RES 0 A
    OPCodes_RES(AF.GetHighRegister(), 0);
}

void Processor::OPCodeCB0x88()
{
    // RES 1 B
    OPCodes_RES(BC.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x89()
{
    // RES 1 C
    OPCodes_RES(BC.GetLowRegister(), 1);
}

void Processor::OPCodeCB0x8A()
{
    // RES 1 D
    OPCodes_RES(DE.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x8B()
{
    // RES 1 E
    OPCodes_RES(DE.GetLowRegister(), 1);
}

void Processor::OPCodeCB0x8C()
{
    // RES 1 H
    OPCodes_RES(HL.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x8D()
{
    // RES 1 L
    OPCodes_RES(HL.GetLowRegister(), 1);
}

void Processor::OPCodeCB0x8E()
{
    // RES 1 (HL)
    OPCodes_RES_HL(1);
}

void Processor::OPCodeCB0x8F()
{
    // RES 1 A
    OPCodes_RES(AF.GetHighRegister(), 1);
}

void Processor::OPCodeCB0x90()
{
    // RES 2 B
    OPCodes_RES(BC.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x91()
{
    // RES 2 C
    OPCodes_RES(BC.GetLowRegister(), 2);
}

void Processor::OPCodeCB0x92()
{
    // RES 2 D
    OPCodes_RES(DE.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x93()
{
    // RES 2 E
    OPCodes_RES(DE.GetLowRegister(), 2);
}

void Processor::OPCodeCB0x94()
{
    // RES 2 H
    OPCodes_RES(HL.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x95()
{
    // RES 2 L
    OPCodes_RES(HL.GetLowRegister(), 2);
}

void Processor::OPCodeCB0x96()
{
    // RES 2 (HL)
    OPCodes_RES_HL(2);
}

void Processor::OPCodeCB0x97()
{
    // RES 2 A
    OPCodes_RES(AF.GetHighRegister(), 2);
}

void Processor::OPCodeCB0x98()
{
    // RES 3 B
    OPCodes_RES(BC.GetHighRegister(), 3);
}

void Processor::OPCodeCB0x99()
{
    // RES 3 C
    OPCodes_RES(BC.GetLowRegister(), 3);
}

void Processor::OPCodeCB0x9A()
{
    // RES 3 D
    OPCodes_RES(DE.GetHighRegister(), 3);
}

void Processor::OPCodeCB0x9B()
{
    // RES 3 E
    OPCodes_RES(DE.GetLowRegister(), 3);
}

void Processor::OPCodeCB0x9C()
{
    // RES 3 H
    OPCodes_RES(HL.GetHighRegister(), 3);
}

void Processor::OPCodeCB0x9D()
{
    // RES 3 L
    OPCodes_RES(HL.GetLowRegister(), 3);
}

void Processor::OPCodeCB0x9E()
{
    // RES 3 (HL)
    OPCodes_RES_HL(3);
}

void Processor::OPCodeCB0x9F()
{
    // RES 3 A
    OPCodes_RES(AF.GetHighRegister(), 3);
}

void Processor::OPCodeCB0xA0()
{
    // RES 4 B
    OPCodes_RES(BC.GetHighRegister(), 4);
}

void Processor::OPCodeCB0xA1()
{
    // RES 4 C
    OPCodes_RES(BC.GetLowRegister(), 4);
}

void Processor::OPCodeCB0xA2()
{
    // RES 4 D
    OPCodes_RES(DE.GetHighRegister(), 4);
}

void Processor::OPCodeCB0xA3()
{
    // RES 4 E
    OPCodes_RES(DE.GetLowRegister(), 4);
}

void Processor::OPCodeCB0xA4()
{
    // RES 4 H
    OPCodes_RES(HL.GetHighRegister(), 4);
}

void Processor::OPCodeCB0xA5()
{
    // RES 4 L
    OPCodes_RES(HL.GetLowRegister(), 4);
}

void Processor::OPCodeCB0xA6()
{
    // RES 4 (HL)
    OPCodes_RES_HL(4);
}

void Processor::OPCodeCB0xA7()
{
    // RES 4 A
    OPCodes_RES(AF.GetHighRegister(), 4);
}

void Processor::OPCodeCB0xA8()
{
    // RES 5 B
    OPCodes_RES(BC.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xA9()
{
    // RES 5 C
    OPCodes_RES(BC.GetLowRegister(), 5);
}

void Processor::OPCodeCB0xAA()
{
    // RES 5 D
    OPCodes_RES(DE.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xAB()
{
    // RES 5 E
    OPCodes_RES(DE.GetLowRegister(), 5);
}

void Processor::OPCodeCB0xAC()
{
    // RES 5 H
    OPCodes_RES(HL.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xAD()
{
    // RES 5 L
    OPCodes_RES(HL.GetLowRegister(), 5);
}

void Processor::OPCodeCB0xAE()
{
    // RES 5 (HL)
    OPCodes_RES_HL(5);
}

void Processor::OPCodeCB0xAF()
{
    // RES 5 A
    OPCodes_RES(AF.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xB0()
{
    // RES 6 B
    OPCodes_RES(BC.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xB1()
{
    // RES 6 C
    OPCodes_RES(BC.GetLowRegister(), 6);
}

void Processor::OPCodeCB0xB2()
{
    // RES 6 D
    OPCodes_RES(DE.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xB3()
{
    // RES 6 E
    OPCodes_RES(DE.GetLowRegister(), 6);
}

void Processor::OPCodeCB0xB4()
{
    // RES 6 H
    OPCodes_RES(HL.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xB5()
{
    // RES 6 L
    OPCodes_RES(HL.GetLowRegister(), 6);
}

void Processor::OPCodeCB0xB6()
{
    // RES 6 (HL)
    OPCodes_RES_HL(6);
}

void Processor::OPCodeCB0xB7()
{
    // RES 6 A
    OPCodes_RES(AF.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xB8()
{
    // RES 7 B
    OPCodes_RES(BC.GetHighRegister(), 7);
}

void Processor::OPCodeCB0xB9()
{
    // RES 7 C
    OPCodes_RES(BC.GetLowRegister(), 7);
}

void Processor::OPCodeCB0xBA()
{
    // RES 7 D
    OPCodes_RES(DE.GetHighRegister(), 7);
}

void Processor::OPCodeCB0xBB()
{
    // RES 7 E
    OPCodes_RES(DE.GetLowRegister(), 7);
}

void Processor::OPCodeCB0xBC()
{
    // RES 7 H
    OPCodes_RES(HL.GetHighRegister(), 7);
}

void Processor::OPCodeCB0xBD()
{
    // RES 7 L
    OPCodes_RES(HL.GetLowRegister(), 7);
}

void Processor::OPCodeCB0xBE()
{
    // RES 7 (HL)
    OPCodes_RES_HL(7);
}

void Processor::OPCodeCB0xBF()
{
    // RES 7 A
    OPCodes_RES(AF.GetHighRegister(), 7);
}

void Processor::OPCodeCB0xC0()
{
    // SET 0 B
    OPCodes_SET(BC.GetHighRegister(), 0);
}

void Processor::OPCodeCB0xC1()
{
    // SET 0 C
    OPCodes_SET(BC.GetLowRegister(), 0);
}

void Processor::OPCodeCB0xC2()
{
    // SET 0 D
    OPCodes_SET(DE.GetHighRegister(), 0);
}

void Processor::OPCodeCB0xC3()
{
    // SET 0 E
    OPCodes_SET(DE.GetLowRegister(), 0);
}

void Processor::OPCodeCB0xC4()
{
    // SET 0 H
    OPCodes_SET(HL.GetHighRegister(), 0);
}

void Processor::OPCodeCB0xC5()
{
    // SET 0 L
    OPCodes_SET(HL.GetLowRegister(), 0);
}

void Processor::OPCodeCB0xC6()
{
    // SET 0 (HL)
    OPCodes_SET_HL(0);
}

void Processor::OPCodeCB0xC7()
{
    // SET 0 A
    OPCodes_SET(AF.GetHighRegister(), 0);
}

void Processor::OPCodeCB0xC8()
{
    // SET 1 B
    OPCodes_SET(BC.GetHighRegister(), 1);
}

void Processor::OPCodeCB0xC9()
{
    // SET 1 C
    OPCodes_SET(BC.GetLowRegister(), 1);
}

void Processor::OPCodeCB0xCA()
{
    // SET 1 D
    OPCodes_SET(DE.GetHighRegister(), 1);
}

void Processor::OPCodeCB0xCB()
{
    // SET 1 E
    OPCodes_SET(DE.GetLowRegister(), 1);
}

void Processor::OPCodeCB0xCC()
{
    // SET 1 H
    OPCodes_SET(HL.GetHighRegister(), 1);
}

void Processor::OPCodeCB0xCD()
{
    // SET 1 L
    OPCodes_SET(HL.GetLowRegister(), 1);
}

void Processor::OPCodeCB0xCE()
{
    // SET 1 (HL)
    OPCodes_SET_HL(1);
}

void Processor::OPCodeCB0xCF()
{
    // SET 1 A
    OPCodes_SET(AF.GetHighRegister(), 1);
}

void Processor::OPCodeCB0xD0()
{
    // SET 2 B
    OPCodes_SET(BC.GetHighRegister(), 2);
}

void Processor::OPCodeCB0xD1()
{
    // SET 2 C
    OPCodes_SET(BC.GetLowRegister(), 2);
}

void Processor::OPCodeCB0xD2()
{
    // SET 2 D
    OPCodes_SET(DE.GetHighRegister(), 2);
}

void Processor::OPCodeCB0xD3()
{
    // SET 2 E
    OPCodes_SET(DE.GetLowRegister(), 2);
}

void Processor::OPCodeCB0xD4()
{
    // SET 2 H
    OPCodes_SET(HL.GetHighRegister(), 2);
}

void Processor::OPCodeCB0xD5()
{
    // SET 2 L
    OPCodes_SET(HL.GetLowRegister(), 2);
}

void Processor::OPCodeCB0xD6()
{
    // SET 2 (HL)
    OPCodes_SET_HL(2);
}

void Processor::OPCodeCB0xD7()
{
    // SET 2 A
    OPCodes_SET(AF.GetHighRegister(), 2);
}

void Processor::OPCodeCB0xD8()
{
    // SET 3 B
    OPCodes_SET(BC.GetHighRegister(), 3);
}

void Processor::OPCodeCB0xD9()
{
    // SET 3 C
    OPCodes_SET(BC.GetLowRegister(), 3);
}

void Processor::OPCodeCB0xDA()
{
    // SET 3 D
    OPCodes_SET(DE.GetHighRegister(), 3);
}

void Processor::OPCodeCB0xDB()
{
    // SET 3 E
    OPCodes_SET(DE.GetLowRegister(), 3);
}

void Processor::OPCodeCB0xDC()
{
    // SET 3 H
    OPCodes_SET(HL.GetHighRegister(), 3);
}

void Processor::OPCodeCB0xDD()
{
    // SET 3 L
    OPCodes_SET(HL.GetLowRegister(), 3);
}

void Processor::OPCodeCB0xDE()
{
    // SET 3 (HL)
    OPCodes_SET_HL(3);
}

void Processor::OPCodeCB0xDF()
{
    // SET 3 A
    OPCodes_SET(AF.GetHighRegister(), 3);
}

void Processor::OPCodeCB0xE0()
{
    // SET 4 B
    OPCodes_SET(BC.GetHighRegister(), 4);
}

void Processor::OPCodeCB0xE1()
{
    // SET 4 C
    OPCodes_SET(BC.GetLowRegister(), 4);
}

void Processor::OPCodeCB0xE2()
{
    // SET 4 D
    OPCodes_SET(DE.GetHighRegister(), 4);
}

void Processor::OPCodeCB0xE3()
{
    // SET 4 E
    OPCodes_SET(DE.GetLowRegister(), 4);
}

void Processor::OPCodeCB0xE4()
{
    // SET 4 H
    OPCodes_SET(HL.GetHighRegister(), 4);
}

void Processor::OPCodeCB0xE5()
{
    // SET 4 L
    OPCodes_SET(HL.GetLowRegister(), 4);
}

void Processor::OPCodeCB0xE6()
{
    // SET 4 (HL)
    OPCodes_SET_HL(4);
}

void Processor::OPCodeCB0xE7()
{
    // SET 4 A
    OPCodes_SET(AF.GetHighRegister(), 4);

}

void Processor::OPCodeCB0xE8()
{
    // SET 5 B
    OPCodes_SET(BC.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xE9()
{
    // SET 5 C
    OPCodes_SET(BC.GetLowRegister(), 5);
}

void Processor::OPCodeCB0xEA()
{
    // SET 5 D
    OPCodes_SET(DE.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xEB()
{
    // SET 5 E
    OPCodes_SET(DE.GetLowRegister(), 5);
}

void Processor::OPCodeCB0xEC()
{
    // SET 5 H
    OPCodes_SET(HL.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xED()
{
    // SET 5 L
    OPCodes_SET(HL.GetLowRegister(), 5);
}

void Processor::OPCodeCB0xEE()
{
    // SET 5 (HL)
    OPCodes_SET_HL(5);
}

void Processor::OPCodeCB0xEF()
{
    // SET 5 A
    OPCodes_SET(AF.GetHighRegister(), 5);
}

void Processor::OPCodeCB0xF0()
{
    // SET 6 B
    OPCodes_SET(BC.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xF1()
{
    // SET 6 C
    OPCodes_SET(BC.GetLowRegister(), 6);
}

void Processor::OPCodeCB0xF2()
{
    // SET 6 D
    OPCodes_SET(DE.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xF3()
{
    // SET 6 E
    OPCodes_SET(DE.GetLowRegister(), 6);
}

void Processor::OPCodeCB0xF4()
{
    // SET 6 H
    OPCodes_SET(HL.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xF5()
{
    // SET 6 L
    OPCodes_SET(HL.GetLowRegister(), 6);
}

void Processor::OPCodeCB0xF6()
{
    // SET 6 (HL)
    OPCodes_SET_HL(6);
}

void Processor::OPCodeCB0xF7()
{
    // SET 6 A
    OPCodes_SET(AF.GetHighRegister(), 6);
}

void Processor::OPCodeCB0xF8()
{
    // SET 7 B
    OPCodes_SET(BC.GetHighRegister(), 7);
}

void Processor::OPCodeCB0xF9()
{
    // SET 7 C
    OPCodes_SET(BC.GetLowRegister(), 7);
}

void Processor::OPCodeCB0xFA()
{
    // SET 7 D
    OPCodes_SET(DE.GetHighRegister(), 7);
}

void Processor::OPCodeCB0xFB()
{
    // SET 7 E
    OPCodes_SET(DE.GetLowRegister(), 7);
}

void Processor::OPCodeCB0xFC()
{
    // SET 7 H
    OPCodes_SET(HL.GetHighRegister(), 7);
}

void Processor::OPCodeCB0xFD()
{
    // SET 7 L
    OPCodes_SET(HL.GetLowRegister(), 7);
}

void Processor::OPCodeCB0xFE()
{
    // SET 7 (HL)
    OPCodes_SET_HL(7);
}

void Processor::OPCodeCB0xFF()
{
    // SET 7 A
    OPCodes_SET(AF.GetHighRegister(), 7);
}
