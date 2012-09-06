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
#include "MainWindow.h"

VideoSettings::VideoSettings(GLFrame* pGLFrame, Emulator* pEmulator)
{
    widget.setupUi(this);
    m_pEmulator = pEmulator;
    m_pGLFrame = pGLFrame;
    m_iCurrentColor = 0;
    InitPointer(m_pColorDialog);
}

VideoSettings::~VideoSettings()
{
    SafeDelete(m_pColorDialog);
}

void VideoSettings::ColorDialogFinished(QColor color)
{
    m_iColors[m_iCurrentColor] = (color.blue() & 0xFF) | ((color.green() & 0xFF) << 8) | ((color.red() & 0xFF) << 16);
    QString qss = QString("background-color: %1").arg(color.name());

    switch (m_iCurrentColor)
    {
        case 0:
            widget.pushButtonColor1->setStyleSheet(qss);
            break;
        case 1:
            widget.pushButtonColor2->setStyleSheet(qss);
            break;
        case 2:
            widget.pushButtonColor3->setStyleSheet(qss);
            break;
        case 3:
            widget.pushButtonColor4->setStyleSheet(qss);
            break;
    }
}

void VideoSettings::Color1()
{
    QColor old_color;
    old_color.setBlue(m_iColors[0] & 0xFF);
    old_color.setGreen((m_iColors[0] >> 8) & 0xFF);
    old_color.setRed((m_iColors[0] >> 16) & 0xFF);
    m_iCurrentColor = 0;
    SafeDelete(m_pColorDialog);
    m_pColorDialog = new QColorDialog();
    m_pColorDialog->setCurrentColor(old_color);
    m_pColorDialog->setModal(true);
    connect(m_pColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(ColorDialogFinished(QColor)));
    m_pColorDialog->show();
}

void VideoSettings::Color2()
{
    QColor old_color;
    old_color.setBlue(m_iColors[1] & 0xFF);
    old_color.setGreen((m_iColors[1] >> 8) & 0xFF);
    old_color.setRed((m_iColors[1] >> 16) & 0xFF);
    m_iCurrentColor = 1;
    SafeDelete(m_pColorDialog);
    m_pColorDialog = new QColorDialog();
    m_pColorDialog->setCurrentColor(old_color);
    m_pColorDialog->setModal(true);
    connect(m_pColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(ColorDialogFinished(QColor)));
    m_pColorDialog->show();
}

void VideoSettings::Color3()
{
    QColor old_color;
    old_color.setBlue(m_iColors[2] & 0xFF);
    old_color.setGreen((m_iColors[2] >> 8) & 0xFF);
    old_color.setRed((m_iColors[2] >> 16) & 0xFF);
    m_iCurrentColor = 2;
    SafeDelete(m_pColorDialog);
    m_pColorDialog = new QColorDialog();
    m_pColorDialog->setCurrentColor(old_color);
    m_pColorDialog->setModal(true);
    connect(m_pColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(ColorDialogFinished(QColor)));
    m_pColorDialog->show();
}

void VideoSettings::Color4()
{
    QColor old_color;
    old_color.setBlue(m_iColors[3] & 0xFF);
    old_color.setGreen((m_iColors[3] >> 8) & 0xFF);
    old_color.setRed((m_iColors[3] >> 16) & 0xFF);
    m_iCurrentColor = 3;
    SafeDelete(m_pColorDialog);
    m_pColorDialog = new QColorDialog();
    m_pColorDialog->setCurrentColor(old_color);
    m_pColorDialog->setModal(true);
    connect(m_pColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(ColorDialogFinished(QColor)));
    m_pColorDialog->show();
}

void VideoSettings::PressedOK()
{
    m_pGLFrame->SetBilinearFiletering(widget.checkBoxFilter->isChecked());

    GB_Color gb_color[4];

    for (int i = 0; i < 4; i++)
    {
        gb_color[i].alpha = 0;
        gb_color[i].red = (m_iColors[i] >> 16) & 0xFF;
        gb_color[i].green = (m_iColors[i] >> 8) & 0xFF;
        gb_color[i].blue = m_iColors[i] & 0xFF;
    }

    m_pEmulator->SetDMGPalette(gb_color[0], gb_color[1], gb_color[2], gb_color[3]);

    m_pGLFrame->ResumeRenderThread();
    this->accept();
}

void VideoSettings::PressedCancel()
{
    m_pGLFrame->ResumeRenderThread();
    this->reject();
}

void VideoSettings::SaveSettings(QSettings& settings)
{
    settings.setValue("DMGColor1", m_iColors[0]);
    settings.setValue("DMGColor2", m_iColors[1]);
    settings.setValue("DMGColor3", m_iColors[2]);
    settings.setValue("DMGColor4", m_iColors[3]);
    settings.setValue("BilinearFiltering", widget.checkBoxFilter->isChecked());
}

void VideoSettings::LoadSettings(QSettings& settings)
{
    m_iColors[0] = settings.value("DMGColor1", 0xEFF3D5).toInt();
    m_iColors[1] = settings.value("DMGColor2", 0xA3B67A).toInt();
    m_iColors[2] = settings.value("DMGColor3", 0x37613B).toInt();
    m_iColors[3] = settings.value("DMGColor4", 0x041C16).toInt();
    widget.checkBoxFilter->setChecked(settings.value("BilinearFiltering", false).toBool());

    QColor color;
    color.setBlue(m_iColors[0] & 0xFF);
    color.setGreen((m_iColors[0] >> 8) & 0xFF);
    color.setRed((m_iColors[0] >> 16) & 0xFF);
    QString qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor1->setStyleSheet(qss);

    color.setBlue(m_iColors[1] & 0xFF);
    color.setGreen((m_iColors[1] >> 8) & 0xFF);
    color.setRed((m_iColors[1] >> 16) & 0xFF);
    qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor2->setStyleSheet(qss);

    color.setBlue(m_iColors[2] & 0xFF);
    color.setGreen((m_iColors[2] >> 8) & 0xFF);
    color.setRed((m_iColors[2] >> 16) & 0xFF);
    qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor3->setStyleSheet(qss);

    color.setBlue(m_iColors[3] & 0xFF);
    color.setGreen((m_iColors[3] >> 8) & 0xFF);
    color.setRed((m_iColors[3] >> 16) & 0xFF);
    qss = QString("background-color: %1").arg(color.name());
    widget.pushButtonColor4->setStyleSheet(qss);

    m_pGLFrame->SetBilinearFiletering(widget.checkBoxFilter->isChecked());

    GB_Color gb_color[4];

    for (int i = 0; i < 4; i++)
    {
        gb_color[i].alpha = 0;
        gb_color[i].red = (m_iColors[i] >> 16) & 0xFF;
        gb_color[i].green = (m_iColors[i] >> 8) & 0xFF;
        gb_color[i].blue = m_iColors[i] & 0xFF;
    }

    m_pEmulator->SetDMGPalette(gb_color[0], gb_color[1], gb_color[2], gb_color[3]);
}
