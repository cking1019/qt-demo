#include "login.hpp"

Login::Login(QMainWindow* parent) : QMainWindow(parent) {
    loginWin.setupUi(this);
    QObject::connect(loginWin.loginBtn, &QPushButton::clicked, this, &Login::loginBtn);
}

Login::~Login() {
}

void Login::loginBtn() {
    QString username = loginWin.accountEd->text();
    QString password = loginWin.passwdEd->text();

    if(username == "admin" && password == "123456") {
        // QMessageBox::information(this, "Login", "Login successful!");
        mainWin.show();
        close();
    } else {
        QMessageBox::critical(this, "Login", "Invalid username or password.");
    }
}