#include "PRUEModule.hpp"

namespace NEBULA
{
PRUEModule::PRUEModule() {
    this->pkgsPRUE = {0x601, 0x201, 0x202};
    this->m_genericHeader = {0x524542, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};
    
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
        if (this->m_isSendRegister01)                             this->m_isSendRegister01 = false;
        if (this->registerStatus == RegisterStatus::registered) this->registerStatus = RegisterStatus::unRegister;
        break;
    case ConnStatus::connecting:
        break;
    case ConnStatus::connected:
        if (this->pReconnectTimer->isActive())    this->pReconnectTimer->stop();
        if (!this->m_isSendRegister01)              this->sendRegister01();
        break;
    default:
        break;
    }
    // 注册状态
    switch (this->registerStatus)
    {
    case RegisterStatus::unRegister:
        if (this->pRequestTimer03->isActive())          this->pRequestTimer03->stop();
        if (this->m_isModuleLocation05)                   this->m_isModuleLocation05  = false;
        if (this->m_isModuleConfigure20)                  this->m_isModuleConfigure20 = false;
        if (this->pModuleStateTimer21->isActive())               this->pModuleStateTimer21->stop();
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
        if (!this->m_isModuleLocation05)                   this->sendModuleLocation05();
        if (!this->m_isModuleConfigure20)                  this->sendModuleFigure20();
        if (!this->pModuleStateTimer21->isActive())               this->pModuleStateTimer21->start(5000);
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
    QByteArray buf = this->pTcpSocket->readAll();
    GenericHeader genericHeader2;
    memcpy(&genericHeader2, buf.data(), HEADER_LEN);
    qint16 pkgID = genericHeader2.packType;
    qDebug("===================================================================");
    qDebug().nospace() << "recv 0x" << QString::number(pkgID, 16) << ": " << buf.toHex();
    qDebug("===================================================================");
    // 策略模式，根据包类型决定转发至哪个函数
    if (this->pkgsComm.contains(this->m_genericHeader.packType)) {
        this->onReadCommData(pkgID, buf);
    }
    if (this->pkgsPRUE.contains(this->m_genericHeader.packType)) {
        this->onReadPRUEData(pkgID, buf);
    }
}

void PRUEModule::onReadPRUEData(qint16 pkgID, const QByteArray& buf) {
    switch (pkgID) {
        case 0x201: this->recvSettingBanSector201(buf);  break;
        case 0x202: this->recvBanRadiation202(buf);      break;
        case 0x601: this->recvUpdatePRUESetting601(buf); break;
        default: {
            break;
        }
    }
}

void PRUEModule::recvSettingBanSector201(const QByteArray& buf) {
    OTrapBanSector oTrapBanSector;
    uint8_t len1 = HEADER_LEN;
    uint8_t len2 = sizeof(ORecvTrapFixed0x601);
    memcpy(&oTrapBanSector, buf.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapBanSector), len2);
}

void PRUEModule::recvBanRadiation202(const QByteArray& buf) {
    OTrapRadiationBan0x202 oTrapRadiationBan;
    uint8_t len1 = HEADER_LEN;
    uint8_t len2 = sizeof(OTrapRadiationBan0x202);
    memcpy(&oTrapRadiationBan, buf.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oTrapRadiationBan), len2);
}

void PRUEModule::recvUpdatePRUESetting601(const QByteArray& buf) {
    ORecvTrapFixed0x601 oRecvTrapFixed;
    uint8_t len1 = HEADER_LEN;
    uint8_t len2 = sizeof(ORecvTrapFixed0x601);
    memcpy(&oRecvTrapFixed, buf.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oRecvTrapFixed), len2);
}

void PRUEModule::sendInstalledBanSectorD01() {
    this->m_genericHeader.packType = 0xD01;
    this->m_genericHeader.dataSize = sizeof(OSendTrapFixed0xD21);
    this->m_genericHeader.packIdx++;
    this->m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->m_genericHeader), HEADER_LEN - 2);

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

    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OTrapBanSector);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->m_genericHeader, len1);
    memcpy(data + len1, &oTrapBanSector, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0xD01:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void PRUEModule::sendPRUESettingsD21() {
    this->m_genericHeader.packType = 0xD21;
    this->m_genericHeader.dataSize = sizeof(OSendTrapFixed0xD21);
    this->m_genericHeader.packIdx++;
    this->m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->m_genericHeader), HEADER_LEN - 2);

    OSendTrapFixed0xD21 oSendTrapFixed;
    oSendTrapFixed.taskREB = 0;
    oSendTrapFixed.taskGeo = 0;
    oSendTrapFixed.reserve = 0;

    oSendTrapFixed.curAzREB = 0;
    oSendTrapFixed.curEpsREB = 0;
    oSendTrapFixed.kGainREB = 0;

    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSendTrapFixed0xD21);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->m_genericHeader, len1);
    memcpy(data + len1, &oSendTrapFixed, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0xD21:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void PRUEModule::sendPRUEFunctionD22() {
    this->m_genericHeader.packType = 0xD22;
    this->m_genericHeader.dataSize = sizeof(OSendTrapFixed0xD21);
    this->m_genericHeader.packIdx++;
    this->m_genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->m_genericHeader), HEADER_LEN - 2);

    OTrapFunc0xD22 oTrapFunc;
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

    quint8 len1 = sizeof(OTrapFunc0xD22);
    quint8 len2 = sizeof(OSendTrapFixed0xD21);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->m_genericHeader, len1);
    memcpy(data + len1, &oTrapFunc, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0xD22:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

}  // namespace NEBULA
