#include "CommonModule.hpp"

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
    this->pkgsComm = {0x2, 0x4, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B};

    this->pTcpSocket =              new QTcpSocket(this);
    this->pReconnectTimer =         new QTimer();

    this->pRequestTimer03 =         new QTimer();
    this->pModuleStateTimer21 =              new QTimer();
    this->pCPTimer22 =              new QTimer();
    this->pModuleStatueTimer24 =    new QTimer();
    this->pNPTimer28 = new QTimer();

    this->connStatus =     ConnStatus::unConnected;
    this->registerStatus = RegisterStatus::unRegister;
    this->timeStatus =     TimeStatus::unTime;
    this->isSendRegister01    = false;
    this->isModuleLocation05  = false;
    this->isModuleConfigure20 = false;
    
    this->m_iStampResult = 0;
    this->m_iN = 1;

    this->isDebugOut = 0;
}

CommonModule::~CommonModule() {
    if (this->pTcpSocket != nullptr)              delete this->pTcpSocket;
    if (this->pReconnectTimer != nullptr)         delete this->pReconnectTimer;

    if (this->pRequestTimer03 != nullptr)         delete this->pRequestTimer03;
    if (this->pModuleStateTimer21 != nullptr)              delete this->pModuleStateTimer21;
    if (this->pCPTimer22 != nullptr)              delete this->pCPTimer22;
    if (this->pModuleStatueTimer24 != nullptr)    delete this->pModuleStatueTimer24;
    if (this->pNPTimer28 != nullptr) delete this->pNPTimer28;
}

// 初始化成员变量
void CommonModule::startup() {
    connect(this->pRequestTimer03,      &QTimer::timeout, this, &CommonModule::sendRequestTime03);
    connect(this->pModuleStateTimer21,           &QTimer::timeout, this, &CommonModule::sendModuleStatus21);
    connect(this->pCPTimer22,           &QTimer::timeout, this, &CommonModule::sendModuleCPStatus22);
    connect(this->pModuleStatueTimer24, &QTimer::timeout, this, &CommonModule::sendModuleStatus24);
    connect(this->pNPTimer28, &QTimer::timeout, this, &CommonModule::sendModuleCPStatus28);

    connect(this->pReconnectTimer, &QTimer::timeout, [=](){
        while (!this->pTcpSocket->waitForConnected(1000))
        {
            // qDebug() << "Attempting to connect...";
            this->connStatus = ConnStatus::connecting; // 只需关注连接状态，因为连接会影响注册，注册会影响对时
            this->pTcpSocket->connectToHost(this->commCfg.serverAddress, this->commCfg.serverPort);
        }
    });
    connect(this->pTcpSocket, &QTcpSocket::connected, [=](){
        qDebug() << "Connected to host!";
        this->connStatus = ConnStatus::connected;
    });
    // abort -> close -> disconnectFromHost
    connect(this->pTcpSocket, &QTcpSocket::disconnected, [=]() {
        qDebug() << "Disconnected from server!";
        this->connStatus = ConnStatus::unConnected;
    });
}

// 接收数据
void CommonModule::onReadCommData(qint16 pkgID, const QByteArray& buff) {
    switch (pkgID) {
        case 0x02: this->recvRegister02(buff); break;
        case 0x04: this->recvRequestTime04(buff); break;
        case 0x40: this->recvStart40(buff); break;
        case 0x41: this->recvStop41(buff); break;
        case 0x42: this->recvRestart42(buff); break;
        case 0x43: this->recvReset43(buff); break;
        case 0x44: this->recvUpdate44(buff); break;
        case 0x45: this->recvNote4Operator45(buff); break;
        case 0x46: this->recvRequestModuleFigure46(buff); break;
        case 0x47: this->recvSettingLang47(buff); break;
        case 0x48: this->recvRadioAndSatellite48(buff); break;
        case 0x49: this->recvSettingTime49(buff); break;
        case 0x4A: this->recvModuleLocation4A(buff); break;
        case 0x4B: this->recvCustomizedParam4B(buff); break;
        default: {
            qDebug() << "this is unknown pkg 0x" << this->genericHeader.packType;
            break;
        }
    }
}

