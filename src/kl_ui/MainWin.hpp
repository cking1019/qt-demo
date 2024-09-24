#ifndef _STUWIN_H_
#define _STUWIN_H_
#include <QWidget>
#include <QDate>
#include "ui_stuwin.h"

class MainWin : public QWidget
{
private:
    Ui::Form form;
public:
    MainWin(QWidget* parent = nullptr);
    ~MainWin();
};

#endif // _STUWIN_H_