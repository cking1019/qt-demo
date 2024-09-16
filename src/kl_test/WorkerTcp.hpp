#ifndef _WorkerTcp_H_
#define _WorkerTcp_H_
#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QHostAddress>
#include <QUdpSocket>

class WorkerTcp : public QObject
{
    Q_OBJECT
public:
    WorkerTcp();
    ~WorkerTcp();
    
    QTcpSocket* m_pTcpSocket;
    QTimer* m_reconnTimer;
public slots:
    void initTcp();
    void run();
};

#endif // _WORKER_H_