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

#include <QKeyEvent>
#include "InputSettings.h"
#include "GLFrame.h"

InputSettings::InputSettings(GLFrame* pGLFrame)
{
    m_pGLFrame = pGLFrame;
    widget.setupUi(this);
    widget.lineEditUp->installEventFilter(this);
    widget.lineEditRight->installEventFilter(this);
    widget.lineEditDown->installEventFilter(this);
    widget.lineEditLeft->installEventFilter(this);
    widget.lineEditA->installEventFilter(this);
    widget.lineEditB->installEventFilter(this);
    widget.lineEditStart->installEventFilter(this);
    widget.lineEditSelect->installEventFilter(this);
}

InputSettings::~InputSettings()
{
}

int InputSettings::GetKey(int key)
{
    for (int i = 0; i < 8; i++)
    {
        if (m_Keys[i].keyCode == key)
            return i;
    }
    return -1;
}

void InputSettings::SaveKeys()
{
    for (int i = 0; i < 8; i++)
    {
        m_Keys[i].keyCode = m_TempKeys[i].keyCode;
        strcpy(m_Keys[i].text, m_TempKeys[i].text);
    }
    m_pGLFrame->ResumeRenderThread();
    this->accept();
}

void InputSettings::RestoreKeys()
{
    for (int i = 0; i < 8; i++)
    {
        m_TempKeys[i].keyCode = m_Keys[i].keyCode;
        strcpy(m_TempKeys[i].text, m_Keys[i].text);
    }
    widget.lineEditUp->setText(m_Keys[0].text);
    widget.lineEditRight->setText(m_Keys[1].text);
    widget.lineEditDown->setText(m_Keys[2].text);
    widget.lineEditLeft->setText(m_Keys[3].text);
    widget.lineEditA->setText(m_Keys[4].text);
    widget.lineEditB->setText(m_Keys[5].text);
    widget.lineEditStart->setText(m_Keys[6].text);
    widget.lineEditSelect->setText(m_Keys[7].text);
    m_pGLFrame->ResumeRenderThread();
    this->reject();
}

bool InputSettings::eventFilter(QObject* pObj, QEvent *pEvent)
{
    if (pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*> (pEvent);

        char text[32];

        if (strcmp(pObj->metaObject()->className(), "QLineEdit") == 0)
        {
            PrintKey(*keyEvent, text);
            QLineEdit* pLineEdit = static_cast<QLineEdit*> (pObj);
            pLineEdit->setText(text);

            if (pObj == widget.lineEditUp)
            {
                m_TempKeys[0].keyCode = keyEvent->key();
                strcpy(m_TempKeys[0].text, text);
            }
            else if (pObj == widget.lineEditRight)
            {
                m_TempKeys[1].keyCode = keyEvent->key();
                strcpy(m_TempKeys[1].text, text);
            }
            else if (pObj == widget.lineEditDown)
            {
                m_TempKeys[2].keyCode = keyEvent->key();
                strcpy(m_TempKeys[2].text, text);
            }
            else if (pObj == widget.lineEditLeft)
            {
                m_TempKeys[3].keyCode = keyEvent->key();
                strcpy(m_TempKeys[3].text, text);
            }
            else if (pObj == widget.lineEditA)
            {
                m_TempKeys[4].keyCode = keyEvent->key();
                strcpy(m_TempKeys[4].text, text);
            }
            else if (pObj == widget.lineEditB)
            {
                m_TempKeys[5].keyCode = keyEvent->key();
                strcpy(m_TempKeys[5].text, text);
            }
            else if (pObj == widget.lineEditStart)
            {
                m_TempKeys[6].keyCode = keyEvent->key();
                strcpy(m_TempKeys[6].text, text);
            }
            else if (pObj == widget.lineEditSelect)
            {
                m_TempKeys[7].keyCode = keyEvent->key();
                strcpy(m_TempKeys[7].text, text);
            }

            return true;
        }
    }

    return QDialog::eventFilter(pObj, pEvent);
}

