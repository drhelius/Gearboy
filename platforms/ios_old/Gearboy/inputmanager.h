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

#pragma once
#ifndef _INPUTMANAGER_H
#define	_INPUTMANAGER_H

#import <UIKit/UIKit.h>
#include <queue>
#include <vector>
#include "singleton.h"
#include "inputcallback.h"
#include "regions.h"
#include "timer.h"

class InputManager : public Singleton<InputManager>
{
    friend class Singleton<InputManager>;

private:

    enum eRegionType
    {
        REGION_RECT,
        REGION_CIRCLE
    };

    struct stRegionEvent
    {
        int id;
        bool receiveMoveEvent;
        eRegionType regionType;
        bool pressed;
        Region* region;
        InputCallbackGeneric* pCallback;
        UITouch* pActualTouch;
    };

    struct stRegionEventResponse
    {
        stInputCallbackParameter parameter;
        InputCallbackGeneric* pCallback;
        int id;
    };

    typedef std::queue<stRegionEventResponse> TRegionEventResponseQueue;

    TRegionEventResponseQueue m_RegionEventResponseQueue;

    typedef std::vector<stRegionEvent> TRegionEventVector;

    TRegionEventVector m_RegionEventVector;

    Timer m_Timer;

    float m_fInputRate;
    
private:
    
    InputManager();

public:

    ~InputManager();

    void Update(void);

    void HandleTouch(UITouch* touch, UIView* view);

    void AddRectRegionEvent(float x, float y, float width, float height, InputCallbackGeneric* pCallback, int id = 0, bool receiveMoveEvent = false);
    void AddCircleRegionEvent(float x, float y, float radius, InputCallbackGeneric* pCallback, int id = 0, bool receiveMoveEvent = false);

    void ClearRegionEvents(void);
};

#endif	/* _INPUTMANAGER_H */

