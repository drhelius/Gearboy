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

#include "timer.h"

Timer::Timer(void)
{
    m_bCalculateFPS = false;

    Reset();
}

void Timer::Reset(void)
{
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    
    m_fResolution = (float) timebase.numer / (float) timebase.denom;
    
    m_fFPS = 0;
    m_bIsRunning = false;
    m_i64StopedTicks = 0;
    m_i64BaseTicks = 0;

    m_fLastUpdate = 0;
    m_iFrameCount = 0;

    m_fFrameTime = 0;
    m_fDeltaTime = 0.016f; ///--- 60 FPS

    m_fOffset = 0;

    for (int i=0; i<FRAME_RATE_AVERAGE; i++)
    {
        m_fDeltaTimeAccumulation[i] = 0.016f;
    }

    m_iCurrentAccumulator = 0;
}

Timer::~Timer(void)
{

}

void Timer::Start(void)
{
    m_i64BaseTicks = mach_absolute_time();
    m_bIsRunning = true;
    m_fLastUpdate = 0;
    m_iFrameCount = 0;
    m_fFrameTime = 0;
    m_fDeltaTime = 0;
    m_fOffset = 0;
}

void Timer::Stop(void)
{
    if (m_bIsRunning)
    {
        m_i64StopedTicks = mach_absolute_time();
        m_bIsRunning = false;
    }
}

void Timer::Continue(void)
{
    if (!m_bIsRunning)
    {
        u64 Ticks;
        Ticks = mach_absolute_time();
        m_i64BaseTicks += Ticks - m_i64StopedTicks;
        m_bIsRunning = true;
    }
}

float Timer::GetActualTime(void) const
{
    u64 Ticks;

    if (m_bIsRunning)
    {
        Ticks = mach_absolute_time();
    }
    else
        Ticks = m_i64StopedTicks;

    Ticks -= m_i64BaseTicks;

    return((((float) Ticks) * m_fResolution) / 1000000000.0f) +m_fOffset;
}

void Timer::Update(void)
{
    m_fDeltaTime = GetActualTime() - m_fFrameTime;    

    if (m_fDeltaTime > 0.1f)
    {
        m_fDeltaTime = 0.1f;
    }

    m_fFrameTime += m_fDeltaTime;

    m_fDeltaTimeAccumulation[m_iCurrentAccumulator] = m_fDeltaTime;
    m_iCurrentAccumulator++;
    m_iCurrentAccumulator %= FRAME_RATE_AVERAGE;

    float finalDeltaFrame = 0.0f;

    for (int i=0; i<FRAME_RATE_AVERAGE; i++)
    {
        finalDeltaFrame += m_fDeltaTimeAccumulation[i];
    }

    m_fDeltaTime = finalDeltaFrame / FRAME_RATE_AVERAGE;

    if (m_bCalculateFPS)
    {
        m_iFrameCount++;

        if (m_fFrameTime - m_fLastUpdate > FPS_REFRESH_TIME)
        {
            m_fFPS = m_iFrameCount / (m_fFrameTime - m_fLastUpdate);
            m_iFrameCount = 0;
            m_fLastUpdate = m_fFrameTime;
        }
    }
}

float Timer::GetFPS(void) const
{
    return m_fFPS;
}

float Timer::GetFrameTime(void) const
{
    return m_fFrameTime;
}

float Timer::GetDeltaTime(void) const
{
    return m_fDeltaTime;
}

bool Timer::IsRunning(void) const
{
    return m_bIsRunning;
}