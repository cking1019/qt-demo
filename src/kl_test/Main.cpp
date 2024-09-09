#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include "Worker.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Worker worker;
    QThread* thread = new QThread();

    QObject::connect(thread, &QThread::started, &worker, &Worker::process);

    worker.moveToThread(thread);
    thread->start();
    qDebug() << "hello wrold";

    return a.exec();
}
