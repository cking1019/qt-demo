#ifndef _ModuleBase_H_
#define _ModuleBase_H_

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
#include <QJsonArray>
#include <QJsonDocument>
#include "ModuleHeader.hpp"
// #include "Logger.hpp"
#define COMM_CFG "./conf/common.ini"

quint16 calcChcekSum(const char* sMess, int nCnt);
QString readJson(QString DevConfig20);
void readjsonArray(QString freq);

namespace NEBULA {

enum class ConnStatus{unConnected, connecting, connected};
enum class RegisterStatus{unRegister, registering, registered};
enum class TimeStatus{unTime, timing, timed};

// 包头长度
const qint8 HEADER_LEN = sizeof(GenericHeader);

class ModuleBase : public QObject {
	 Q_OBJECT
 public:
	ModuleBase();
    // 基类的析构加上virtual，便于当基类指针指向派生类时，delete基类指针也会调用派生类的析构函数
	virtual ~ModuleBase();
	virtual void startup() = 0;
    virtual void onRecvData() = 0;
	void reqAndResTime(quint64 time1, quint64 time2);

	void sendRegister01();
	void sendRequestTime03();
	void sendModuleLocation05();

	void sendModuleFigure20();
	void sendModuleStatus21();
	void sendModuleCPStatus22();
    void sendModuleCPStatus28();
	void sendModuleStatus24();

	void sendControlledOrder23(uint8_t code, quint16 pkgId);

	void sendLogMsg25(QString msg);
	void sendNote2Operator26(QString msg);

	void recvRegister02(const QByteArray& buf);
	void recvRequestTime04(const QByteArray& buf);

	void recvNote4Operator45(const QByteArray& buf);
	void recvRequestModuleFigure46(const QByteArray& buf);

	void recvRadioAndSatellite48(const QByteArray& buf);
	void recvSettingTime49(const QByteArray& buf);
	void recvModuleLocation4A(const QByteArray& buf);
	void recvCustomizedParam4B(const QByteArray& buf);

	// 公共配置,包括0x20配置信息
	QSettings* commCfgini;
	bool m_isDebugOut;
protected:
	qint16 m_id;
    // 公共包头
	GenericHeader m_genericHeader;
	// Socket网络传输
	QTcpSocket* m_pTcpSocket;
	// 公共包类型
	QSet<qint16> pkgsComm;

	QString serverAddress;
    qint16 serverPort;
	QString moduleAddress;
    qint16 modulePort;
	QString moduleCfg20;

	// 连接、注册、对时
	ConnStatus m_connStatus;
	RegisterStatus m_registerStatus;
	TimeStatus m_timeStatus;

	// 连接状态,此类协议不是定期发送，因此用bool类型判断是否已发送
	bool m_isSendRegister01;
	bool m_isSendModuleLocation05;
	bool m_isSendModuleConfigure20;
	

	// 再次连接定时器器
	QTimer* m_pReconnectTimer;
	QTimer* m_pRequestTimer03;
	
	QTimer* m_pModuleStatueTimer24;
	QTimer* m_pModuleStateTimer21;
	QTimer* m_pCPTimer22;
	QTimer* m_pNPTimer28;

	// 会被0x4A修改
	ModuleGeoLocation0x5 m_ModuleGeoLocation0x5;
    OModuleStatus0x24 m_oModuleStatus0x24;

	/*
	0x20的配置容器，分为三个部分，分别是0x21、0x22、0x28
	0x21是总模块配置，0x21是模块配置的CP参数，0x28是模块配置的NP参数
	*/
    QVector<OElemStatus0x21> m_vecOElemStatus0x21;
    QVector<OCPStatus0x22> m_vecOCPStatus0x22;
    QVector<CustomisedNP0x28> m_vecCustomisedNP0x28;

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
#endif // _ModuleBase_H_
