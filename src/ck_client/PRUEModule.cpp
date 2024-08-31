#include "PRUEModule.hpp"

namespace NEBULA
{
PRUEModule::PRUEModule() {
    this->pkgsPRUE = {0x601, 0x201, 0x202};
    this->genericHeader = {0x524542, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};
    this->pCurrentSettingTimerD21 = new QTimer();
    this->pCurrentFunctionTimerD22 = new QTimer();
    this->pStateMachineTimer = new QTimer();

    connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &PRUEModule::onRecvData);
    connect(this->pStateMachineTimer, &QTimer::timeout, this, &PRUEModule::stateMachine);
    connect(this->pCurrentSettingTimerD21, &QTimer::timeout, this, &PRUEModule::sendPRUESettingsD21);
    connect(this->pCurrentFunctionTimerD22, &QTimer::timeout, this, &PRUEModule::sendPRUEFunctionD22);
    
    this->pStateMachineTimer->start();
}

PRUEModule::~PRUEModule() {
    if (this->pStateMachineTimer == nullptr)       delete this->pStateMachineTimer;
    if (this->pCurrentSettingTimerD21 == nullptr)  delete this->pCurrentSettingTimerD21;
    if (this->pCurrentFunctionTimerD22 == nullptr) delete this->pCurrentFunctionTimerD22;
}

void PRUEModule::stateMachine() {
    switch (this->connStatus)
    {
    case unConnected:
        if (!this->pReconnectTimer->isActive())  this->pReconnectTimer->start(1000);
        if (this->pRequestTimer03->isActive())   this->pRequestTimer03->stop();

        if (this->isModuleLocation05)  this->isModuleLocation05  = false;
        if (this->isModuleConfigure20) this->isModuleConfigure20 = false;
        break;
    case connecting:
        break;
    case connected:
        if (this->pReconnectTimer->isActive())  this->pReconnectTimer->stop();
        if (!this->pRequestTimer03->isActive()) this->pRequestTimer03->start(1000);
        break;
    default:
        break;
    }

    switch (this->registerStatus)
    {
    case unRegister:
        if (this->pNPTimer21->isActive())               this->pNPTimer21->stop();
        if (this->pCPTimer22->isActive())               this->pCPTimer22->stop();
        if (this->pModuleStatueTimer24->isActive())     this->pModuleStatueTimer24->stop();
        if (this->pCurrentSettingTimerD21->isActive())  this->pCurrentSettingTimerD21->stop();
        if (this->pCurrentFunctionTimerD22->isActive()) this->pCurrentFunctionTimerD22->stop();
        break;
    case registering:
        break;
    case registered:
        if (!this->isModuleLocation05)  this->sendModuleLocation05();
        if (!this->isModuleConfigure20) this->sendModuleFigure20();
        if (!this->pNPTimer21->isActive())  this->pNPTimer21->start(5000);
        if (!this->pCPTimer22->isActive())  this->pCPTimer22->start(5000);
        if (!this->pModuleStatueTimer24->isActive()) this->pModuleStatueTimer24->start(1000);
        if (!this->pCurrentSettingTimerD21->isActive())  this->pCurrentSettingTimerD21->start(1000);
        if (!this->pCurrentFunctionTimerD22->isActive()) this->pCurrentFunctionTimerD22->start(1000);
    default:
        break;
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
        case 0x601: this->recvUpdatePRUESetting601(buff); break;
        case 0x201: this->recvSettingBanSector201(buff); break;
        case 0x202: this->recvBanRadiation202(buff); break;
        default: {
            // QString msg = "this is unknown pkg 0x" + QString::number(this->genericHeader.packType, 16);
            // this->sendLogMsg25(msg);
            // this->sendNote2Operator26(msg);
            break;
        }
    }
}

// 0x601,收到更改PRUE设置
void PRUEModule::recvUpdatePRUESetting601(const QByteArray& buff) {
    // this->sendLogMsg25("recv changing PRUE settings");
    ORecvTrapFixed oRecvTrapFixed;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(ORecvTrapFixed);
    memcpy(&oRecvTrapFixed, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oRecvTrapFixed), len2);
    qDebug() << "recv 0x601:" << byteArray.toHex();
    /*
        deal with data
    */
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

// 0x201,收到设置辐射禁止扇区
void PRUEModule::recvSettingBanSector201(const QByteArray& buff) {
    // this->sendLogMsg25("recv setting PRUE ban sector");
    OTrapBanSector oTrapBanSector;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(ORecvTrapFixed);
    memcpy(&oTrapBanSector, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapBanSector), len2);
    qDebug() << "recv 0x601:" << byteArray.toHex();
    /*
        deal with data
    */
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

// 0x202,收到设置辐射禁止
void PRUEModule::recvBanRadiation202(const QByteArray& buff) {
    // this->sendLogMsg25("recv setting PRUE ban sector");
    OTrapRadiationBan oTrapRadiationBan;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OTrapRadiationBan);
    memcpy(&oTrapRadiationBan, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapRadiationBan), len2);
    qDebug() << "recv 0x601:" << byteArray.toHex();
    /*
        deal with data
    */
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

// 发送当前PRUE设置,0xD21
void PRUEModule::sendPRUESettingsD21() {
    this->genericHeader.packType = 0xD21;
    this->genericHeader.dataSize = sizeof(OSendTrapFixed);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0xD21:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 发送当前PRUE功能,0xD22
void PRUEModule::sendPRUEFunctionD22() {
    this->genericHeader.packType = 0xD22;
    this->genericHeader.dataSize = sizeof(OSendTrapFixed);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0xD22:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 发送已安装的辐射禁止扇区,0xD01
void PRUEModule::sendInstalledBanSectorD01() {
    this->genericHeader.packType = 0xD01;
    this->genericHeader.dataSize = sizeof(OSendTrapFixed);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0xD01:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}


}  // namespace NEBULA
