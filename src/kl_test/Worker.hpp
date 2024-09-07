#ifndef _WORKER_H_
#define _WORKER_H_
#include <QCoreApplication>
#include <QThread>
#include <QDebug>

class Worker : public QObject
{
    Q_OBJECT
public slots:
    void process();
};

#endif // _WORKER_H_