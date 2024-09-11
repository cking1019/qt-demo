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
#define COMM_CFG "./conf/common.ini"

namespace NEBULA {
class NebulaCommon : public QObject 
{
Q_OBJECT
private:
  QUdpSocket* m_pUdpSock2Nebula;
public:
    NebulaCommon(/* args */);
    ~NebulaCommon();
    void sendUdpData2Nebula();
public slots:
    void onRecvUdpData();
signals:
    void signalSendDetectTarget2Ctl();
};
}



#endif // _NEBULACOMMON_H_