#include "CommonModule.hpp"

namespace NEBULA
{

// 计算包头校验和
quint16 calcChcekSum(const char* sMess,int nCnt) {
    quint16 nSum = 0;
    quint8 *pb = (quint8*)sMess;
    for(int i = 0; i < nCnt; i++) nSum += *pb++;
    return nSum;
}

CommonModule::CommonModule(QObject *parent):QObject(parent){
    this->pTcpSocket = new QTcpSocket(this);
    this->pRequestTimer = new QTimer();
    this->pReconnectTimer = new QTimer();
    this->pCPandNPTimer = new QTimer();
    this->pModuleStatueTimer = new QTimer();
    // 公共包类型
    this->pkgsComm = {0x2, 0x4, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B};

    this->m_iStampResult = 0;
    this->m_iN = 1;

    // 初始化公共包头数据
    this->genericHeader.sender = 0x50454C; 
    this->genericHeader.moduleId = 0xff;
    this->genericHeader.vMajor = 0x02;
    this->genericHeader.vMinor = 0x02;
    this->genericHeader.packIdx = 0;
    this->genericHeader.dataSize = 0;
    this->genericHeader.isAsku = 1;
    this->genericHeader.packType = 0;
    this->genericHeader.checkSum = 0;

    this->isRegister = false;
    this->isModuleConfigure = false;
    this->isNPandCPStatus = false;
    this->isModuleLocation = false;
    this->isConnected = false;
}

CommonModule::~CommonModule(){
    if(this->pTcpSocket != nullptr)         delete this->pTcpSocket;
    if(this->pReconnectTimer != nullptr)    delete this->pReconnectTimer; 
    if(this->pRequestTimer != nullptr)      delete this->pRequestTimer; 
    if(this->pCPandNPTimer != nullptr)      delete this->pCPandNPTimer; 
    if(this->pModuleStatueTimer != nullptr) delete this->pModuleStatueTimer;
}

// 初始化成员变量
void CommonModule::startup() {
    // 定时请求连接
    connect(this->pRequestTimer, &QTimer::timeout, this, &CommonModule::sendRequestTime);
    // 定时发送CP和NP
    connect(this->pCPandNPTimer, &QTimer::timeout, this, &CommonModule::sendCPandNPStatus);
    // 定时发送模块状态
    connect(this->pModuleStatueTimer, &QTimer::timeout, this, &CommonModule::sendModuleStatus);

    // 尝试连接
    connect(this->pReconnectTimer, &QTimer::timeout, [=](){
        if(this->pTcpSocket->state() == QAbstractSocket::UnconnectedState) {
            this->pTcpSocket->connectToHost(QHostAddress(this->cfg.serverAddress), this->cfg.serverPort);
        }
        // LOGI("Attempting to reconnect...");
        qDebug() << "Attempting to reconnect...";
    });
    // 成功连接
    connect(this->pTcpSocket, &QTcpSocket::connected, [=](){
        this->isConnected = true;
        this->pReconnectTimer->stop();
        this->sendRegister();
        qDebug() << "Connected to host!";
    });
    // 断开连接
    connect(this->pTcpSocket, &QTcpSocket::disconnected, [=]() {
        this->isConnected = false;
        qDebug() << "Disconnected from server!";
    });
    // 每秒发送一次连接请求
    this->pReconnectTimer->start(1000);
}

// 接收数据
void CommonModule::onReadCommData(QByteArray& buff) {
    switch (genericHeader.packType) {
        case 0x2:  this->recvRegister(buff); break;
        case 0x4:  this->recvRequestTime(buff); break;
        case 0x40: this->recvStart(buff); break;
        case 0x41: this->recvStop(buff); break;
        case 0x42: this->recvRestart(buff); break;
        case 0x43: this->recvReset(buff); break;
        case 0x44: this->recvUpdate(buff); break;
        case 0x45: this->recvNote4Operator(buff); break;
        case 0x46: this->recvRequestModuleFigure(buff); break;
        case 0x47: this->recvSettingLang(buff); break;
        case 0x48: this->recvRadioAndSatellite(buff); break;
        case 0x49: this->recvSettingTime(buff); break;
        case 0x4A: this->recvModuleLocation(buff); break;
        case 0x4B: this->recvCustomizedParam(buff); break;
        default: {
            qDebug() << "this is unknown pkg 0x" << this->genericHeader.packType;
            this->sendLogMsg("this is unknown pkg");
            this->sendNote2Operator("this is unknown pkg");
            break;
        }
    }
}

// 0x1,发送注册消息
void CommonModule::sendRegister() {
    this->genericHeader.packType = 0x1;
    this->genericHeader.dataSize = sizeof(ModuleRegister);
    this->genericHeader.packIdx ++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0x1 register to server: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x2,确定注册
void CommonModule::recvRegister(QByteArray buff) {
    ServerRegister serverRegister;
    memcpy(&serverRegister, buff.data() + sizeof(GenericHeader), sizeof(ServerRegister));
    this->genericHeader.moduleId = serverRegister.idxModule;
    qDebug() << "the module id is " << QString::number(serverRegister.idxModule, 16);
    qDebug() << "the connection status is " << QString::number(serverRegister.errorConnect, 16);
    switch(serverRegister.errorConnect) {
        case 0x1:  qDebug() << "execeed the limited number of same type device"; break;
        case 0x2:  qDebug() << "try to reconnect same device"; break;
        case 0x4:  qDebug() << "don't support the device type"; break;
        case 0x8:  qDebug() << "don't support the prototal version"; break;
        case 0x10: qDebug() << "module id is not be supported"; break;
        case 0x20: 
        case 0x40: qDebug() << "unknown error"; break;
        case 0x0: {
            this->genericHeader.moduleId = serverRegister.idxModule;
            this->isRegister = true;
            // 注册成功发送日志
            this->sendLogMsg("register successfully");
            // 注册成果发送给操作员
            this->sendNote2Operator("register successfully");
            break;
        }
        default: {
            qDebug() << "fail to register";
        }
    }
}

// 0x3,发送时间请求
void CommonModule::sendRequestTime() {
    this->genericHeader.packType = 0x3;
    this->genericHeader.dataSize = sizeof(ModuleTimeControl);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);
    
    ModuleTimeControl moduleTimeControl;
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    moduleTimeControl.timeRequest1 = reqTimestamp & 0xFFFFFFFF;
    moduleTimeControl.timeRequest2 = (reqTimestamp >> 32) & 0xFFFFFFFF;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleTimeControl);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &genericHeader, len1);
    memcpy(data + len1, &moduleTimeControl, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x3 time request to server: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x4,确定时间请求
void CommonModule::recvRequestTime(QByteArray buff) {
    qDebug() << "recv time request and begin to check the time.";
    ServerTimeControl serverTimeControl;
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ServerTimeControl);
    memcpy(&serverTimeControl, buff.data() + len1, len2);
    quint64 timeStampRcv = QDateTime::currentMSecsSinceEpoch();
    quint64 timeStampReq = (serverTimeControl.timeRequest1 << 32) | serverTimeControl.timeRequest1;
    quint64 timeStampAns = (serverTimeControl.timeAnswer1 << 32) | serverTimeControl.timeAnswer1;

    qint64 delTime1 = timeStampAns - timeStampReq;
    qint64 delTime2 = timeStampRcv - timeStampAns;

    qint64 timeOut = (0.5 * (delTime1 - delTime2) + this->m_iStampResult) / this->m_iN;
    this->m_iN++;
    this->m_iStampResult = timeOut + this->m_iStampResult;
}

