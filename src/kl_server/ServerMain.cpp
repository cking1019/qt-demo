#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>

QTcpServer* tcpServer = new QTcpServer();
QList<QTcpSocket*> tcpSocks;

// 定时发送数据
void sendData2RTM() {
    QTimer* pSend2clientTimer = new QTimer();
    QObject::connect(pSend2clientTimer, &QTimer::timeout, [=](){
        for(auto c : tcpSocks) {
            c->write("Hello! I am VOI tcpServer.");
        }
    });
    pSend2clientTimer->start(1000);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!tcpServer->listen(QHostAddress::AnyIPv4, 1234))
    {
        qDebug() << "Server could not be started!";
        return 0;
    }
    qDebug() << "Server is be started and is listening port 1234";

    QObject::connect(tcpServer, &QTcpServer::newConnection, [=](){
        QTcpSocket* tcpSock = tcpServer->nextPendingConnection();
        if(tcpSock == nullptr) return;
        QString addr = tcpSock->peerAddress().toString();
        quint16 port = tcpSock->peerPort();
        qDebug() << QString("connected from %1:%2").arg(addr).arg(port);

        QObject::connect(tcpSock, &QTcpSocket::readyRead, [=](){
            QByteArray buf = tcpSock->readAll();
            qDebug() << QString("recv from %1:%2:").arg(addr).arg(port) 
                     << buf.toHex();
        });
        QObject::connect(tcpSock, &QTcpSocket::connected, [=](){
            qDebug() << QString("connected from %1:%2").arg(addr).arg(port);
        });
        QObject::connect(tcpSock, &QTcpSocket::disconnected, [=](){
            qDebug() << QString("disconnected from %1:%2").arg(addr).arg(port);
            tcpSocks.removeOne(tcpSock);
            tcpSock->deleteLater();
        });
        tcpSocks.append(tcpSock);
    });
    
    QTimer* qTimer = new QTimer();
    QObject::connect(qTimer, &QTimer::timeout, [=](){
        for(auto c : tcpSocks) {
            c->write("Hello! I am VOI tcpServer.");
        }
    });
    qTimer->start(1000);

    
    return app.exec();
}