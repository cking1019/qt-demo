#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include "WorkerTcp.hpp"
#include "WorkerUdp.hpp"
#include <QTcpSocket>


// int main(int argc, char *argv[])
// {
//     QCoreApplication app(argc, argv);

//     auto workerTcp = new WorkerTcp();
//     auto workerUdp = new WorkerUdp();
//     QThread *workerThreadTcp = new QThread();
//     QThread *workerThreadUdp = new QThread();
//     workerTcp->moveToThread(workerThreadTcp);
//     workerUdp->moveToThread(workerThreadUdp);

//     // auto workerUdp = new WorkerUdp();
//     // QThread *workerThreadUdp = new QThread();
//     // workerUdp->moveToThread(workerThreadUdp);
    
//     // 连接信号与槽
//     QObject::connect(workerThreadTcp, &QThread::started, workerTcp, &WorkerTcp::initTcp);
//     QObject::connect(workerThreadUdp, &QThread::started, workerUdp, &WorkerUdp::initUdp);
//     // QObject::connect(workerTcp, &Worker::finished, workerThreadTcp, &QThread::quit);
//     // QObject::connect(workerThreadTcp, &QThread::finished, workerTcp, &Worker::deleteLater);
//     // QObject::connect(workerThreadTcp, &QThread::finished, workerThreadTcp, &QThread::deleteLater);

//     // 启动新线程
//     workerThreadTcp->start();
//     workerThreadUdp->start();
//     // workerTcp->run();
//     // qDebug() << "this is main thread";
//     while(true) {
//         qDebug() << "this is main thread";
//         QThread::sleep(1);
//     }
    
//     return app.exec();
// }

// 在Qt中，可以在两个不同线程的类之间建立信号与槽连接

// 在线程1中的类


#include "ClassA.hpp"
#include "ClassB.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 创建两个对象，分别在不同线程中运行
    ClassA objA;
    ClassB objB;

    QThread threadA;
    QThread threadB;

    // 将对象移动到对应的线程
    objA.moveToThread(&threadA);
    objB.moveToThread(&threadB);

    // 建立信号与槽连接
    QObject::connect(&objB, &ClassB::dataReady, &objA, &ClassA::processData);

    // 启动线程
    threadA.start();
    threadB.start();

    // 发送信号
    QTimer::singleShot(0, &objB, &ClassB::sendDataToClassA);

    return app.exec();
}