void CommonModule::sendRegister01() {
    this->genericHeader.packType = 0x1;
    this->genericHeader.dataSize = sizeof(ModuleRegister0x1);
    this->genericHeader.packIdx = 0;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    ModuleRegister0x1 ModuleRegister0x1;
    ModuleRegister0x1.idManuf = 0x1;
    ModuleRegister0x1.serialNum = 0x0;
    ModuleRegister0x1.versHardMaj = this->genericHeader.vMajor;
    ModuleRegister0x1.versHardMin = this->genericHeader.vMinor;
    ModuleRegister0x1.versProgMaj = 0x0;
    ModuleRegister0x1.isInfo = 0x1;
    ModuleRegister0x1.versProgMin = 0x0;
    ModuleRegister0x1.isAsku = this->genericHeader.isAsku;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleRegister0x1);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &ModuleRegister0x1, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    this->isSendRegister01 = true;
    qDebug() << "send 0x01:" << byteArray.toHex();
}

void CommonModule::recvRegister02(const QByteArray& buff) {
    ServerRegister0x2 ServerRegister0x2;
    memcpy(&ServerRegister0x2, buff.data() + sizeof(GenericHeader), sizeof(ServerRegister0x2));
    this->genericHeader.moduleId = ServerRegister0x2.idxModule;
    qDebug() << "the module id is " << QString::number(ServerRegister0x2.idxModule, 16);
    qDebug() << "the connection status is " << QString::number(ServerRegister0x2.errorConnect, 16);
    switch (ServerRegister0x2.errorConnect) {
        case 0x1:  this->sendLogMsg25("execeed the limited number of same type device"); break;
        case 0x2:  this->sendLogMsg25("try to reconnect same device"); break;
        case 0x4:  this->sendLogMsg25("don't support the device type"); break;
        case 0x8:  this->sendLogMsg25("don't support the prototal version"); break;
        case 0x10: this->sendLogMsg25("module id is not be supported"); break;
        case 0x20:
        case 0x40: qDebug() << "unknown error"; break;
        case 0x0: {
            this->genericHeader.moduleId = ServerRegister0x2.idxModule;
            this->registerStatus = RegisterStatus::registered;
            break;
        }
        default: {
            qDebug("fail to register");
        }
    }
}

void CommonModule::sendRequestTime03() {
    this->genericHeader.packType = 0x3;
    this->genericHeader.dataSize = sizeof(ModuleTimeControl0x3);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    this->myModuleTimeControl0x3.time1 = reqTimestamp & 0xFFFFFFFF;
    this->myModuleTimeControl0x3.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleTimeControl0x3);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->myModuleTimeControl0x3, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x03:" << byteArray.toHex();
}

// 对时
void CommonModule::reqAndResTime(quint64 timeStampReq, quint64 timeStampAns) {
    quint64 timeStampRcv = QDateTime::currentMSecsSinceEpoch();
    qint64 delTime1 = timeStampAns - timeStampReq; // 请求时间与响应时间的时间差
    qint64 delTime2 = timeStampRcv - timeStampAns; // 当前时间与响应时间的时间差

    qint64 timeOut = (0.5 * (delTime1 - delTime2) + this->m_iStampResult) / this->m_iN;
    this->m_iN++;
    this->m_iStampResult = timeOut + this->m_iStampResult;
    // 授时
    if (timeOut > 200 || timeOut < -200) {
        quint64 timeCurrentStamp = QDateTime::currentMSecsSinceEpoch() + timeOut;
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timeCurrentStamp);
        // 设置本地系统时间
        QString strDate = "date " + dateTime.toString("yyyy-MM-dd");
        QString strTime = "time " + dateTime.toString("hh:mm:ss");
        if(this->isDebugOut) {
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
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ServerTimeControl0x4);
    memcpy(&serverTimeControl0x4, buff.data() + len1, len2);
    
    quint64 timeStampReq = serverTimeControl0x4.time1;
    quint64 timeStampAns = serverTimeControl0x4.time2;
    this->reqAndResTime(timeStampReq, timeStampAns);

    this->timeStatus = TimeStatus::timed;
}

void CommonModule::sendModuleLocation05() {
    this->genericHeader.packType = 0x5;
    this->genericHeader.dataSize = sizeof(ModuleGeoLocation0x5);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleGeoLocation0x5);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->myModuleGeoLocation0x5, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    this->isModuleLocation05 = true;
    qDebug() << "send 0x05:" << byteArray.toHex();
}

void CommonModule::recvRequestModuleFigure46(const QByteArray& buff) {
    this->sendModuleFigure20();
}

void CommonModule::sendModuleFigure20() {
    this->genericHeader.packType = 0x20;
    this->genericHeader.dataSize = this->commCfg.moduleCfg20.length();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint16 len2 = this->commCfg.moduleCfg20.length();
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, this->commCfg.moduleCfg20.toStdString().data(), len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    this->isModuleConfigure20 = true;
    qDebug() << "send 0x20:" << byteArray.toHex() << this->commCfg.moduleCfg20;
    qDebug() << this->commCfg.moduleCfg20.toStdString().data();
}

