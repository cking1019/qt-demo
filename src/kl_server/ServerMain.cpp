#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>

QTcpServer* server = new QTcpServer();
QList<QTcpSocket*> clientSockets;

// 定时发送数据
void sendData2RTM() {
    QTimer* pSend2clientTimer = new QTimer();
    QObject::connect(pSend2clientTimer, &QTimer::timeout, [=](){
        for(auto c : clientSockets) {
            c->write("Hello! I am VOI server.");
        }
    });
    pSend2clientTimer->start(1000);
}

// 读取数据
void readDataFromClient(QTcpSocket* clientSocket)
{
    QByteArray data = clientSocket->readAll();
    qDebug() << "Received message from client: " + data.toHex();
}

// 断开连接
void clientDisconnected(QTcpSocket* clientSocket)
{
    clientSockets.removeOne(clientSocket);
    clientSocket->deleteLater();
    qDebug() << "Client disconnected: " << clientSocket->peerAddress().toString();
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!server->listen(QHostAddress::AnyIPv4, 1234))
    {
        qDebug() << "Server could not be started!";
        return -1;
    }

    // 监听新连接
    QObject::connect(server, &QTcpServer::newConnection, [&](){
        qDebug() << "This is  a new connection from client:";
        QTcpSocket* clientSocket = server->nextPendingConnection();
        if (clientSocket != nullptr) {
            clientSockets.append(clientSocket);
            QObject::connect(clientSocket, &QTcpSocket::readyRead, [=](){
                readDataFromClient(clientSocket);
            });
            QObject::connect(clientSocket, &QTcpSocket::disconnected, [=](){
                clientDisconnected(clientSocket);
            });
        }
    });
    
    sendData2RTM();

    qDebug() << "Server is be started and is listening port 1234";
    return app.exec();
}