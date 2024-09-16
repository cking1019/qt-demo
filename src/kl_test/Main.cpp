#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include "WorkerTcp.hpp"
#include "WorkerUdp.hpp"
#include <QTcpSocket>


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    auto workerTcp = new WorkerTcp();
    auto workerUdp = new WorkerUdp();
    QThread *workerThreadTcp = new QThread();
    QThread *workerThreadUdp = new QThread();
    workerTcp->moveToThread(workerThreadTcp);
    workerUdp->moveToThread(workerThreadUdp);

    // auto workerUdp = new WorkerUdp();
    // QThread *workerThreadUdp = new QThread();
    // workerUdp->moveToThread(workerThreadUdp);
    
    // 连接信号与槽
    QObject::connect(workerThreadTcp, &QThread::started, workerTcp, &WorkerTcp::initTcp);
    QObject::connect(workerThreadUdp, &QThread::started, workerUdp, &WorkerUdp::initUdp);
    // QObject::connect(workerTcp, &Worker::finished, workerThreadTcp, &QThread::quit);
    // QObject::connect(workerThreadTcp, &QThread::finished, workerTcp, &Worker::deleteLater);
    // QObject::connect(workerThreadTcp, &QThread::finished, workerThreadTcp, &QThread::deleteLater);

    // 启动新线程
    workerThreadTcp->start();
    workerThreadUdp->start();
    // workerTcp->run();
    // qDebug() << "this is main thread";
    while(true) {
        qDebug() << "this is main thread";
        QThread::sleep(1);
    }
    
    return app.exec();
}
