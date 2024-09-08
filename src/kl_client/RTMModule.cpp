#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    pkgsRTM = {0x561, 0x563, 0x564};

    m_pStateMachineTimer =       new QTimer();
    // m_pSTateMachinethread =      new QThread();
    m_pCurrentSettingTimer823 =  new QTimer();

    connect(pTcpSocket,         &QTcpSocket::readyRead, this, &RTMModule::onRecvData);
    connect(m_pStateMachineTimer,       &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(m_pCurrentSettingTimer823,  &QTimer::timeout, this, &RTMModule::sendRTMSettings823);
    // 线程会阻塞主线程
    // connect(m_pSTateMachinethread,      &QThread::started, this, [=](){
    //     while(true) {
    //         stateMachine();
    //         QThread::msleep(100);
    //         qDebug() << "m_pSTateMachinethread";
    //     }
    // });
    
    // 启动状态机定时器
    m_pStateMachineTimer->start();
    // m_pSTateMachinethread->start();
    // moveToThread(m_pSTateMachinethread);
}

RTMModule::~RTMModule() {
    if (m_pStateMachineTimer == nullptr)       delete m_pStateMachineTimer;
    // if (m_pSTateMachinethread == nullptr)      delete m_pSTateMachinethread;
    if (m_pCurrentSettingTimer823 == nullptr)  delete m_pCurrentSettingTimer823;
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
        if (!m_isSendForbiddenIRIList828)         sendForbiddenIRIList828();
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
        if (m_pCurrentSettingTimer823->isActive())    m_pCurrentSettingTimer823->stop();
        break;
    }
    case TimeStatus::timing: break;
    case TimeStatus::timed:
    {
        if (!m_pModuleStateTimer21->isActive())        m_pModuleStateTimer21->start(30000);
        if (!m_pCPTimer22->isActive())                 m_pCPTimer22->start(6000);
        if (!m_pModuleStatueTimer24->isActive())       m_pModuleStatueTimer24->start(10000);
        if (!m_pNPTimer28->isActive())                 m_pNPTimer28->start(12000);
        if (!m_pCurrentSettingTimer823->isActive())    m_pCurrentSettingTimer823->start(17000);
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

void RTMModule::onRecvData() {
    QByteArray buf = pTcpSocket->readAll();
    quint16 pkgID = 0;
    memcpy(&pkgID, buf.mid(12, 2).constData(), 2);
    if (pkgsComm.contains(pkgID)) {
        onReadCommData(pkgID, buf);
    }
    if (pkgsRTM.contains(pkgID)) {
        onReadRTMData(pkgID, buf);
    }
}

void RTMModule::onReadRTMData(qint16 pkgID, QByteArray& buf) {
    switch (pkgID) {
        case 0x561: recvChangingRTMSettings561(buf);     break;
        case 0x563: recvRequestForbiddenIRIList563(buf); break;
        case 0x564: recvSettingForbiddenIRIList564(buf); break;
        default: {
            break;
        }
    }
}

// 修改频率、频段 561-823
void RTMModule::recvChangingRTMSettings561(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    if(buf.length() > HEADER_LEN) {
        m_oSubRezhRTR0x823.N = buf.at(HEADER_LEN + 0);
    }
    // 4 + 8 * n
    m_freqs823.clear();
    for(qint8 i = HEADER_LEN + 4; i < buf.length(); i += 8) {
        float freq1, dFreq1;
        memcpy(&freq1,  buf.mid(i, 8).constData(), 4);
        memcpy(&dFreq1, buf.mid(i, 8).constData() + 4, 4);
        m_freqs823.append({freq1, dFreq1});
    }
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x561:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "N:"          << m_oSubRezhRTR0x823.N
             << "pkgIdx:"     << pkgIdx;
    for(auto &item : m_freqs823) {
        qDebug() << "freq:" << item.freq << ",DFreq" << item.dfreq;
    }
    sendControlledOrder23(0, pkgIdx);
    sendRTMSettings823();
}

// 823-561
void RTMModule::sendRTMSettings823() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubRezhRTR0x823);
    qint32 len3 = m_freqs823.size() * sizeof(FreqAndDFreq);
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x823;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSubRezhRTR0x823, len2);
    // 容器赋值错误，造成赋值失败
    int offset = len1 + len2;
    for (auto& item : m_freqs823) {
        memcpy(buf.data() + offset, &item, sizeof(FreqAndDFreq));
        offset += sizeof(FreqAndDFreq);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    // 4c455001 02027900 10000080 23081a02 01000005 000080bf 500e4a16 6a010000
    // 4c4550ff 02020100 10000080 2308a002 01003200 000080bf 40d0e000 00000000
    // 4c4550ff 02020100 18000080 2308a802 02003200 000080bf b0d2e600 00000000 60cee600 00000000
    // 4c4550ff 02020100 18000080 2308a802 02003200 000080bf 0000f142 00800243 0000f142 00800243
    qDebug() << "send 0x823:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "N:"          << m_oSubRezhRTR0x823.N;
    }

//564修改IRI的中心评率 564-828
void RTMModule::recvSettingForbiddenIRIList564(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6).constData(), 2);
    qint16 offset = HEADER_LEN;
    m_oSetBanIRIlist0x828.NIRI = buf.at(HEADER_LEN);
    m_freqs828.clear();
    for(qint8 i = HEADER_LEN + 4; i < buf.length(); i += 8) {
        FreqAndDFreq freqAndDFreq;
        memcpy(&freqAndDFreq.freq,  buf.mid(i, 8).constData(), 4);
        memcpy(&freqAndDFreq.dfreq, buf.mid(i, 8).constData() + 4, 4);
        m_freqs828.append(freqAndDFreq);
    }
    /* ------------------------------------------------------------------------ */
    // 494f5601 0202ae00 04000000 64050e02 00e4e652
    // 对方发送数据有问题，需要核实
    qDebug() << "recv 0x564:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "NIRI:"       << m_oSetBanIRIlist0x828.NIRI
             << "pkgIdx:"     << pkgIdx;
    for(auto &item : m_freqs828) {
        qDebug() << item.freq << "," << item.dfreq;
    }
    sendControlledOrder23(0, pkgIdx);
    sendRTMSettings823();
}

// 修改频率、频段 828-564
void RTMModule::sendForbiddenIRIList828() {
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
void RTMModule::recvRequestForbiddenIRIList563(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    qDebug() << "recv 0x563:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "pkgIdx:"     << pkgIdx;
    sendControlledOrder23(0, pkgIdx);
    sendForbiddenIRIList828();
}

void RTMModule::sendBearingMarker822() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OBearingMark0x822);
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_oBearingMark0x822.timePel1 = reqTimestamp & 0xFFFFFFFF;
    m_oBearingMark0x822.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    m_oBearingMark0x822.idxPoint++;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x822;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oBearingMark0x822, len2);
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
             << "freqMhz:"    << m_oBearingMark0x822.freqMhz
             << "dFreqMhz:"   << m_oBearingMark0x822.dFreqMhz;
}

void RTMModule::sendRTMFunction825() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubPosobilRTR0x825);
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x825;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSubPosobilRTR0x825, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0x825:"   << buf.toHex()
             << "pkgSize:"      << buf.length()
             << "numDiap:"      << m_oSubPosobilRTR0x825.numDiap
             << "minFreqRTR:"   << m_oSubPosobilRTR0x825.minFreqRTR
             << "maxFreqRTR:"   << m_oSubPosobilRTR0x825.maxFreqRTR;
}

}