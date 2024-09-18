#ifndef _NEBULACOMMON_H_
#define _NEBULACOMMON_H_

#include <QObject>
#include <QDebug>
#include <QUdpSocket>
#include <QString>
#include <QSettings>
#include <QHostAddress>
#include <QTextCodec>
#include <QTcpSocket>
#include <QTimer>
#include "ModuleHeader.hpp"
#define COMM_CFG "./conf/common.ini"

namespace NEBULA {
class NebulaCommon : public QObject 
{
Q_OBJECT
public:
  QUdpSocket* m_pUdpSock2Nebula;
  QSettings* commCfg;

  QString nebulaAddress;
  qint16 nebulaPort;
  QString clientAddress;
  qint16 clientPort;

  QThread* m_recvUdpData;
public:
    NebulaCommon(qint16 id);
    ~NebulaCommon();
    void sendUdpData();
    void sendDetectTarget2Ctl(const QByteArray& buf);
    virtual void initUdp();
    void startup();
public slots:
    
signals:
    void signalSendDetectTarget2Ctl(OTarget822& oTarget822);
};
}



#endif // _NEBULACOMMON_H_