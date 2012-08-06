#include "gearboy.h"

void testfunc()
{
    Core* gb = new Core();
            
    //gb->m_pProcessor->PC.SetValue(10);
    
    SafeDelete(gb);
}

int main(int argc, char** argv)
{
    testfunc();
    return 0;
}