void CommonModule::sendModuleStatus21() {
    this->genericHeader.packType = 0x21;
    this->genericHeader.dataSize = sizeof(ONPStatus0x21);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
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
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ONPStatus0x21);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oNPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x21:" << byteArray.toHex();
}

void CommonModule::sendModuleCPStatus22() {
    this->genericHeader.packType = 0x22;
    this->genericHeader.dataSize = sizeof(OCPStatus0x22);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
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
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OCPStatus0x22);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oCPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x22:" << byteArray.toHex();
}

void CommonModule::sendModuleStatus24() {
    this->genericHeader.packType = 0x24;
    this->genericHeader.dataSize = sizeof(OModuleStatus0x24);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
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
    oModuleStatus.reserve = 0;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OModuleStatus0x24);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oModuleStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x24:" << byteArray.toHex();
}

void CommonModule::sendControlledOrder23(uint8_t code) {
    this->genericHeader.packType = 0x23;
    this->genericHeader.dataSize = sizeof(OReqCtl0x23);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    OReqCtl0x23 oReqCtl;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oReqCtl.time1 = timestamp & 0xFFFFFFFF;
    oReqCtl.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oReqCtl.n_id_Com = 0x1;
    oReqCtl.n_code = code;
    
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OReqCtl0x23);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oReqCtl, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x23: " << byteArray.toHex();
}

void CommonModule::sendLogMsg25(QString msg) {
    this->genericHeader.packType = 0x25;
    this->genericHeader.dataSize = msg.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    LogMsg0x25 logMsg;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    logMsg.time1 = timestamp & 0xFFFFFFFF;
    logMsg.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    logMsg.IDParam = 0xffff;
    logMsg.type = 0;
    logMsg.reserve = 0;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(LogMsg0x25);
    quint8 len3 = msg.length();
    char* data = (char*)malloc(len1 + len2 + len3);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &logMsg, len2);
    memcpy(data + len1 + len2, msg.toUtf8().constData(), len3);
    QByteArray byteArray(data, len1 + len2 + len3);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x25:" << byteArray.toHex() << ", " << msg;
}

void CommonModule::sendNote2Operator26(QString msg) {
    this->genericHeader.packType = 0x26;
    this->genericHeader.dataSize = msg.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.time1 = timestamp & 0xFFFFFFFF;
    oTimeReq.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OTimeReq);
    quint8 len3 = msg.length();
    char* data = (char*)malloc(len1 + len2 + len3);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oTimeReq, len2);
    memcpy(data + len1 + len2, msg.toUtf8().constData(), len3);
    QByteArray byteArray(data, len1 + len2 + len3);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
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
    this->genericHeader.packType = 0x28;
    this->genericHeader.packIdx++;
    this->genericHeader.dataSize = sizeof(CustomisedParm0x28);
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(CustomisedParm0x28);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &customisedParm, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x28:" << byteArray.toHex()
             << "IDParam:"   << customisedParm.IDParam
             << "size:"      << customisedParm.size
             << "reserve:"   << customisedParm.reserve
             << "np_v:"      << customisedParm.np_v;
}


void CommonModule::recvStart40(const QByteArray& buff) {
    quint8 code = 0;
    this->sendControlledOrder23(code);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x40:";
}

void CommonModule::recvStop41(const QByteArray& buff) {
    quint8 code = 0;
    this->sendControlledOrder23(code);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x41:";
}

void CommonModule::recvRestart42(const QByteArray& buff) {
    quint8 code = 0;
    this->sendControlledOrder23(code);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x42:";
}

void CommonModule::recvReset43(const QByteArray& buff) {
    quint8 code = 0;
    this->sendControlledOrder23(code);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x43:";
}

void CommonModule::recvUpdate44(const QByteArray& buff) {
    quint8 code = 0;
    this->sendControlledOrder23(code);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x44:";
}

void CommonModule::recvNote4Operator45(const QByteArray& buff) {
    qint8 len1 = sizeof(GenericHeader);
    qint8 len2 = sizeof(Note2Oprator450x45);
    QByteArray stringData = buff.right(buff.size() - len1 - len2);
    QString msg = QString::fromUtf8(stringData);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x45:" << msg;
}

void CommonModule::recvSettingLang47(const QByteArray& buff) {
    quint8 code = 0;
    this->sendControlledOrder23(code);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x47:";
}

