#include "PRUEModule.hpp"

namespace NEBULA
{
PRUEModule::PRUEModule() {
    pkgsPRUE = {0x601, 0x201, 0x202};
    m_genericHeader = {0x524542, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};
    
    pStateMachineTimer =       new QTimer();
    pCurrentSettingTimerD21 =  new QTimer();
    pCurrentFunctionTimerD22 = new QTimer();

    connect(pTcpSocket,         &QTcpSocket::readyRead, this, &PRUEModule::onRecvData);
    connect(pStateMachineTimer,       &QTimer::timeout, this, &PRUEModule::stateMachine);
    connect(pCurrentSettingTimerD21,  &QTimer::timeout, this, &PRUEModule::sendPRUESettingsD21);
    connect(pCurrentFunctionTimerD22, &QTimer::timeout, this, &PRUEModule::sendPRUEFunctionD22);
    
    pStateMachineTimer->start();
}

PRUEModule::~PRUEModule() {
    if (pStateMachineTimer == nullptr)       delete pStateMachineTimer;
    if (pCurrentSettingTimerD21 == nullptr)  delete pCurrentSettingTimerD21;
    if (pCurrentFunctionTimerD22 == nullptr) delete pCurrentFunctionTimerD22;
}

// 状态机
void PRUEModule::stateMachine() {
    // 连接状态
    switch (connStatus)
    {
    case ConnStatus::unConnected:
        if (!pReconnectTimer->isActive())                 pReconnectTimer->start(1000);
        if (m_isSendRegister01)                             m_isSendRegister01 = false;
        if (registerStatus == RegisterStatus::registered) registerStatus = RegisterStatus::unRegister;
        break;
    case ConnStatus::connecting:
        break;
    case ConnStatus::connected:
        if (pReconnectTimer->isActive())    pReconnectTimer->stop();
        if (!m_isSendRegister01)              sendRegister01();
        break;
    default:
        break;
    }
    // 注册状态
    switch (registerStatus)
    {
    case RegisterStatus::unRegister:
        if (pRequestTimer03->isActive())          pRequestTimer03->stop();
        if (m_isModuleLocation05)                   m_isModuleLocation05  = false;
        if (m_isModuleConfigure20)                  m_isModuleConfigure20 = false;
        if (pModuleStateTimer21->isActive())               pModuleStateTimer21->stop();
        if (pCPTimer22->isActive())               pCPTimer22->stop();
        if (pModuleStatueTimer24->isActive())     pModuleStatueTimer24->stop();
        if (pCurrentSettingTimerD21->isActive())  pCurrentSettingTimerD21->stop();
        if (pCurrentFunctionTimerD22->isActive()) pCurrentFunctionTimerD22->stop();
        if (timeStatus == TimeStatus::timed)      timeStatus = TimeStatus::unTime;
        break;
    case RegisterStatus::registering:
        break;
    case RegisterStatus::registered:
        if (!pRequestTimer03->isActive())          pRequestTimer03->start(1000);
        if (!m_isModuleLocation05)                   sendModuleLocation05();
        if (!m_isModuleConfigure20)                  sendModuleFigure20();
        if (!pModuleStateTimer21->isActive())               pModuleStateTimer21->start(5000);
        if (!pCPTimer22->isActive())               pCPTimer22->start(5000);
        if (!pModuleStatueTimer24->isActive())     pModuleStatueTimer24->start(1000);
        if (!pCurrentSettingTimerD21->isActive())  pCurrentSettingTimerD21->start(1000);
        if (!pCurrentFunctionTimerD22->isActive()) pCurrentFunctionTimerD22->start(1000);
        break;
    default:
        break;
    }
    // 对时状态
    switch (timeStatus)
    {
    case TimeStatus::unTime: break;
    case TimeStatus::timing: break;
    case TimeStatus::timed:  break;
    default:
        break;
    }
}

void PRUEModule::onRecvData() {
    QByteArray buf = pTcpSocket->readAll();
    quint16 pkgID = 0;
    memcpy(&pkgID, buf.mid(12, 2).constData(), 2);
    if (pkgsComm.contains(pkgID)) {
        onReadCommData(pkgID, buf);
    }
    if (pkgsPRUE.contains(pkgID)) {
        onReadPRUEData(pkgID, buf);
    }
}

void PRUEModule::onReadPRUEData(qint16 pkgID, const QByteArray& buf) {
    switch (pkgID) {
        case 0x201: recvSettingBanSector201(buf);  break;
        case 0x202: recvBanRadiation202(buf);      break;
        case 0x601: recvUpdatePRUESetting601(buf); break;
        default: {
            break;
        }
    }
}

// 0xD01-0x201,发送已安装的辐射禁止扇区或接收设置辐射禁止扇区
void PRUEModule::recvSettingBanSector201(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    quint8 num = 0;
    if(buf.length() > HEADER_LEN) {
        num = buf.at(HEADER_LEN + 8);
    }
    qint16 offset = HEADER_LEN + sizeof(OTrapBanSectorD01);
    for(int i = 0; i < num; i++) {
        OTrapBanSector201 oTrapBanSector201;
        memcpy(&oTrapBanSector201, buf.mid(offset).constData(), sizeof(OTrapBanSector201));
        m_vecOTrapBanSector201.append(oTrapBanSector201);
        offset += sizeof(OTrapBanSector201);
    }
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x561:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "num:"        << m_oTrapBanSectorD01.num
             << "pkgIdx:"     << pkgIdx;
    for(auto& item : m_vecOTrapBanSector201) {
        qDebug() << QString("AzBegin=%1,AzEnd=%2,EpsBegin=%3,EpsEnd=%4,Freq=%5,delFreq=%6")
                    .arg(item.AzBegin)
                    .arg(item.AzEnd)
                    .arg(item.EpsBegin)
                    .arg(item.EpsEnd)
                    .arg(item.Freq)
                    .arg(item.delFreq);
    }
    sendControlledOrder23(0, pkgIdx);
}

void PRUEModule::recvBanRadiation202(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    memcpy(&m_oTrapRadiationBan0x202, buf.data(), sizeof(OTrapRadiationBan0x202));
    qDebug() << "recv 0x202:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "num:"        << m_oTrapBanSectorD01.num
             << "pkgIdx:"     << pkgIdx
             << "isOn:"       << m_oTrapRadiationBan0x202.isOn;
    sendControlledOrder23(0, pkgIdx);
}

void PRUEModule::recvUpdatePRUESetting601(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    qint16 offset = HEADER_LEN;
    memcpy(&m_oRecvTrapFixed0x601, buf.data() + offset, sizeof(ORecvTrapFixed0x601));
    offset += sizeof(ORecvTrapFixed0x601);
    for(uint32_t i = 0; i < m_oRecvTrapFixed0x601.N; i++) {
        FreqAndDFreq freqAndDFreq;
        memcpy(&freqAndDFreq, buf.data() + offset, sizeof(FreqAndDFreq));
        m_vecORecvTrapFixed0x601.append(freqAndDFreq);
        offset += sizeof(FreqAndDFreq);
    }
    if(m_oRecvTrapFixed0x601.taskREB == 3) {
        memcpy(&m_NavigationInfluence, buf.data() + offset, sizeof(NavigationInfluence));
    }
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x601:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "num:"        << m_oTrapBanSectorD01.num
             << "pkgIdx:"     << pkgIdx
             << QString("lat=%1,long=%2").arg(m_NavigationInfluence.latitude).arg(m_NavigationInfluence.longitude);
    sendControlledOrder23(0, pkgIdx);
}

void PRUEModule::sendInstalledBanSectorD01() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapBanSectorD01);
    qint32 len3 = m_vecOTrapBanSector201.size() * sizeof(m_vecOTrapBanSector201);
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0xD01;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTrapBanSectorD01, len2);
    qint16 offset = 0;
    for(const auto& item : m_vecOTrapBanSector201) {
        memcpy(buf.data() + len1 + len2 + offset, buf.mid(HEADER_LEN + 12 + offset), sizeof(OTrapBanSector201));
        offset += sizeof(OTrapBanSector201);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0xD01:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

void PRUEModule::sendPRUESettingsD21() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSendTrapFixed0xD21);
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0xD21;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSendTrapFixed0xD21, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0xD21:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

void PRUEModule::sendPRUEFunctionD22() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapFunc0xD22);
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0xD22;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTrapFunc0xD22, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0xD22:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

}  // namespace NEBULA
