#include "PRUEModule.hpp"

namespace NEBULA
{
PRUEModule::PRUEModule() {
    pkgsPRUE = {0x601, 0x201, 0x202};
    m_genericHeader = {0x524542, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};
    
    m_pStateMachineTimer =       new QTimer();
    m_pCurrentSettingTimerD21 =  new QTimer();
    m_isSendInstalledBanSectorD01 = false;
    m_isSendPRUEFunctionD22 = false;

    connect(pTcpSocket,         &QTcpSocket::readyRead, this, &PRUEModule::onRecvData);
    connect(m_pStateMachineTimer,       &QTimer::timeout, this, &PRUEModule::stateMachine);
    connect(m_pCurrentSettingTimerD21,  &QTimer::timeout, this, &PRUEModule::sendPRUESettingsD21);
    
    m_pStateMachineTimer->start();
}

PRUEModule::~PRUEModule() {
    if (m_pStateMachineTimer == nullptr)       delete m_pStateMachineTimer;
    if (m_pCurrentSettingTimerD21 == nullptr)  delete m_pCurrentSettingTimerD21;
}

// 状态机
void PRUEModule::stateMachine() {
    // 连接状态
    switch (connStatus)
    {
    case ConnStatus::unConnected:
        if (!m_pReconnectTimer->isActive())                 m_pReconnectTimer->start(1000);
        if (m_isSendRegister01)                             m_isSendRegister01 = false;
        if (registerStatus == RegisterStatus::registered)   registerStatus = RegisterStatus::unRegister;
        break;
    case ConnStatus::connecting:
        break;
    case ConnStatus::connected:
        if (m_pReconnectTimer->isActive())    m_pReconnectTimer->stop();
        if (!m_isSendRegister01)              sendRegister01();
        break;
    default:
        if(m_isDebugOut) {
            sendLogMsg25("the connection state is unknown");
            sendNote2Operator26("the connection state is unknown");
        }
        break;
    }
    // 注册状态
    switch (registerStatus)
    {
    case RegisterStatus::unRegister:
        if (m_pRequestTimer03->isActive())            m_pRequestTimer03->stop();
        if (m_isSendModuleLocation05)                 m_isSendModuleLocation05  = false;
        if (m_isSendModuleConfigure20)                m_isSendModuleConfigure20 = false;
        if (m_isSendInstalledBanSectorD01)            m_isSendInstalledBanSectorD01 = false;
        if (m_isSendPRUEFunctionD22)                  m_isSendPRUEFunctionD22 = false;
        if (timeStatus == TimeStatus::timed)          timeStatus = TimeStatus::unTime;
        break;
    case RegisterStatus::registering:
        break;
    case RegisterStatus::registered:
        if (!m_pRequestTimer03->isActive())          m_pRequestTimer03->start(1000);
        if (!m_isSendModuleLocation05)               sendModuleLocation05();
        if (!m_isSendModuleConfigure20)              sendModuleFigure20();
        if (!m_isSendInstalledBanSectorD01)          sendInstalledBanSectorD01();
        if (!m_isSendPRUEFunctionD22)                sendPRUEFunctionD22();
        break;
    default:
        if(m_isDebugOut) {
            sendLogMsg25("the regsiter state is unknown");
            sendNote2Operator26("the regsiter state is unknown");
        }
        break;
    }
    // 对时状态
    switch (timeStatus)
    {
    case TimeStatus::unTime: {
        if (m_pModuleStateTimer21->isActive())        m_pModuleStateTimer21->stop();
        if (m_pCPTimer22->isActive())                 m_pCPTimer22->stop();
        if (m_pModuleStatueTimer24->isActive())       m_pModuleStatueTimer24->stop();
        if (m_pNPTimer28->isActive())                 m_pNPTimer28->stop();
        if (m_pCurrentSettingTimerD21->isActive())    m_pCurrentSettingTimerD21->stop();
        break;
    }
    case TimeStatus::timing: break;
    case TimeStatus::timed:
    {
        if (!m_pModuleStateTimer21->isActive())        m_pModuleStateTimer21->start(60000);
        if (!m_pCPTimer22->isActive())                 m_pCPTimer22->start(60000);
        if (!m_pModuleStatueTimer24->isActive())       m_pModuleStatueTimer24->start(60000);
        if (!m_pNPTimer28->isActive())                 m_pNPTimer28->start(60000);
        if (!m_pCurrentSettingTimerD21->isActive())    m_pCurrentSettingTimerD21->start(60000);
        break;
    }
    default:
        if(m_isDebugOut) {
            sendLogMsg25("the regsiter state is unknown");
            sendNote2Operator26("the regsiter state is unknown");
        }
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
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(OTrapBanSectorD01);
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    quint8 num = 0;
    if(buf.length() > HEADER_LEN) {
        num = buf.at(HEADER_LEN + 8);
    }
    qint16 offset = len1 + len2;
    for(int i = 0; i < num; i++) {
        OTrapBanSector201 oTrapBanSector201;
        memcpy(&oTrapBanSector201, buf.data() + offset, sizeof(OTrapBanSector201));
        m_vecOTrapBanSector201.append(oTrapBanSector201);
        offset += sizeof(OTrapBanSector201);
    }
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x201:" << buf.toHex()
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
    sendInstalledBanSectorD01();
}

// 0x601,接收更改设置
void PRUEModule::recvUpdatePRUESetting601(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(OTrapRadiationBan0x202);
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    memcpy(&m_oRecvTrapFixed0x601, buf.data() + len1, len2);
    qint16 offset = len1 + len2;
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
    sendPRUESettingsD21();
}

// 0xD21,发送设置
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

// 0xD01-0x201,发送辐射禁止扇区,其中num值是禁止扇区个数，其参数是OTrapBanSector201结构体
void PRUEModule::sendInstalledBanSectorD01() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapBanSectorD01);
    qint32 len3 = m_vecOTrapBanSector201.size() * sizeof(m_vecOTrapBanSector201);
    m_isSendInstalledBanSectorD01 = true;
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
    qint16 offset = len1 + len2;
    for(auto& item : m_vecOTrapBanSector201) {
        memcpy(buf.data() + offset, &item, sizeof(OTrapBanSector201));
        offset += sizeof(OTrapBanSector201);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0xD01:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

// 0xD22,发送功能，其中numDiap2值是频率、频段、最大暴露范围结构体的数量
void PRUEModule::sendPRUEFunctionD22() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapFunc0xD22);
    quint32 len3 = m_vecOTrapFunc0xD22_2.size() * sizeof(OTrapFunc0xD22_2);
    m_isSendPRUEFunctionD22 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0xD22;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTrapFunc0xD22, len2);
    quint16 offset = len1 + len2;
    for(auto& item : m_vecOTrapFunc0xD22_2) {
        memcpy(buf.data() + offset, &item, sizeof(OTrapFunc0xD22_2));
        offset += sizeof(OTrapFunc0xD22_2);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0xD22:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

// 0x202,接收设置辐射禁止
void PRUEModule::recvBanRadiation202(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(OTrapRadiationBan0x202);
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    memcpy(&m_oTrapRadiationBan0x202, buf.data() + len1, len2);
    qDebug() << "recv 0x202:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "pkgIdx:"     << pkgIdx
             << "isOn:"       << m_oTrapRadiationBan0x202.isOn;
    sendControlledOrder23(0, pkgIdx);
}

}  // namespace NEBULA
