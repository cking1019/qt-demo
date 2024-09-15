#include "Worker.hpp"
#include "ModuleBase.hpp"
using namespace NEBULA;


Worker::Worker(/* args */)
{
}

Worker::~Worker()
{
}

void Worker::run() {
    while(true) {
        QThread::msleep(1000);
        qDebug() << "m_pthread";
    }
    exec();
}