#ifndef _LOGIN_H_
#define _LOGIN_H_

#include <QMessageBox>
#include "MainWin.hpp"
#include "ui/ui_Login.hpp"

class Login : public QMainWindow
{
Q_OBJECT
private:
    Ui::MainWindow loginWin;
    MainWin mainWin;
public:
    Login(QMainWindow* parent = nullptr);
    ~Login();
public slots:
    void loginBtn();
};

#endif // _LOGIN_H_