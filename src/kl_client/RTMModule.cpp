#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    pkgsRTM = {0x561, 0x563, 0x564};
    m_genericHeader = {0x50454C, 0xff, 0x02, 0x02, 0x0000, 0x000000, 0x01, 0x0000, 0x0000};

    pStateMachineTimer =       new QTimer();
    pCurrentTargetTimer822 =   new QTimer();
    pCurrentSettingTimer823 =  new QTimer();
    pCurrentFunctionTimer825 = new QTimer();

    connect(pTcpSocket,         &QTcpSocket::readyRead, this, &RTMModule::onRecvData);
    connect(pStateMachineTimer,       &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(pCurrentTargetTimer822,   &QTimer::timeout, this, &RTMModule::sendBearingMarker822);
    connect(pCurrentSettingTimer823,  &QTimer::timeout, this, &RTMModule::sendRTMSettings823);
    connect(pCurrentFunctionTimer825, &QTimer::timeout, this, &RTMModule::sendRTMFunction825);
    
    // 启动状态机定时器
    pStateMachineTimer->start();

    m_oBearingMark0x822.idxCeilVOI = 0xffff;
    m_oBearingMark0x822.idxCeilSPP = 1;
    m_oBearingMark0x822.idxPoint = 0;
    m_oBearingMark0x822.typeCeilSPP = 4;
    m_oBearingMark0x822.typeChannel = 0;
    m_oBearingMark0x822.typeSignal = 0;
    m_oBearingMark0x822.azim = 180;
    m_oBearingMark0x822.elev = 10.5;
    m_oBearingMark0x822.range = 1060;
    m_oBearingMark0x822.freqMhz = 5840;
    m_oBearingMark0x822.dFreqMhz = 10;
    m_oBearingMark0x822.Pow_dBm = -80;
    m_oBearingMark0x822.SNR_dB = 15;

    m_oSubRezhRTR0x823.N = 1;
    m_oSubRezhRTR0x823.curAz = -1;
    m_freqs823.append({5850.5, 120.5});

    m_oSubPosobilRTR0x825.isRotate = 0;
    m_oSubPosobilRTR0x825.maxTasks = 5;
    m_oSubPosobilRTR0x825.numDiap = 1;
    m_oSubPosobilRTR0x825.dAz = -1;
    m_oSubPosobilRTR0x825.dElev = -1;
    m_oSubPosobilRTR0x825.minFreqRTR = 300;
    m_oSubPosobilRTR0x825.maxFreqRTR = 6000;

    m_oSetBanIRIlist0x828.NIRI = 1;
    m_freqs828.append({5850.5, 120.5});

    m_oSubRadioTime0x829.taskNum = 0;
    m_oSubRadioTime0x829.powType = 0;
    m_oSubRadioTime0x829.freqBegin = 0;
    m_oSubRadioTime0x829.freqStep = 0;
    m_oSubRadioTime0x829.N = 1;
    m_oSubRadioTime0x829.pow1 = 100;
}

RTMModule::~RTMModule() {
    if (pStateMachineTimer == nullptr)       delete pStateMachineTimer;
    if (pCurrentTargetTimer822 == nullptr)   delete pCurrentTargetTimer822;
    if (pCurrentSettingTimer823 == nullptr)  delete pCurrentSettingTimer823;
    if (pCurrentFunctionTimer825 == nullptr) delete pCurrentFunctionTimer825;
}

