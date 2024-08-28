#include "CommonModule.hpp"

// 计算包头校验和
quint16 calcChcekSum(const char* sMess,int nCnt) {
    quint16 nSum = 0;
    quint8 *pb = (quint8*)sMess;
    for(int i = 0; i < nCnt; i++) nSum += *pb++;
    return nSum;
}

namespace NEBULA
{

CommonModule::CommonModule(QObject *parent):QObject(parent){
    // LOGI("this is a test");
}

CommonModule::~CommonModule(){
    if(this->pTcpSocket != nullptr)        delete this->pTcpSocket;
    if(this->pRequestTimer != nullptr)     delete this->pRequestTimer; 
    if(this->pReconnectTimer != nullptr)   delete this->pReconnectTimer; 
    if(this->pCPandNPTimer != nullptr)     delete this->pCPandNPTimer; 
}

// 初始化成员变量
void CommonModule::startup() {
    this->pTcpSocket = new QTcpSocket(this);

    this->pRequestTimer = new QTimer();
    this->pReconnectTimer = new QTimer();
    this->pCPandNPTimer = new QTimer();

    this->m_iConnectHostFlag = false;
    this->m_iStampResult = 0;
    this->m_iN = 1;

    this->genericHeader.sender = 0x50454C; // 对应ID_PEL的id
    this->genericHeader.moduleId = 0xff;
    this->genericHeader.vMajor = 0x02;
    this->genericHeader.vMinor = 0x02;
    this->genericHeader.packIdx = 0;
    this->genericHeader.dataSize = 0;
    this->genericHeader.isAsku = 1;
    this->genericHeader.packType = 0;
    this->genericHeader.checkSum = 0;

    // 定时请求连接
    connect(this->pRequestTimer, &QTimer::timeout, this, &CommonModule::sendRequestTime);
    // 定时发送CP和NP
    connect(this->pCPandNPTimer, &QTimer::timeout, this, &CommonModule::sendCPandNPStatus);
    // 初始化socket
    this->initSocket();
}

// 初始化socket
void CommonModule::initSocket() {
    qDebug() << "server address is " << this->cfg.serverAddress;
    qDebug() << "server port is " << this->cfg.serverPort;

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
        qDebug() << "Connected to host!";
        // LOGI("Connected to host!");
        this->pReconnectTimer->stop();
        this->sendRegister();
        // 实时接收来自服务器的数据
        connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &CommonModule::onReadData);
    });
    // 断开连接
    connect(this->pTcpSocket, &QTcpSocket::disconnected, [=]() {
        qDebug() << "Disconnected from server!";
        // LOGI("Disconnected from server!");
        // 重新尝试连接
        this->pReconnectTimer->start(1000);
        // 断开连接后，停止发送时间请求
        this->pRequestTimer->stop();
        // 断开连接后，停止发送CP与NP
        this->pCPandNPTimer->stop();
    });
    this->pReconnectTimer->start(1000);
}

// 接收数据
void CommonModule::onReadData() {
    QByteArray buff = this->pTcpSocket->readAll();
    memcpy(&this->genericHeader, buff.data(), sizeof(GenericHeader));
    qDebug() << "=======================================";
    qDebug() << "received data from server: " << buff.toHex();
    qDebug() << "the size of pkg: " << buff.size();
    qDebug() << "the type of pkg: " << QString::number(this->genericHeader.packType, 16);
    qDebug() << "the sender is: " << QString::number(this->genericHeader.sender, 16);
    qDebug() << "=======================================";
    // LOGI("received data from server: " + buff.toHex());
    switch (genericHeader.packType) {
        case 0x2: this->recvRegister(buff); break;
        case 0x4: this->recvRequestTime(buff); break;
        case 0x46: this->recvRequestModuleFigure(buff); break;
        case 0x40: this->recvStart(buff);break;
        case 0x41: this->recvStop(buff);break;
        case 0x42: this->recvRestart(buff);break;
        case 0x43: this->recvReset(buff);break;
        case 0x44: this->recvUpdate(buff);break;
        default: {
            qDebug() << "this is unknown pkg 0x" << this->genericHeader.packType;
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
    qDebug() << "send register to server: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}

// 494f5601 02020000 04000000 0200f900 | 00000000
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
            this->m_iConnectHostFlag = true;
            // 注册成功后发起心跳机制，每秒发送一次
            this->pRequestTimer->start(1000);
            // 注册成功后发送NP与CP
            this->pCPandNPTimer->start(5000);
            // 注册成功后立刻发送模块原理图
            this->sendModuleFigure();
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
    qDebug() << "send register to server: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    this->genericHeader.packIdx++;
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
    qDebug() << "send module location: " << byteArray.toHex();
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

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = this->cfg.module0x20Cfg.size();
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, this->cfg.module0x20Cfg.data(), len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send module location: " << byteArray.toHex();
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
    qDebug() << "send module np status";
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
    qDebug() << "send module location: " << byteArray.toHex();
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
    qDebug() << "send module cp status";
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
    qDebug() << "send module status";
    
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
    qDebug() << "send module status: " << byteArray.toHex();
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
    qDebug() << "send log message";

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
    qDebug() << "send log msg: " << byteArray.toHex();
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
    qDebug() << "send message to operator";

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
    qDebug() << "send msg: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    free(data);
}



// 0x40,收到开始命令
void CommonModule::recvStart(QByteArray buff) {
    qDebug() << "recvStart";
}

// 0x41,收到关闭命令
void CommonModule::recvStop(QByteArray buff) {
    qDebug() << "recvStop";
}

// 0x42,收到重启命令
void CommonModule::recvRestart(QByteArray buff) {
    qDebug() << "recvRestart";
}

// 0x43,收到重置命令
void CommonModule::recvReset(QByteArray buff) {
    qDebug() << "recvReset";
}

// 0x44,收到更新命令
void CommonModule::recvUpdate(QByteArray buff) {
    qDebug() << "recvUpdate";
}

}