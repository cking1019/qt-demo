#include "WorkerUdp.hpp"


WorkerUdp::WorkerUdp() {
    m_pUdpSocket = new QUdpSocket(this);
    // if(m_pUdpSocket->state() == QAbstractSocket::UnconnectedState) {
    //     if(m_pUdpSocket->bind(QHostAddress::Any, 5555)) {
    //         qDebug() << "connected udp successfully";
    //     } else {
    //         qDebug() << "connected udp fail";
    //     }
    // } else {
    //     qDebug() << "UDP socket is already connected or in a different state";
    //     qDebug() << m_pUdpSocket->state();
    // }
}

WorkerUdp::~WorkerUdp(){

}

void WorkerUdp::initUdp() {
    if(m_pUdpSocket->bind(QHostAddress::LocalHost, 5555)) {
        qDebug() << "connected udp successfully";
        m_pUdpSocket->connectToHost(QHostAddress::LocalHost, 17002);
    } else {
        qDebug() << "udp bind port fail, please choose unused port";
    }
    
    connect(m_pUdpSocket, &QUdpSocket::readyRead, [&](){
        while (m_pUdpSocket->hasPendingDatagrams())
        {
            QByteArray buf;
            buf.resize(m_pUdpSocket->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            m_pUdpSocket->readDatagram(buf.data(), buf.size(), &sender, &senderPort);
            qDebug() << "Received UDP datagram from" << sender.toString() << ":" << senderPort;
            qDebug() << "Data:" << buf.toHex();
        }
    });
}
