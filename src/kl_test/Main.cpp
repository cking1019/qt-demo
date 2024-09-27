#include <QDebug>
#include <QDate>
#include <QTcpSocket>
#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QCoreApplication>

#pragma pack(push, 1)
struct TimeC {
    uint8_t id;
    uint64_t s_time;
};
#pragma pack(pop)

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QTcpSocket tcpSock;
    QTimer qtimer1;
    QObject::connect(&qtimer1, &QTimer::timeout, [&](){
        while (!tcpSock.waitForConnected()) {
            qDebug() << "Attempting to connect...";
            // tcpSock.connectToHost(QHostAddress("192.168.1.77"), 4444);
            tcpSock.connectToHost(QHostAddress("127.0.0.1"), 17001);
        }
    });
    QObject::connect(&tcpSock, &QTcpSocket::connected, [&](){
        qDebug() << "Connected to host!";
        qtimer1.stop();
    });
    QObject::connect(&tcpSock, &QTcpSocket::disconnected, [&](){
        qDebug() << "Disconnected from server!";
        qtimer1.start(1000);
    });
    QObject::connect(&tcpSock, &QTcpSocket::readyRead, [&](){
        QByteArray buf;
        if(tcpSock.bytesAvailable() > 0) {
            buf = tcpSock.read(tcpSock.bytesAvailable());
        }
        qDebug() << "recv:" << buf.toHex();
    });

    QTimer qtimer2;
    QObject::connect(&qtimer2, &QTimer::timeout, [&](){
        QByteArray buf;
        buf.append(QByteArray::fromHex("EEEEEEEE 03022b00 0000e807 09190f31 31720010 00040000 00000000 0000"));
        // eeeeeeee 03022b00 0000e807 09190f31 
        // 31720010 00040000 00000000 00 00 cdeaf36600000000 00aa aaaaaa
        // 1,727,261,389
        TimeC t;
        t.id = 0;
        // t.s_time = time(NULL);
        t.s_time = QDateTime::currentDateTime().toSecsSinceEpoch();
        qDebug() << t.s_time << "  " << buf.length();
        buf.resize(buf.length() + 8);
        qDebug() << buf.length();
        memcpy(buf.data() + 30, &t.s_time, 8);
        // QByteArray buf_time;
        // buf_time.resize(8);
        // memcpy(buf_time.data(), &t.s_time, 8);
        // buf.append(buf_time);
        buf.append(QByteArray::fromHex("00 AAAAAAAA"));

        tcpSock.write(buf);
        tcpSock.flush();
        qDebug() << "send:" << buf.toHex()
                 << "size:" << buf.size();
    });

    qtimer1.start(1000);
    qtimer2.start(1000);
    return app.exec();
}