#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include "Demo.hpp"
#include "../kl_client/CommonHeader.hpp"


template <class T>
void printStruct(const T& s)
{
    const char* structName = typeid(s).name();
    qDebug() << "Printing information for struct: ==" << structName;
    const char* data = reinterpret_cast<const char*>(&s);
    QByteArray byteArray(data);
    // char* data2 = (char*)malloc(sizeof(T));
    // memcpy(data2, &s, sizeof(T));
    qDebug("%x", byteArray);
    // qDebug() << data2;

    // long long memberValue = 0;
    // memcpy(&memberValue, data + 0, 16);
    // qDebug("Member 1 is %x", data);
    // // qDebug("Member 1 is %x", data2);
    // int memberValue2 = 0;
    // memcpy(&memberValue, data + 3, 1);
    // qDebug("Member 2 is %x", memberValue2);
    // 使用位操作获取位字段的值
    // for (int i = 0; i < sizeof(T); i += 1) {
    //     int memberValue = 0;
    //     memcpy(&memberValue, reinterpret_cast<const char*>(&s) + i, 1);
    //     qDebug("Member %d is %x", i, memberValue);
    // }
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Demo demo;
    demo.pTimer->start(1000);
    demo.pReconnTimer->start();

    // if (demo.pSocket->waitForConnected()) {
    //     demo.pSocket->write("Hello, server!");
    //     demo.pSocket->flush();
    // }


    return a.exec();
}