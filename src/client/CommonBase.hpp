#ifndef _COMMONBASE_H_
#define _COMMONBASE_H_

#include <QObject>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QHostAddress>
#include <QThread>
#include <QDateTime>
#include <QFile>
#include <QSettings>
#include <QTextCodec>
#include "NebulaController.hpp"
#include "CommonHeader.hpp"
#include "CommonBase.hpp"
#define RELAY_PATH "./conf/module.ini"

class CommonBase : public QObject
{
    Q_OBJECT
public:
    explicit CommonBase(QObject *parent = nullptr);
    ~CommonBase();
    void init();
    void initSocket();

    // 请求注册,0x1
    void sendRegister();
    // 请求对时,0x3
    void sendRequestTime();
    // 发送模块位置,0x5
    void sendModuleLocation();
    // 发送模块图,0x20
    void sendModuleFigure();
    // 发送电路元素状态,0x21
    void sendModuleNPStatus();
    // 发送电路元素受控状态,0x22
    void sendModuelCPStatus();
    // 发送接收控制命令的收据,0x23
    void sendControlledOrder();
    // 发送模块状态,0x24
    void sendModuleStatus();
    // 发送消息日志,0x25
    void sendLogMsg();
    // 发送短信给操作员,0x26
    void sendNote2Operator();

    // 确定注册,0x2
    void recvRegister(QByteArray buff);
    // 确定对时,0x4
    void recvRequestTime(QByteArray buff);
    // 收到开始命令,0x40
    void recvStart(QByteArray buff);
    // 收到关闭命令,0x41
    void recvStop(QByteArray buff);
    // 收到重启命令,0x42
    void recvRestart(QByteArray buff);
    // 收到重置命令,0x43
    void recvReset(QByteArray buff);
    // 收到更新命令,0x44
    void recvUpdate(QByteArray buff);
    // 收到操作员的短信,0x45
    void recvNote4Operator(QByteArray buff);
    // 收到请求模块模块原理图,0x46
    void recvRequestModuleFigure(QByteArray buff);
    // 收到设置模块坐标,0x4A
    void recvModuleLocation(QByteArray buff);
    // 收到设置自定义参数,0x4B
    void recvCustomizedParam(QByteArray buff);

private:
    // Tcp Socket网络传输
    QTcpSocket* pTcpSocket;
    // nebula通信控制器
    NebulaController* pNebulaController;
    // 请求时间定时器
    QTimer* pRequestTimer;
    // 再次连接定时器器
    QTimer* pReconnectTimer;
    // 连接主机标识
    int m_iConnectHostFlag;
    // 时间差结果
    qint64 m_iStampResult;
    // 时间间隔
    quint64 m_iN;

    // 公共包头
    GenericHeader genericHeader;

signals:
    void signals_msg();

public slots:
    // 接收服务器数据
    void onReadData();
};

#endif // _COMMONBASE_H_