void InputSettings::PrintKey(QKeyEvent& pEvent, char* buffer)
{
    switch (pEvent.key())
    {
        case Qt::Key_Control:
            strcpy(buffer, "CONTROL");
            break;
        case Qt::Key_Alt:
            strcpy(buffer, "ALT");
            break;
        case Qt::Key_Enter:
            strcpy(buffer, "ENTER");
            break;
        case Qt::Key_Shift:
            strcpy(buffer, "SHIFT");
            break;
        case Qt::Key_Backspace:
            strcpy(buffer, "BACKSPACE");
            break;
        case Qt::Key_Up:
            strcpy(buffer, "UP");
            break;
        case Qt::Key_Left:
            strcpy(buffer, "LEFT");
            break;
        case Qt::Key_Right:
            strcpy(buffer, "RIGHT");
            break;
        case Qt::Key_Down:
            strcpy(buffer, "DOWN");
            break;
        case Qt::Key_Return:
            strcpy(buffer, "RETURN");
            break;
        case Qt::Key_Space:
            strcpy(buffer, "SPACE");
            break;
        case Qt::Key_Tab:
            strcpy(buffer, "TAB");
            break;
        case Qt::Key_Home:
            strcpy(buffer, "HOME");
            break;
        case Qt::Key_End:
            strcpy(buffer, "END");
            break;
        case Qt::Key_PageUp:
            strcpy(buffer, "PAGE UP");
            break;
        case Qt::Key_PageDown:
            strcpy(buffer, "PAGE DOWN");
            break;
        case Qt::Key_Insert:
            strcpy(buffer, "INSERT");
            break;
        case Qt::Key_Delete:
            strcpy(buffer, "DELETE");
            break;
        default:
            strcpy(buffer, pEvent.text().toUpper().toLatin1());
    }
}

void InputSettings::SaveSettings(QSettings& settings)
{
    settings.setValue("KeyUP", m_Keys[0].keyCode);
    settings.setValue("KeyRIGHT", m_Keys[1].keyCode);
    settings.setValue("KeyDOWN", m_Keys[2].keyCode);
    settings.setValue("KeyLEFT", m_Keys[3].keyCode);
    settings.setValue("KeyA", m_Keys[4].keyCode);
    settings.setValue("KeyB", m_Keys[5].keyCode);
    settings.setValue("KeySTART", m_Keys[6].keyCode);
    settings.setValue("KeySELECT", m_Keys[7].keyCode);

    settings.setValue("KeyNameUP", m_Keys[0].text);
    settings.setValue("KeyNameRIGHT", m_Keys[1].text);
    settings.setValue("KeyNameDOWN", m_Keys[2].text);
    settings.setValue("KeyNameLEFT", m_Keys[3].text);
    settings.setValue("KeyNameA", m_Keys[4].text);
    settings.setValue("KeyNameB", m_Keys[5].text);
    settings.setValue("KeyNameSTART", m_Keys[6].text);
    settings.setValue("KeyNameSELECT", m_Keys[7].text);
}

void InputSettings::LoadSettings(QSettings& settings)
{
    m_Keys[0].keyCode = settings.value("KeyUP", Qt::Key_Up).toInt();
    m_Keys[1].keyCode = settings.value("KeyRIGHT", Qt::Key_Right).toInt();
    m_Keys[2].keyCode = settings.value("KeyDOWN", Qt::Key_Down).toInt();
    m_Keys[3].keyCode = settings.value("KeyLEFT", Qt::Key_Left).toInt();
    m_Keys[4].keyCode = settings.value("KeyA", Qt::Key_S).toInt();
    m_Keys[5].keyCode = settings.value("KeyB", Qt::Key_A).toInt();
    m_Keys[6].keyCode = settings.value("KeySTART", Qt::Key_Return).toInt();
    m_Keys[7].keyCode = settings.value("KeySELECT", Qt::Key_Space).toInt();

    strcpy(m_Keys[0].text, settings.value("KeyNameUP", "UP").toString().toLatin1().constData());
    strcpy(m_Keys[1].text, settings.value("KeyNameRIGHT", "RIGHT").toString().toLatin1().constData());
    strcpy(m_Keys[2].text, settings.value("KeyNameDOWN", "DOWN").toString().toLatin1().constData());
    strcpy(m_Keys[3].text, settings.value("KeyNameLEFT", "LEFT").toString().toLatin1().constData());
    strcpy(m_Keys[4].text, settings.value("KeyNameA", "S").toString().toLatin1().constData());
    strcpy(m_Keys[5].text, settings.value("KeyNameB", "A").toString().toLatin1().constData());
    strcpy(m_Keys[6].text, settings.value("KeyNameSTART", "RETURN").toString().toLatin1().constData());
    strcpy(m_Keys[7].text, settings.value("KeyNameSELECT", "SPACE").toString().toLatin1().constData());

    widget.lineEditUp->setText(m_Keys[0].text);
    widget.lineEditRight->setText(m_Keys[1].text);
    widget.lineEditDown->setText(m_Keys[2].text);
    widget.lineEditLeft->setText(m_Keys[3].text);
    widget.lineEditA->setText(m_Keys[4].text);
    widget.lineEditB->setText(m_Keys[5].text);
    widget.lineEditStart->setText(m_Keys[6].text);
    widget.lineEditSelect->setText(m_Keys[7].text);

    for (int i = 0; i < 8; i++)
    {
        m_TempKeys[i].keyCode = m_Keys[i].keyCode;
        strcpy(m_TempKeys[i].text, m_Keys[i].text);
    }
}