// 0x5,发送模块位置
void CommonModule::sendModuleLocation() {
    this->genericHeader.packType = 0x5;
    this->genericHeader.dataSize = sizeof(ModuleGeoLocation);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    this->isModuleLocation = true;

    ModuleGeoLocation oModulPos;
    oModulPos.typeData = 0x1;
    oModulPos.isValid = 0x1;
    oModulPos.reserve = 0x0;
    oModulPos.xLat = 0x1;
    oModulPos.yLong = 0x2;
    oModulPos.zHeight = 0x3;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(ModuleGeoLocation);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &genericHeader, len1);
    memcpy(data + len1, &oModulPos, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x5 module location: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x46,收到请求模块模块原理图
void CommonModule::recvRequestModuleFigure(QByteArray buff) {
    this->sendModuleFigure();
}

// 0x20,发送模块图
void CommonModule::sendModuleFigure() {
    this->genericHeader.packType = 0x20;
    this->genericHeader.dataSize = this->cfg.module0x20Cfg.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    this->isModuleConfigure=true;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = this->cfg.module0x20Cfg.size();
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, this->cfg.module0x20Cfg.data(), len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x20 json content is " << this->cfg.module0x20Cfg.data();
    this->pTcpSocket->write(data, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x21,发送电路元素状态
void CommonModule::sendModuleNPStatus() {
    this->genericHeader.packType = 0x21;
    this->genericHeader.dataSize = sizeof(ONPStatus);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);
    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oTimeReq.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oTimeReq.o_Header = this->genericHeader;
    // 消息体
    ONPStatus oNPStatus;
    oNPStatus.IDElem = 0;
    oNPStatus.status = 1;
    oNPStatus.workF1 = 1;
    oNPStatus.local = 0;
    oNPStatus.isImit = 0;
    oNPStatus.reserve = 0;

    quint8 len1 = sizeof(oTimeReq);
    quint8 len2 = sizeof(oNPStatus);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &oTimeReq, len1);
    memcpy(data + len1, &oNPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x21 module np status: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x22,发送电路元素受控状态
void CommonModule::sendModuleCPStatus() {
    this->genericHeader.packType = 0x22;
    this->genericHeader.dataSize = sizeof(OCPStatus);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oTimeReq.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oTimeReq.o_Header = this->genericHeader;

    // 消息体
    OCPStatus oCPStatus;
    oCPStatus.IDParam = 0;
    oCPStatus.status = 0;
    oCPStatus.size = 0;
    oCPStatus.isNewStatus = 0;
    oCPStatus.isNewValue = 0;
    oCPStatus.reserve = 0;

    quint8 len1 = sizeof(oTimeReq);
    quint8 len2 = sizeof(oCPStatus);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &oTimeReq, len1);
    memcpy(data + len1, &oCPStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x22 module cp status: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 发送0x21与0x22
void CommonModule::sendCPandNPStatus() {
    this->sendModuleNPStatus();
    this->sendModuleCPStatus();
}

// 0x24,发送模块状态
void CommonModule::sendModuleStatus() {
    this->genericHeader.packType = 0x24;
    this->genericHeader.dataSize = sizeof(OModuleStatus);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);
    
    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oTimeReq.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oTimeReq.o_Header = this->genericHeader;
    
    // 消息体
    OModuleStatus oModuleStatus;
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

    quint8 len1 = sizeof(oTimeReq);
    quint8 len2 = sizeof(oModuleStatus);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &oTimeReq, len1);
    memcpy(data + len1, &oModuleStatus, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x24 module status: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x23,发送控制命令
void CommonModule::sendControlledOrder() {
    this->genericHeader.packType = 0x23;
    this->genericHeader.dataSize = sizeof(OReqCtl);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oTimeReq.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oTimeReq.o_Header = this->genericHeader;

    // 消息体
    OReqCtl oReqCtl;
    oReqCtl.n_id_Com = 0x1;
    oReqCtl.n_code = 0;
    oReqCtl.o_Header = this->genericHeader;
    switch(oReqCtl.n_code) {
        case 0: qDebug() << "no error";break;
        case 1: qDebug() << "be execiting order";break;
        case 2: qDebug() << "incorrect number of parmeters";break;
        case 6: {
            qDebug() << "unknow type of message";
            // 无法处理，发送0x27作为响应
            this->sendExtendedOrder("unknow type of message");
            return;
        }
    }

    quint8 len1 = sizeof(oTimeReq);
    quint8 len2 = sizeof(OReqCtl);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &oTimeReq, len1);
    memcpy(data + len1, &oReqCtl, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x23 controlled order: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x27,发生扩展命令
void CommonModule::sendExtendedOrder(QString msg) {
    this->genericHeader.packType = 0x27;
    this->genericHeader.dataSize = sizeof(OModuleStatus);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oTimeReq.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oTimeReq.o_Header = this->genericHeader;

    // 消息体
    OReqCtl oReqCtl;
    oReqCtl.n_id_Com = 0x1;
    oReqCtl.n_code = 0;
    oReqCtl.o_Header = this->genericHeader;

    // 发送0x23协议失败的原因
    QByteArray utf8Bytes = msg.toUtf8();

    quint8 len1 = sizeof(oTimeReq);
    quint8 len2 = sizeof(OReqCtl);
    quint8 len3 = sizeof(utf8Bytes);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &oTimeReq, len1);
    memcpy(data + len1, &oReqCtl, len2);
    memcpy(data + len1 + len2, &utf8Bytes, len3);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x27 extended order: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 0x25,发送消息日志
void CommonModule::sendLogMsg(QString msg) {
    this->genericHeader.packType = 0x25;
    this->genericHeader.dataSize = msg.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oTimeReq.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oTimeReq.o_Header = this->genericHeader;

    // 消息体
    LogMsg logMsg;
    logMsg.IDParam = 0xffff;
    logMsg.type = 0;
    logMsg.reserve = 0;

    // 消息日志
    QByteArray utf8Bytes = msg.toUtf8();

    quint8 len1 = sizeof(oTimeReq);
    quint8 len2 = sizeof(logMsg);
    quint8 len3 = sizeof(utf8Bytes);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &oTimeReq, len1);
    memcpy(data + len1, &logMsg, len2);
    memcpy(data + len1 + len2, &utf8Bytes, len3);
    QByteArray byteArray(data, len1 + len2 + len3);
    qDebug() << "send 0x25 log msg: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2 + len3);
    this->pTcpSocket->flush();
    free(data);
}

// 0x26,发送消息给操作员
void CommonModule::sendNote2Operator(QString msg) {
    this->genericHeader.packType = 0x26;
    this->genericHeader.dataSize = msg.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    // 消息头
    OTimeReq oTimeReq;
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    oTimeReq.n_TimeReq1 = timestamp & 0xFFFFFFFF;
    oTimeReq.n_TimeReq2 = (timestamp >> 32) & 0xFFFFFFFF;
    oTimeReq.o_Header = this->genericHeader;

    // 消息日志
    QByteArray utf8Bytes = msg.toUtf8();

    quint8 len1 = sizeof(oTimeReq);
    quint8 len2 = sizeof(utf8Bytes);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &oTimeReq, len1);
    memcpy(data + len1, &utf8Bytes, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x26 msg to operator: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}



// 0x40,收到开始命令
void CommonModule::recvStart(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recvStart";
}

// 0x41,收到关闭命令
void CommonModule::recvStop(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recvStop";
}

// 0x42,收到重启命令
void CommonModule::recvRestart(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recvRestart";
}

// 0x43,收到重置命令
void CommonModule::recvReset(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recvReset";
}

// 0x44,收到更新命令
void CommonModule::recvUpdate(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recvUpdate";
}

// 0x45,收到操作员的短信
void CommonModule::recvNote4Operator(QByteArray buff) {
    QByteArray stringData = buff.right(buff.size() - sizeof(OTimeReq));
    QString msg = QString::fromUtf8(stringData);
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recive the msg from operator: " << msg;
}

// 0x47,收到设置语言
void CommonModule::recvSettingLang(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recive the setting language";
}

// 0x48,收到无线电与卫星导航
void CommonModule::recvRadioAndSatellite(QByteArray buff){
    // 发送受控状态0x23
    this->sendControlledOrder();
    qDebug() << "recive radio and statllite";
}

// 0x49,收到设置时间
void CommonModule::recvSettingTime(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();

    OTimeReq oTimeReq;
    memcpy(&oTimeReq, buff.data(), sizeof(OTimeReq));
    qDebug() << "lower byte: " << oTimeReq.n_TimeReq1;
    qDebug() << "higher byte: " << oTimeReq.n_TimeReq2;
}

// 0x4A,收到设置模块坐标
void CommonModule::recvModuleLocation(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    // 发送模块位置0x5
    this->sendModuleLocation();
}

// 0x4B,设置自定义参数的值
void CommonModule::recvCustomizedParam(QByteArray buff) {
    // 发送受控状态0x23
    this->sendControlledOrder();
    // 发送0x24模块状态
    this->sendModuleStatus();
}

}