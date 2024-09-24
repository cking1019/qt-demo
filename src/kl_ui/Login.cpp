#include "login.hpp"

Login::Login(QMainWindow* parent) : QMainWindow(parent) {
    mainWindow.setupUi(this);

    QObject::connect(mainWindow.loginBtn, &QPushButton::clicked, this, Login::loginBtn);
}

Login::~Login() {
}

void Login::loginBtn() {
    QString username = mainWindow.accountEd->text();
    QString password = mainWindow.passwdEd->text();

    if(username == "admin" && password == "123456") {
        QMessageBox::information(this, "Login", "Login successful!");
    } else {
        QMessageBox::critical(this, "Login", "Invalid username or password.");
    }
}