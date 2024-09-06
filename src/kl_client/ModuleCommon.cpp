#include "ModuleCommon.hpp"

namespace NEBULA
{

// 计算包头校验和
quint16 calcChcekSum(const char* sMess,int nCnt) {
    quint16 nSum = 0;
    quint8 *pb = (quint8*)sMess;
    for (int i = 0; i < nCnt; i++) nSum += *pb++;
    return nSum;
}

QString readJson(QString DevConfig20) {
    QFile file(DevConfig20);
    QString jsonContent;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        jsonContent = in.readAll();
        file.close();
    }
    return jsonContent;
}

CommonModule::CommonModule(QObject *parent):QObject(parent) {
    pkgsComm = {0x2, 0x4, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B};

    pTcpSocket =              new QTcpSocket(this);
    pReconnectTimer =         new QTimer();

    pRequestTimer03 =         new QTimer();
    pModuleStateTimer21 =              new QTimer();
    pCPTimer22 =              new QTimer();
    pModuleStatueTimer24 =    new QTimer();
    pNPTimer28 = new QTimer();

    connStatus =     ConnStatus::unConnected;
    registerStatus = RegisterStatus::unRegister;
    timeStatus =     TimeStatus::unTime;
    m_isSendRegister01    = false;
    m_isModuleLocation05  = false;
    m_isModuleConfigure20 = false;
    m_isSendForbiddenIRIList828 = false;
    
    m_iStampResult = 0;
    m_iN = 1;

    isDebugOut = 0;

    m_ModuleGeoLocation0x5.typeData = 1;
    m_ModuleGeoLocation0x5.isValid = 1;
    m_ModuleGeoLocation0x5.xLat = 10;
    m_ModuleGeoLocation0x5.yLong = 20;
    m_ModuleGeoLocation0x5.zHeight = 30;
}

CommonModule::~CommonModule() {
    if (pTcpSocket != nullptr)              delete pTcpSocket;
    if (pReconnectTimer != nullptr)         delete pReconnectTimer;

    if (pRequestTimer03 != nullptr)         delete pRequestTimer03;
    if (pModuleStateTimer21 != nullptr)              delete pModuleStateTimer21;
    if (pCPTimer22 != nullptr)              delete pCPTimer22;
    if (pModuleStatueTimer24 != nullptr)    delete pModuleStatueTimer24;
    if (pNPTimer28 != nullptr) delete pNPTimer28;
}

// 初始化成员变量
void CommonModule::startup() {
    connect(pRequestTimer03,      &QTimer::timeout, this, &CommonModule::sendRequestTime03);
    connect(pModuleStateTimer21,           &QTimer::timeout, this, &CommonModule::sendModuleStatus21);
    connect(pCPTimer22,           &QTimer::timeout, this, &CommonModule::sendModuleCPStatus22);
    connect(pModuleStatueTimer24, &QTimer::timeout, this, &CommonModule::sendModuleStatus24);
    connect(pNPTimer28, &QTimer::timeout, this, &CommonModule::sendModuleCPStatus28);

    connect(pReconnectTimer, &QTimer::timeout, [=](){
        while (!pTcpSocket->waitForConnected(1000))
        {
            // qDebug() << "Attempting to connect...";
            connStatus = ConnStatus::connecting; // 只需关注连接状态，因为连接会影响注册，注册会影响对时
            pTcpSocket->connectToHost(commCfg.serverAddress, commCfg.serverPort);
        }
    });
    connect(pTcpSocket, &QTcpSocket::connected, [=](){
        qDebug() << "Connected to host!";
        connStatus = ConnStatus::connected;
    });
    // abort -> close -> disconnectFromHost
    connect(pTcpSocket, &QTcpSocket::disconnected, [=]() {
        qDebug() << "Disconnected from server!";
        connStatus = ConnStatus::unConnected;
    });
}

