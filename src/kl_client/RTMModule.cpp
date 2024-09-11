#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    pkgsRTM = {0x561, 0x563, 0x564};

    // m_pSTateMachinethread =      new QThread();
    m_pStateMachineTimer =       new QTimer();
    m_pSettingTimer823 =         new QTimer();
    m_isSendRTMFunction825 = false;

    connect(pTcpSocket,           &QTcpSocket::readyRead, this, &RTMModule::onRecvData);
    connect(m_pStateMachineTimer,     &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(m_pSettingTimer823, &QTimer::timeout, this, &RTMModule::sendRTMSettings823);
    // 线程会阻塞主线程
    // connect(m_pSTateMachinethread,     &QThread::started, this, [=](){
    //     while(true) {
    //         stateMachine();
    //         QThread::msleep(10);
    //         // qDebug() << "m_pSTateMachinethread";
    //     }
    // });
    
    // 启动状态机定时器
    m_pStateMachineTimer->start();
    // m_pSTateMachinethread->start();
    // moveToThread(m_pSTateMachinethread);
    
    // 初始化成员变量
    m_genericHeader.sender   = 5260620;
    m_genericHeader.moduleId = 255;
    m_genericHeader.vMajor   = 2;
    m_genericHeader.vMinor   = 2;
    m_genericHeader.isAsku   = 1;

    m_oSetBanIRIlist0x828.NIRI = 1;
    m_freqs828 = {{5850.5, 120.5}};
}

RTMModule::~RTMModule() {
    if (m_pStateMachineTimer == nullptr)       delete m_pStateMachineTimer;
    // if (m_pSTateMachinethread == nullptr)      delete m_pSTateMachinethread;
    if (m_pSettingTimer823 == nullptr)         delete m_pSettingTimer823;
}

void RTMModule::startup() {
    qDebug() << QString("RTM %1 is running....").arg(m_genericHeader.sender);
}

// 状态机,连接->注册->对时
void RTMModule::stateMachine() {
    // 连接状态,
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
        break;
    }
    // 注册状态
    switch (registerStatus)
    {
    case RegisterStatus::unRegister:
        if (m_pRequestTimer03->isActive())        m_pRequestTimer03->stop();
        if (m_isSendModuleLocation05)             m_isSendModuleLocation05  = false;
        if (m_isSendModuleConfigure20)            m_isSendModuleConfigure20 = false;
        if (m_isSendRTMFunction825)               m_isSendRTMFunction825 = false;
        if (m_isSendForbiddenIRIList828)          m_isSendForbiddenIRIList828 = false;
        if (timeStatus == TimeStatus::timed)      timeStatus = TimeStatus::unTime;
        break;
    case RegisterStatus::registering:
        break;
    case RegisterStatus::registered:
        if (!m_pRequestTimer03->isActive())       m_pRequestTimer03->start(1000);
        if (!m_isSendModuleLocation05)            sendModuleLocation05();
        if (!m_isSendModuleConfigure20)           sendModuleFigure20();
        if (!m_isSendRTMFunction825)              sendRTMFunction825();
        if (!m_isSendForbiddenIRIList828)         sendIRI828();
        break;
    default:
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
        if (m_pSettingTimer823->isActive())           m_pSettingTimer823->stop();
        break;
    }
    case TimeStatus::timing: break;
    case TimeStatus::timed:
    {
        if (!m_pModuleStateTimer21->isActive())        m_pModuleStateTimer21->start(5000);
        if (!m_pCPTimer22->isActive())                 m_pCPTimer22->start(5000);
        if (!m_pModuleStatueTimer24->isActive())       m_pModuleStatueTimer24->start(10000);
        if (!m_pNPTimer28->isActive())                 m_pNPTimer28->start(5000);
        if (!m_pSettingTimer823->isActive())           m_pSettingTimer823->start(10000);
        break;
    }
    default:
        break;
    }
}

void RTMModule::onRecvData() {
    QByteArray buf = pTcpSocket->readAll();
    quint16 pkgID = 0;
    memcpy(&pkgID, buf.data() + 12, 2);
    if (pkgsComm.contains(pkgID)) {
        onReadCommData(pkgID, buf);
    }
    if (pkgsRTM.contains(pkgID)) {
        switch (pkgID) {
        case 0x561: recvRTMSettings561(buf); break;
        case 0x563: recvRequestIRI563(buf);  break;
        case 0x564: recvSettingIRI564(buf);  break;
        default: break;
        }
    }
}

// 0x561,修改频率、频段 561-823
void RTMModule::recvRTMSettings561(const QByteArray& buf) {
    quint32 len1 = HEADER_LEN;
    quint32 len2 = 4;
    quint32 len3 = sizeof(FreqAndDFreq);
    memcpy(&m_oSetting0x823, buf.data() + len1, len2);
    // 4 + 8 * n
    m_freqs823.clear();
    quint16 offset = len1 + len2;
    for(uint32_t i = 0; i < m_oSetting0x823.N; i++) {
        FreqAndDFreq item;
        memcpy(&item, buf.data() + offset, len3);
        m_freqs823.append(item);
        offset += len3;
    }
    /* ------------------------------------------------------------------------ */
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x561:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "N:"          << m_oSetting0x823.N
             << "pkgIdx:"     << pkgIdx;
    for(auto &item : m_freqs823) {
        qDebug() << QString("freq=%1, DFreq=%2").arg(item.freq).arg(item.dfreq);
    }
    sendControlledOrder23(0, pkgIdx);
    sendRTMSettings823();
}

