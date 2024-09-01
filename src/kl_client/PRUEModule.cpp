#include "PRUEModule.hpp"

namespace NEBULA
{
PRUEModule::PRUEModule() {
    this->pkgsPRUE = {0x601, 0x201, 0x202};
    this->genericHeader = {0x524542, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};
    
    this->pStateMachineTimer =       new QTimer();
    this->pCurrentSettingTimerD21 =  new QTimer();
    this->pCurrentFunctionTimerD22 = new QTimer();

    connect(this->pTcpSocket,         &QTcpSocket::readyRead, this, &PRUEModule::onRecvData);
    connect(this->pStateMachineTimer,       &QTimer::timeout, this, &PRUEModule::stateMachine);
    connect(this->pCurrentSettingTimerD21,  &QTimer::timeout, this, &PRUEModule::sendPRUESettingsD21);
    connect(this->pCurrentFunctionTimerD22, &QTimer::timeout, this, &PRUEModule::sendPRUEFunctionD22);
    
    this->pStateMachineTimer->start();
}

PRUEModule::~PRUEModule() {
    if (this->pStateMachineTimer == nullptr)       delete this->pStateMachineTimer;
    if (this->pCurrentSettingTimerD21 == nullptr)  delete this->pCurrentSettingTimerD21;
    if (this->pCurrentFunctionTimerD22 == nullptr) delete this->pCurrentFunctionTimerD22;
}

// 状态机
void PRUEModule::stateMachine() {
    // 连接状态
    switch (this->connStatus)
    {
    case ConnStatus::unConnected:
        if (!this->pReconnectTimer->isActive())                 this->pReconnectTimer->start(1000);
        if (this->isSendRegister01)                             this->isSendRegister01 = false;
        if (this->registerStatus == RegisterStatus::registered) this->registerStatus = RegisterStatus::unRegister;
        break;
    case ConnStatus::connecting:
        break;
    case ConnStatus::connected:
        if (this->pReconnectTimer->isActive())    this->pReconnectTimer->stop();
        if (!this->isSendRegister01)              this->sendRegister01();
        break;
    default:
        break;
    }
    // 注册状态
    switch (this->registerStatus)
    {
    case RegisterStatus::unRegister:
        if (this->pRequestTimer03->isActive())          this->pRequestTimer03->stop();
        if (this->isModuleLocation05)                   this->isModuleLocation05  = false;
        if (this->isModuleConfigure20)                  this->isModuleConfigure20 = false;
        if (this->pNPTimer21->isActive())               this->pNPTimer21->stop();
        if (this->pCPTimer22->isActive())               this->pCPTimer22->stop();
        if (this->pModuleStatueTimer24->isActive())     this->pModuleStatueTimer24->stop();
        if (this->pCurrentSettingTimerD21->isActive())  this->pCurrentSettingTimerD21->stop();
        if (this->pCurrentFunctionTimerD22->isActive()) this->pCurrentFunctionTimerD22->stop();
        if (this->timeStatus == TimeStatus::timed)      this->timeStatus = TimeStatus::unTime;
        break;
    case RegisterStatus::registering:
        break;
    case RegisterStatus::registered:
        if (!this->pRequestTimer03->isActive())          this->pRequestTimer03->start(1000);
        if (!this->isModuleLocation05)                   this->sendModuleLocation05();
        if (!this->isModuleConfigure20)                  this->sendModuleFigure20();
        if (!this->pNPTimer21->isActive())               this->pNPTimer21->start(5000);
        if (!this->pCPTimer22->isActive())               this->pCPTimer22->start(5000);
        if (!this->pModuleStatueTimer24->isActive())     this->pModuleStatueTimer24->start(1000);
        if (!this->pCurrentSettingTimerD21->isActive())  this->pCurrentSettingTimerD21->start(1000);
        if (!this->pCurrentFunctionTimerD22->isActive()) this->pCurrentFunctionTimerD22->start(1000);
        break;
    default:
        break;
    }
    // 对时状态
    switch (this->timeStatus)
    {
    case TimeStatus::unTime: break;
    case TimeStatus::timing: break;
    case TimeStatus::timed:  break;
    default:
        break;
    }
}

void PRUEModule::onRecvData() {
    QByteArray buff = this->pTcpSocket->readAll();
    GenericHeader genericHeader2;
    memcpy(&genericHeader2, buff.data(), sizeof(GenericHeader));
    qint16 pkgID = genericHeader2.packType;
    qDebug("===================================================================");
    qDebug().nospace().noquote() << "recv 0x" << QString(pkgID).toUtf8().toHex() << ": " << buff.toHex();
    // qDebug("the size of pkg: %d", buff.size());
    // qDebug("the type of pkg: %x", this->genericHeader.packType);
    qDebug("===================================================================");
    // 策略模式，根据包类型决定转发至哪个函数
    if (this->pkgsComm.contains(this->genericHeader.packType)) {
        this->onReadCommData(pkgID, buff);
    }
    if (this->pkgsPRUE.contains(this->genericHeader.packType)) {
        this->onReadPRUEData(pkgID, buff);
    }
}

void PRUEModule::onReadPRUEData(qint16 pkgID, const QByteArray& buff) {
    switch (pkgID) {
        case 0x201: this->recvSettingBanSector201(buff);  break;
        case 0x202: this->recvBanRadiation202(buff);      break;
        case 0x601: this->recvUpdatePRUESetting601(buff); break;
        default: {
            break;
        }
    }
}

void PRUEModule::recvSettingBanSector201(const QByteArray& buff) {
    OTrapBanSector oTrapBanSector;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(ORecvTrapFixed);
    memcpy(&oTrapBanSector, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapBanSector), len2);
    qDebug() << "recv 0x601:" << byteArray.toHex();
    /*
        deal with data
    */
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

void PRUEModule::recvBanRadiation202(const QByteArray& buff) {
    OTrapRadiationBan oTrapRadiationBan;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OTrapRadiationBan);
    memcpy(&oTrapRadiationBan, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapRadiationBan), len2);
    qDebug() << "recv 0x601:" << byteArray.toHex();
    /*
        deal with data
    */
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

void PRUEModule::recvUpdatePRUESetting601(const QByteArray& buff) {
    ORecvTrapFixed oRecvTrapFixed;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(ORecvTrapFixed);
    memcpy(&oRecvTrapFixed, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oRecvTrapFixed), len2);
    qDebug() << "recv 0x601:" << byteArray.toHex();
    /*
        deal with data
    */
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

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



}  // namespace NEBULA
