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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QThread>

#include <QDir>
#include <QMutex>
#include "ModuleCommonHeader.hpp"
// #include "Logger.hpp"
#define RELAY_PATH "./conf/module.ini"


namespace NEBULA {

struct CommonCfg {
    QString serverAddress;
    qint16 serverPort;
    QString moduleAddress;
    qint16 modulePort;
    QString moduleCfg20;
};

// struct RTMCustomizedCfg {
//     QString moduleAddress;
//     qint16 modulePort;
// };

enum class ConnStatus{unConnected, connecting, connected};
enum class RegisterStatus{unRegister, registering, registered};
enum class TimeStatus{unTime, timing, timed};

// 包头长度
const qint8 HEADER_LEN = 16;

quint16 calcChcekSum(const char* sMess,int nCnt);
QString readJson(QString DevConfig20);

class CommonModule : public QObject {
	 Q_OBJECT
 public:
	explicit CommonModule(QObject *parent = nullptr);
	~CommonModule();
	// 设备启动
	void startup();
	void reqAndResTime(quint64 time1, quint64 time2);

	void sendRegister01();
	void sendRequestTime03();
	void sendModuleLocation05();

	void sendModuleFigure20();
	void sendModuleStatus21();
	void sendModuleCPStatus22();
	void sendModuleStatus24();

	void sendControlledOrder23(uint8_t code, quint16 pkgId);

	void sendLogMsg25(QString msg);
	void sendNote2Operator26(QString msg);

	void sendModuleCPStatus28();

	void recvRegister02(const QByteArray& buf);
	void recvRequestTime04(const QByteArray& buf);

	void recvStart40(const QByteArray& buf);
	void recvStop41(const QByteArray& buf);
	void recvRestart42(const QByteArray& buf);
	void recvReset43(const QByteArray& buf);
	void recvUpdate44(const QByteArray& buf);

	void recvNote4Operator45(const QByteArray& buf);
	void recvRequestModuleFigure46(const QByteArray& buf);

	void recvSettingLang47(const QByteArray& buf);
	void recvRadioAndSatellite48(const QByteArray& buf);
	void recvSettingTime49(const QByteArray& buf);
	void recvModuleLocation4A(const QByteArray& buf);
	void recvCustomizedParam4B(const QByteArray& buf);

	// 公共配置
	CommonCfg m_commCfg;
	// 是否发送日志	
	bool m_isDebugOut;
    // 公共包头
	GenericHeader m_genericHeader;

 protected:
	// Socket网络传输
	QTcpSocket* pTcpSocket;
	// 公共包类型
	QSet<qint16> pkgsComm;

	// 连接状态
	ConnStatus connStatus;
	// 注册状态
	RegisterStatus registerStatus;
	// 对时状态
	TimeStatus timeStatus;

	// 连接状态,此类协议不是定期发送，因此用bool类型判断是否已发送
	bool m_isSendRegister01;
	bool m_isModuleLocation05;
	bool m_isModuleConfigure20;
	bool m_isSendForbiddenIRIList828;

	// 再次连接定时器器
	QTimer* pReconnectTimer;
	QTimer* pRequestTimer03;
	QTimer* pCPTimer22;
	QTimer* pModuleStateTimer21;
	QTimer* pModuleStatueTimer24;
	QTimer* pNPTimer28;

    ModuleRegister0x1 m_moduleRegister0x1;
	ModuleTimeControl0x3 m_moduleTimeControl0x3;
	ModuleGeoLocation0x5 m_ModuleGeoLocation0x5;
    ONPStatus0x21 m_oNPStatus0x21;
    OCPStatus0x22 m_oCPStatus0x22;
    OModuleStatus0x24 m_oModuleStatus0x24;
    OReqCtl0x23 m_oReqCtl0x23;
    LogMsg0x25 m_logMsg0x25;
    CustomisedParm0x28 m_customisedParm0x28;

 private:
	// 时间差结果
	qint64 m_iStampResult;
	// 时间间隔
	quint64 m_iN;
 signals:
	void signals_msg();

 public slots:
	// 接收服务器数据
	void onReadCommData(qint16 pkgID, const QByteArray& buf);
};


};  // namespace NEBULA
#endif // _COMMONMODULE_H_
