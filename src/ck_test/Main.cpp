#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include "../ck_client/CommonHeader.hpp"


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
    GenericHeader genericHeader;
    genericHeader.sender = 0x524542;
    genericHeader.moduleId = 0xFF;
    printStruct(genericHeader);
    return 0;
}