#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QtDebug>
#include <QMessageBox>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "Login.hpp"
#include "MainWin.hpp"
using namespace Ui;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("192.168.72.128");
    db.setPort(3306);
    db.setDatabaseName("test");
    db.setUserName("root");
    db.setPassword("123456");

    if (!db.open()) {
        qDebug() << "Error: Unable to open database";
    } else {
        qDebug() << "Connected to database";
    }

    Login login;
    login.show();
    // MainWin mainWin;
    // mainWin.show();

    return a.exec();
}
