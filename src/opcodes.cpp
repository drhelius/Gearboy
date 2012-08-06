#include "Processor.h"
#include "Memory.h"

void Processor::OPCode0x00()
{
    // NOP
}

void Processor::OPCode0x01()
{
    // LD BC,nn
    OPCodes_LD(BC.GetLowRegister(), PC.GetValue());
    PC.Increment();
    OPCodes_LD(BC.GetHighRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x02()
{
    // LD (BC),A
    OPCodes_LD(BC.GetValue(), AF.GetHigh());
}

void Processor::OPCode0x03()
{
    // INC BC
    BC.Increment();
}

void Processor::OPCode0x04()
{
    // INC B
    OPCodes_INC(BC.GetHighRegister());
}

void Processor::OPCode0x05()
{
    // DEC B
    OPCodes_DEC(BC.GetHighRegister());
}

void Processor::OPCode0x06()
{
    // LD B,n
    OPCodes_LD(BC.GetHighRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x07()
{
    // RLCA
    OPCodes_RLC(AF.GetHighRegister());
}

void Processor::OPCode0x08()
{
    // LD (nn),SP
    u8 l = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    u8 h = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    u16 address = ((h << 8) + l);
    m_pMemory->Write(address, SP.GetLow());
    m_pMemory->Write(address + 1, SP.GetHigh());
}

void Processor::OPCode0x09()
{
    // ADD HL,BC
    OPCodes_ADD_HL(BC.GetValue());
}

void Processor::OPCode0x0A()
{
    // LD A,(BC)
    OPCodes_LD(AF.GetHighRegister(), BC.GetValue());
}

void Processor::OPCode0x0B()
{
    // DEC BC
    BC.Decrement();
}

void Processor::OPCode0x0C()
{
    // INC C
    OPCodes_INC(BC.GetLowRegister());
}

void Processor::OPCode0x0D()
{
    // DEC C
    OPCodes_DEC(BC.GetLowRegister());
}

void Processor::OPCode0x0E()
{
    // LD C,n
    OPCodes_LD(BC.GetLowRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x0F()
{
    // RRCA
    OPCodes_RRC(AF.GetHighRegister());
}

void Processor::OPCode0x10()
{
    // NOT IMPLEMENTED
    // STOP
    /*
    if (GameBoyMemory.Read(PC) != 0)
    {
        InvalidOPCode();
    }
     */
    PC.Increment();
}

void Processor::OPCode0x11()
{
    // LD DE,nn
    OPCodes_LD(DE.GetLowRegister(), PC.GetValue());
    PC.Increment();
    OPCodes_LD(DE.GetHighRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x12()
{
    // LD (DE),A
    OPCodes_LD(DE.GetValue(), AF.GetHigh());
}

void Processor::OPCode0x13()
{
    // INC DE
    DE.Increment();
}

void Processor::OPCode0x14()
{
    // INC D
    OPCodes_INC(DE.GetHighRegister());
}

void Processor::OPCode0x15()
{
    // DEC D
    OPCodes_DEC(DE.GetHighRegister());
}

void Processor::OPCode0x16()
{
    // LD D,n
    OPCodes_LD(DE.GetHighRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x17()
{
    // RLA
    OPCodes_RL(AF.GetHighRegister());
}

void Processor::OPCode0x18()
{
    // JR n
    PC.SetValue(PC.GetValue() + 1 + (static_cast<s8>(m_pMemory->Read(PC.GetValue()))));
}

void Processor::OPCode0x19()
{
    // ADD HL,DE
    OPCodes_ADD_HL(DE.GetValue());
}

void Processor::OPCode0x1A()
{
    // LD A,(DE)
    OPCodes_LD(AF.GetHighRegister(), DE.GetValue());
}

void Processor::OPCode0x1B()
{
    // DEC DE
    DE.Decrement();
}

void Processor::OPCode0x1C()
{
    // INC E
    OPCodes_INC(DE.GetLowRegister());
}

void Processor::OPCode0x1D()
{
    // DEC E
    OPCodes_DEC(DE.GetLowRegister());
}

void Processor::OPCode0x1E()
{
    // LD E,n
    OPCodes_LD(DE.GetLowRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x1F()
{
    // RRA
    OPCodes_RR(AF.GetHighRegister());
}

void Processor::OPCode0x20()
{
    // JR NZ,n
    if (!IsSetFlag(FLAG_ZERO))
    {
        PC.SetValue(PC.GetValue() + 1 + (static_cast<s8>(m_pMemory->Read(PC.GetValue()))));
    }
    else
    {
        PC.Increment();
    }
}

void Processor::OPCode0x21()
{
    // LD HL,nn
    OPCodes_LD(HL.GetLowRegister(), PC.GetValue());
    PC.Increment();
    OPCodes_LD(HL.GetHighRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x22()
{
    // LD (HLI),A
    OPCodes_LD(HL.GetValue(), AF.GetHigh());
    HL.Increment();
}

void Processor::OPCode0x23()
{
    // INC HL
    HL.Increment();
}

void Processor::OPCode0x24()
{
    // INC H
    OPCodes_INC(HL.GetHighRegister());
}

void Processor::OPCode0x25()
{
    // DEC H
    OPCodes_DEC(HL.GetHighRegister());
}

void Processor::OPCode0x26()
{
    // LD H,n
    OPCodes_LD(HL.GetHighRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x27()
{
    // DAA
    u8 result = AF.GetHigh();
    u8 flags = AF.GetLow();
    
    if ((flags & FLAG_SUB) != 0)
    {
        if ((result & 0x0F) > 0x09 || ((flags & 0x20) != 0))
        {
            result -= 0x06;

            if ((result & 0xF0) == 0xF0)
                flags |= 0x10;
            else
                flags &= 0xEF;
        }

        if ((result & 0xF0) > 0x90 || ((flags & 0x10) != 0))
            result -= 0x60;
    }
    else
    {
        if ((result & 0x0F) > 0x09 || ((flags & 0x20) != 0))
        {
            result += 0x06;

            if ((result & 0xF0) == 0xF0)
                flags |= 0x10;
            else
                flags &= 0xEF;
        }

        if ((result & 0xF0) > 0x90 || ((flags & 0x10) != 0))
            result+= 0x60;

    }

    if (result == 0)
        flags |= 0x80;
    else
        flags &= 0x7F;
}

void Processor::OPCode0x28()
{
    // JR Z,n
    if (IsSetFlag(FLAG_ZERO))
    {
        PC.SetValue(PC.GetValue() + 1 + (static_cast<s8>(m_pMemory->Read(PC.GetValue()))));
    }
    else
    {
        PC.Increment();
    }
}

void Processor::OPCode0x29()
{
    // ADD HL,HL
    OPCodes_ADD_HL(HL.GetValue());
}

void Processor::OPCode0x2A()
{
    // LD A,(HLI)
    OPCodes_LD(AF.GetHighRegister(), HL.GetValue());
    HL.Increment();
}

void Processor::OPCode0x2B()
{
    // DEC HL
    HL.Decrement();
}

void Processor::OPCode0x2C()
{
    // INC L
    OPCodes_INC(HL.GetLowRegister());
}

void Processor::OPCode0x2D()
{
    // DEC L
    OPCodes_DEC(HL.GetLowRegister());
}

void Processor::OPCode0x2E()
{
    // LD L,n
    OPCodes_LD(HL.GetLowRegister(), PC.GetValue());
    PC.Increment();

}

void Processor::OPCode0x2F()
{
    // CPL
    AF.SetHigh(~AF.GetHigh());
    ToggleFlag(FLAG_HALF);
    ToggleFlag(FLAG_SUB);
}

void Processor::OPCode0x30()
{
    // JR NC,n
    if (!IsSetFlag(FLAG_CARRY))
    {
        PC.SetValue(PC.GetValue() + 1 + (static_cast<s8>(m_pMemory->Read(PC.GetValue()))));
    }
    else
    {
        PC.Increment();
    }
}

void Processor::OPCode0x31()
{
    // LD SP,nn
    SP.SetLow(m_pMemory->Read(PC.GetValue()));
    PC.Increment();
    SP.SetHigh(m_pMemory->Read(PC.GetValue()));
    PC.Increment();
}

void Processor::OPCode0x32()
{
    // LD (HLD), A
    OPCodes_LD(HL.GetValue(), AF.GetHigh());
    HL.Decrement();
}

void Processor::OPCode0x33()
{
    // INC SP
    SP.Increment();
}

void Processor::OPCode0x34()
{
    // INC (HL)
    OPCodes_INC_HL();
}

void Processor::OPCode0x35()
{
    // DEC (HL)
    OPCodes_DEC_HL();
}

void Processor::OPCode0x36()
{
    // LD (HL),n    
    m_pMemory->Write(HL.GetValue(), m_pMemory->Read(PC.GetValue()));
    PC.Increment();
}

void Processor::OPCode0x37()
{
    // SCF
    ToggleFlag(FLAG_CARRY);
    UntoggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

void Processor::OPCode0x38()
{
    // JR C,n
    if (IsSetFlag(FLAG_CARRY))
    {
        PC.SetValue(PC.GetValue() + 1 + (static_cast<s8>(m_pMemory->Read(PC.GetValue()))));
    }
    else
    {
        PC.Increment();
    }
}

void Processor::OPCode0x39()
{
    // ADD HL,SP
    OPCodes_ADD_HL(SP.GetValue());
}

void Processor::OPCode0x3A()
{
    // LD A,(HLD)
    OPCodes_LD(AF.GetHighRegister(), HL.GetValue());
    HL.Decrement();
}

void Processor::OPCode0x3B()
{
    // DEC SP
    SP.Decrement();
}

void Processor::OPCode0x3C()
{
    // INC A
    OPCodes_INC(AF.GetHighRegister());
}

void Processor::OPCode0x3D()
{
    // DEC A
    OPCodes_DEC(AF.GetHighRegister());

}

void Processor::OPCode0x3E()
{
    // LD A,n
    OPCodes_LD(AF.GetHighRegister(), PC.GetValue());
    PC.Increment();
}

void Processor::OPCode0x3F()
{
    // CCF
    FlipFlag(FLAG_CARRY);
    UntoggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_SUB);
}

void Processor::OPCode0x40()
{
    // LD B,B
    OPCodes_LD(BC.GetHighRegister(), BC.GetHigh());
}

void Processor::OPCode0x41()
{
    // LD B,C
    OPCodes_LD(BC.GetHighRegister(), BC.GetLow());
}

void Processor::OPCode0x42()
{
    // LD B,D
    OPCodes_LD(BC.GetHighRegister(), DE.GetHigh());
}

void Processor::OPCode0x43()
{
    // LD B,E
    OPCodes_LD(BC.GetHighRegister(), DE.GetLow());
}

void Processor::OPCode0x44()
{
    // LD B,H
    OPCodes_LD(BC.GetHighRegister(), HL.GetHigh());
}

void Processor::OPCode0x45()
{
    // LD B,L
    OPCodes_LD(BC.GetHighRegister(), HL.GetLow());
}

void Processor::OPCode0x46()
{
    // LD B,(HL)
    OPCodes_LD(BC.GetHighRegister(), HL.GetValue());
}

void Processor::OPCode0x47()
{
    // LD B,A
    OPCodes_LD(BC.GetHighRegister(), AF.GetHigh());
}

void Processor::OPCode0x48()
{
    // LD C,B
    OPCodes_LD(BC.GetLowRegister(), BC.GetHigh());
}

void Processor::OPCode0x49()
{
    // LD C,C
    OPCodes_LD(BC.GetLowRegister(), BC.GetLow());
}

void Processor::OPCode0x4A()
{
    // LD C,D
    OPCodes_LD(BC.GetLowRegister(), DE.GetHigh());
}

void Processor::OPCode0x4B()
{
    // LD C,E
    OPCodes_LD(BC.GetLowRegister(), DE.GetLow());
}

void Processor::OPCode0x4C()
{
    // LD C,H
    OPCodes_LD(BC.GetLowRegister(), HL.GetHigh());
}

void Processor::OPCode0x4D()
{
    // LD C,L
    OPCodes_LD(BC.GetLowRegister(), HL.GetLow());
}

void Processor::OPCode0x4E()
{
    // LD C,(HL)
    OPCodes_LD(BC.GetLowRegister(), HL.GetValue());
}

void Processor::OPCode0x4F()
{
    // LD C,A
    OPCodes_LD(BC.GetLowRegister(), AF.GetHigh());
}

void Processor::OPCode0x50()
{
    // LD D,B
    OPCodes_LD(DE.GetHighRegister(), BC.GetHigh());
}

void Processor::OPCode0x51()
{
    // LD D,C
    OPCodes_LD(DE.GetHighRegister(), BC.GetLow());
}

void Processor::OPCode0x52()
{
    // LD D,D
    OPCodes_LD(DE.GetHighRegister(), DE.GetHigh());
}

void Processor::OPCode0x53()
{
    // LD D,E
    OPCodes_LD(DE.GetHighRegister(), DE.GetLow());
}

void Processor::OPCode0x54()
{
    // LD D,H
    OPCodes_LD(DE.GetHighRegister(), HL.GetHigh());
}

void Processor::OPCode0x55()
{
    // LD D,L
    OPCodes_LD(DE.GetHighRegister(), HL.GetLow());
}

void Processor::OPCode0x56()
{
    // LD D,(HL)
    OPCodes_LD(DE.GetHighRegister(), HL.GetValue());
}

void Processor::OPCode0x57()
{
    // LD D,A
    OPCodes_LD(DE.GetHighRegister(), AF.GetHigh());
}

void Processor::OPCode0x58()
{
    // LD E,B
    OPCodes_LD(DE.GetLowRegister(), BC.GetHigh());
}

void Processor::OPCode0x59()
{
    // LD E,C
    OPCodes_LD(DE.GetLowRegister(), BC.GetLow());
}

void Processor::OPCode0x5A()
{
    // LD E,D
    OPCodes_LD(DE.GetLowRegister(), DE.GetHigh());
}

void Processor::OPCode0x5B()
{
    // LD E,E
    OPCodes_LD(DE.GetLowRegister(), DE.GetLow());
}

void Processor::OPCode0x5C()
{
    // LD E,H
    OPCodes_LD(DE.GetLowRegister(), HL.GetHigh());
}

void Processor::OPCode0x5D()
{
    // LD E,L
    OPCodes_LD(DE.GetLowRegister(), HL.GetLow());
}

void Processor::OPCode0x5E()
{
    // LD E,(HL)
    OPCodes_LD(DE.GetLowRegister(), HL.GetValue());
}

void Processor::OPCode0x5F()
{
    // LD E,A
    OPCodes_LD(DE.GetLowRegister(), AF.GetHigh());
}

void Processor::OPCode0x60()
{
    // LD H,B
    OPCodes_LD(HL.GetHighRegister(), BC.GetHigh());
}

void Processor::OPCode0x61()
{
    // LD H,C
    OPCodes_LD(HL.GetHighRegister(), BC.GetLow());
}

void Processor::OPCode0x62()
{
    // LD H,D
    OPCodes_LD(HL.GetHighRegister(), DE.GetHigh());
}

void Processor::OPCode0x63()
{
    // LD H,E
    OPCodes_LD(HL.GetHighRegister(), DE.GetLow());
}

void Processor::OPCode0x64()
{
    // LD H,H
    OPCodes_LD(HL.GetHighRegister(), HL.GetHigh());
}

void Processor::OPCode0x65()
{
    // LD H,L
    OPCodes_LD(HL.GetHighRegister(), HL.GetLow());
}

void Processor::OPCode0x66()
{
    // LD H,(HL)
    OPCodes_LD(HL.GetHighRegister(), HL.GetValue());
}

void Processor::OPCode0x67()
{
    // LD H,A
    OPCodes_LD(HL.GetHighRegister(), AF.GetHigh());
}

void Processor::OPCode0x68()
{
    // LD L,B
    OPCodes_LD(HL.GetLowRegister(), BC.GetHigh());
}

void Processor::OPCode0x69()
{
    // LD L,C
    OPCodes_LD(HL.GetLowRegister(), BC.GetLow());
}

void Processor::OPCode0x6A()
{
    // LD L,D
    OPCodes_LD(HL.GetLowRegister(), DE.GetHigh());
}

void Processor::OPCode0x6B()
{
    // LD L,E
    OPCodes_LD(HL.GetLowRegister(), DE.GetLow());
}

void Processor::OPCode0x6C()
{
    // LD L,H
    OPCodes_LD(HL.GetLowRegister(), HL.GetHigh());
}

void Processor::OPCode0x6D()
{
    // LD L,L
    OPCodes_LD(HL.GetLowRegister(), HL.GetLow());
}

void Processor::OPCode0x6E()
{
    // LD L,(HL)
    OPCodes_LD(HL.GetLowRegister(), HL.GetValue());
}

void Processor::OPCode0x6F()
{
    // LD L,A
    OPCodes_LD(HL.GetLowRegister(), AF.GetHigh());
}

void Processor::OPCode0x70()
{
    // LD (HL),B
    OPCodes_LD((ushort) ((H.Value << 8) + L.Value), B);

}

void Processor::OPCode0x71()
{
    // LD (HL),C
    OPCodes_LD((ushort) ((H.Value << 8) + L.Value), C);

}

void Processor::OPCode0x72()
{
    // LD (HL),D
    OPCodes_LD((ushort) ((H.Value << 8) + L.Value), D);

}

void Processor::OPCode0x73()
{
    // LD (HL),E
    OPCodes_LD((ushort) ((H.Value << 8) + L.Value), E);

}

void Processor::OPCode0x74()
{
    // LD (HL),H
    OPCodes_LD((ushort) ((H.Value << 8) + L.Value), H);

}

void Processor::OPCode0x75()
{
    // LD (HL),L
    OPCodes_LD((ushort) ((H.Value << 8) + L.Value), L);

}

void Processor::OPCode0x76()
{
    // HALT
    HALT = true;

}

void Processor::OPCode0x77()
{
    // LD (HL),A
    OPCodes_LD((ushort) ((H.Value << 8) + L.Value), A);

}

void Processor::OPCode0x78()
{
    // LD A,B
    OPCodes_LD(A, B);

}

void Processor::OPCode0x79()
{
    // LD A,C
    OPCodes_LD(A, C);

}

void Processor::OPCode0x7A()
{
    // LD A,D
    OPCodes_LD(A, D);

}

void Processor::OPCode0x7B()
{
    // LD A,E
    OPCodes_LD(A, E);

}

void Processor::OPCode0x7C()
{
    // LD A,H
    OPCodes_LD(A, H);

}

void Processor::OPCode0x7D()
{
    // LD A,L
    OPCodes_LD(A, L);

}

void Processor::OPCode0x7E()
{
    // LD A,(HL)
    OPCodes_LD(A, (ushort) ((H.Value << 8) + L.Value));

}

void Processor::OPCode0x7F()
{
    // LD A,A
    OPCodes_LD(A, A);

}

void Processor::OPCode0x80()
{
    // ADD A,B
    OPCodes_ADD(B.Value);

}

void Processor::OPCode0x81()
{
    // ADD A,C
    OPCodes_ADD(C.Value);

}

void Processor::OPCode0x82()
{
    // ADD A,D
    OPCodes_ADD(D.Value);

}

void Processor::OPCode0x83()
{
    // ADD A,E
    OPCodes_ADD(E.Value);

}

void Processor::OPCode0x84()
{
    // ADD A,H
    OPCodes_ADD(H.Value);

}

void Processor::OPCode0x85()
{
    // ADD A,L
    OPCodes_ADD(L.Value);

}

void Processor::OPCode0x86()
{
    // ADD A,(HL)
    OPCodes_ADD(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0x87()
{
    // ADD A,A
    OPCodes_ADD(A.Value);

}

void Processor::OPCode0x88()
{
    // ADC A,B
    OPCodes_ADC(B.Value);

}

void Processor::OPCode0x89()
{
    // ADC A,C
    OPCodes_ADC(C.Value);

}

void Processor::OPCode0x8A()
{
    // ADC A,D
    OPCodes_ADC(D.Value);

}

void Processor::OPCode0x8B()
{
    // ADC A,E
    OPCodes_ADC(E.Value);

}

void Processor::OPCode0x8C()
{
    // ADC A,H
    OPCodes_ADC(H.Value);

}

void Processor::OPCode0x8D()
{
    // ADC A,L
    OPCodes_ADC(L.Value);

}

void Processor::OPCode0x8E()
{
    // ADC A,(HL)
    OPCodes_ADC(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0x8F()
{
    // ADC A,A
    OPCodes_ADC(A.Value);

}

void Processor::OPCode0x90()
{
    // SUB B
    OPCodes_SUB(B.Value);

}

void Processor::OPCode0x91()
{
    // SUB C
    OPCodes_SUB(C.Value);

}

void Processor::OPCode0x92()
{
    // SUB D
    OPCodes_SUB(D.Value);

}

void Processor::OPCode0x93()
{
    // SUB E
    OPCodes_SUB(E.Value);

}

void Processor::OPCode0x94()
{
    // SUB H
    OPCodes_SUB(H.Value);

}

void Processor::OPCode0x95()
{
    // SUB L
    OPCodes_SUB(L.Value);

}

void Processor::OPCode0x96()
{
    // SUB (HL)
    OPCodes_SUB(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0x97()
{
    // SUB A
    OPCodes_SUB(A.Value);

}

void Processor::OPCode0x98()
{
    // SBC B
    OPCodes_SBC(B.Value);

}

void Processor::OPCode0x99()
{
    // SBC C
    OPCodes_SBC(C.Value);

}

void Processor::OPCode0x9A()
{
    // SBC D
    OPCodes_SBC(D.Value);

}

void Processor::OPCode0x9B()
{
    // SBC E
    OPCodes_SBC(E.Value);

}

void Processor::OPCode0x9C()
{
    // SBC H
    OPCodes_SBC(H.Value);

}

void Processor::OPCode0x9D()
{
    // SBC L
    OPCodes_SBC(L.Value);

}

void Processor::OPCode0x9E()
{
    // SBC (HL)
    OPCodes_SBC(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0x9F()
{
    // SBC A
    OPCodes_SBC(A.Value);

}

void Processor::OPCode0xA0()
{
    // AND B
    OPCodes_AND(B.Value);

}

void Processor::OPCode0xA1()
{
    // AND C
    OPCodes_AND(C.Value);

}

void Processor::OPCode0xA2()
{
    // AND D
    OPCodes_AND(D.Value);

}

void Processor::OPCode0xA3()
{
    // AND E
    OPCodes_AND(E.Value);

}

void Processor::OPCode0xA4()
{
    // AND H
    OPCodes_AND(H.Value);

}

void Processor::OPCode0xA5()
{
    // AND L
    OPCodes_AND(L.Value);

}

void Processor::OPCode0xA6()
{
    // AND (HL)
    OPCodes_AND(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0xA7()
{
    // AND A
    OPCodes_AND(A.Value);

}

void Processor::OPCode0xA8()
{
    // XOR B
    OPCodes_XOR(B.Value);

}

void Processor::OPCode0xA9()
{
    // XOR C
    OPCodes_XOR(C.Value);

}

void Processor::OPCode0xAA()
{
    // XOR D
    OPCodes_XOR(D.Value);

}

void Processor::OPCode0xAB()
{
    // XOR E
    OPCodes_XOR(E.Value);

}

void Processor::OPCode0xAC()
{
    // XOR H
    OPCodes_XOR(H.Value);

}

void Processor::OPCode0xAD()
{
    // XOR L
    OPCodes_XOR(L.Value);

}

void Processor::OPCode0xAE()
{
    // XOR (HL)
    OPCodes_XOR(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0xAF()
{
    // XOR A
    OPCodes_XOR(A.Value);

}

void Processor::OPCode0xB0()
{
    // OR B
    OPCodes_OR(B.Value);

}

void Processor::OPCode0xB1()
{
    // OR C
    OPCodes_OR(C.Value);

}

void Processor::OPCode0xB2()
{
    // OR D
    OPCodes_OR(D.Value);

}

void Processor::OPCode0xB3()
{
    // OR E
    OPCodes_OR(E.Value);

}

void Processor::OPCode0xB4()
{
    // OR H
    OPCodes_OR(H.Value);

}

void Processor::OPCode0xB5()
{
    // OR L
    OPCodes_OR(L.Value);

}

void Processor::OPCode0xB6()
{
    // OR (HL)
    OPCodes_OR(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0xB7()
{
    // OR A
    OPCodes_OR(A.Value);

}

void Processor::OPCode0xB8()
{
    // CP B
    OPCodes_CP(B.Value);

}

void Processor::OPCode0xB9()
{
    // CP C
    OPCodes_CP(C.Value);

}

void Processor::OPCode0xBA()
{
    // CP D
    OPCodes_CP(D.Value);

}

void Processor::OPCode0xBB()
{
    // CP E
    OPCodes_CP(E.Value);

}

void Processor::OPCode0xBC()
{
    // CP H
    OPCodes_CP(H.Value);

}

void Processor::OPCode0xBD()
{
    // CP L
    OPCodes_CP(L.Value);

}

void Processor::OPCode0xBE()
{
    // CP (HL)
    OPCodes_CP(GameBoyMemory.Read((ushort) ((H.Value << 8) + L.Value)));

}

void Processor::OPCode0xBF()
{
    // CP A
    OPCodes_CP(A.Value);

}

void Processor::OPCode0xC0()
{
    // RET NZ
    if ((F.Value & FLAG_ZERO) == 0)
    {
        byte l = GameBoyMemory.Read(SP);
        SP++;
        byte h = GameBoyMemory.Read(SP);
        SP++;
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC++;

    }
}

void Processor::OPCode0xC1()
{
    // POP BC
    C.Value = GameBoyMemory.Read(SP);
    SP++;
    B.Value = GameBoyMemory.Read(SP);
    SP++;

}

void Processor::OPCode0xC2()
{
    // JP NZ,nn
    if ((F.Value & FLAG_ZERO) == 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xC3()
{
    // JP nn
    byte l = GameBoyMemory.Read(PC);
    PC++;
    byte h = GameBoyMemory.Read(PC);
    PC = (ushort) ((h << 8) + l);

}

void Processor::OPCode0xC4()
{
    // CALL NZ,nn
    if ((F.Value & FLAG_ZERO) == 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC++;

        SP--;
        GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
        SP--;
        GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

        PC = (ushort) ((h << 8) + l);


    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xC5()
{
    // PUSH BC
    SP--;
    GameBoyMemory.Write(SP, B.Value);
    SP--;
    GameBoyMemory.Write(SP, C.Value);

}

void Processor::OPCode0xC6()
{
    // ADD A,n
    OPCodes_ADD(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xC7()
{
    // RST 00H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x00;


}

void Processor::OPCode0xC8()
{
    // RET Z
    if ((F.Value & FLAG_ZERO) != 0)
    {
        byte l = GameBoyMemory.Read(SP);
        SP++;
        byte h = GameBoyMemory.Read(SP);
        SP++;
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC++;

    }
}

void Processor::OPCode0xC9()
{
    // RET
    byte l = GameBoyMemory.Read(SP);
    SP++;
    byte h = GameBoyMemory.Read(SP);
    SP++;
    PC = (ushort) ((h << 8) + l);

}

void Processor::OPCode0xCA()
{
    // JP Z,nn
    if ((F.Value & FLAG_ZERO) != 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xCC()
{
    // CALL Z,nn
    if ((F.Value & FLAG_ZERO) != 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC++;

        SP--;
        GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
        SP--;
        GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

        PC = (ushort) ((h << 8) + l);


    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xCD()
{
    // CALL nn
    byte l = GameBoyMemory.Read(PC);
    PC++;
    byte h = GameBoyMemory.Read(PC);
    PC++;

    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = (ushort) ((h << 8) + l);


}

void Processor::OPCode0xCE()
{
    // ADC A,n
    OPCodes_ADC(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xCF()
{
    // RST 08H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x08;


}

void Processor::OPCode0xD0()
{
    // RET NC
    if ((F.Value & FLAG_CARRY) == 0)
    {
        byte l = GameBoyMemory.Read(SP);
        SP++;
        byte h = GameBoyMemory.Read(SP);
        SP++;
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC++;

    }
}

void Processor::OPCode0xD1()
{
    // POP DE
    E.Value = GameBoyMemory.Read(SP);
    SP++;
    D.Value = GameBoyMemory.Read(SP);
    SP++;

}

void Processor::OPCode0xD2()
{
    // JP NC,nn
    if ((F.Value & FLAG_CARRY) == 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xD3()
{
    InvalidOPCode();
}

void Processor::OPCode0xD4()
{
    // CALL NC,nn
    if ((F.Value & FLAG_CARRY) == 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC++;

        SP--;
        GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
        SP--;
        GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

        PC = (ushort) ((h << 8) + l);


    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xD5()
{
    // PUSH DE
    SP--;
    GameBoyMemory.Write(SP, D.Value);
    SP--;
    GameBoyMemory.Write(SP, E.Value);

}

void Processor::OPCode0xD6()
{
    // SUB n
    OPCodes_SUB(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xD7()
{
    // RST 10H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x10;


}

void Processor::OPCode0xD8()
{
    // RET C
    if ((F.Value & FLAG_CARRY) != 0)
    {
        byte l = GameBoyMemory.Read(SP);
        SP++;
        byte h = GameBoyMemory.Read(SP);
        SP++;
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC++;

    }
}

void Processor::OPCode0xD9()
{
    // RETI
    byte l = GameBoyMemory.Read(SP);
    SP++;
    byte h = GameBoyMemory.Read(SP);
    SP++;
    PC = (ushort) ((h << 8) + l);


    IME = true;
}

void Processor::OPCode0xDA()
{
    // JP C,nn
    if ((F.Value & FLAG_CARRY) != 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC = (ushort) ((h << 8) + l);

    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xDB()
{
    InvalidOPCode();
}

void Processor::OPCode0xDC()
{
    // CALL C,nn
    if ((F.Value & FLAG_CARRY) != 0)
    {
        byte l = GameBoyMemory.Read(PC);
        PC++;
        byte h = GameBoyMemory.Read(PC);
        PC++;

        SP--;
        GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
        SP--;
        GameBoyMemory.Write(SP, (byte) (PC & 0xFF));


        PC = (ushort) ((h << 8) + l);


    }
    else
    {
        PC += 2;

    }
}

void Processor::OPCode0xDD()
{
    InvalidOPCode();
}

void Processor::OPCode0xDE()
{
    // SBC n
    OPCodes_SBC(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xDF()
{
    // RST 18H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x18;


}

void Processor::OPCode0xE0()
{
    // LD (0xFF00+n),A
    OPCodes_LD((ushort) (0xFF00 + GameBoyMemory.Read(PC)), A);
    PC++;

}

void Processor::OPCode0xE1()
{
    // POP HL
    L.Value = GameBoyMemory.Read(SP);
    SP++;
    H.Value = GameBoyMemory.Read(SP);
    SP++;

}

void Processor::OPCode0xE2()
{
    // LD (C),A
    OPCodes_LD((ushort) (0xFF00 + C.Value), A);

}

void Processor::OPCode0xE3()
{
    InvalidOPCode();
}

void Processor::OPCode0xE4()
{
    InvalidOPCode();
}

void Processor::OPCode0xE5()
{
    // PUSH HL
    SP--;
    GameBoyMemory.Write(SP, H.Value);
    SP--;
    GameBoyMemory.Write(SP, L.Value);

}

void Processor::OPCode0xE6()
{
    // AND n
    OPCodes_AND(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xE7()
{
    // RST 20H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x20;


}

void Processor::OPCode0xE8()
{
    // ADD SP,n
    OPCodes_ADD_SP((sbyte) GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xE9()
{
    // JP (HL)
    PC = (ushort) ((H.Value << 8) + L.Value);

}

void Processor::OPCode0xEA()
{
    // LD (nn),A
    OPCodes_LD((ushort) ((GameBoyMemory.Read((ushort) (PC + 1)) << 8) + GameBoyMemory.Read(PC)), A);
    PC += 2;

}

void Processor::OPCode0xEB()
{
    InvalidOPCode();
}

void Processor::OPCode0xEC()
{
    InvalidOPCode();
}

void Processor::OPCode0xED()
{
    InvalidOPCode();
}

void Processor::OPCode0xEE()
{
    // XOR n
    OPCodes_XOR(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xEF()
{
    // RST 28H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x28;


}

void Processor::OPCode0xF0()
{
    // LD A,(0xFF00+n)
    OPCodes_LD(A, (ushort) (0xFF00 + GameBoyMemory.Read(PC)));
    PC++;

}

void Processor::OPCode0xF1()
{
    // POP AF
    F.Value = GameBoyMemory.Read(SP);
    SP++;
    A.Value = GameBoyMemory.Read(SP);
    SP++;

}

void Processor::OPCode0xF2()
{
    // LD A,(C)            
    A.Value = GameBoyMemory.Read((ushort) (0xFF00 + C.Value));

}

void Processor::OPCode0xF3()
{
    // DI
    IME = false;

}

void Processor::OPCode0xF4()
{
    InvalidOPCode();
}

void Processor::OPCode0xF5()
{
    // PUSH AF
    SP--;
    GameBoyMemory.Write(SP, A.Value);
    SP--;
    GameBoyMemory.Write(SP, F.Value);

}

void Processor::OPCode0xF6()
{
    // OR n
    OPCodes_OR(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xF7()
{
    // RST 30H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x30;


}

void Processor::OPCode0xF8()
{
    // LD HL,SP+n
    sbyte temp = (sbyte) GameBoyMemory.Read(PC);
    ushort res = (ushort) (SP + temp);

    F.Value = 0;

    if (res > 0xFF)
    {
        F.Value |= FLAG_CARRY;
    }

    if ((((SP & 0x0F) + (temp & 0x0F)) & 0xF0) != 0)
    {
        F.Value |= FLAG_HALF;
    }

    H.Value = (byte) ((res >> 8) & 0xFF);
    L.Value = (byte) (res & 0xFF);


    PC++;

    // REVISAR
    /*
                unsigned char temp=mem_read8(regPC);

        if((temp&0x80)==0)
        {
            regF=(((regSP&0x0F)+(temp&0x0F)) & 0x10) << 1;
            regF|=((regSP+temp) & 0x100) >> 4;

            regHL=regSP+temp;
        }
        else
        {
            regF=(((regSP&0x0F)+(((signed char)temp)&0x0F)) & 0x10) << 1;
            regF|=(((regSP&0xFF)+((signed char)temp)) & 0x100) >> 4;

            regHL=regSP+(signed char)temp;

        }

        regPC++;*/
}

void Processor::OPCode0xF9()
{
    // LD SP,HL
    SP = (ushort) ((H.Value << 8) + L.Value);

}

void Processor::OPCode0xFA()
{
    // LD A,(nn)
    OPCodes_LD(A, (ushort) ((GameBoyMemory.Read((ushort) (PC + 1)) << 8) + GameBoyMemory.Read(PC)));
    PC += 2;

}

void Processor::OPCode0xFB()
{
    // EI
    IME = true;

}

void Processor::OPCode0xFC()
{
    InvalidOPCode();
}

void Processor::OPCode0xFD()
{
    InvalidOPCode();
}

void Processor::OPCode0xFE()
{
    // CP n
    OPCodes_CP(GameBoyMemory.Read(PC));
    PC++;

}

void Processor::OPCode0xFF()
{
    // RST 38H
    SP--;
    GameBoyMemory.Write(SP, (byte) ((PC >> 8) & 0xFF));
    SP--;
    GameBoyMemory.Write(SP, (byte) (PC & 0xFF));

    PC = 0x38;


}