// 状态机
void RTMModule::stateMachine() {
    // 连接状态,连接->注册->对时
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
    default: {
        if(isDebugOut) {
            sendLogMsg25("the connection state is unknown");
            sendNote2Operator26("the connection state is unknown");
        }
        break;
    }
        
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
        break;
    default:{
        if(isDebugOut) {
            sendLogMsg25("the regsiter state is unknown");
            sendNote2Operator26("the regsiter state is unknown");
        }
        break;
    }
        
    }
    // 对时状态
    switch (timeStatus)
    {
    case TimeStatus::unTime: {
        if (m_isModuleLocation05)                   m_isModuleLocation05  = false;
        if (m_isModuleConfigure20)                  m_isModuleConfigure20 = false;
        if (pModuleStateTimer21->isActive())               pModuleStateTimer21->stop();
        if (pCPTimer22->isActive())               pCPTimer22->stop();
        if (pModuleStatueTimer24->isActive())     pModuleStatueTimer24->stop();
        if (pNPTimer28->isActive())  pNPTimer28->stop();
        if (pCurrentTargetTimer822->isActive())   pCurrentTargetTimer822->stop();
        if (pCurrentSettingTimer823->isActive())  pCurrentSettingTimer823->stop();
        if (pCurrentFunctionTimer825->isActive()) pCurrentFunctionTimer825->stop();
        break;
    }
    case TimeStatus::timing: break;
    case TimeStatus::timed:
    {
        if (!m_isModuleLocation05)                   sendModuleLocation05();
        if (!m_isModuleConfigure20)                  sendModuleFigure20();
        if (!pModuleStateTimer21->isActive())      pModuleStateTimer21->start(30000);
        // if (!pCPTimer22->isActive())            pCPTimer22->start(6000);
        if (!pModuleStatueTimer24->isActive())     pModuleStatueTimer24->start(10000);
        if (!pNPTimer28->isActive())               pNPTimer28->start(12000);
        if (!pCurrentTargetTimer822->isActive())   pCurrentTargetTimer822->start(15000);
        if (!pCurrentSettingTimer823->isActive())  pCurrentSettingTimer823->start(17000);
        if (!pCurrentFunctionTimer825->isActive()) pCurrentFunctionTimer825->start(19000);
        break;
    }
    default:{
        if(isDebugOut) {
            sendLogMsg25("the regsiter state is unknown");
            sendNote2Operator26("the regsiter state is unknown");
        }
        break;
    }
    }
}

void RTMModule::onRecvData() {
    QByteArray buff = pTcpSocket->readAll();
    GenericHeader genericHeader2;
    memcpy(&genericHeader2, buff.data(), HEADER_LEN);
    qint16 pkgID = genericHeader2.packType;
    qDebug() << "recv 0x" << QString::number(pkgID, 16) << ":" << buff.toHex() << "," << buff.size();
    if (pkgsComm.contains(genericHeader2.packType)) {
        onReadCommData(pkgID, buff);
    }
    if (pkgsRTM.contains(genericHeader2.packType)) {
        onReadRTMData(pkgID, buff);
    }
}

void RTMModule::onReadRTMData(qint16 pkgID, QByteArray& buff) {
    switch (pkgID) {
        case 0x561: recvChangingRTMSettings561(buff);     break;
        case 0x563: recvRequestForbiddenIRIList563(buff); break;
        case 0x564: recvSettingForbiddenIRIList564(buff); break;
        default: {
            break;
        }
    }
}

// 修改频率、频段
void RTMModule::recvChangingRTMSettings561(const QByteArray& buff) {
    if(buff.length() > HEADER_LEN) {
        m_oSubRezhRTR0x823.N = buff.at(HEADER_LEN + 0);
    }
    // 4 + 8 * n
    m_freqs823.clear();
    for(qint8 i = HEADER_LEN + 4; i < buff.length(); i += 8) {
        float freq1, dFreq1;
        memcpy(&freq1,  buff.mid(i, 8).constData(), 4);
        memcpy(&dFreq1, buff.mid(i, 8).constData() + 4, 4);
        m_freqs823.append({freq1, dFreq1});
    }
    qDebug() << "N:"  << m_oSubRezhRTR0x823.N;
    for(auto const &item : m_freqs823) {
        qDebug() << item[0] << "," << item[1];
    }
    /* ------------------------------------------------------------------------ */
    sendControlledOrder23(0);
    sendRTMSettings823();
}

void RTMModule::sendRTMSettings823() {
    m_genericHeader.packType = 0x823;
    m_genericHeader.dataSize = 8 + m_freqs823.size() * 8;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubRezhRTR0x823);
    quint8 len3 = m_freqs823.size() * 8;
    char* data = (char*)malloc(HEADER_LEN + m_genericHeader.dataSize);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &m_oSubRezhRTR0x823, len2);
    memcpy(data + len1 + len2, m_freqs823.data(), len3);
    QByteArray byteArray(data, HEADER_LEN + m_genericHeader.dataSize);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x823:" << byteArray.toHex()
             << "N:"          << m_oSubRezhRTR0x823.N
             << "curAz:"      << m_oSubRezhRTR0x823.curAz;
}

