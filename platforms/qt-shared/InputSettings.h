/* 
 * File:   InputSettings.h
 * Author: nacho
 *
 * Created on 27 de agosto de 2012, 0:59
 */

#ifndef _INPUTSETTINGS_H
#define	_INPUTSETTINGS_H

#include "ui_InputSettings.h"

class InputSettings : public QDialog
{
    Q_OBJECT
    
public:
    InputSettings();
    ~InputSettings();
    
private:
    Ui::InputSettings widget;
};

#endif	/* _INPUTSETTINGS_H */
