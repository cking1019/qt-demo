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

    this->pTcpSocket =           new QTcpSocket(this);
    this->pReconnectTimer =      new QTimer();
    this->pRequestTimer03 =      new QTimer();
    this->pNPTimer21 =           new QTimer();
    this->pCPTimer22 =           new QTimer();
    this->pModuleStatueTimer24 = new QTimer();

    this->connStatus =     ConnStatus::unConnected;
    this->registerStatus = RegisterStatus::unRegister;
    this->timeStatus =     TimeStatus::unTime;
    this->isSendRegister01    = false;
    this->isModuleLocation05  = false;
    this->isModuleConfigure20 = false;
    
    this->m_iStampResult = 0;
    this->m_iN = 1;
}

CommonModule::~CommonModule() {
    if (this->pTcpSocket != nullptr)           delete this->pTcpSocket;
    if (this->pReconnectTimer != nullptr)      delete this->pReconnectTimer;
    if (this->pRequestTimer03 != nullptr)      delete this->pRequestTimer03;
    if (this->pNPTimer21 != nullptr)           delete this->pNPTimer21;
    if (this->pCPTimer22 != nullptr)           delete this->pCPTimer22;
    if (this->pModuleStatueTimer24 != nullptr) delete this->pModuleStatueTimer24;
}

