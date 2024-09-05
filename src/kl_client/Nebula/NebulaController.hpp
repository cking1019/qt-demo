#ifndef _NEBULACONTROLLER_H_
#define _NEBULACONTROLLER_H_

#include <QObject>
#include <QDebug>
#include <QUdpSocket>
#include <QString>
#include <QSettings>
#include <QHostAddress>

namespace NEBULA {
class NebulaController : public QObject {
  Q_OBJECT
 private:
  QUdpSocket* m_pUdpSock2Nebula;
 public:
    NebulaController(/* args */);
    ~NebulaController();

    void InitUdpSock(QString strIP, int port);

    void dealUdpData(QByteArray data, int port);

    void readNebulaUdpData();

    void sendUdpData2Nebula(QByteArray data, int port);
};
} // namespace

#endif // _NEBULACONTROLLER_H_
