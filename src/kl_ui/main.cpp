#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QtDebug>
#include <QMessageBox>
#include "ui_login.h"
#include "ui_stuwin.h"
using namespace Ui;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    QMainWindow widget;
    MainWindow ui_mainWindow;
    ui_mainWindow.setupUi(&widget);

    QWidget formWidget;
    Form ui_subWindow;
    ui_subWindow.setupUi(&formWidget);

    // ui_mainWindow.setupUi(&widget);
    QObject::connect(ui_mainWindow.loginBtn, &QPushButton::clicked, [&](){
        QString username = ui_mainWindow.accountEd->text();
        QString password = ui_mainWindow.passwdEd->text();

        if(username == "admin" && password == "123456") {
            QMessageBox::information(&widget, "Login", "Login successful!");
            formWidget.setWindowTitle("student windows");
            formWidget.show();
        } else {
            QMessageBox::critical(&widget, "Login", "Invalid username or password.");
        }}
    );
    QObject::connect(ui_mainWindow.registerBtn, &QPushButton::clicked, [&](){
        qDebug() << "regesiter";
    });
    
    widget.show();
    return a.exec();
}
