#ifndef _CommonModule_H_
#define _CommonModule_H_

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
#include <iostream>
#include "NebulaController.hpp"
#include "DataHeader.hpp"
// #include "Logger.hpp"
#define RELAY_PATH "./conf/module.ini"
using namespace std;

namespace NEBULA
{
struct Cfg{
    QString serverAddress;
    QString moduleAddress;
    qint16 serverPort;
    qint16 modulePort;
    QString module0x20Cfg;
};
class CommonModule : public QObject
{
    Q_OBJECT
public:
    explicit CommonModule(QObject *parent = nullptr);
    ~CommonModule();
    // 设备启动
    void startup();
    void initSocket();
    // 发送0x21和0x22
    void sendCPandNPStatus();


    // 0x1,请求注册
    void sendRegister();
    // 0x3,请求对时
    void sendRequestTime();
    // 0x5,发送模块位置
    void sendModuleLocation();

    // 0x20,发送模块图
    void sendModuleFigure();
    // 0x21,发送电路元素状态
    void sendModuleNPStatus();
    // 0x22,发送电路元素受控状态
    void sendModuleCPStatus();
    // 0x24,发送模块状态
    void sendModuleStatus();
    
    // 0x25,发送消息日志
    void sendLogMsg(QString msg);
    // 0x26,发送短信给操作员
    void sendNote2Operator(QString msg);

    // 0x2,确定注册
    void recvRegister(QByteArray buff);
    // 0x4,确定对时
    void recvRequestTime(QByteArray buff);

    
    // 0x40,收到开始命令
    void recvStart(QByteArray buff);
    // 0x41,收到关闭命令
    void recvStop(QByteArray buff);
    // 0x42,收到重启命令
    void recvRestart(QByteArray buff);
    // 0x43,收到重置命令
    void recvReset(QByteArray buff);
    // 0x44,收到更新命令
    void recvUpdate(QByteArray buff);
    // 0x45,收到操作员的短信
    void recvNote4Operator(QByteArray buff);
    // 0x46,收到请求模块模块原理图
    void recvRequestModuleFigure(QByteArray buff);
    // 0x4A,收到设置模块坐标
    void recvModuleLocation(QByteArray buff);
    // 0x4B,收到设置自定义参数
    void recvCustomizedParam(QByteArray buff);
    // 模块配置文件
    Cfg cfg;
private:
    // Socket网络传输
    QTcpSocket* pTcpSocket;
    
    // 再次连接定时器器
    QTimer* pReconnectTimer;
    // 定时发送请求时间0x3
    QTimer* pRequestTimer;
    // 定时发送0x21、0x22
    QTimer* pCPandNPTimer;

    // 连接主机标识
    bool m_iConnectHostFlag;
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


}
#endif // _CommonModule_H_
