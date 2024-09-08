#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    pkgsRTM = {0x561, 0x563, 0x564};

    pStateMachineTimer =       new QTimer();
    pSTateMachinethread =      new QThread();
    pCurrentTargetTimer822 =   new QTimer();
    pCurrentSettingTimer823 =  new QTimer();
    pCurrentFunctionTimer825 = new QTimer();

    connect(pTcpSocket,         &QTcpSocket::readyRead, this, &RTMModule::onRecvData);
    connect(pStateMachineTimer,       &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(pCurrentTargetTimer822,   &QTimer::timeout, this, &RTMModule::sendBearingMarker822);
    connect(pCurrentSettingTimer823,  &QTimer::timeout, this, &RTMModule::sendRTMSettings823);
    connect(pCurrentFunctionTimer825, &QTimer::timeout, this, &RTMModule::sendRTMFunction825);
    // 线程会阻塞主线程
    connect(pSTateMachinethread,      &QThread::started, this, [=](){
        while(true) {
            stateMachine();
            QThread::msleep(100);
            qDebug() << "pSTateMachinethread";
        }
    });
    
    // 启动状态机定时器
    pStateMachineTimer->start();
    // pSTateMachinethread->start();
    // moveToThread(pSTateMachinethread);
}

RTMModule::~RTMModule() {
    if (pStateMachineTimer == nullptr)       delete pStateMachineTimer;
    if (pSTateMachinethread == nullptr)      delete pSTateMachinethread;
    if (pCurrentTargetTimer822 == nullptr)   delete pCurrentTargetTimer822;
    if (pCurrentSettingTimer823 == nullptr)  delete pCurrentSettingTimer823;
    if (pCurrentFunctionTimer825 == nullptr) delete pCurrentFunctionTimer825;
}