// 接收数据
void CommonModule::onReadCommData(qint16 pkgID, const QByteArray& buff) {
    switch (pkgID) {
        case 0x02: recvRegister02(buff); break;
        case 0x04: recvRequestTime04(buff); break;
        case 0x40: recvStart40(buff); break;
        case 0x41: recvStop41(buff); break;
        case 0x42: recvRestart42(buff); break;
        case 0x43: recvReset43(buff); break;
        case 0x44: recvUpdate44(buff); break;
        case 0x45: recvNote4Operator45(buff); break;
        case 0x46: recvRequestModuleFigure46(buff); break;
        case 0x47: recvSettingLang47(buff); break;
        case 0x48: recvRadioAndSatellite48(buff); break;
        case 0x49: recvSettingTime49(buff); break;
        case 0x4A: recvModuleLocation4A(buff); break;
        case 0x4B: recvCustomizedParam4B(buff); break;
        default: {
            qDebug() << "this is unknown pkg 0x" << m_genericHeader.packType;
            break;
        }
    }
}

void CommonModule::sendRegister01() {
    m_genericHeader.packType = 0x1;
    m_genericHeader.dataSize = sizeof(ModuleRegister0x1);
    m_genericHeader.packIdx = 0;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    ModuleRegister0x1 ModuleRegister0x1;
    ModuleRegister0x1.idManuf = 0x1;
    ModuleRegister0x1.serialNum = 0x0;
    ModuleRegister0x1.versHardMaj = m_genericHeader.vMajor;
    ModuleRegister0x1.versHardMin = m_genericHeader.vMinor;
    ModuleRegister0x1.versProgMaj = 0x0;
    ModuleRegister0x1.isInfo = 0x1;
    ModuleRegister0x1.versProgMin = 0x0;
    ModuleRegister0x1.isAsku = m_genericHeader.isAsku;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ModuleRegister0x1);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &ModuleRegister0x1, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    m_isSendRegister01 = true;
    qDebug() << "send 0x01:" << byteArray.toHex();
}

void CommonModule::recvRegister02(const QByteArray& buff) {
    ServerRegister0x2 ServerRegister0x2;
    memcpy(&ServerRegister0x2, buff.data() + HEADER_LEN, sizeof(ServerRegister0x2));
    m_genericHeader.moduleId = ServerRegister0x2.idxModule;
    qDebug() << "the module id is " << QString::number(ServerRegister0x2.idxModule, 16);
    qDebug() << "the connection status is " << QString::number(ServerRegister0x2.errorConnect, 16);
    switch (ServerRegister0x2.errorConnect) {
        case 0x1:  sendLogMsg25("execeed the limited number of same type device"); break;
        case 0x2:  sendLogMsg25("try to reconnect same device"); break;
        case 0x4:  sendLogMsg25("don't support the device type"); break;
        case 0x8:  sendLogMsg25("don't support the prototal version"); break;
        case 0x10: sendLogMsg25("module id is not be supported"); break;
        case 0x20:
        case 0x40: qDebug() << "unknown error"; break;
        case 0x0: {
            m_genericHeader.moduleId = ServerRegister0x2.idxModule;
            registerStatus = RegisterStatus::registered;
            break;
        }
        default: {
            qDebug("fail to register");
        }
    }
}

void CommonModule::sendRequestTime03() {
    m_genericHeader.packType = 0x3;
    m_genericHeader.dataSize = sizeof(ModuleTimeControl0x3);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_moduleTimeControl0x3.time1 = reqTimestamp & 0xFFFFFFFF;
    m_moduleTimeControl0x3.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ModuleTimeControl0x3);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &m_moduleTimeControl0x3, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x03:" << byteArray.toHex();
}

