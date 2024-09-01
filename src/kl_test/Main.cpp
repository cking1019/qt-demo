#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include "Demo.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Demo demo;
    demo.pTimer->start(1000);
    demo.pReconnTimer->start();

    if (demo.pSocket->waitForConnected()) {
        demo.pSocket->write("Hello, server!");
        demo.pSocket->flush();
    }


    return a.exec();
}