#include "Processor.h"

Processor::Processor()
{
}

Processor::~Processor()
{
}

void Processor::ClearAllFlags()
{
    SetFlag(FLAG_NONE);
}

void Processor::ToggleZeroFlagFromResult(u8 result)
{
    if (result == 0)
    {
        ToggleFlag(FLAG_ZERO);
    }
}

void Processor::SetFlag(u8 flag)
{
    AF.SetLow(flag);
}

void Processor::ToggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() | flag);
}

void Processor::UntoggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() & (~flag));
}

bool Processor::IsSetFlag(u8 flag)
{
    return (AF.GetLow() & flag);
}

