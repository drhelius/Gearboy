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

#include "SoundSettings.h"
#include "GLFrame.h"
#include "Emulator.h"

SoundSettings::SoundSettings(GLFrame* pGLFrame, Emulator* pEmulator)
{
    m_iRate = 1;
    m_bEnabled = true;
    m_pGLFrame = pGLFrame;
    m_pEmulator = pEmulator;
    widget.setupUi(this);

    widget.comboBoxSampleRate->addItem("48000");
    widget.comboBoxSampleRate->addItem("44100");
    widget.comboBoxSampleRate->addItem("22050");

    widget.comboBoxSampleRate->setCurrentIndex(m_iRate);
    widget.checkBoxSoundEnabled->setChecked(m_bEnabled);
}

SoundSettings::~SoundSettings()
{
}

void SoundSettings::Init(bool enabled, int rate)
{
    m_iRate = rate;
    m_bEnabled = enabled;

    widget.comboBoxSampleRate->setCurrentIndex(m_iRate);
    widget.checkBoxSoundEnabled->setChecked(m_bEnabled);
}

void SoundSettings::PressedOK()
{
    m_iRate = widget.comboBoxSampleRate->currentIndex();
    m_bEnabled = widget.checkBoxSoundEnabled->isChecked();

    int sampleRate = 0;
    switch (m_iRate)
    {
        case 0:
            sampleRate = 48000;
            break;
        case 1:
            sampleRate = 44100;
            break;
        case 2:
            sampleRate = 22050;
            break;
        default:
            sampleRate = 44100;
    }

    m_pEmulator->SetSoundSettings(m_bEnabled, sampleRate);
    m_pGLFrame->ResumeRenderThread();
    this->accept();
}

void SoundSettings::PressedCancel()
{
    widget.comboBoxSampleRate->setCurrentIndex(m_iRate);
    widget.checkBoxSoundEnabled->setChecked(m_bEnabled);
    m_pGLFrame->ResumeRenderThread();
    this->reject();
}

void SoundSettings::SaveSettings(QSettings& settings)
{
}

void SoundSettings::LoadSettings(QSettings& settings)
{
}
