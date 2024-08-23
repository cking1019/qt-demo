#include "RTMController.hpp"

RTMController::RTMController(QObject *parent):QObject(parent){
}

RTMController::~RTMController(){
    if(this->pNebulaController != nullptr) delete this->pNebulaController;
    if(this->pRecvTimer != nullptr)        delete this->pRecvTimer; 
    if(this->pSendTimer != nullptr)        delete this->pSendTimer; 
    if(this->pRequestTimer != nullptr)     delete this->pRequestTimer; 
    if(this->pTcpSocket != nullptr)        delete this->pTcpSocket;
}

// 初始化成员变量
void RTMController::init() {
    this->pNebulaController = new NebulaController();
    this->pRecvTimer = new QTimer();
    this->pSendTimer = new QTimer();
    this->pRequestTimer = new QTimer();
    this->pTcpSocket = new QTcpSocket(this);

    this->packIdx = 0;
    this->m_moduleIdx = 0;
    this->m_iConnectHostFlag = 0;
    this->m_iStampResult = 0;
    this->m_iN = 1;
    
    connect(this, &RTMController::signals_msg,  this, &RTMController::onReadData);
    connect(this->pSendTimer, &QTimer::timeout, this, &RTMController::sendTime2Ctl);
    connect(this->pRequestTimer, &QTimer::timeout, this, &RTMController::sendRequestTime);
    // 初始化socket
    this->initSocket();
    // 实时接收来自服务器的数据
    connect(this->pRecvTimer, &QTimer::timeout, this, &RTMController::onReadData);
    this->pRecvTimer->start(1000);
}

// 初始化socket
void RTMController::initSocket() {
    // int retries = 0;
    // while(true) {
    //     qDebug() << "Attempting to reconnect... Retry attempt: " << retries + 1;
    //     this->pTcpSocket->connectToHost(QHostAddress::LocalHost, 1234);
    //     if(!this->pTcpSocket->waitForConnected(2000)) {
    //         retries++;
    //         continue;
    //     }
    //     qDebug() << "Connected to host!";
    //     break;
    // }
    this->pTcpSocket->connectToHost(QHostAddress::LocalHost, 1234);
    connect(this->pTcpSocket, &QTcpSocket::connected, [=](){
        qDebug() << "Connected to host!";
        this->sendRegister();
    });

    connect(this->pTcpSocket, &QTcpSocket::disconnected, [=]() {
        qDebug() << "Disconnected from server!";
        this->pRequestTimer->stop();
    });
    
}

// 移除连接
// void RTMController::removeConnectionBySocket() {
//     // 重连发起连接服务器
//     this->initSocket();
//     // 连接成功后发送注册消息
//     this->sendRegister();
// }

// 发送数据给服务器
void RTMController::sendTime2Ctl() {
    std::string msg = "hello! I am RTM client.";
    QByteArray buff(msg.c_str(), static_cast<int>(msg.length()));
    // 如果成功，则返回写入的字节数量；如果失败，则返回-1.
    qint64 cnt = this->pTcpSocket->write(buff, buff.size());
}

// 接收服务器的数据
void RTMController::onReadData() {
    QByteArray buff = this->pTcpSocket->readAll();
    qDebug() << "received data from server: " << QString(buff);
    GenericHeader genericHeader;
    memcpy(&genericHeader, buff.data(), sizeof(GenericHeader));
    // 假设解包后收到了注册包类型
    genericHeader.packType = 0x2;
    switch (genericHeader.packType)
    {
    case 0x2:
        this->recvRegister(buff);
        break;
    case 0x4:
        this->recvRequestTime(buff);
        break;
    default:
        break;
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
    moduleRegister.isAsku = 0x0;

    char* data = (char*)malloc(sizeof(GenericHeader) + sizeof(ModuleRegister));
    memcpy(data, &genericHeader, sizeof(genericHeader));
    memcpy(data + sizeof(genericHeader), &moduleRegister, sizeof(moduleRegister));
    qint64 cnt = this->pTcpSocket->write(data, sizeof(genericHeader) + sizeof(moduleRegister));
    this->pTcpSocket->flush();
    this->packIdx += 1;
    free(data);
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
    qint64 timestamp2 = QDateTime::currentSecsSinceEpoch();
    moduleTimeControl.timeRequest1 = timestamp & 0xFFFFFFFF;
    moduleTimeControl.timeRequest2 = (timestamp >> 32) & 0xFFFFFFFF;

    char* data = (char*)malloc(sizeof(GenericHeader) + sizeof(ModuleTimeControl));
    qint64 cnt = this->pTcpSocket->write(data, sizeof(genericHeader) + sizeof(moduleTimeControl));
    this->pTcpSocket->flush();
    this->packIdx += 1;
    free(data);
}

// 0x2,确定注册
void RTMController::recvRegister(QByteArray buff) {
    
    ServerRegister serverRegister;
    memcpy(&serverRegister, buff.data(), sizeof(ServerRegister));
    // serverRegister.errorConnect = 0x0;
    switch(serverRegister.errorConnect) {
        case 0x1: qDebug() << "execeed the limited number of same type device"; break;
        case 0x2: qDebug() << "try to reconnect same device"; break;
        case 0x3: qDebug() << "don't support the device type"; break;
        case 0x4: qDebug() << "don't support the prototal version"; break;
        case 0x5: qDebug() << "module id is not be supported"; break;
        case 0x6: 
        case 0x7: qDebug() << "unknown error"; break;
        case 0x0: {
            serverRegister.errorConnect = 0x0;
            this->m_moduleIdx = serverRegister.idxModule;
            this->m_iConnectHostFlag = 1;
            // 注册成功后发起心跳机制，每秒发送一次
            this->pRequestTimer->start(1000);
            qDebug() << "send request time every seconed";
            break;
        }
        default: {
        }
    }
}

// 0x4,确定时间请求
void RTMController::recvRequestTime(QByteArray buff) {
    ServerTimeControl serverTimeControl;
    
    memcpy(&serverTimeControl, buff.data(), sizeof(ServerTimeControl));
    quint64 timeStampRcv = QDateTime::currentMSecsSinceEpoch();
    quint64 timeStampReq = ((quint64)serverTimeControl.timeRequest2 << 32) | serverTimeControl.timeRequest1;
    quint64 timeStampAns = ((quint64)serverTimeControl.timeAnswer2 << 32) | serverTimeControl.timeAnswer1;

    qint64 delTime1 = timeStampAns - timeStampReq;
    qint64 delTime2 = timeStampRcv - timeStampAns;

    qint64 timeOut = (0.5 * (delTime1 - delTime2) + this->m_iStampResult) / this->m_iN;
    this->m_iN++;
    this->m_iStampResult = timeOut + this->m_iStampResult;
}