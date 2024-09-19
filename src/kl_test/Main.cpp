#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include "WorkerTcp.hpp"
#include "WorkerUdp.hpp"
#include <QTcpSocket>

int val = 10;

int main(int argc, char *argv[])
{
    
    QCoreApplication app(argc, argv);
    int val = 100;
    ::val += 10;
    qDebug() << ::val;
    qDebug() << val;

    return app.exec();
}
