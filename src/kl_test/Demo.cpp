#include "Demo.hpp"

Demo::Demo(QObject *parent):QObject(parent) {
    this->pSocket = new QTcpSocket();
    this->pTimer = new QTimer();
    this->pReconnTimer = new QTimer();
    connect(this->pTimer, &QTimer::timeout, this, &Demo::checkSockState);
    connect(this->pReconnTimer, &QTimer::timeout, this, &Demo::reconnect);
}

void Demo::checkSockState() {
    if(this->pSocket->state() == QTcpSocket::UnconnectedState)  {
        qDebug("UnconnectedState");
        this->pSocket->abort();
    }
    if(this->pSocket->state() == QTcpSocket::ConnectedState)    qDebug("ConnectedState");
    if(this->pSocket->state() == QTcpSocket::ConnectingState)   qDebug("ConnectingState");
    if(this->pSocket->state() == QTcpSocket::ListeningState)   qDebug("ListeningState");
    if(this->pSocket->state() == QTcpSocket::ClosingState)   qDebug("ClosingState");
    if(this->pSocket->state() == QTcpSocket::BoundState)   qDebug("BoundState");
}

void Demo::reconnect() {
    while(!this->pSocket->waitForConnected(1000)) {
        qDebug("wait for connected %d", QDateTime::currentSecsSinceEpoch());
        this->pSocket->connectToHost("127.0.0.1", 1234);
    }
}
