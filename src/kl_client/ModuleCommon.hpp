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
#include "ModuleCommonHeader.hpp"
// #include "Logger.hpp"
#define RELAY_PATH "./conf/module.ini"


namespace NEBULA {

struct CommonCfg {
    QString serverAddress;
    qint16 serverPort;
    QString moduleCfg20;
};

struct RTMCustomizedCfg {
    QString moduleAddress;
    qint16 serverPort;
    qint16 modulePort;
    
    float elev;
    float range;
    float freqMhz;
    float dFreqMhz;
    float Pow_dBm;
    float SNR_dB;
};

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

	void sendControlledOrder23(uint8_t code);

	void sendLogMsg25(QString msg);
	void sendNote2Operator26(QString msg);

	void sendModuleCPStatus28();

	void recvRegister02(const QByteArray& buff);
	void recvRequestTime04(const QByteArray& buff);

	void recvStart40(const QByteArray& buff);
	void recvStop41(const QByteArray& buff);
	void recvRestart42(const QByteArray& buff);
	void recvReset43(const QByteArray& buff);
	void recvUpdate44(const QByteArray& buff);

	void recvNote4Operator45(const QByteArray& buff);
	void recvRequestModuleFigure46(const QByteArray& buff);

	void recvSettingLang47(const QByteArray& buff);
	void recvRadioAndSatellite48(const QByteArray& buff);
	void recvSettingTime49(const QByteArray& buff);
	void recvModuleLocation4A(const QByteArray& buff);
	void recvCustomizedParam4B(const QByteArray& buff);

	// 公共配置
	CommonCfg commCfg;
	// 是否发送日志	
	bool isDebugOut;

 protected:
	// Socket网络传输
	QTcpSocket* pTcpSocket;
	// 公共包头
	GenericHeader m_genericHeader;
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

	// 再次连接定时器器
	QTimer* pReconnectTimer;
	QTimer* pRequestTimer03;
	QTimer* pCPTimer22;
	QTimer* pModuleStateTimer21;
	QTimer* pModuleStatueTimer24;
	QTimer* pNPTimer28;

	// 时间和位置需要设置，所以单独用变量保存
	ModuleTimeControl0x3 m_moduleTimeControl0x3;
	ModuleGeoLocation0x5 m_ModuleGeoLocation0x5;

 private:
	// 时间差结果
	qint64 m_iStampResult;
	// 时间间隔
	quint64 m_iN;
 signals:
	void signals_msg();

 public slots:
	// 接收服务器数据
	void onReadCommData(qint16 pkgID, const QByteArray& buff);
};


};  // namespace NEBULA
#endif // _COMMONMODULE_H_
