#ifndef _WorkerUdp_H_
#define _WorkerUdp_H_
#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QHostAddress>
#include <QUdpSocket>

class WorkerUdp : public QObject
{
    Q_OBJECT
public:
    WorkerUdp();
    ~WorkerUdp();
    QUdpSocket* m_pUdpSocket;
public slots:
    void initUdp();
};

#endif // _WORKER_H_