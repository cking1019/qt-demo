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

    pTcpSocket =           new QTcpSocket(this);
    m_pReconnectTimer =      new QTimer();

    m_pRequestTimer03 =      new QTimer();
    m_pModuleStateTimer21 =  new QTimer();
    m_pCPTimer22 =           new QTimer();
    m_pModuleStatueTimer24 = new QTimer();
    m_pNPTimer28 =           new QTimer();

    connStatus =     ConnStatus::unConnected;
    registerStatus = RegisterStatus::unRegister;
    timeStatus =     TimeStatus::unTime;
    m_isSendRegister01    = false;
    m_isSendModuleLocation05  = false;
    m_isSendModuleConfigure20 = false;
    m_isSendForbiddenIRIList828 = false;
    
    m_iStampResult = 0;
    m_iN = 1;
    m_isDebugOut = 0;

    m_moduleRegister0x1.idManuf = 0x1;
    m_moduleRegister0x1.serialNum = 0x0;
    m_moduleRegister0x1.versHardMaj = m_genericHeader.vMajor;
    m_moduleRegister0x1.versHardMin = m_genericHeader.vMinor;
    m_moduleRegister0x1.versProgMaj = 0x0;
    m_moduleRegister0x1.isInfo = 0x1;
    m_moduleRegister0x1.versProgMin = 0x0;
    m_moduleRegister0x1.isAsku = m_genericHeader.isAsku;

    m_ModuleGeoLocation0x5.typeData = 1;
    m_ModuleGeoLocation0x5.isValid = 1;
    m_ModuleGeoLocation0x5.xLat = 10;
    m_ModuleGeoLocation0x5.yLong = 20;
    m_ModuleGeoLocation0x5.zHeight = 30;

    OElemStatus0x21 m_oElemStatus0x21;
    m_oElemStatus0x21.IDElem = 0;
    m_oElemStatus0x21.status = 1;
    m_oElemStatus0x21.workF1 = 1;
    m_oElemStatus0x21.local = 0;
    m_oElemStatus0x21.isImit = 0;
    m_vecOElemStatus0x21.append(m_oElemStatus0x21);

    OCPStatus0x22 m_oCPStatus0x22;
    m_oCPStatus0x22.IDParam = 0; // 20中CP参数的IDParamID，决定哪种受控状态
    m_oCPStatus0x22.status = 4;  // 参数不受控
    m_oCPStatus0x22.size = 0;
    m_oCPStatus0x22.isNewStatus = 0;
    m_oCPStatus0x22.isNewValue = 0;
    m_oCPStatus0x22.n_val = 2;   // "Value": 2, "Text": "PrepareWork"
    m_vecOCPStatus0x22.append(m_oCPStatus0x22);

    CustomisedNP0x28 m_customisedNPx28;
    m_customisedNPx28.IDParam = 0;
    m_customisedNPx28.size = 4;   // 如果0x20中传输了values数据，那么设置为4
    m_customisedNPx28.np_v = 1;   // 0x20中索引的值
    m_vecCustomisedNP0x28.append(m_customisedNPx28);

    m_oModuleStatus0x24.status = 1;
    m_oModuleStatus0x24.work = 1;
    m_oModuleStatus0x24.isRGDV = 1;  // 被控参数
    m_oModuleStatus0x24.isRAF = 1;   // 被控参数
    m_oModuleStatus0x24.isLocal = 0;
    m_oModuleStatus0x24.isImit = 0;
    m_oModuleStatus0x24.hasTP = 0;
    m_oModuleStatus0x24.isTP = 0;
    m_oModuleStatus0x24.isWP = 0;
    m_oModuleStatus0x24.isTPValid = 0;
    m_oModuleStatus0x24.isWpValid = 0;
    m_oModuleStatus0x24.statusTwp = 0;
    m_oModuleStatus0x24.mode = 0;
}

CommonModule::~CommonModule() {
    if (pTcpSocket != nullptr)                delete pTcpSocket;
    if (m_pReconnectTimer != nullptr)         delete m_pReconnectTimer;

    if (m_pRequestTimer03 != nullptr)         delete m_pRequestTimer03;
    if (m_pModuleStateTimer21 != nullptr)     delete m_pModuleStateTimer21;
    if (m_pCPTimer22 != nullptr)              delete m_pCPTimer22;
    if (m_pModuleStatueTimer24 != nullptr)    delete m_pModuleStatueTimer24;
    if (m_pNPTimer28 != nullptr)              delete m_pNPTimer28;
}

