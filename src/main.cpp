#include "gearboy.h"
#include "Memory.h"
#include "Processor.h"


void testfunc()
{
    Memory* mem = new Memory();
    Processor* cpu = new Processor(mem);
    
    cpu->PC.SetValue(10);
    
    SafeDelete(cpu);
    SafeDelete(mem);
}

int main(int argc, char** argv)
{
    testfunc();
    return 0;
}


