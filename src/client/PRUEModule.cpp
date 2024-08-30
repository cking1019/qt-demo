#include "PRUEModule.hpp"

namespace NEBULA
{
PRUEModule::PRUEModule() {
    // RTM的发送者标识
    this->genericHeader.sender = 0x524542;
    this->pCurrentSettingTimer = new QTimer();
    this->pCurrentFunctionTimer = new QTimer();
    this->pCurrentStatusTimer = new QTimer();

    // 实时接收来自服务器的数据,使用基类的socket指针来接收数据，保证基类与派生类都可发送数据，但只允许派生类接收数据，然后发送给基类。
    connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &PRUEModule::onRecvData);

    // 发送RTM设置定时器,0xD21
    connect(this->pCurrentSettingTimer, &QTimer::timeout, this, &PRUEModule::sendPRUESettings);
    // 发送RTM功能定时器,0xD22
    connect(this->pCurrentFunctionTimer, &QTimer::timeout, this, &PRUEModule::sendPRUEFunction);

    // 查看RTM状态检查定时器
    connect(this->pCurrentStatusTimer, &QTimer::timeout, this, &PRUEModule::checkStatus);
    this->pCurrentStatusTimer->start();
    this->pkgsPRUE = {0x601, 0x201, 0x202};
}

PRUEModule::~PRUEModule() {
    if (this->pCurrentSettingTimer == nullptr)  delete this->pCurrentSettingTimer;
    if (this->pCurrentFunctionTimer == nullptr) delete this->pCurrentFunctionTimer;
    if (this->pCurrentStatusTimer == nullptr)   delete this->pCurrentStatusTimer;
}

void PRUEModule::checkStatus() {
    if (!this->isConnected) {
        // 重新尝试连接
        if (!this->pReconnectTimer->isActive())  this->pReconnectTimer->start(1000);
        // 断开连接后，停止发送时间请求0x01
        if (this->pRequestTimer->isActive()) this->pRequestTimer->stop();
        // 断开连接后，停止发送CP与NP021&0x22
        if (this->pCPandNPTimer->isActive()) this->pCPandNPTimer->stop();
        // 断开连接后，停止发送模块状态0x24
        if (this->pModuleStatueTimer->isActive()) this->pModuleStatueTimer->stop();

        if (this->pCurrentFunctionTimer->isActive()) this->pCurrentFunctionTimer->stop();
        if (this->pCurrentSettingTimer->isActive())  this->pCurrentSettingTimer->stop();

        if (this->isModuleConfigure) this->isModuleConfigure = false;
        if (this->isModuleLocation)  this->isModuleLocation   = false;
        return;
    }

    if (this->isRegister) {
        // 定时发送当前设置,0x823
        if (!this->pCurrentSettingTimer->isActive())  this->pCurrentSettingTimer->start(1000);
        // 定时发送当前功能,0x825
        if (!this->pCurrentFunctionTimer->isActive()) this->pCurrentFunctionTimer->start(1000);
        // 注册成功后发起心跳机制,每秒发送一次,0x3
        if (!this->pRequestTimer->isActive()) this->pRequestTimer->start(1000);
        // 注册成功后发送NP与CP,0x21&0x22
        if (!this->pCPandNPTimer->isActive()) this->pCPandNPTimer->start(5000);
        // 注册成功后定时发送模块状态,0x24
        if (!this->pModuleStatueTimer->isActive()) this->pModuleStatueTimer->start(1000);
        // 注册成功后立刻发送模块原理图,0x20
        if (!this->isModuleConfigure) this->sendModuleFigure();
        // 注册成功后立刻发送模块位置,0x5
        if (!this->isModuleLocation)  this->sendModuleLocation();
    }

    if (!this->isRegister) {
        if (this->pCurrentFunctionTimer->isActive()) this->pCurrentFunctionTimer->stop();
        if (this->pCurrentSettingTimer->isActive())  this->pCurrentSettingTimer->stop();
    }
}

// 接收数据统一接口
void PRUEModule::onRecvData() {
    QByteArray buff = this->pTcpSocket->readAll();
    memcpy(&this->genericHeader, buff.data(), sizeof(GenericHeader));
    qDebug("===============================================================");
    qDebug() << "received data from server: " << buff.toHex();
    qDebug("the size of pkg: %d", buff.size());
    qDebug("the type of pkg: %x", this->genericHeader.packType);
    qDebug("the type of pkg: %x", this->genericHeader.sender);
    qDebug("===============================================================");
    // 策略模式，根据包类型决定转发至哪个函数
    if (this->pkgsComm.contains(this->genericHeader.packType)) {
        this->onReadCommData(buff);
    }
    if (this->pkgsPRUE.contains(this->genericHeader.packType)) {
        this->onReadPRUEData(buff);
    }
}

