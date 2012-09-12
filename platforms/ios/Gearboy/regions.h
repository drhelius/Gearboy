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

#ifndef _REGIONS_H
#define	_REGIONS_H
#pragma once

#include "Vector.h"

class Region
{

public:

    virtual ~Region() { };
    virtual bool PointInRegion(float px, float py) = 0;
    virtual Vec3 GetPosition(void) = 0;
};

class RectRegion : public Region
{

private:

    float m_fRectPosX;
    float m_fRectPosY;
    float m_fRectWidth;
    float m_fRectHeight;

public:

    RectRegion(float rx, float ry, float width, float height)
    {
        m_fRectPosX = rx;
        m_fRectPosY = ry;
        m_fRectWidth = width;
        m_fRectHeight = height;
    };

    virtual ~RectRegion() { };

    bool PointInRegion(float px, float py)
    {
        return(px >= m_fRectPosX) && (px <= (m_fRectPosX + m_fRectWidth)) && (py >= m_fRectPosY) && (py <= (m_fRectPosY + m_fRectHeight));
    };

    Vec3 GetPosition(void)
    {
        return Vec3(m_fRectPosX, m_fRectPosY, 0.0f);
    }
};

class CircleRegion : public Region
{

private:

    Vec2 m_CirclePos;
    float m_fCircleRadius;

public:

    CircleRegion(float rx, float ry, float radius)
    {
        m_CirclePos.x = rx;
        m_CirclePos.y = ry;
        m_fCircleRadius = radius;
    };

    virtual ~CircleRegion() { };

    bool PointInRegion(float px, float py)
    {
        Vec2 point = Vec2(px, py);
        Vec2 res = point - m_CirclePos;

        return(res.length() <= m_fCircleRadius);
    };

    Vec3 GetPosition(void)
    {
        return Vec3(m_CirclePos.x, m_CirclePos.y, 0.0f);
    }
};



#endif	/* _REGIONS_H */

