#ifndef _NEBULACONTROLLER_H_
#define _NEBULACONTROLLER_H_

#include <QObject>
#include <QDebug>
#include <QUdpSocket>
#include <QString>
#include <QSettings>
#include <QHostAddress>
#include <QTextCodec>
#include <QTcpSocket>
#define COMM_CFG "./conf/common.ini"
#define RTM_CFG "./conf/RTMCfg.ini"
#define PRUE_CFG "./conf/PRUECfg.ini"

namespace NEBULA {
class NebulaController : public QObject {
  Q_OBJECT
 private:
  QUdpSocket* m_pUdpSock2Nebula;
  QTcpSocket* m_pTcpSock2Nebula;
 public:
    explicit NebulaController(QObject* parent = nullptr);
    ~NebulaController();
    void init();
    void sendUdpData2Nebula();
public slots:
    void onRecvUdpData();
    void onRecvTcpData();
signals:
    void signalSendDetectTarget2Ctl();
};
} // namespace

#endif // _NEBULACONTROLLER_H_