// 初始化成员变量
void CommonModule::startup() {
    connect(m_pRequestTimer03,      &QTimer::timeout, this, &CommonModule::sendRequestTime03);
    connect(m_pModuleStateTimer21,  &QTimer::timeout, this, &CommonModule::sendModuleStatus21);
    connect(m_pCPTimer22,           &QTimer::timeout, this, &CommonModule::sendModuleCPStatus22);
    connect(m_pModuleStatueTimer24, &QTimer::timeout, this, &CommonModule::sendModuleStatus24);
    connect(m_pNPTimer28,           &QTimer::timeout, this, &CommonModule::sendModuleCPStatus28);

    connect(m_pReconnectTimer, &QTimer::timeout, [=](){
        while (!pTcpSocket->waitForConnected(1000))
        {
            // qDebug() << "Attempting to connect...";
            connStatus = ConnStatus::connecting;
            pTcpSocket->connectToHost(m_commCfg.serverAddress, m_commCfg.serverPort);
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
void CommonModule::onReadCommData(qint16 pkgID, const QByteArray& buf) {
    switch (pkgID) {
        case 0x02: recvRegister02(buf); break;
        case 0x04: recvRequestTime04(buf); break;
        case 0x40: recvStart40(buf); break;
        case 0x41: recvStop41(buf); break;
        case 0x42: recvRestart42(buf); break;
        case 0x43: recvReset43(buf); break;
        case 0x44: recvUpdate44(buf); break;
        case 0x45: recvNote4Operator45(buf); break;
        case 0x46: recvRequestModuleFigure46(buf); break;
        case 0x48: recvRadioAndSatellite48(buf); break;
        case 0x49: recvSettingTime49(buf); break;
        case 0x4A: recvModuleLocation4A(buf); break;
        case 0x4B: recvCustomizedParam4B(buf); break;
        default: {
            qDebug() << "this is unknown pkg 0x" << m_genericHeader.packType;
            break;
        }
    }
}

void CommonModule::sendRegister01() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ModuleRegister0x1);
    m_isSendRegister01 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x1;
    m_genericHeader.dataSize = sizeof(ModuleRegister0x1);
    m_genericHeader.packIdx = 0;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_moduleRegister0x1, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x001:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

void CommonModule::recvRegister02(const QByteArray& buf) {
    ServerRegister0x2 ServerRegister0x2;
    memcpy(&ServerRegister0x2, buf.data() + HEADER_LEN, sizeof(ServerRegister0x2));
    m_genericHeader.moduleId = ServerRegister0x2.idxModule;
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
    qDebug() << "recv 0x002:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "moduleID: "  << QString::number(ServerRegister0x2.idxModule, 16)
             << "connStatus:" << QString::number(ServerRegister0x2.errorConnect, 16);
}

void CommonModule::sendRequestTime03() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ModuleTimeControl0x3);
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    m_moduleTimeControl0x3.time1 = reqTimestamp & 0xFFFFFFFF;
    m_moduleTimeControl0x3.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x3;
    m_genericHeader.dataSize = sizeof(ModuleTimeControl0x3);
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_moduleTimeControl0x3, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x003:" << buf.toHex()
             << "pkgSize:"   << buf.length();
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
        if(m_isDebugOut) {
            qDebug() << "UTC Time: " + dateTime.toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz") << strDate << strTime;
        }
        system(strDate.toStdString().c_str());
        system(strTime.toStdString().c_str());
        m_iN = 1;
        m_iStampResult = 0;
    }
}

void CommonModule::recvRequestTime04(const QByteArray& buf) {
    ServerTimeControl0x4 serverTimeControl0x4;
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ServerTimeControl0x4);
    memcpy(&serverTimeControl0x4, buf.data() + len1, len2);
    quint64 timeStampReq = serverTimeControl0x4.time1;
    quint64 timeStampAns = serverTimeControl0x4.time2;
    reqAndResTime(timeStampReq, timeStampAns);
    timeStatus = TimeStatus::timed;
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x004:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

void CommonModule::sendModuleLocation05() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(ModuleGeoLocation0x5);
    m_isSendModuleLocation05 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x5;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_ModuleGeoLocation0x5, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x005:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "Lat:"        << m_ModuleGeoLocation0x5.xLat
             << "lon:"        << m_ModuleGeoLocation0x5.yLong;
}

void CommonModule::recvRequestModuleFigure46(const QByteArray& buf) {
    qDebug() << "recv 0x046:" << buf.toHex()
             << "pkgSize:"    << buf.length();
    sendModuleFigure20();
}

void CommonModule::sendModuleFigure20() {
    quint8  len1 = HEADER_LEN;
    quint16 len2 = m_commCfg.moduleCfg20.length();
    m_isSendModuleConfigure20 = true;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x20;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, m_commCfg.moduleCfg20.toStdString().data(), len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x020:" << buf.toHex()
             << "pkgSize:"   << buf.length()
             << "json msg:"  << m_commCfg.moduleCfg20;
}

