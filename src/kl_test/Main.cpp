#include <QDebug>
#include <QDate>
#include <QTcpSocket>
#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QCoreApplication>
#include <future>
#include <QThread>
#include <iostream>
using namespace std;

#pragma pack(push, 1)
struct TimeC {
    uint8_t  id;
    uint64_t s_time;
};
#pragma pack(pop)
// 测试时间
void testTime() {
    time_t now ;
    struct tm *tm_now ;
    time(&now) ;
    printf("now time is %d\n", now);
    tm_now = localtime(&now) ;
    printf("now datetime: %d-%d-%d %d:%d:%d\n", tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec) ;
}

// 测试tcpSocket连接
int testConn(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QTcpSocket tcpSock;
    QTimer qtimer1;
    QTimer qtimer2;

    QObject::connect(&tcpSock, &QTcpSocket::connected, [&]{
        qDebug() << "Connected to host!";
        qtimer1.stop();
        qtimer2.start(1000);
    });
    QObject::connect(&tcpSock, &QTcpSocket::disconnected, [&]{
        qDebug() << "Disconnected from server!";
        qtimer1.start(1000);
        qtimer2.stop();
    });
    QObject::connect(&tcpSock, &QTcpSocket::readyRead, [&]{
        QByteArray buf;
        if(tcpSock.bytesAvailable() > 0) {
            buf = tcpSock.read(tcpSock.bytesAvailable());
        }
        qDebug() << "recv:" << buf.toHex();
    });

    QObject::connect(&qtimer1, &QTimer::timeout, [&]{
        while (!tcpSock.waitForConnected(1000)) {
            qDebug() << "Attempting to connect...";
            // tcpSock.connectToHost(QHostAddress("192.168.1.77"), 4444);
            tcpSock.connectToHost(QHostAddress("127.0.0.1"), 17001);
        }
    });
    QObject::connect(&qtimer2, &QTimer::timeout, [&]{
        QByteArray buf;
        buf.append(QByteArray::fromHex("EEEEEEEE 03022b00 0000e807 09190f31 31720010 00040000 00000000 0000"));
        TimeC tm;
        tm.id = 0;
        // tm.s_time = time(NULL);
        tm.s_time = QDateTime::currentDateTime().toSecsSinceEpoch();
        qDebug() << tm.s_time << "  " << buf.length();
        buf.resize(buf.length() + 8);
        qDebug() << buf.length();
        memcpy(buf.data() + 30, &tm.s_time, 8);
        buf.append(QByteArray::fromHex("00 AAAAAAAA"));
        tcpSock.write(buf);
        tcpSock.flush();
        qDebug() << "send:" << buf.toHex()
                 << "size:" << buf.size();
    });

    qtimer1.start(1000);
    return app.exec();
}

// 测试异步编程
void testAsync() {
    std::vector<std::future<void>> futures;
    for(int i = 0; i < 5; i++) {
        futures.push_back(std::async(std::launch::async, [=](){
            while(true) {
                qDebug() << "Handling connection on socket " << i;
                QThread::msleep(1000);
            }
        }));
    }
    for(auto &f : futures) {
        f.get();
    }
}

// 测试lambda表达式
void testLambda() {
    int a = 1, b = 2, c = 3;
    auto add = [=] ()mutable{
        a++;
        return a + b + c;
    };
    cout << add() << endl;
    cout << a << endl;

    int a2 = 1, b2 = 2, c2 = 3;
    auto add2 = [&] {
        a2++;
        return a2 + b2 + c2;
    };
    cout << add2() << endl;
    cout << a2 << endl;

    int a3 = 10, b3 = 20;
    auto swap1 = [&a3, &b3] {
        int t = a3;
        a3 = b3;
        b3 = t;
    };
    swap1();
    cout << a3 << endl;
    cout << b3 << endl;
}

int main(int argc, char *argv[]) {
    // testTime();
    testConn(argc, argv);
    // testAsync();
    // testLambda();
    return 0;
}