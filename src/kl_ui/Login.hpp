#ifndef _LOGIN_H_
#define _LOGIN_H_
#include "ui_login.h"
#include <QMessageBox>

class Login : public QMainWindow
{
Q_OBJECT
private:
    Ui::MainWindow mainWindow;
public:
    Login(QMainWindow* parent = nullptr);
    ~Login();
public slots:
    void loginBtn();
};

#endif // _LOGIN_H_