// 对时
void CommonModule::reqAndResTime(quint64 timeStampReq, quint64 timeStampAns) {
    quint64 timeStampRcv = QDateTime::currentMSecsSinceEpoch();
    qint64 delTime1 = timeStampAns - timeStampReq; // 请求时间与响应时间的时间差
    qint64 delTime2 = timeStampRcv - timeStampAns; // 当前时间与响应时间的时间差

    qint64 timeOut = (0.5 * (delTime1 - delTime2) + m_iStampResult) / m_iN;
    m_iN++;
    m_iStampResult = timeOut + m_iStampResult;
    // 授时
    if (timeOut > 200 || timeOut < -200) {
        quint64 timeCurrentStamp = QDateTime::currentMSecsSinceEpoch() + timeOut;
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timeCurrentStamp);
        // 设置本地系统时间
        QString strDate = "date " + dateTime.toString("yyyy-MM-dd");
        QString strTime = "time " + dateTime.toString("hh:mm:ss");
        if(isDebugOut) {
            qDebug() << "UTC Time: " + dateTime.toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << strDate << strTime;
        }
        system(strDate.toStdString().c_str());
        system(strTime.toStdString().c_str());

        m_iN = 1;
        m_iStampResult = 0;
    }
}

void CommonModule::recvRequestTime04(const QByteArray& buff) {
    ServerTimeControl0x4 serverTimeControl0x4;
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ServerTimeControl0x4);
    memcpy(&serverTimeControl0x4, buff.data() + len1, len2);
    
    quint64 timeStampReq = serverTimeControl0x4.time1;
    quint64 timeStampAns = serverTimeControl0x4.time2;
    reqAndResTime(timeStampReq, timeStampAns);

    timeStatus = TimeStatus::timed;
}

void CommonModule::sendModuleLocation05() {
    m_genericHeader.packType = 0x5;
    m_genericHeader.dataSize = sizeof(ModuleGeoLocation0x5);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ModuleGeoLocation0x5);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &m_ModuleGeoLocation0x5, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    m_isModuleLocation05 = true;
    qDebug() << "send 0x05:" << byteArray.toHex();
}

void CommonModule::recvRequestModuleFigure46(const QByteArray& buff) {
    sendModuleFigure20();
}

void CommonModule::sendModuleFigure20() {
    m_genericHeader.packType = 0x20;
    m_genericHeader.dataSize = commCfg.moduleCfg20.length();
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint16 len2 = commCfg.moduleCfg20.length();
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, commCfg.moduleCfg20.toStdString().data(), len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    m_isModuleConfigure20 = true;
    qDebug() << "send 0x20:" << byteArray.toHex() << commCfg.moduleCfg20;
    qDebug() << commCfg.moduleCfg20.toStdString().data();
}

void CommonModule::sendModuleStatus21() {
    m_genericHeader.packType = 0x21;
    m_genericHeader.dataSize = sizeof(ONPStatus0x21);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    ONPStatus0x21 oNPStatus;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oNPStatus.time1 = timestamp & 0xFFFFFFFF;
    oNPStatus.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oNPStatus.IDElem = 0;
    oNPStatus.status = 1;
    oNPStatus.workF1 = 1;
    oNPStatus.local = 0;
    oNPStatus.isImit = 0;
    oNPStatus.reserve = 0;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ONPStatus0x21);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &oNPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x21:" << byteArray.toHex();
}

void CommonModule::sendModuleCPStatus22() {
    m_genericHeader.packType = 0x22;
    m_genericHeader.dataSize = sizeof(OCPStatus0x22);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    OCPStatus0x22 oCPStatus;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oCPStatus.time1 = timestamp & 0xFFFFFFFF;
    oCPStatus.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oCPStatus.IDParam = 0; // 包索引
    oCPStatus.status = 4;  //  参数不受控
    oCPStatus.size = 0;
    oCPStatus.isNewStatus = 0;
    oCPStatus.isNewValue = 0;
    oCPStatus.reserve = 0;
    oCPStatus.n_val = 2; // "Value": 2, "Text": "PrepareWork"
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OCPStatus0x22);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &oCPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x22:" << byteArray.toHex();
}