void CommonModule::sendModuleStatus21() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTime);
    quint32 len3 = m_vecOElemStatus0x21.size() * sizeof(OElemStatus0x21);
    OTime otime;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    otime.time1 = timestamp & 0xFFFFFFFF;
    otime.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x21;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &otime, len2);
    qint16 offset = len1 + len2;
    for(auto& item : m_vecOElemStatus0x21) {
        memcpy(buf.data() + offset, &item, sizeof(OElemStatus0x21));
        offset += sizeof(OElemStatus0x21);
    }
    
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x021:" << buf.toHex()
             << "pkgSize:"    << buf.length();
    for(auto& item : m_vecOElemStatus0x21) {
        qDebug() << QString("IDElem:%1,status:%2,workF1:%3").arg(item.IDElem).arg(item.status).arg(item.workF1);
    }
}

void CommonModule::sendModuleCPStatus22() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTime);
    quint32 len3 = m_vecOCPStatus0x22.size() * sizeof(OCPStatus0x22);
    OTime otime;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    otime.time1 = timestamp & 0xFFFFFFFF;
    otime.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x22;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &otime, len2);
    qint16 offset = len1 + len2;
    for(auto& item : m_vecOCPStatus0x22) {
        memcpy(buf.data() + offset, &item, sizeof(OCPStatus0x22));
        offset += sizeof(OCPStatus0x22);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x022:" << buf.toHex()
             << "pkgSize:"   << buf.length();
    for(auto& item : m_vecOCPStatus0x22) {
        qDebug() << QString("IDParam:%1,size:%2,n_val:%3").arg(item.IDParam).arg(item.size).arg(item.n_val);
    }
}


void CommonModule::sendModuleStatus24() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OModuleStatus0x24);
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    m_oModuleStatus0x24.time1 = timestamp & 0xFFFFFFFF;
    m_oModuleStatus0x24.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x24;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oModuleStatus0x24, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x024:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "status:"     << m_oModuleStatus0x24.status
             << "work:"       << m_oModuleStatus0x24.work
             << "isRGDV:"     << m_oModuleStatus0x24.isRGDV
             << "isRAF:"      << m_oModuleStatus0x24.isRAF
             << "mode:"       << m_oModuleStatus0x24.mode;
}

void CommonModule::sendControlledOrder23(uint8_t code, quint16 pkgidx) {
    quint8  len1 = HEADER_LEN;
    quint16 len2 = sizeof(OReqCtl0x23);
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    m_oReqCtl0x23.time1 = timestamp & 0xFFFFFFFF;
    m_oReqCtl0x23.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    m_oReqCtl0x23.n_id_Com = pkgidx; // unknown
    m_oReqCtl0x23.n_code = code;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x23;
    m_genericHeader.dataSize = len2;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_oReqCtl0x23, len2);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x023:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "n_id_Com: "  << m_oReqCtl0x23.n_id_Com
             << "n_code: "    << m_oReqCtl0x23.n_code;
             
}

void CommonModule::sendLogMsg25(QString msg) {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(LogMsg0x25);
    quint8 len3 = msg.length();
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    m_logMsg0x25.time1 = timestamp & 0xFFFFFFFF;
    m_logMsg0x25.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    m_logMsg0x25.IDParam = 0xffff;
    m_logMsg0x25.type = 0;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x25;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &m_logMsg0x25, len2);
    memcpy(buf.data() + len1 + len2, msg.toUtf8().constData(), len3);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x025:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "msg: "       << msg;
}

void CommonModule::sendNote2Operator26(QString msg) {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTime);
    quint8 len3 = msg.length();
    OTime oTime;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTime.time1 = timestamp & 0xFFFFFFFF;
    oTime.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x26;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.packIdx++;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &oTime, len2);
    memcpy(buf.data() + len1 + len2, msg.toUtf8().constData(), len3);
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x026:" << buf.toHex()
             << "pkgSize:"    << buf.length();
}

void CommonModule::sendModuleCPStatus28() {
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTime);
    quint32 len3 = m_vecCustomisedNP0x28.size() * sizeof(CustomisedNP0x28);
    OTime oTime;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTime.time1 = timestamp & 0xFFFFFFFF;
    oTime.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    m_genericHeader.packType = 0x28;
    m_genericHeader.packIdx++;
    m_genericHeader.dataSize = len2 + len3;
    m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&m_genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QByteArray buf;
    buf.resize(len1 + len2 + len3);
    memcpy(buf.data(), &m_genericHeader, len1);
    memcpy(buf.data() + len1, &oTime, len2);
    qint16 offset = len1 + len2;
    for(auto& item : m_vecCustomisedNP0x28) {
        memcpy(buf.data() + offset, &item, sizeof(CustomisedNP0x28));
        offset += sizeof(CustomisedNP0x28);
    }
    pTcpSocket->write(buf);
    pTcpSocket->flush();
    /* ------------------------------------------------------------------------ */
    qDebug() << "send 0x028:" << buf.toHex()
             << "pkgSize:"    << buf.length();
    for(auto& item : m_vecCustomisedNP0x28) {
        qDebug() << QString("IDParam:%1,size:%2,np_v:%3").arg(item.IDParam).arg(item.size).arg(item.np_v);
    }
}