// 状态机,连接->注册->对时
void RTMModule::stateMachine() {
    // 连接状态,
    switch (connStatus)
    {
    case ConnStatus::unConnected:
        if (!pReconnectTimer->isActive())                   pReconnectTimer->start(1000);
        if (m_isSendRegister01)                             m_isSendRegister01 = false;
        if (registerStatus == RegisterStatus::registered)   registerStatus = RegisterStatus::unRegister;
        break;
    case ConnStatus::connecting:
        break;
    case ConnStatus::connected:
        if (pReconnectTimer->isActive())    pReconnectTimer->stop();
        if (!m_isSendRegister01)            sendRegister01();
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
        if (pRequestTimer03->isActive())          pRequestTimer03->stop();
        if (timeStatus == TimeStatus::timed)      timeStatus = TimeStatus::unTime;
        break;
    case RegisterStatus::registering:
        break;
    case RegisterStatus::registered:
        if (!pRequestTimer03->isActive())         pRequestTimer03->start(1000);
        if(m_isSendForbiddenIRIList828)           sendForbiddenIRIList828();
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
            if (m_isModuleLocation05)                   m_isModuleLocation05  = false;
            if (m_isModuleConfigure20)                  m_isModuleConfigure20 = false;
            if (m_isSendForbiddenIRIList828)            m_isSendForbiddenIRIList828 = false;
            if (pModuleStateTimer21->isActive())        pModuleStateTimer21->stop();
            if (pCPTimer22->isActive())                 pCPTimer22->stop();
            if (pModuleStatueTimer24->isActive())       pModuleStatueTimer24->stop();
            if (pNPTimer28->isActive())                 pNPTimer28->stop();
            if (pCurrentTargetTimer822->isActive())     pCurrentTargetTimer822->stop();
            if (pCurrentSettingTimer823->isActive())    pCurrentSettingTimer823->stop();
            if (pCurrentFunctionTimer825->isActive())   pCurrentFunctionTimer825->stop();
            break;
        }
        case TimeStatus::timing: break;
        case TimeStatus::timed:
        {
            if (!m_isModuleLocation05)                   sendModuleLocation05();
            if (!m_isModuleConfigure20)                  sendModuleFigure20();
            if (!pModuleStateTimer21->isActive())        pModuleStateTimer21->start(30000);
            // if (!pCPTimer22->isActive())              pCPTimer22->start(6000);
            if (!pModuleStatueTimer24->isActive())       pModuleStatueTimer24->start(10000);
            if (!pNPTimer28->isActive())                 pNPTimer28->start(12000);
            if (!pCurrentTargetTimer822->isActive())     pCurrentTargetTimer822->start(15000);
            if (!pCurrentSettingTimer823->isActive())    pCurrentSettingTimer823->start(17000);
            if (!pCurrentFunctionTimer825->isActive())   pCurrentFunctionTimer825->start(19000);
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
    for(auto const &item : m_freqs823) {
        qDebug() << "freq:" << item[0] << ",DFreq" << item[1];
    }
    sendControlledOrder23(0, pkgIdx);
    sendRTMSettings823();
}

// 823-561
void RTMModule::sendRTMSettings823() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubRezhRTR0x823);
    qint32 len3 = m_freqs823.size() * sizeof(float) * 2;
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
    int offset = 0;
    for (const QVector<float>& innerVec : m_freqs823) {
        memcpy(buf.data() + len1 + len2 + offset, innerVec.data(), innerVec.size() * sizeof(float));
        offset += innerVec.size() * sizeof(float);
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
    memcpy(&pkgIdx, buf.mid(6, 2).constData(), 2);
    if(buf.length() > HEADER_LEN) {
        m_oSetBanIRIlist0x828.NIRI = buf.at(HEADER_LEN + 0);
    }
    m_freqs828.clear();
    for(qint8 i = HEADER_LEN + 4; i < buf.length(); i += 8) {
        float freq1, dFreq1;
        memcpy(&freq1,  buf.mid(i, 8).constData(), 4);
        memcpy(&dFreq1, buf.mid(i, 8).constData() + 4, 4);
        m_freqs828.append({freq1, dFreq1});
    }
    /* ------------------------------------------------------------------------ */
    // 494f5601 0202ae00 04000000 64050e02 00e4e652
    // 对方发送数据有问题，需要核实
    qDebug() << "recv 0x564:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "NIRI:"       << m_oSetBanIRIlist0x828.NIRI
             << "pkgIdx:"     << pkgIdx;
    for(auto const &item : m_freqs828) {
        qDebug() << item[0] << "," << item[1];
    }
    sendControlledOrder23(0, pkgIdx);
    sendForbiddenIRIList828();
}

// 修改频率、频段 828-564
void RTMModule::sendForbiddenIRIList828() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSetBanIRIlist0x828);
    qint32 len3 = m_freqs828.size() * sizeof(float) * 2;
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
    int offset = 0;
    for (const QVector<float>& innerVec : m_freqs828) {
        memcpy(buf.data() + len1 + len2 + offset, innerVec.data(), innerVec.size() * sizeof(float));
        offset += innerVec.size() * sizeof(float);
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
    qDebug() << "recv 0x563:"<< buf.toHex()
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

void RTMModule::sendBearingAndRoute827() {
    QString jsonStr = "{'Number':12685126,'DateTime':'2018-09-28 13:11:37','StartPoint':{'Latitude':55.12345,'Longitude':38.12345,'Altitude':155.12},'TypeBLA':{'InfoName':'无人机类型','InfoValue':'phantom-3'}";
    quint8 len1 = HEADER_LEN;
    quint16 len2 = jsonStr.length();
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x827;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, jsonStr.toUtf8().constData(), len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0x827:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "jsonStr:"    << jsonStr;
}

void RTMModule::sendWirelessEnvInfo829() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubRadioTime0x829);
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_oSubRadioTime0x829.time1 = reqTimestamp & 0xFFFFFFFF;
    m_oSubRadioTime0x829.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x829;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oSubRadioTime0x829, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    qDebug() << "send 0x829:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "N:"          << m_oSubRadioTime0x829.N
             << "pow1:"       << m_oSubRadioTime0x829.pow1;
}
}