void CommonModule::sendModuleStatus24() {
    m_genericHeader.packType = 0x24;
    m_genericHeader.dataSize = sizeof(OModuleStatus0x24);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    OModuleStatus0x24 oModuleStatus;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oModuleStatus.time1 = timestamp & 0xFFFFFFFF;
    oModuleStatus.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oModuleStatus.status = 1;
    oModuleStatus.work = 1;
    oModuleStatus.isRGDV = 1;  // 被控参数
    oModuleStatus.isRAF = 1;   // 被控参数
    oModuleStatus.isLocal = 0;
    oModuleStatus.isImit = 0;
    oModuleStatus.hasTP = 0;
    oModuleStatus.isTP = 0;
    oModuleStatus.isWP = 0;
    oModuleStatus.isTPValid = 0;
    oModuleStatus.isWpValid = 0;
    oModuleStatus.statusTwp = 0;
    oModuleStatus.mode = 0;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OModuleStatus0x24);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &oModuleStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x24:" << byteArray.toHex();
}

void CommonModule::sendControlledOrder23(uint8_t code, quint16 pkgId) {
    m_genericHeader.packType = 0x23;
    m_genericHeader.dataSize = sizeof(OReqCtl0x23);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    OReqCtl0x23 oReqCtl;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oReqCtl.time1 = timestamp & 0xFFFFFFFF;
    oReqCtl.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oReqCtl.n_id_Com = pkgId; // unknown
    oReqCtl.n_code = code;
    
    
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OReqCtl0x23);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &oReqCtl, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x23: " << byteArray.toHex()
             << "n_id_Com: "  << oReqCtl.n_id_Com;
}

void CommonModule::sendLogMsg25(QString msg) {
    m_genericHeader.packType = 0x25;
    m_genericHeader.dataSize = msg.size();
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    LogMsg0x25 logMsg;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    logMsg.time1 = timestamp & 0xFFFFFFFF;
    logMsg.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    logMsg.IDParam = 0xffff;
    logMsg.type = 0;
    logMsg.reserve = 0;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(LogMsg0x25);
    quint8 len3 = msg.length();
    char* data = (char*)malloc(len1 + len2 + len3);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &logMsg, len2);
    memcpy(data + len1 + len2, msg.toUtf8().constData(), len3);
    QByteArray byteArray(data, len1 + len2 + len3);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x25:" << byteArray.toHex() << ", " << msg;
}

void CommonModule::sendNote2Operator26(QString msg) {
    m_genericHeader.packType = 0x26;
    m_genericHeader.dataSize = msg.size();
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.time1 = timestamp & 0xFFFFFFFF;
    oTimeReq.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTimeReq);
    quint8 len3 = msg.length();
    char* data = (char*)malloc(len1 + len2 + len3);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &oTimeReq, len2);
    memcpy(data + len1 + len2, msg.toUtf8().constData(), len3);
    QByteArray byteArray(data, len1 + len2 + len3);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x26:" << byteArray.toHex();
}

void CommonModule::sendModuleCPStatus28() {
    CustomisedParm0x28 customisedParm;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    customisedParm.time1 = timestamp & 0xFFFFFFFF;
    customisedParm.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    customisedParm.IDParam = 0;
    customisedParm.size = 4;
    customisedParm.reserve = 0;
    customisedParm.np_v = 1;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x28;
    m_genericHeader.packIdx++;
    m_genericHeader.dataSize = sizeof(CustomisedParm0x28);
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(CustomisedParm0x28);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &m_genericHeader, len1);
    memcpy(data + len1, &customisedParm, len2);
    QByteArray byteArray(data, len1 + len2);
    pTcpSocket->write(byteArray);
    pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x28:" << byteArray.toHex()
             << "IDParam:"   << customisedParm.IDParam
             << "size:"      << customisedParm.size
             << "reserve:"   << customisedParm.reserve
             << "np_v:"      << customisedParm.np_v;
}


void CommonModule::recvStart40(const QByteArray& buff) {
}

void CommonModule::recvStop41(const QByteArray& buff) {
}