//564修改IRI的中心评率
void RTMModule::recvSettingForbiddenIRIList564(const QByteArray& buff) {
    if(buff.length() > HEADER_LEN) {
        m_oSetBanIRIlist0x828.NIRI = buff.at(HEADER_LEN + 0);
    }
    m_freqs828.clear();
    for(qint8 i = HEADER_LEN + 4; i < buff.length(); i += 8) {
        float freq1, dFreq1;
        memcpy(&freq1,  buff.mid(i, 8).constData(), 4);
        memcpy(&dFreq1, buff.mid(i, 8).constData() + 4, 4);
        m_freqs828.append({freq1, dFreq1});
    }
    qDebug() << "NIRI:"       << m_oSetBanIRIlist0x828.NIRI;
    for(auto const &item : m_freqs828) {
        qDebug() << item[0] << "," << item[1];
    }
    /* ------------------------------------------------------------------------ */
    sendForbiddenIRIList828();
    sendControlledOrder23(0);
}

// 修改频率、频段 828-564
void RTMModule::sendForbiddenIRIList828() {
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x828;
    m_genericHeader.dataSize = sizeof(OSetBanIRIlist0x828) + m_freqs828.size() * 8;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSetBanIRIlist0x828);
    quint8 len3 = m_freqs828.size() * 8;
    char* data = (char*)malloc(HEADER_LEN + m_genericHeader.dataSize);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &m_oSetBanIRIlist0x828, len2);
    memcpy(data + len1 + len2, m_freqs828.data(), len3);
    QByteArray byteArray(data, HEADER_LEN + m_genericHeader.dataSize);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x828:" << byteArray.toHex()
             << "NIRI:"       << m_oSetBanIRIlist0x828.NIRI;
}

// 563->828
void RTMModule::recvRequestForbiddenIRIList563(const QByteArray& buff) {
    sendControlledOrder23(0);
    sendForbiddenIRIList828();
}

void RTMModule::sendBearingMarker822() {
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x822;
    m_genericHeader.dataSize = sizeof(OBearingMark0x822);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_oBearingMark0x822.timePel1 = reqTimestamp & 0xFFFFFFFF;
    m_oBearingMark0x822.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    m_oBearingMark0x822.idxPoint++;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OBearingMark0x822);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &m_oBearingMark0x822, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    /* ------------------------------------------------------------------------ */
    if(isDebugOut) {
        sendLogMsg25("a target has being detected");
        sendNote2Operator26("a target has being detected");
    }
    qDebug() << "send 0x822:"   << byteArray.toHex() 
             << "checksum:"<< m_genericHeader.checkSum
             << "azim:"    << m_oBearingMark0x822.azim
             << "elev:"    << m_oBearingMark0x822.elev
             << "range:"   << m_oBearingMark0x822.range
             << "freqMhz:" << m_oBearingMark0x822.freqMhz
             << "dFreqMhz:"<< m_oBearingMark0x822.dFreqMhz
             << "Pow_dBm:" << m_oBearingMark0x822.Pow_dBm
             << "SNR_dB:"  << m_oBearingMark0x822.SNR_dB;
}



void RTMModule::sendRTMFunction825() {
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x825;
    m_genericHeader.dataSize = sizeof(OSubPosobilRTR0x825);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubPosobilRTR0x825);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &m_oSubPosobilRTR0x825, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x825:"   << byteArray.toHex()
             << "numDiap:"      << m_oSubPosobilRTR0x825.numDiap
             << "minFreqRTR:"   << m_oSubPosobilRTR0x825.minFreqRTR
             << "maxFreqRTR:"   << m_oSubPosobilRTR0x825.maxFreqRTR;
}

void RTMModule::sendBearingAndRoute827() {
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x827;
    m_genericHeader.dataSize = sizeof(OBearingMark0x822);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QString jsonStr = "{'Number':12685126,'DateTime':'2018-09-28 13:11:37','StartPoint':{'Latitude':55.12345,'Longitude':38.12345,'Altitude':155.12},'TypeBLA':{'InfoName':'无人机类型','InfoValue':'phantom-3'}";
    /* ------------------------------------------------------------------------ */
    QByteArray byteArray = jsonStr.toUtf8();
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    qDebug() << "send 0x827:"   << byteArray.toHex()
             << "jsonStr:"      << jsonStr;
}



void RTMModule::sendWirelessEnvInfo829() {
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x829;
    m_genericHeader.dataSize = sizeof(OSubRadioTime0x829);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_oSubRadioTime0x829.time1 = reqTimestamp & 0xFFFFFFFF;
    m_oSubRadioTime0x829.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubRadioTime0x829);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &m_oSubRadioTime0x829, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x829:"     << byteArray.toHex()
             << "N:"              << m_oSubRadioTime0x829.N
             << "pow1:"           << m_oSubRadioTime0x829.pow1;
}
}