// 从服务器中读取RTM数据
void PRUEModule::onReadPRUEData(const QByteArray& buff) {
    switch (genericHeader.packType) {
        case 0x601: this->recvUpdatePRUESetting(buff); break;
        case 0x201: this->recvSettingBanSector(buff); break;
        case 0x202: this->recvBanRadiation(buff); break;
        default: {
            QString msg = "this is unknown pkg 0x" + QString::number(this->genericHeader.packType, 16);
            this->sendLogMsg(msg);
            this->sendNote2Operator(msg);
            break;
        }
    }
}

// 0x601,收到更改PRUE设置
void PRUEModule::recvUpdatePRUESetting(const QByteArray& buff) {
    this->sendLogMsg("recv changing PRUE settings");
    ORecvTrapFixed oRecvTrapFixed;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(ORecvTrapFixed);
    memcpy(&oRecvTrapFixed, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oRecvTrapFixed), len2);
    qDebug() << "0x601 msg body: " << byteArray.toHex();
    /*
        deal with data
    */
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder(code);
}

// 0x201,收到设置辐射禁止扇区
void PRUEModule::recvSettingBanSector(const QByteArray& buff) {
    this->sendLogMsg("recv setting PRUE ban sector");
    OTrapBanSector oTrapBanSector;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(ORecvTrapFixed);
    memcpy(&oTrapBanSector, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapBanSector), len2);
    qDebug() << "0x601 msg body: " << byteArray.toHex();
    /*
        deal with data
    */
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder(code);
}

// 0x202,收到设置辐射禁止
void PRUEModule::recvBanRadiation(const QByteArray& buff) {
    this->sendLogMsg("recv setting PRUE ban sector");
    OTrapRadiationBan oTrapRadiationBan;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OTrapRadiationBan);
    memcpy(&oTrapRadiationBan, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapRadiationBan), len2);
    qDebug() << "0x601 msg body: " << byteArray.toHex();
    /*
        deal with data
    */
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder(code);
}

// 发送当前PRUE设置,0xD21
void PRUEModule::sendPRUESettings() {
    this->genericHeader.packType = 0xD21;
    this->genericHeader.dataSize = sizeof(OSendTrapFixed);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

    // 消息体
    OSendTrapFixed oSendTrapFixed;
    oSendTrapFixed.taskREB = 0;
    oSendTrapFixed.taskGeo = 0;
    oSendTrapFixed.reserve = 0;
    oSendTrapFixed.curAzREB = 0;
    oSendTrapFixed.curEpsREB = 0;
    oSendTrapFixed.kGainREB = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSendTrapFixed);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oSendTrapFixed, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0xD21: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 发送当前PRUE功能,0xD22
void PRUEModule::sendPRUEFunction() {
    this->genericHeader.packType = 0xD22;
    this->genericHeader.dataSize = sizeof(OSendTrapFixed);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

    // 消息体
    OTrapFunc oTrapFunc;
    oTrapFunc.numDiap = 0;
    oTrapFunc.isGeo = 0;
    oTrapFunc.numDiap2 = 0;
    oTrapFunc.reserve = 0;
    oTrapFunc.dTgeo = 0;
    oTrapFunc.maxPowREB = 0;
    oTrapFunc.dAzREB = 0;
    oTrapFunc.dElevREB = 0;
    oTrapFunc.azMinREB = 0;
    oTrapFunc.azMaxREB = 0;
    oTrapFunc.dAzGeo = 0;
    oTrapFunc.dElevGeo = 0;
    oTrapFunc.azMinGeo = 0;
    oTrapFunc.azMaxGeo = 0;
    oTrapFunc.minFreqREB = 0;
    oTrapFunc.maxFreqREB = 0;
    oTrapFunc.maxDFreq = 0;

    quint8 len1 = sizeof(OTrapFunc);
    quint8 len2 = sizeof(OSendTrapFixed);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oTrapFunc, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0xD22: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 发送已安装的辐射禁止扇区,0xD01
void PRUEModule::sendInstalledBanSector() {
    this->genericHeader.packType = 0xD01;
    this->genericHeader.dataSize = sizeof(OSendTrapFixed);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

    // 消息体
    OTrapBanSector oTrapBanSector;
    oTrapBanSector.time1 = 0;
    oTrapBanSector.time2 = 0;
    oTrapBanSector.num = 0;
    oTrapBanSector.reserve = 0;

    oTrapBanSector.type = 0;
    oTrapBanSector.isUse = 0;
    oTrapBanSector.isUseEps = 0;
    oTrapBanSector.isUseFrep = 0;
    oTrapBanSector.reserve2 = 0;

    oTrapBanSector.AzBegin = 0;
    oTrapBanSector.AzEnd = 0;
    oTrapBanSector.EpsBegin = 0;
    oTrapBanSector.EpsEnd = 0;
    oTrapBanSector.Freq = 0;
    oTrapBanSector.delFreq = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OTrapBanSector);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oTrapBanSector, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0xD01: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}


}  // namespace NEBULA
