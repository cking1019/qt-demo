#include "RTMController.hpp"

RTMController::RTMController(QObject *parent):QObject(parent){
}

RTMController::~RTMController(){
    if(this->pTcpSocket != nullptr)        delete this->pTcpSocket;
    if(this->pNebulaController != nullptr) delete this->pNebulaController;
    if(this->pRequestTimer != nullptr)     delete this->pRequestTimer; 
    if(this->pReconnectTimer != nullptr)   delete this->pReconnectTimer; 
}

// 初始化成员变量
void RTMController::init() {
    this->pTcpSocket = new QTcpSocket(this);
    this->pNebulaController = new NebulaController();

    this->pRequestTimer = new QTimer();
    this->pReconnectTimer = new QTimer();

    this->packIdx = 0;
    this->m_moduleIdx = 0;
    this->m_iConnectHostFlag = 0;
    this->m_iStampResult = 0;
    this->m_iN = 1;
    
    // 定时再次请求连接
    connect(this->pRequestTimer, &QTimer::timeout, this, &RTMController::sendRequestTime);
    // 实时接收来自服务器的数据
    connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &RTMController::onReadData);

    // 初始化socket
    this->initSocket();
}

// 初始化socket
void RTMController::initSocket() {
    // 尝试连接
    connect(this->pReconnectTimer, &QTimer::timeout, [&](){
        if(this->pTcpSocket->state() == QAbstractSocket::UnconnectedState) {
            this->pTcpSocket->connectToHost(QHostAddress::LocalHost, 1234);
        }
        qDebug() << "Attempting to reconnect...";
    });
    // 成功连接
    connect(this->pTcpSocket, &QTcpSocket::connected, [&](){
        qDebug() << "Connected to host!";
        this->pReconnectTimer->stop();
        this->sendRegister();
    });
    // 断开连接
    connect(this->pTcpSocket, &QTcpSocket::disconnected, [=]() {
        qDebug() << "Disconnected from server!";
        // 重新尝试连接
        this->pReconnectTimer->start(2000);
        // 断开连接后，停止发送时间请求
        this->pRequestTimer->stop();
    });
    this->pReconnectTimer->start(2000);
}

// 接收数据
void RTMController::onReadData() {
    QByteArray buff = this->pTcpSocket->readAll();
    qDebug() << buff.toHex();
    qDebug() << "received data from server: " << buff.toHex();
    GenericHeader genericHeader;
    memcpy(&genericHeader, buff.data(), sizeof(GenericHeader));
    qDebug() << "received data from server: " << genericHeader.sender;
    switch (genericHeader.packType)
    {
        case 0x2: this->recvRegister(buff); break;
        case 0x4: this->recvRequestTime(buff); break;
        default:break;
    }
}

// 0x1,发送注册消息
void RTMController::sendRegister() {
    GenericHeader genericHeader;
    genericHeader.sender = 0x564950;
    genericHeader.moduleId = 0x0;
    genericHeader.vMajor = 0x2;
    genericHeader.vMinor = 0x2;
    genericHeader.packIdx = this->packIdx;
    genericHeader.dataSize = sizeof(ModuleRegister);
    genericHeader.isAsku = 0x1;
    genericHeader.packType = 0x1;
    genericHeader.checkSum = CommonBase::calcChcekSum((char*)&genericHeader, sizeof(genericHeader) - 2);

    ModuleRegister moduleRegister;
    moduleRegister.idManuf = 0x1;
    moduleRegister.serialNum = 0x0;
    moduleRegister.versHardMaj = 0x0;
    moduleRegister.versHardMin = 0x0;
    moduleRegister.versProgMaj = 0x0;
    moduleRegister.isInfo = 0x0;
    moduleRegister.versProgMin = 0x0;
    moduleRegister.isAsku = 0x1;

    char* data = (char*)malloc(sizeof(GenericHeader) + sizeof(ModuleRegister));
    memcpy(data, &genericHeader, sizeof(genericHeader));
    memcpy(data + sizeof(genericHeader), &moduleRegister, sizeof(moduleRegister));
    qint64 cnt = this->pTcpSocket->write(data, sizeof(genericHeader) + sizeof(moduleRegister));

    this->pTcpSocket->flush();
    this->packIdx += 1;
    free(data);
}

// 0x2,确定注册
void RTMController::recvRegister(QByteArray buff) {
    ServerRegister serverRegister;
    memcpy(&serverRegister, buff.data() + sizeof(GenericHeader), sizeof(ServerRegister));
    switch(serverRegister.errorConnect & 0xff) {
        case 0x1:  qDebug() << "execeed the limited number of same type device"; break;
        case 0x2:  qDebug() << "try to reconnect same device"; break;
        case 0x4:  qDebug() << "don't support the device type"; break;
        case 0x8:  qDebug() << "don't support the prototal version"; break;
        case 0x10: qDebug() << "module id is not be supported"; break;
        case 0x20: 
        case 0x40: qDebug() << "unknown error"; break;
        case 0x0: {
            serverRegister.errorConnect = 0x0;
            this->m_moduleIdx = serverRegister.idxModule;
            this->m_iConnectHostFlag = 1;
            // 注册成功后发起心跳机制，每秒发送一次
            this->pRequestTimer->start(1000);
            qDebug() << "heartbeat detection";
            break;
        }
        default: {
        }
    }
}


// 0x3,发送时间请求
void RTMController::sendRequestTime() {
    GenericHeader genericHeader;
    genericHeader.sender = 0x564950;
    genericHeader.moduleId = 0x0;
    genericHeader.vMajor = 0x2;
    genericHeader.vMinor = 0x2;
    genericHeader.packIdx = this->packIdx;
    genericHeader.dataSize = sizeof(ModuleRegister);
    genericHeader.isAsku = 0x1;
    genericHeader.packType = 0x1;
    genericHeader.checkSum = CommonBase::calcChcekSum((char*)&genericHeader, sizeof(genericHeader) - 2);
    ModuleTimeControl moduleTimeControl;
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    moduleTimeControl.timeRequest1 = timestamp & 0xFFFFFFFF;
    moduleTimeControl.timeRequest2 = (timestamp >> 32) & 0xFFFFFFFF;

    char* data = (char*)malloc(sizeof(GenericHeader) + sizeof(ModuleTimeControl));
    memcpy(data, &genericHeader, sizeof(genericHeader));
    memcpy(data + sizeof(genericHeader), &moduleTimeControl, sizeof(moduleTimeControl));
    qint64 cnt = this->pTcpSocket->write(data, sizeof(genericHeader) + sizeof(moduleTimeControl));

    this->pTcpSocket->flush();
    this->packIdx++;
    free(data);
}


// 0x4,确定时间请求
void RTMController::recvRequestTime(QByteArray buff) {
    ServerTimeControl serverTimeControl;
    memcpy(&serverTimeControl, buff.data() + sizeof(GenericHeader), sizeof(serverTimeControl));
    quint64 timeStampRcv = QDateTime::currentMSecsSinceEpoch();
    quint64 timeStampReq = ((quint64)serverTimeControl.timeRequest2 << 32) | serverTimeControl.timeRequest1;
    quint64 timeStampAns = ((quint64)serverTimeControl.timeAnswer2 << 32) | serverTimeControl.timeAnswer1;

    qint64 delTime1 = timeStampAns - timeStampReq;
    qint64 delTime2 = timeStampRcv - timeStampAns;

    qint64 timeOut = (0.5 * (delTime1 - delTime2) + this->m_iStampResult) / this->m_iN;
    this->m_iN++;
    this->m_iStampResult = timeOut + this->m_iStampResult;
}