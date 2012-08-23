#ifndef EMULATOR_H
#define	EMULATOR_H

#include <QMutex>
#include "../../../src/gearboy.h"

class Emulator
{
public:
    Emulator();
    ~Emulator();
    void Init();
    void RunToVBlank(GB_Color* pFrameBuffer);
    void LoadRom(const char* szFilePath);
    void Pause();
    void Resume();
private:
    GearboyCore* m_pGearboyCore;
    QMutex m_Mutex;

};

#endif	/* EMULATOR_H */

