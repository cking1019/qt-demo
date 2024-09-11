#include "NebulaCommon.hpp"

namespace NEBULA {

NebulaCommon::NebulaCommon(/* args */)
{
    m_pUdpSock2Nebula = new QUdpSocket(this);
    if(!m_pUdpSock2Nebula->bind(5555)) {
        qDebug() << QString("m_pUdpSock2Nebula bind port:%1 fali!!!").arg(5555);
    } else {
        qDebug() << "udp connected successfully";
        connect(m_pUdpSock2Nebula, &QUdpSocket::readyRead, this, &NebulaCommon::onRecvUdpData); 
    }

    QTimer* mTimer = new QTimer();
    connect(mTimer, &QTimer::timeout, this, &NebulaCommon::sendUdpData2Nebula);
    mTimer->start(1000);//每隔一秒发送一次数据
}

NebulaCommon::~NebulaCommon()
{
}

void NebulaCommon::sendUdpData2Nebula() {
    QHostAddress receiverAddress("127.0.0.1");
    quint16 receiverPort = 5554;
    QString msg = "hello world";
    QByteArray data = msg.toUtf8();
    m_pUdpSock2Nebula->writeDatagram(data, receiverAddress, receiverPort);
    emit signalSendDetectTarget2Ctl();
}

void NebulaCommon::onRecvUdpData() {
    if (m_pUdpSock2Nebula->hasPendingDatagrams()) {
        QByteArray buf;
        buf.resize(m_pUdpSock2Nebula->pendingDatagramSize());
        qint64 len = m_pUdpSock2Nebula->readDatagram(buf.data(), buf.size());
        if(len > 0) {
            QString str = buf.toHex();
            qDebug() << "buf data:"  << buf.toHex()
                     << "data size:" << len;
        }
    }
}

}