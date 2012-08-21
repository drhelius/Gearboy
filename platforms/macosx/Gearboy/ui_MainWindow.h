/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created: Tue Aug 21 20:54:21 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSafs;
    QWidget *centralwidget;
    QMenuBar *menubar;
    QMenu *menuHola;
    QMenu *menuPepe;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        actionSafs = new QAction(MainWindow);
        actionSafs->setObjectName(QString::fromUtf8("actionSafs"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        menuHola = new QMenu(menubar);
        menuHola->setObjectName(QString::fromUtf8("menuHola"));
        menuPepe = new QMenu(menubar);
        menuPepe->setObjectName(QString::fromUtf8("menuPepe"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuHola->menuAction());
        menubar->addAction(menuPepe->menuAction());
        menuPepe->addAction(actionSafs);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "newForm1", 0, QApplication::UnicodeUTF8));
        actionSafs->setText(QApplication::translate("MainWindow", "safs", 0, QApplication::UnicodeUTF8));
        menuHola->setTitle(QApplication::translate("MainWindow", "hola", 0, QApplication::UnicodeUTF8));
        menuPepe->setTitle(QApplication::translate("MainWindow", "pepe", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