// 初始化成员变量
void CommonModule::startup() {
    connect(this->pRequestTimer03,      &QTimer::timeout, this, &CommonModule::sendRequestTime03);
    connect(this->pNPTimer21,           &QTimer::timeout, this, &CommonModule::sendModuleNPStatus21);
    connect(this->pCPTimer22,           &QTimer::timeout, this, &CommonModule::sendModuleCPStatus22);
    connect(this->pModuleStatueTimer24, &QTimer::timeout, this, &CommonModule::sendModuleStatus24);

    connect(this->pReconnectTimer, &QTimer::timeout, [=](){
        while (!this->pTcpSocket->waitForConnected(1000))
        {
            qDebug() << "Attempting to connect...";
            this->connStatus = ConnStatus::connecting; // 只需关注连接状态，因为连接状态会影响注册，注册状态会影响对时
            this->pTcpSocket->connectToHost(this->cfg.serverAddress, this->cfg.serverPort);
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
        case 0x2:  this->recvRegister02(buff); break;
        case 0x4:  this->recvRequestTime04(buff); break;
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
    this->genericHeader.dataSize = sizeof(ModuleRegister);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    ModuleRegister moduleRegister;
    moduleRegister.idManuf = 0x1;
    moduleRegister.serialNum = 0x0;
    moduleRegister.versHardMaj = this->genericHeader.vMajor;
    moduleRegister.versHardMin = this->genericHeader.vMinor;
    moduleRegister.versProgMaj = 0x0;
    moduleRegister.isInfo = 0x0;
    moduleRegister.versProgMin = 0x0;
    moduleRegister.isAsku = this->genericHeader.isAsku;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleRegister);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &moduleRegister, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x1:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);

    this->isSendRegister01 = true;
}

void CommonModule::recvRegister02(const QByteArray& buff) {
    ServerRegister serverRegister;
    memcpy(&serverRegister, buff.data() + sizeof(GenericHeader), sizeof(ServerRegister));
    this->genericHeader.moduleId = serverRegister.idxModule;
    qDebug() << "the module id is " << QString::number(serverRegister.idxModule, 16);
    qDebug() << "the connection status is " << QString::number(serverRegister.errorConnect, 16);
    switch (serverRegister.errorConnect) {
        case 0x1:  this->sendLogMsg25("execeed the limited number of same type device"); break;
        case 0x2:  this->sendLogMsg25("try to reconnect same device"); break;
        case 0x4:  this->sendLogMsg25("don't support the device type"); break;
        case 0x8:  this->sendLogMsg25("don't support the prototal version"); break;
        case 0x10: this->sendLogMsg25("module id is not be supported"); break;
        case 0x20:
        case 0x40: qDebug() << "unknown error"; break;
        case 0x0: {
            this->genericHeader.moduleId = serverRegister.idxModule;
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
    this->genericHeader.dataSize = sizeof(ModuleTimeControl);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    ModuleTimeControl moduleTimeControl;
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    moduleTimeControl.timeRequest1 = reqTimestamp & 0xFFFFFFFF;
    moduleTimeControl.timeRequest2 = (reqTimestamp >> 32) & 0xFFFFFFFF;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleTimeControl);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &moduleTimeControl, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x3:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::recvRequestTime04(const QByteArray& buff) {
    ServerTimeControl serverTimeControl;
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ServerTimeControl);
    memcpy(&serverTimeControl, buff.data() + len1, len2);
    quint64 timeStampRcv = QDateTime::currentMSecsSinceEpoch();
    quint64 timeStampReq = serverTimeControl.timeRequest1;
    quint64 timeStampAns = serverTimeControl.timeAnswer1;

    qint64 delTime1 = timeStampAns - timeStampReq;
    qint64 delTime2 = timeStampRcv - timeStampAns;

    qint64 timeOut = (0.5 * (delTime1 - delTime2) + this->m_iStampResult) / this->m_iN;
    this->m_iN++;
    this->m_iStampResult = timeOut + this->m_iStampResult;
    // 授时
    if (timeOut > 200 || timeOut < -200) {
        quint64 timeCurrentStamp = QDateTime::currentMSecsSinceEpoch() + timeOut;

        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timeCurrentStamp);
        qDebug() << "UTC Time: " + dateTime.toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz");

        // 设置本地系统时间
        QString strDate = "date " + dateTime.toString("yyyy-MM-dd");
        QString strTime = "time " + dateTime.toString("hh:mm:ss");
        qDebug() <<strDate;
        qDebug() <<strTime;
        system(strDate.toStdString().c_str());
        system(strTime.toStdString().c_str());

        m_iN = 1;
        m_iStampResult = 0;
    }

    this->timeStatus = TimeStatus::timed;
}

void CommonModule::sendModuleLocation05() {
    this->genericHeader.packType = 0x5;
    this->genericHeader.dataSize = sizeof(ModuleGeoLocation);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    this->isModuleLocation05 = true;

    ModuleGeoLocation oModulPos;
    oModulPos.typeData = 1;
    oModulPos.isValid = 1;
    oModulPos.reserve = 0;
    oModulPos.xLat = 0;
    oModulPos.yLong = 0;
    oModulPos.zHeight = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleGeoLocation);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oModulPos, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x5:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::recvRequestModuleFigure46(const QByteArray& buff) {
    this->sendModuleFigure20();
}

void CommonModule::sendModuleFigure20() {
    this->genericHeader.packType = 0x20;
    this->genericHeader.dataSize = this->cfg.moduleCfg20.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
 
    // 发送模块图状态
    this->isModuleConfigure20 = true;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = this->cfg.moduleCfg20.size();
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, this->cfg.moduleCfg20.data(), len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x20:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::sendModuleNPStatus21() {
    this->genericHeader.packType = 0x21;
    this->genericHeader.dataSize = sizeof(ONPStatus);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    ONPStatus oNPStatus;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oNPStatus.time1 = timestamp & 0xFFFFFFFF;
    oNPStatus.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oNPStatus.IDElem = 0;
    oNPStatus.status = 1;
    oNPStatus.workF1 = 1;
    oNPStatus.local = 0;
    oNPStatus.isImit = 0;
    oNPStatus.reserve = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ONPStatus);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oNPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x21:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::sendModuleCPStatus22() {
    this->genericHeader.packType = 0x22;
    this->genericHeader.dataSize = sizeof(OCPStatus);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OCPStatus oCPStatus;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oCPStatus.time1 = timestamp & 0xFFFFFFFF;
    oCPStatus.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oCPStatus.IDParam = 0;
    oCPStatus.status = 0;
    oCPStatus.size = 0;
    oCPStatus.isNewStatus = 0;
    oCPStatus.isNewValue = 0;
    oCPStatus.reserve = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OCPStatus);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oCPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x22:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::sendModuleStatus24() {
    this->genericHeader.packType = 0x24;
    this->genericHeader.dataSize = sizeof(OModuleStatus);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OModuleStatus oModuleStatus;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oModuleStatus.time1 = timestamp & 0xFFFFFFFF;
    oModuleStatus.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    oModuleStatus.status = 1;
    oModuleStatus.work = 1;
    oModuleStatus.isRGDV = 0;
    oModuleStatus.isRAF = 0;
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

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OModuleStatus);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oModuleStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x24:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::sendControlledOrder23(uint8_t code) {
    this->genericHeader.packType = 0x23;
    this->genericHeader.dataSize = sizeof(OReqCtl);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OReqCtl oReqCtl;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oReqCtl.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oReqCtl.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oReqCtl.n_id_Com = 0x1;
    oReqCtl.n_code = code;

    if (code == 0x6 || code == 0x7f) {
        QString msg = "unknow type of message";
        // 0x27,发送扩展命令
        this->genericHeader.packType = 0x27;
        // 发送0x23协议失败的原因
        QByteArray utf8Bytes = msg.toUtf8();

        quint8 len1 = sizeof(GenericHeader);
        quint8 len2 = sizeof(OReqCtl);
        quint8 len3 = utf8Bytes.size();
        char* data = (char*)malloc(len1 + len2 + len3);
        memcpy(data, &this->genericHeader, len1);
        memcpy(data + len1, &oReqCtl, len2);
        memcpy(data + len1 + len2, &utf8Bytes, len3);
        QByteArray byteArray(data, len1 + len2 + len3);
        qDebug() << "send 0x27:" << byteArray.toHex();
        this->pTcpSocket->write(byteArray);
        this->pTcpSocket->flush();
        free(data);
        return;
    }

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OReqCtl);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oReqCtl, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x23: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::sendLogMsg25(QString msg) {
    this->genericHeader.packType = 0x25;
    this->genericHeader.dataSize = msg.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    LogMsg logMsg;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    logMsg.time1 = timestamp & 0xFFFFFFFF;
    logMsg.time2 = (timestamp >> 32) & 0xFFFFFFFF;
    logMsg.IDParam = 0xffff;
    logMsg.type = 0;
    logMsg.reserve = 0;

    // 消息日志
    QByteArray utf8Bytes = msg.toUtf8();

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(LogMsg);
    quint8 len3 = utf8Bytes.size();
    char* data = (char*)malloc(len1 + len2 + len3);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &logMsg, len2);
    memcpy(data + len1 + len2, &utf8Bytes, len3);
    QByteArray byteArray(data, len1 + len2 + len3);
    qDebug() << "send 0x25:" << byteArray.toHex() << ", " << msg;
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void CommonModule::sendNote2Operator26(QString msg) {
    this->genericHeader.packType = 0x26;
    this->genericHeader.dataSize = msg.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.time1 = timestamp & 0xFFFFFFFF;
    oTimeReq.time2 = (timestamp >> 32) & 0xFFFFFFFF;

    // 消息日志
    QByteArray utf8Bytes = msg.toUtf8();

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OTimeReq);
    quint8 len3 = utf8Bytes.size();
    char* data = (char*)malloc(len1 + len2 + len3);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oTimeReq, len2);
    memcpy(data + len1 + len2, &utf8Bytes, len3);
    QByteArray byteArray(data, len1 + len2 + len3);
    qDebug() << "send 0x26:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}


void CommonModule::recvStart40(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recvStart40";
}

void CommonModule::recvStop41(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recvStop41";
}

void CommonModule::recvRestart42(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recvRestart42";
}

void CommonModule::recvReset43(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recvReset43";
}

void CommonModule::recvUpdate44(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recvUpdate44";
}

void CommonModule::recvNote4Operator45(const QByteArray& buff) {
    QByteArray stringData = buff.right(buff.size() - sizeof(GenericHeader));
    QString msg = QString::fromUtf8(stringData);
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recive the msg from operator: " << msg;
}

void CommonModule::recvSettingLang47(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recive the setting language";
}

void CommonModule::recvRadioAndSatellite48(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    qDebug() << "recive radio and statllite";
}

void CommonModule::recvSettingTime49(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
}

void CommonModule::recvModuleLocation4A(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    // 发送模块位置0x5
    this->sendModuleLocation05();
}

void CommonModule::recvCustomizedParam4B(const QByteArray& buff) {
    // 发送受控状态0x23
    this->sendControlledOrder23(0);
    // 发送0x24模块状态
    this->sendModuleStatus24();
}

}