void CommonModule::recvRestart42(const QByteArray& buff) {
}

void CommonModule::recvReset43(const QByteArray& buff) {
}

void CommonModule::recvUpdate44(const QByteArray& buff) {
}

void CommonModule::recvNote4Operator45(const QByteArray& buff) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(Note2Oprator450x45);
    QByteArray stringData = buff.right(buff.size() - len1 - len2);
    QString msg = QString::fromUtf8(stringData);
}

void CommonModule::recvSettingLang47(const QByteArray& buff) {
}

void CommonModule::recvRadioAndSatellite48(const QByteArray& buff) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(RadioAndSatellite0x48);
    RadioAndSatellite0x48 radioAndSatellite48;
    memcpy(&radioAndSatellite48, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&radioAndSatellite48), len2);
    qDebug() << "isREB:"   << radioAndSatellite48.isREB 
             << "isGeo:"   << radioAndSatellite48.isGeo
             << "reserve:" << radioAndSatellite48.reserve;
}


// 49->23
void CommonModule::recvSettingTime49(const QByteArray& buff) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(ReqSettingTime0x49);
    ReqSettingTime0x49 reqSettingTime49;
    memcpy(&reqSettingTime49, buff.data() + len1, len2);

    qint64 time1 = QDateTime::currentMSecsSinceEpoch();
    qint64 time2 = static_cast<qint64>(reqSettingTime49.time2) << 32 | reqSettingTime49.time1;
    // 对时接口
    reqAndResTime(time1, time2);
    /* ------------------------------------------------------------------------ */
    if(isDebugOut) {
        sendLogMsg25("setting time successfully");
        sendNote2Operator26("setting time successfully");
    }
}

// 4A -> 05
void CommonModule::recvModuleLocation4A(const QByteArray& buff) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(ReqSettingLocation0x4A);
    ReqSettingLocation0x4A reqSettingLocation0x4A;
    memcpy(&reqSettingLocation0x4A, buff.data() + len1, len2);

    m_ModuleGeoLocation0x5.xLat    = reqSettingLocation0x4A.lat;
    m_ModuleGeoLocation0x5.yLong   = reqSettingLocation0x4A.lon;
    m_ModuleGeoLocation0x5.zHeight = reqSettingLocation0x4A.alt;

    sendModuleLocation05();
    /* ------------------------------------------------------------------------ */
    if(isDebugOut) {
        sendLogMsg25("setting location successfully");
        sendNote2Operator26("setting location successfully");
    }
    qDebug() << "lat=" << reqSettingLocation0x4A.lat 
             << "lon=" << reqSettingLocation0x4A.lon
             << "alt=" << reqSettingLocation0x4A.alt;
}

// 4B -> 28
void CommonModule::recvCustomizedParam4B(const QByteArray& buff) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(ReqSettingCustomizedParam0x4B);
    ReqSettingCustomizedParam0x4B reqSettingCustomizedParam0x4B;
    memcpy(&reqSettingCustomizedParam0x4B, buff.data() + len1, len2);

    sendModuleCPStatus28();
    /* ------------------------------------------------------------------------ */
    if(isDebugOut) {
        sendLogMsg25("setting customized param successfully");
        sendNote2Operator26("setting customized param successfully");
        if(reqSettingCustomizedParam0x4B.isSave == 0) sendLogMsg25("zero is saving new value and not save harddisk");
        if(reqSettingCustomizedParam0x4B.isSave == 1) sendLogMsg25("one is saving new value and need to save harddisk");
    }
    QByteArray byteArray(reinterpret_cast<char*>(&reqSettingCustomizedParam0x4B), len2);
    qDebug() << "IDCfg="  << reqSettingCustomizedParam0x4B.IDConfigParam
             << "size="   << reqSettingCustomizedParam0x4B.size
             << "isSave=" << reqSettingCustomizedParam0x4B.isSave
             << "npVal="  << reqSettingCustomizedParam0x4B.npVal;
}

}