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
public slots:
    void onRecvUdpData();
    
signals:
    void signalSendDetectTarget2Ctl(OTargetMark0x822& oTargetMark0x822);
};
}



#endif // _NEBULACOMMON_H_