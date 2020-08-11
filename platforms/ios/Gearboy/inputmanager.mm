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

#include "inputmanager.h"

InputManager::InputManager()
{
    m_fInputRate = 0.15f;
    m_Timer.Start();
}

InputManager::~InputManager()
{
}

void InputManager::Update(void)
{
    while (!m_RegionEventResponseQueue.empty())
    {
        stRegionEventResponse event = m_RegionEventResponseQueue.front();
        m_RegionEventResponseQueue.pop();

        event.pCallback->Execute(event.parameter, event.id);
    }
}

void InputManager::AddCircleRegionEvent(float x, float y, float radius, InputCallbackGeneric* pCallback, int id, bool receiveMoveEvent)
{
    stRegionEvent tmp;

    tmp.receiveMoveEvent = receiveMoveEvent;
    tmp.region = new CircleRegion(x, y, radius);
    tmp.regionType = REGION_CIRCLE;
    tmp.pCallback = pCallback;
    tmp.pressed = false;
    tmp.pActualTouch = NULL;
    tmp.id = id;

    m_RegionEventVector.push_back(tmp);
}

void InputManager::AddRectRegionEvent(float x, float y, float width, float height, InputCallbackGeneric* pCallback, int id, bool receiveMoveEvent)
{
    stRegionEvent tmp;

    tmp.receiveMoveEvent = receiveMoveEvent;
    tmp.region = new RectRegion(x, y, width, height);
    tmp.regionType = REGION_RECT;
    tmp.pCallback = pCallback;
    tmp.pressed = false;
    tmp.pActualTouch = NULL;
    tmp.id = id;

    m_RegionEventVector.push_back(tmp);
}

void InputManager::ClearRegionEvents(void)
{
    int size = static_cast<int>(m_RegionEventVector.size());

    for (int i = 0; i < size; i++)
    {
        SafeDelete(m_RegionEventVector[i].region);
    }

    m_RegionEventVector.clear();

    while (!m_RegionEventResponseQueue.empty())
    {
        m_RegionEventResponseQueue.pop();
    }
}

void InputManager::HandleTouch(UITouch* touch, UIView* view)
{
    CGPoint location = [touch locationInView : view];
    CGPoint previousLocation;

    if (touch.phase == UITouchPhaseMoved)
        previousLocation = [touch previousLocationInView : view];
    else
        previousLocation = location;

    TRegionEventVector::size_type size = m_RegionEventVector.size();

    for (u32 i = 0; i < size; i++)
    {
        stRegionEvent regionEvent = m_RegionEventVector[i];

        stRegionEventResponse event;
        event.pCallback = regionEvent.pCallback;
        event.id = regionEvent.id;

        bool sendEvent = false;

        if (touch.phase == UITouchPhaseMoved)
        {
            if (regionEvent.region->PointInRegion(previousLocation.x, previousLocation.y))
            {
                if (!regionEvent.region->PointInRegion(location.x, location.y))
                {
                    if (regionEvent.pressed)
                    {
                        event.parameter.type = PRESS_END;
                        regionEvent.pressed = false;
                        sendEvent = true;
                    }
                }
                else
                {
                    if (regionEvent.pressed && regionEvent.receiveMoveEvent)
                    {
                        if (m_Timer.GetActualTime() > m_fInputRate)
                        {
                            m_Timer.Start();

                            event.parameter.type = PRESS_MOVE;
                            if (regionEvent.regionType == REGION_RECT)
                            {
                                event.parameter.vector = Vec3(location.x, location.y, 0.0f);
                            }
                            else
                            {
                                Vec3 point = Vec3(location.x, location.y, 0.0f);
                                event.parameter.vector = point - regionEvent.region->GetPosition();
                            }
                            m_RegionEventVector[i].pActualTouch = touch;
                            sendEvent = true;
                        }
                    }
                    else if (!regionEvent.pressed)
                    {
                        
                        event.parameter.type = PRESS_START;
                        if (regionEvent.regionType == REGION_RECT)
                        {
                            event.parameter.vector = Vec3(location.x, location.y, 0.0f);
                        }
                        else
                        {
                            Vec3 point = Vec3(location.x, location.y, 0.0f);
                            event.parameter.vector = point - regionEvent.region->GetPosition();
                        }
                        regionEvent.pressed = true;
                        m_RegionEventVector[i].pActualTouch = touch;
                        sendEvent = true;
                    }
                }
            }
        }
        else if (touch.phase == UITouchPhaseBegan)
        {
            if (regionEvent.region->PointInRegion(previousLocation.x, previousLocation.y))
            {
                if (!regionEvent.pressed)
                {
                    event.parameter.type = PRESS_START;
                    if (regionEvent.regionType == REGION_RECT)
                    {
                        event.parameter.vector = Vec3(previousLocation.x, previousLocation.y, 0.0f);
                    }
                    else
                    {
                        Vec3 point = Vec3(previousLocation.x, previousLocation.y, 0.0f);
                        event.parameter.vector = point - regionEvent.region->GetPosition();
                    }
                    regionEvent.pressed = true;
                    m_RegionEventVector[i].pActualTouch = touch;
                    sendEvent = true;

                    UIImpactFeedbackGenerator *generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
                    [generator impactOccurred];
                }
            }
        }
        else if ((touch.phase == UITouchPhaseEnded) || (touch.phase == UITouchPhaseCancelled))
        {
            if (regionEvent.pActualTouch == touch)
            {
                if (regionEvent.pressed)
                {
                    event.parameter.type = PRESS_END;
                    regionEvent.pressed = false;
                    sendEvent = true;
                }
            }
        }

        if (sendEvent)
        {
            m_RegionEventVector[i].pressed = regionEvent.pressed;

            m_RegionEventResponseQueue.push(event);
        }
    }
}


