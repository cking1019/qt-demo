#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <iostream>
#include "Demo.hpp"
#include "../kl_client/ModuleCommonHeader.hpp"

struct OSubRezhRTR0x8233 {
    uint32_t N    :8;
    uint32_t reserve :24;

    float curAz;
    QVector<float> freqs = {1, 2};
};



int main(int argc, char *argv[])
{
    OSubRezhRTR0x8233 obj;
    qDebug() << sizeof(OSubRezhRTR0x8233);
    qDebug() << sizeof(obj.freqs);
    return 0;
}