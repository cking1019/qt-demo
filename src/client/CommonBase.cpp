#include "CommonBase.hpp"

// 计算包头校验和
qint16 calcChcekSum(const char* sMess,int nCnt) {
    quint16 nSum = 0;
    quint8 *pb = (quint8*)sMess;
    for(int i = 0; i < nCnt; i++) nSum += *pb++;
    return nSum;
}

// 将结构体转换为十六进制字符串
template<typename T>
QString structToHexString(const T &myStruct) {
    // 计算结构体的大小
    size_t size = sizeof(myStruct);
    // 创建一个字节数组
    QByteArray byteArray(size, 0);
    // 将结构体内容复制到字节数组
    memcpy(byteArray.data(), &myStruct, size);
    // 将字节数组转换为十六进制字符串
    return byteArray.toHex();
}

CommonBase::CommonBase(QObject *parent):QObject(parent){
}

CommonBase::~CommonBase(){
    if(this->pTcpSocket != nullptr)        delete this->pTcpSocket;
    if(this->pNebulaController != nullptr) delete this->pNebulaController;
    if(this->pRequestTimer != nullptr)     delete this->pRequestTimer; 
    if(this->pReconnectTimer != nullptr)   delete this->pReconnectTimer; 
}

// 初始化成员变量
void CommonBase::init() {
    this->pTcpSocket = new QTcpSocket(this);
    this->pNebulaController = new NebulaController();

    this->pRequestTimer = new QTimer();
    this->pReconnectTimer = new QTimer();

    this->m_iConnectHostFlag = 0;
    this->m_iStampResult = 0;
    this->m_iN = 1;

    this->genericHeader.sender = 0x564950;
    this->genericHeader.moduleId = 0;
    this->genericHeader.vMajor = 0x02;
    this->genericHeader.vMinor = 0x02;
    this->genericHeader.packIdx = 0;
    this->genericHeader.dataSize = 0;
    this->genericHeader.isAsku = 1;
    this->genericHeader.packType = 0;
    this->genericHeader.checkSum = 0;

    // 定时再次请求连接
    connect(this->pRequestTimer, &QTimer::timeout, this, &CommonBase::sendRequestTime);
    // 实时接收来自服务器的数据
    connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &CommonBase::onReadData);

    // 初始化socket
    this->initSocket();
}

// 初始化socket
void CommonBase::initSocket() {
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
void CommonBase::onReadData() {
    QByteArray buff = this->pTcpSocket->readAll();
    memcpy(&this->genericHeader, buff.data(), sizeof(GenericHeader));
    qDebug() << "received data from server: " << buff.toHex() << ", the size of pkg: " << buff.size() 
             << ", the type of pkg: " << this->genericHeader.packType;
    switch (genericHeader.packType)
    {
        case 0x2: this->recvRegister(buff); break;
        case 0x4: this->recvRequestTime(buff); break;
        case 0x46: this->recvRequestModuleFigure(buff); break;
        default: {
            qDebug() << "this is unknown pkg 0x" << this->genericHeader.packType;
            break;
        }
    }
}

// 0x1,发送注册消息
void CommonBase::sendRegister() {
    this->genericHeader.dataSize = sizeof(ModuleRegister);
    this->genericHeader.packType = 0x1;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);
    qDebug() << structToHexString(this->genericHeader);

    ModuleRegister moduleRegister;
    moduleRegister.idManuf = 0x1;
    moduleRegister.serialNum = 0x0;
    moduleRegister.versHardMaj = this->genericHeader.vMajor;
    moduleRegister.versHardMin = this->genericHeader.vMinor;
    moduleRegister.versProgMaj = 0x0;
    moduleRegister.isInfo = 0x0;
    moduleRegister.versProgMin = 0x0;
    moduleRegister.isAsku = this->genericHeader.isAsku;

    qint16 len1 = sizeof(GenericHeader);
    qint16 len2 = sizeof(ModuleRegister);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &moduleRegister, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send register to server: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray, len1 + len2);
    this->pTcpSocket->flush();
    this->genericHeader.packIdx += 1;
    free(data);
}

// 0x2,确定注册
void CommonBase::recvRegister(QByteArray buff) {
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
            this->genericHeader.moduleId = serverRegister.idxModule;
            this->m_iConnectHostFlag = true;
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
void CommonBase::sendRequestTime() {
    this->genericHeader.dataSize = sizeof(ModuleTimeControl);
    this->genericHeader.packType = 0x3;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);
    
    ModuleTimeControl moduleTimeControl;
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    moduleTimeControl.timeRequest1 = timestamp & 0xFFFFFFFF;
    moduleTimeControl.timeRequest2 = (timestamp >> 32) & 0xFFFFFFFF;

    qint16 len1 = sizeof(GenericHeader);
    qint16 len2 = sizeof(ModuleTimeControl);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &genericHeader, len1);
    memcpy(data + len1, &moduleTimeControl, len2);
    this->pTcpSocket->write(data, len1 + len2);
    this->pTcpSocket->flush();
    this->genericHeader.packIdx++;
    free(data);
}

// 0x4,确定时间请求
void CommonBase::recvRequestTime(QByteArray buff) {
    qDebug() << "recv time request and begin to check the time.";
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

// 0x46,收到请求模块模块原理图
void CommonBase::recvRequestModuleFigure(QByteArray buff) {
    this->sendModuleFigure();
}

// 0x20,发送模块图
void CommonBase::sendModuleFigure() {
    // 读取配置文件
    QSettings *relayConfig = new QSettings(RELAY_PATH, QSettings::IniFormat);
    relayConfig->setIniCodec(QTextCodec::codecForName("utf-8"));

    QString Dev0x20Config = relayConfig->value("DetectDev1/Dev0x20Config").toString();
    QFile file(Dev0x20Config);
    QString jsonContent;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        jsonContent = in.readAll();
        qDebug() << jsonContent;
        file.close();
    }
    // 设置公共包头参数
    this->genericHeader.packType = 0x20;
    this->genericHeader.dataSize = jsonContent.size();
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(this->genericHeader) - 2);

    qint16 len1 = sizeof(GenericHeader);
    qint16 len2 = jsonContent.size();
    char data[len1 + len2];
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, jsonContent.data(), len2);
    this->pTcpSocket->write(data, len1 + len2);
    this->pTcpSocket->flush();
}