// 823-561
void RTMModule::sendRTMSettings823() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSetting0x823);
    qint32 len3 = m_freqs823.size() * sizeof(FreqAndDFreq);
    // 设置
    m_oSetting0x823.N = 1;
    m_oSetting0x823.curAz = -1;
    m_freqs823 = {{5850.5, 120.5}};
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x823;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSetting0x823, len2);
    int offset = len1 + len2;
    for (auto& item : m_freqs823) {
        memcpy(buf.data() + offset, &item, sizeof(FreqAndDFreq));
        offset += sizeof(FreqAndDFreq);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0x823:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "N:"          << m_oSetting0x823.N;
    }

// 564修改IRI的中心评率 564-828
void RTMModule::recvSettingIRI564(const QByteArray& buf) {
    quint32 len1 = HEADER_LEN;
    quint32 len2 = sizeof(OSetBanIRIlist0x828);
    quint32 len3 = sizeof(FreqAndDFreq);
    memcpy(&m_oSetBanIRIlist0x828, buf.data() + len1, len2);
    m_freqs828.clear();
    qint16 offset = len1 + len2;
    for(uint32_t i = 0; i < m_oSetBanIRIlist0x828.NIRI; i++) {
        FreqAndDFreq freqAndDFreq;
        memcpy(&freqAndDFreq, buf.data() + offset, len3);
        m_freqs828.append(freqAndDFreq);
        offset += len3;
    }
    /* ------------------------------------------------------------------------ */
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x564:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "NIRI:"       << m_oSetBanIRIlist0x828.NIRI
             << "pkgIdx:"     << pkgIdx;
    for(auto &item : m_freqs828) {
        qDebug() << QString("freq=%1, DFreq=%2").arg(item.freq).arg(item.dfreq);
    }
    sendControlledOrder23(0, pkgIdx);
    sendRTMSettings823();
}

// 修改频率、频段 828-564
void RTMModule::sendIRI828() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSetBanIRIlist0x828);
    qint32 len3 = sizeof(FreqAndDFreq);
    m_isSendForbiddenIRIList828 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x828;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSetBanIRIlist0x828, len2);
    int offset = len1 + len2;
    for (auto& item : m_freqs828) {
        memcpy(buf.data() + offset, &item, sizeof(FreqAndDFreq));
        offset += sizeof(FreqAndDFreq);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0x828:" << buf.toHex()
             << "NIRI:"       << m_oSetBanIRIlist0x828.NIRI;
}

// 563->828
void RTMModule::recvRequestIRI563(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qDebug() << "recv 0x563:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "pkgIdx:"     << pkgIdx;
    sendControlledOrder23(0, pkgIdx);
    sendIRI828();
}

void RTMModule::sendTargetMarker822() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTargetMark0x822);
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    // 目标
    OTargetMark0x822 m_oTargetMark0x822;
    m_oTargetMark0x822.timePel1 = reqTimestamp & 0xFFFFFFFF;
    m_oTargetMark0x822.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    m_oTargetMark0x822.idxPoint++;
    m_oTargetMark0x822.idxCeilVOI  = 0xffff;
    m_oTargetMark0x822.idxCeilSPP  = 1;
    m_oTargetMark0x822.idxPoint    = 0;
    m_oTargetMark0x822.typeCeilSPP = 4;
    m_oTargetMark0x822.typeChannel = 0;
    m_oTargetMark0x822.typeSignal  = 0;
    m_oTargetMark0x822.azim        = 180;
    m_oTargetMark0x822.elev        = 10.5;
    m_oTargetMark0x822.range       = 1060;
    m_oTargetMark0x822.freqMhz     = 5840;
    m_oTargetMark0x822.dFreqMhz    = 10;
    m_oTargetMark0x822.Pow_dBm     = 80;
    m_oTargetMark0x822.SNR_dB      = 15;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x822;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oTargetMark0x822, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    if(m_isDebugOut) {
        sendLogMsg25("a target has being detected");
        sendNote2Operator26("a target has being detected");
    }
    qDebug() << "send 0x822:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "checksum:"   << m_genericHeader.checkSum
             << "freqMhz:"    << m_oTargetMark0x822.freqMhz
             << "dFreqMhz:"   << m_oTargetMark0x822.dFreqMhz;
}

void RTMModule::sendRTMFunction825() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OFunc0x825);
    m_isSendRTMFunction825 = true;
    // 功能
    OFunc0x825 m_oFunc0x825;
    m_oFunc0x825.isRotate   = 0;
    m_oFunc0x825.maxTasks   = 5;
    m_oFunc0x825.numDiap    = 1;
    m_oFunc0x825.dAz        = -1;
    m_oFunc0x825.dElev      = -1;
    m_oFunc0x825.minFreqRTR = 300;
    m_oFunc0x825.maxFreqRTR = 6000;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x825;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oFunc0x825, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0x825:"   << buf.toHex()
             << "pkgSize:"      << buf.length()
             << "numDiap:"      << m_oFunc0x825.numDiap
             << "minFreqRTR:"   << m_oFunc0x825.minFreqRTR
             << "maxFreqRTR:"   << m_oFunc0x825.maxFreqRTR;
}

}