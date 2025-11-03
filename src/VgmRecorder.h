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

#ifndef VGM_RECORDER_H
#define VGM_RECORDER_H

#include "definitions.h"
#include <vector>
#include <string>
#include <fstream>

class VgmRecorder
{
public:
    VgmRecorder();
    ~VgmRecorder();

    void Start(const char* file_path, int clock_rate, bool is_double_speed);
    void Stop();
    bool IsRecording() const { return m_bRecording; }

    void WriteGbDmg(u16 address, u8 data);
    void UpdateTiming(int elapsed_samples);
    
private:
    void WriteCommand(u8 command);
    void WriteCommand(u8 command, u8 data);
    void WriteCommand(u8 command, u8 data1, u8 data2);
    void WriteWait(int samples);
    void FlushPendingWait();

private:
    bool m_bRecording;
    std::string m_FilePath;
    std::vector<u8> m_CommandBuffer;
    int m_PendingWait;
    int m_TotalSamples;
    int m_ClockRate;
    bool m_bDoubleSpeed;
    bool m_bGbDmgUsed;
};

#endif /* VGM_RECORDER_H */
