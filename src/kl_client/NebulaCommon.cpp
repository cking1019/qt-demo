#include "NebulaCommon.hpp"
#include "NebulaProtol.hpp"

namespace NEBULA {
NebulaCommon::NebulaCommon(qint16 id)
{
    commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));
    nebulaAddress = commCfg->value("Nebula/nebulaAddress").toString();
    nebulaPort = commCfg->value("Nebula/nebulaPort").toInt();
    clientAddress = commCfg->value(QString("Detector%1/clientAddress").arg(id)).toString();
    clientPort = commCfg->value(QString("Detector%1/clientPort").arg(id)).toInt();
    qDebug() << QString("Module address is %1:%2").arg(clientAddress).arg(clientPort);
    m_pUdpSock2Nebula = new QUdpSocket(this);
    connect(m_pUdpSock2Nebula, &QUdpSocket::readyRead, this, &NebulaCommon::onRecvUdpData);

    if(!m_pUdpSock2Nebula->bind(clientPort)) qDebug() << QString("UDP bind port %1 fali!!!").arg(clientPort);

    // QTimer* mTimer = new QTimer();
    // connect(mTimer, &QTimer::timeout, this, &NebulaCommon::sendUdpData);
    // mTimer->start(1000);
}

NebulaCommon::~NebulaCommon()
{
    delete m_pUdpSock2Nebula;
}

void NebulaCommon::sendUdpData() {
    QString msg = "hello world";
    QByteArray buf = msg.toUtf8();
    qint64 len = m_pUdpSock2Nebula->writeDatagram(buf, QHostAddress(nebulaAddress), nebulaPort);
    if(len != -1) qDebug() << "data size:" << len;
}

void NebulaCommon::onRecvUdpData() {
    if (m_pUdpSock2Nebula->hasPendingDatagrams()) {
        QByteArray buf;
        buf.resize(m_pUdpSock2Nebula->pendingDatagramSize());
        qint64 len = m_pUdpSock2Nebula->readDatagram(buf.data(), buf.size());
        qDebug() << buf.toHex();
        sendDetectTarget2Ctl(buf);
    }
}

void NebulaCommon::sendDetectTarget2Ctl(const QByteArray& buf) {
    qint16 len1 = sizeof(DetectHead);
    qint16 len2 = sizeof(DetectTargetData);
    qint16 len3 = sizeof(DetectTail);
    DetectHead detectHead;
    DetectTargetData detectTargetData;
    DetectTail detectTail;
    memcpy(&detectHead, buf.data(), len1);
    memcpy(&detectTargetData, buf.data() + len1, len2);
    memcpy(&detectHead, buf.data() + len1 + len2, len3);

    OTargetMark0x822 oTargetMark0x822;
    oTargetMark0x822.azim = detectTargetData.tDataAfter.tAzimuth;
    oTargetMark0x822.freqMhz = detectTargetData.tDataAfter.tFrequency;
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    oTargetMark0x822.timePel1 = reqTimestamp & 0xFFFFFFFF;
    oTargetMark0x822.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    emit signalSendDetectTarget2Ctl(oTargetMark0x822);
}

}