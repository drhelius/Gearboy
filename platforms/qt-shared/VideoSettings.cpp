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

#include <QColorDialog>
#include "VideoSettings.h"
#include "GLFrame.h"
#include "Emulator.h"

VideoSettings::VideoSettings(GLFrame* pGLFrame, Emulator* pEmulator)
{
    widget.setupUi(this);
    m_pEmulator = pEmulator;
    m_pGLFrame = pGLFrame;
}

VideoSettings::~VideoSettings()
{
}

void VideoSettings::Init(int color1, int color2, int color3, int color4)
{
    QColor color;
    color.setBlue(color1 & 0xFF);
    color.setGreen((color1 >> 8) & 0xFF);
    color.setRed((color1 >> 8) & 0xFF);
    QString qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor1->setStyleSheet(qss);

    color.setBlue(color2 & 0xFF);
    color.setGreen((color2 >> 8) & 0xFF);
    color.setRed((color2 >> 8) & 0xFF);
    qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor2->setStyleSheet(qss);

    color.setBlue(color3 & 0xFF);
    color.setGreen((color3 >> 8) & 0xFF);
    color.setRed((color3 >> 8) & 0xFF);
    qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor3->setStyleSheet(qss);

    color.setBlue(color4 & 0xFF);
    color.setGreen((color4 >> 8) & 0xFF);
    color.setRed((color4 >> 8) & 0xFF);
    qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor4->setStyleSheet(qss);
}

void VideoSettings::Color1()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid()) 
    {
        QString qss = QString("background-color: %1").arg(color.name());
        widget.pushButtonColor1->setStyleSheet(qss);
    }
}

void VideoSettings::Color2()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid()) 
    {
        QString qss = QString("background-color: %1").arg(color.name());
        widget.pushButtonColor2->setStyleSheet(qss);
    }
}

void VideoSettings::Color3()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid()) 
    {
        QString qss = QString("background-color: %1").arg(color.name());
        widget.pushButtonColor3->setStyleSheet(qss);
    }
}

void VideoSettings::Color4()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid()) 
    {
        QString qss = QString("background-color: %1").arg(color.name());
        widget.pushButtonColor4->setStyleSheet(qss);
    }
}

void VideoSettings::PressedOK()
{
    m_pGLFrame->SetBilinearFiletering(widget.checkBoxFilter->isChecked());
    m_pGLFrame->ResumeRenderThread();
    this->accept();
}

void VideoSettings::PressedCancel()
{
    m_pGLFrame->ResumeRenderThread();
    this->reject();
}
