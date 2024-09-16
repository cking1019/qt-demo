#include "WorkerTcp.hpp"


WorkerTcp::WorkerTcp() {
    m_pTcpSocket = new QTcpSocket(this);
    m_reconnTimer = new QTimer(this);
}

WorkerTcp::~WorkerTcp(){

}

void WorkerTcp::run()
{
    // while(true)
    // {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentDateTimeString = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << "Current Date and Time: " << currentDateTimeString;
        QThread::msleep(1000); // 每秒执行一次
    // }
}

void WorkerTcp::initTcp() {
    connect(m_reconnTimer, &QTimer::timeout, [&](){
        while (!m_pTcpSocket->waitForConnected(1000))
        {
            qDebug() << "Attempting to connect...";
            m_pTcpSocket->connectToHost(QHostAddress::LocalHost, 17001);
        }
        qDebug() << "connect successfully";
        m_reconnTimer->stop();
    });

    connect(m_pTcpSocket,  &QTcpSocket::disconnected, [&](){
        m_reconnTimer->start(1000);
    });

    connect(m_pTcpSocket, &QTcpSocket::readyRead, [&](){
        // while(m_pTcpSocket->waitForReadyRead()) {
        //     QByteArray buf = m_pTcpSocket->readAll();
        //     qDebug() << buf.toHex();
        // }
        QByteArray buf = m_pTcpSocket->readAll();
        qDebug() << buf.toHex();
    });

    m_reconnTimer->start(1000);
}