void CommonModule::recvRadioAndSatellite48(const QByteArray& buff) {
    qint8 len1 = sizeof(GenericHeader);
    qint8 len2 = sizeof(RadioAndSatellite0x48);
    RadioAndSatellite0x48 radioAndSatellite48;
    memcpy(&radioAndSatellite48, buff.data() + len1, len2);
    /* ------------------------------------------------------------------------ */
    QByteArray byteArray(reinterpret_cast<char*>(&radioAndSatellite48), len2);
    qDebug() << "recv 0x48:" << byteArray.toHex() 
             << "isREB:"   << radioAndSatellite48.isREB 
             << "isGeo:"   << radioAndSatellite48.isGeo
             << "reserve:" << radioAndSatellite48.reserve;
}


// 49->23
void CommonModule::recvSettingTime49(const QByteArray& buff) {
    qint8 len1 = sizeof(GenericHeader);
    qint8 len2 = sizeof(ReqSettingTime0x49);
    ReqSettingTime0x49 reqSettingTime49;
    memcpy(&reqSettingTime49, buff.data() + len1, len2);

    qint64 time1 = QDateTime::currentMSecsSinceEpoch();
    qint64 time2 = static_cast<qint64>(reqSettingTime49.time2) << 32 | reqSettingTime49.time1;
    // 对时接口
    this->reqAndResTime(time1, time2);
    /* ------------------------------------------------------------------------ */
    if(this->isDebugOut) {
        sendLogMsg25("setting time successfully");
        sendNote2Operator26("setting time successfully");
    }
    QByteArray byteArray(reinterpret_cast<char*>(&reqSettingTime49), len2);
    qDebug() << "recv 0x49:" << byteArray.toHex() 
             << "time1=" << reqSettingTime49.time1 
             << "time2=" << reqSettingTime49.time2;
}

// 4A -> 05
void CommonModule::recvModuleLocation4A(const QByteArray& buff) {
    qint8 len1 = sizeof(GenericHeader);
    qint8 len2 = sizeof(ReqSettingLocation0x4A);
    ReqSettingLocation0x4A reqSettingLocation0x4A;
    memcpy(&reqSettingLocation0x4A, buff.data() + len1, len2);

    this->myModuleGeoLocation0x5.xLat    = reqSettingLocation0x4A.lat;
    this->myModuleGeoLocation0x5.yLong   = reqSettingLocation0x4A.lon;
    this->myModuleGeoLocation0x5.zHeight = reqSettingLocation0x4A.alt;

    this->sendModuleLocation05();
    /* ------------------------------------------------------------------------ */
    if(this->isDebugOut) {
        sendLogMsg25("setting location successfully");
        sendNote2Operator26("setting location successfully");
    }
    QByteArray byteArray(reinterpret_cast<char*>(&reqSettingLocation0x4A), len2);
    qDebug() << "recv 0x4A:" << byteArray.toHex() 
             << "lat=" << reqSettingLocation0x4A.lat 
             << "lon=" << reqSettingLocation0x4A.lon
             << "alt=" << reqSettingLocation0x4A.alt
             << "x="   << reqSettingLocation0x4A.x
             << "y="   << reqSettingLocation0x4A.y
             << "z="   << reqSettingLocation0x4A.z;
}

// 4B -> 28
void CommonModule::recvCustomizedParam4B(const QByteArray& buff) {
    qint8 len1 = sizeof(GenericHeader);
    qint8 len2 = sizeof(ReqSettingCustomizedParam0x4B);
    ReqSettingCustomizedParam0x4B reqSettingCustomizedParam0x4B;
    memcpy(&reqSettingCustomizedParam0x4B, buff.data() + len1, len2);

    this->sendModuleCPStatus28();
    /* ------------------------------------------------------------------------ */
    if(this->isDebugOut) {
        sendLogMsg25("setting customized param successfully");
        sendNote2Operator26("setting customized param successfully");
        if(reqSettingCustomizedParam0x4B.isSave == 0) sendLogMsg25("zero is saving new value and not save harddisk");
        if(reqSettingCustomizedParam0x4B.isSave == 1) sendLogMsg25("one is saving new value and need to save harddisk");
    }
    QByteArray byteArray(reinterpret_cast<char*>(&reqSettingCustomizedParam0x4B), len2);
    qDebug() << "recv 0x4B:" << byteArray.toHex()
             << "time1="  << reqSettingCustomizedParam0x4B.time1 
             << "time2="  << reqSettingCustomizedParam0x4B.time2
             << "IDCfg="  << reqSettingCustomizedParam0x4B.IDConfigParam
             << "size="   << reqSettingCustomizedParam0x4B.size
             << "isSave=" << reqSettingCustomizedParam0x4B.isSave
             << "reserv=" << reqSettingCustomizedParam0x4B.reserve
             << "npVal="  << reqSettingCustomizedParam0x4B.npVal;
}

}