void CommonModule::recvStart40(const QByteArray& buf) {
}

void CommonModule::recvStop41(const QByteArray& buf) {
}

void CommonModule::recvRestart42(const QByteArray& buf) {
}

void CommonModule::recvReset43(const QByteArray& buf) {
}

void CommonModule::recvUpdate44(const QByteArray& buf) {
}

void CommonModule::recvNote4Operator45(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(Note2Oprator450x45);
    QByteArray stringData = buf.right(buf.size() - len1 - len2);
    QString msg = QString::fromUtf8(stringData);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x045:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "msg:"        << msg;
}

void CommonModule::recvRadioAndSatellite48(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(RadioAndSatellite0x48);
    RadioAndSatellite0x48 radioAndSatellite48;
    memcpy(&radioAndSatellite48, buf.data() + len1, len2);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x048:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "isREB:"      << radioAndSatellite48.isREB 
             << "isGeo:"      << radioAndSatellite48.isGeo
             << "reserve:"    << radioAndSatellite48.reserve;
    if(m_isDebugOut) {
        sendLogMsg25("recvRadioAndSatellite48");
        sendNote2Operator26("recvRadioAndSatellite48");
    }
}


// 49->23
void CommonModule::recvSettingTime49(const QByteArray& buf) {
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(ReqSettingTime0x49);
    ReqSettingTime0x49 reqSettingTime49;
    memcpy(&reqSettingTime49, buf.data() + len1, len2);
    qint64 time1 = QDateTime::currentMSecsSinceEpoch();
    qint64 time2 = static_cast<qint64>(reqSettingTime49.time2) << 32 | reqSettingTime49.time1;
    // 对时接口
    reqAndResTime(time1, time2);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x049:" << buf.toHex()
             << "pkgSize:"    << buf.length();
    if(m_isDebugOut) {
        sendLogMsg25("recvSettingTime49");
        sendNote2Operator26("recvSettingTime49");
    }
}

// 4A -> 05
void CommonModule::recvModuleLocation4A(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(ReqSettingLocation0x4A);
    ReqSettingLocation0x4A reqSettingLocation0x4A;
    memcpy(&reqSettingLocation0x4A, buf.data() + len1, len2);
    m_ModuleGeoLocation0x5.xLat    = reqSettingLocation0x4A.lat;
    m_ModuleGeoLocation0x5.yLong   = reqSettingLocation0x4A.lon;
    m_ModuleGeoLocation0x5.zHeight = reqSettingLocation0x4A.alt;
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x04A:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "lat="        << reqSettingLocation0x4A.lat 
             << "lon="        << reqSettingLocation0x4A.lon
             << "alt="        << reqSettingLocation0x4A.alt;
    if(m_isDebugOut) {
        sendLogMsg25("recvModuleLocation4A");
        sendNote2Operator26("recvModuleLocation4A");
    }
    sendControlledOrder23(0, pkgIdx);
}

// 4B -> 28
void CommonModule::recvCustomizedParam4B(const QByteArray& buf) {
    quint16 pkgIdx = 0;
    memcpy(&pkgIdx, buf.data() + 6, 2);
    qint8 len1 = HEADER_LEN;
    qint8 len2 = sizeof(ReqSettingCustomizedParam0x4B);
    ReqSettingCustomizedParam0x4B reqSettingCustomizedParam0x4B;
    memcpy(&reqSettingCustomizedParam0x4B, buf.data() + len1, len2);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x04B:" << buf.toHex()
             << "pkgSize:"    << buf.length()
             << "IDCfg="      << reqSettingCustomizedParam0x4B.IDConfigParam
             << "size="       << reqSettingCustomizedParam0x4B.size
             << "isSave="     << reqSettingCustomizedParam0x4B.isSave
             << "npVal="      << reqSettingCustomizedParam0x4B.npVal;
    if(m_isDebugOut) {
        sendLogMsg25("recvCustomizedParam4B");
        sendNote2Operator26("recvCustomizedParam4B");
        if(reqSettingCustomizedParam0x4B.isSave == 0) sendLogMsg25("zero is saving new value and not save harddisk");
        if(reqSettingCustomizedParam0x4B.isSave == 1) sendLogMsg25("one is saving new value and need to save harddisk");
    }
    sendControlledOrder23(0, pkgIdx);
    sendModuleCPStatus28();
}

}