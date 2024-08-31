#include "NebulaController.hpp"

namespace NEBULA
{
NebulaController::NebulaController(/* args */) {
    this->m_pUdpSock2Nebula = new QUdpSocket();
}

NebulaController::~NebulaController() {
}

void NebulaController::InitUdpSock(QString strIP, int port) {
	this->m_pUdpSock2Nebula = new QUdpSocket();
	if (!this->m_pUdpSock2Nebula->bind(port)) {
		qDebug() << "bind port fail";
  	} else {
		connect(m_pUdpSock2Nebula, &QUdpSocket::readyRead, this, &NebulaController::readNebulaUdpData);
  	}
}

void NebulaController::readNebulaUdpData() {
  QByteArray data;
  QHostAddress addr = QHostAddress(QString("127.0.0.1"));
  quint16 port = 4321;
  data.resize(this->m_pUdpSock2Nebula->pendingDatagramSize());
  this->m_pUdpSock2Nebula->readDatagram(data.data(), data.size(), &addr, &port);
}

}