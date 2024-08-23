#ifndef _RTMCONTROLLER_H_
#define _RTMCONTROLLER_H_

#include <QObject>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QHostAddress>
#include <QThread>
#include <QDateTime>
#include "NebulaController.hpp"
#include "CommonHeader.hpp"
#include "CommonBase.hpp"

class RTMController : public QObject
{
    Q_OBJECT
public:
    explicit RTMController(QObject *parent = nullptr);
    ~RTMController();
    void init();
    void initSocket();
    void removeConnectionBySocket();
    // 注册请求,0x1
    void sendRegister();
    // 对时请求,0x3
    void sendRequestTime();

    // 确定注册,0x2
    void recvRegister(QByteArray buff);
    // 确定对时,0x4
    void recvRequestTime(QByteArray buff);
private:
    NebulaController* pNebulaController;
    QTimer* pRecvTimer;
    QTimer* pSendTimer;
    QTimer* pRequestTimer;
    QTcpSocket* pTcpSocket;
    // 包的序列号
    qint32 packIdx;
    // 模块ID
    int m_moduleIdx;
    // 连接主机标识
    int m_iConnectHostFlag;
    // 时间差结果
    qint64 m_iStampResult;
    // 时间间隔
    quint64 m_iN;
signals:
    void signals_msg();

public slots:
    void sendTime2Ctl();
    void onReadData();
};

#endif // _RTMCONTROLLER_H_
