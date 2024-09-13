#include "Worker.hpp"

void Worker::process()
{
    while(true)
    {
        qDebug() << "Function is running in another thread...";
        QThread::msleep(1000); // 每秒执行一次
    }
}