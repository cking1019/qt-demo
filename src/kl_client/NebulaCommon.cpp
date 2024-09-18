#include "NebulaCommon.hpp"
#include "NebulaProtol.hpp"

namespace NEBULA {
NebulaCommon::NebulaCommon(qint16 id)
{
    commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));
    m_pUdpSock2Nebula = new QUdpSocket(this);

    nebulaAddress = commCfg->value("Nebula/nebulaAddress").toString();
    nebulaPort    = commCfg->value("Nebula/nebulaPort").toInt();
    clientAddress = commCfg->value(QString("Detector%1/clientAddress").arg(id)).toString();
    clientPort    = commCfg->value(QString("Detector%1/clientPort").arg(id)).toInt();
    // qDebug() << QString("RTM address is %1:%2").arg(clientAddress).arg(clientPort);
    // QTimer* mTimer = new QTimer();
    // connect(mTimer, &QTimer::timeout, this, &NebulaCommon::sendUdpData);
    // mTimer->start(1000);
}

NebulaCommon::~NebulaCommon()
{
    if(m_pUdpSock2Nebula != nullptr) delete m_pUdpSock2Nebula;
    if(commCfg != nullptr)           delete commCfg;
}

void NebulaCommon::initUdp() {
    if(m_pUdpSock2Nebula->bind(QHostAddress::LocalHost, clientPort)) {
        // qDebug() << QString("udp bind port %1 successfully").arg(clientPort);
        m_pUdpSock2Nebula->connectToHost(QHostAddress::LocalHost, nebulaPort);
    } else {
        qDebug() << QString("udp bind port %1 fail").arg(clientPort);
    }
    
    connect(m_pUdpSock2Nebula, &QUdpSocket::readyRead, [&](){
        while (m_pUdpSock2Nebula->hasPendingDatagrams()) {
            QByteArray buf;
            buf.resize(m_pUdpSock2Nebula->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            m_pUdpSock2Nebula->readDatagram(buf.data(), buf.size(), &sender, &senderPort);
            qDebug() << QString("Received UDP datagram from %1:%2, recv data:")
                        .arg(sender.toString())
                        .arg(senderPort)
                        << buf.toHex();
            sendDetectTarget2Ctl(buf);
        }
    });
}

void NebulaCommon::startup() {
    initUdp();
}

void NebulaCommon::sendUdpData() {
    QString msg = "hello world";
    QByteArray buf = msg.toUtf8();
    qint64 len = m_pUdpSock2Nebula->writeDatagram(buf, QHostAddress(nebulaAddress), nebulaPort);
    if(len != -1) qDebug() << "data size:" << len;
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

    OTarget822 oTarget822;
    oTarget822.azim = detectTargetData.tDataAfter.tAzimuth;
    oTarget822.freqMhz = detectTargetData.tDataAfter.tFrequency;
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    oTarget822.timePel1 = reqTimestamp & 0xFFFFFFFF;
    oTarget822.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    emit signalSendDetectTarget2Ctl(oTarget822);
}

}