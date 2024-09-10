#include "NebulaController.hpp"
#include <QUdpSocket>

namespace NEBULA
{
NebulaController::NebulaController(QObject* parent):QObject(parent) {
    m_pUdpSock2Nebula = new QUdpSocket(this);
    // m_pUdpSock2Nebula->bind(QHostAddress::LocalHost, 5555);
    // m_pUdpSock2Nebula->bind(QHostAddress::Any, 5555);
    // connect(m_pUdpSock2Nebula, &QUdpSocket::readyRead, this, &NebulaController::onRecvUdpData);
    if(!m_pUdpSock2Nebula->bind(5555))
    {
        qDebug() << QString("m_pUdpSock2Nebula bind port:%1 fali!!!").arg(5555);
    }
    else
    {
        connect(m_pUdpSock2Nebula,&QUdpSocket::readyRead,this,&NebulaController::onRecvUdpData); 
    }

    m_pTcpSock2Nebula = new QTcpSocket(this);
    m_pTcpSock2Nebula->connectToHost(QHostAddress::LocalHost, 4321);
    int flag = m_pTcpSock2Nebula->waitForConnected(500) ? 1 : 0;
    if (!flag) {
        //qCritical() << QStringLiteral("连接[%1]失败").arg(strAddress);
    } else {  
        connect(m_pTcpSock2Nebula, &QTcpSocket::readyRead, this, &NebulaController::onRecvTcpData);
    }

    sendUdpData2Nebula();
}

NebulaController::~NebulaController() {
}

void NebulaController::sendUdpData2Nebula() {
    QHostAddress receiverAddress("127.0.0.1");
    quint16 receiverPort = 4321;
    QString msg = "hello world";
    QByteArray data = msg.toUtf8();
    m_pUdpSock2Nebula->writeDatagram(data, receiverAddress, receiverPort);
}

void NebulaController::onRecvUdpData() {
    qDebug() << "recv udp";
    while (m_pUdpSock2Nebula->hasPendingDatagrams()) {
        QByteArray Recivedata;
        Recivedata.resize(m_pUdpSock2Nebula->pendingDatagramSize());
        QHostAddress peerAddr;
        quint16 peerPort;
        m_pUdpSock2Nebula->readDatagram(Recivedata.data(), Recivedata.size(), &peerAddr, &peerPort);
        QString str = Recivedata.data();
        QString peer = "[From" + peerAddr.toString() + ":" + QString::number(peerPort) +"]";
    }
}

void NebulaController::onRecvTcpData() {
    QByteArray buf = m_pTcpSock2Nebula->readAll();
    qDebug() << buf.toHex();
}

}