#ifndef _COMMONMODULE_H_
#define _COMMONMODULE_H_


#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QHostAddress>
#include <QDateTime>
#include <QFile>
#include <QSettings>
#include <QTextCodec>
#include "NebulaController.hpp"
#include "CommonHeader.hpp"
// #include "Logger.hpp"
#define RELAY_PATH "./conf/module.ini"


namespace NEBULA {

quint16 calcChcekSum(const char* sMess,int nCnt);

struct Cfg{
    QString serverAddress;
    QString moduleAddress;
    qint16 serverPort;
    qint16 modulePort;
    QString module0x20Cfg;
};

class CommonModule : public QObject {
    Q_OBJECT
 public:
    explicit CommonModule(QObject *parent = nullptr);
    ~CommonModule();
    // 设备启动
    void startup();
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

    // 0x23,发送控制命令
    void sendControlledOrder(uint8_t code);

    // 0x25,发送消息日志
    void sendLogMsg(QString msg);
    // 0x26,发送短信给操作员
    void sendNote2Operator(QString msg);

    // 0x2,确定注册
    void recvRegister(const QByteArray& buff);
    // 0x4,确定对时
    void recvRequestTime(const QByteArray& buff);

    // 0x40,收到开始命令
    void recvStart(const QByteArray& buff);
    // 0x41,收到关闭命令
    void recvStop(const QByteArray& buff);
    // 0x42,收到重启命令
    void recvRestart(const QByteArray& buff);
    // 0x43,收到重置命令
    void recvReset(const QByteArray& buff);
    // 0x44,收到更新命令
    void recvUpdate(const QByteArray& buff);
    // 0x45,收到操作员的短信
    void recvNote4Operator(const QByteArray& buff);
    // 0x46,收到请求模块模块原理图
    void recvRequestModuleFigure(const QByteArray& buff);
    // 0x47,收到设置语言
    void recvSettingLang(const QByteArray& buff);
    // 0x48,收到无线电与卫星导航
    void recvRadioAndSatellite(const QByteArray& buff);
    // 0x49,收到设置时间
    void recvSettingTime(const QByteArray& buff);
    // 0x4A,收到设置模块坐标
    void recvModuleLocation(const QByteArray& buff);
    // 0x4B,收到设置自定义参数
    void recvCustomizedParam(const QByteArray& buff);

    // 模块配置文件
    Cfg cfg;

// 子类需要使用的变量
 protected:
    // Socket网络传输
    QTcpSocket* pTcpSocket;
    // 公共包头
    GenericHeader genericHeader;
    // 公共包类型名称
    QSet<qint16> pkgsComm;

    // 是否注册成功
    bool isRegister;
    // 是否发送模块原理图0x20
    bool isModuleConfigure;
    // 是否发送NP与CP状态0x21&0x22
    bool isNPandCPStatus;
    // 是否发送模块位置0x5
    bool isModuleLocation;
    // 是否连接成功
    bool isConnected;

    // 再次连接定时器器
    QTimer* pReconnectTimer;
    // 定时发送请求时间0x3
    QTimer* pRequestTimer;
    // 定时发送0x21、0x22
    QTimer* pCPandNPTimer;
    // 定时发送0x24
    QTimer* pModuleStatueTimer;

 private:
    // 时间差结果
    qint64 m_iStampResult;
    // 时间间隔
    quint64 m_iN;
 signals:
    void signals_msg();

 // cppcheck-suppress unknownMacro
 public slots:
    // 接收服务器数据
    void onReadCommData(const QByteArray& buff);
};


};  // namespace NEBULA
#endif // _COMMONMODULE_H_
