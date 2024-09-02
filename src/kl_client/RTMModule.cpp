#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    this->pkgsRTM = {0x561, 0x563, 0x564};
    this->genericHeader = {0x50454C, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};

    this->pStateMachineTimer =       new QTimer();
    this->pCurrentSettingTimer823 =  new QTimer();
    this->pCurrentFunctionTimer825 = new QTimer();

    connect(this->pTcpSocket,         &QTcpSocket::readyRead, this, &RTMModule::onRecvData);
    connect(this->pStateMachineTimer,       &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(this->pCurrentSettingTimer823,  &QTimer::timeout, this, &RTMModule::sendRTMSettings823);
    connect(this->pCurrentFunctionTimer825, &QTimer::timeout, this, &RTMModule::sendRTMFunction825);
    
    this->pStateMachineTimer->start();
}

RTMModule::~RTMModule() {
    if (this->pStateMachineTimer == nullptr)       delete this->pStateMachineTimer;
    if (this->pCurrentSettingTimer823 == nullptr)  delete this->pCurrentSettingTimer823;
    if (this->pCurrentFunctionTimer825 == nullptr) delete this->pCurrentFunctionTimer825;
}

// 状态机
void RTMModule::stateMachine() {
    // 连接状态,连接->注册->对时
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
        if (this->pCurrentSettingTimer823->isActive())  this->pCurrentSettingTimer823->stop();
        if (this->pCurrentFunctionTimer825->isActive()) this->pCurrentFunctionTimer825->stop();
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
        if (!this->pCurrentSettingTimer823->isActive())  this->pCurrentSettingTimer823->start(1000);
        if (!this->pCurrentFunctionTimer825->isActive()) this->pCurrentFunctionTimer825->start(1000);
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

void RTMModule::onRecvData() {
    QByteArray buff = this->pTcpSocket->readAll();
    GenericHeader genericHeader2;
    memcpy(&genericHeader2, buff.data(), sizeof(GenericHeader));
    qint16 pkgID = genericHeader2.packType;
    qDebug("===================================================================");
    qDebug().nospace().noquote() << "recv 0x" << QString::number(pkgID, 16) << ": " << buff.toHex();
    qDebug("===================================================================");
    // 策略模式，根据包类型决定转发至哪个函数
    if (this->pkgsComm.contains(genericHeader2.packType)) {
        this->onReadCommData(pkgID, buff);
    }
    if (this->pkgsRTM.contains(genericHeader2.packType)) {
        this->onReadRTMData(pkgID, buff);
    }
}

void RTMModule::onReadRTMData(qint16 pkgID, QByteArray& buff) {
    switch (pkgID) {
        case 0x561: this->recvChangingRTMSettings561(buff);     break;
        case 0x563: this->recvRequestForbiddenIRIList563(buff); break;
        case 0x564: this->recvSettingForbiddenIRIList564(buff); break;
        default: {
            break;
        }
    }
}

void RTMModule::recvChangingRTMSettings561(const QByteArray& buff) {
    OUpdateRTMSetting oUpdateRTMSetting;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OUpdateRTMSetting);
    memcpy(&oUpdateRTMSetting, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oUpdateRTMSetting), len2);
    qDebug() << "recv 0x561:" << byteArray.toHex();
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

void RTMModule::recvRequestForbiddenIRIList563(const QByteArray& buff) {
    uint8_t code = 0;
    this->sendControlledOrder23(code);
    // 如果响应代码正常，则发送0x828作为响应
    if (code == 0) this->sendForbiddenIRIList828();
}

void RTMModule::recvSettingForbiddenIRIList564(const QByteArray& buff) {
    OSetBanIRIlist oSetBanIRIlist;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OSetBanIRIlist);
    memcpy(&oSetBanIRIlist, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oSetBanIRIlist), len2);
    qDebug() << "recv 0x564:" << byteArray.toHex();
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

void RTMModule::sendBearingMarker822() {
    this->genericHeader.packType = 0x822;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    OBearingMark oBearingMark;
    oBearingMark.idxCeilVOI = 0xffff;
    oBearingMark.iReserve = 0;

    oBearingMark.idxCeilSPP = 0;

    oBearingMark.idxPoint = 0;
    oBearingMark.typeCeilSPP = 0xff;
    oBearingMark.typeChannel = 0xff;
    oBearingMark.typeSignal = 0xff;

    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    oBearingMark.timePel1 = reqTimestamp & 0xFFFFFFFF;
    oBearingMark.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    
    oBearingMark.azim = 0;
    oBearingMark.elev = 0;
    oBearingMark.range = 0;
    oBearingMark.freqMhz = 0;
    oBearingMark.dFreqMhz = 0;
    oBearingMark.Pow_dBm = 0;
    oBearingMark.SNR_dB = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OBearingMark);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oBearingMark, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x822:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void RTMModule::sendRTMSettings823() {
    this->genericHeader.packType = 0x823;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OSubRezhRTR20 oSubRezhRTR20;
    oSubRezhRTR20.n_Cnt = 0;
    oSubRezhRTR20.n_Reserv = 0;
    oSubRezhRTR20.f_CurAz = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSubRezhRTR20);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oSubRezhRTR20, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x823:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void RTMModule::sendRTMFunction825() {
    this->genericHeader.packType = 0x825;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OSubPosobilRTR22 oSubPosobilRTR22;
    oSubPosobilRTR22.n_IsRotate = 0;
    oSubPosobilRTR22.n_MaxTask = 0;
    oSubPosobilRTR22.n_MaxSubDiap = 0;
    oSubPosobilRTR22.n_Rezerv = 0;

    oSubPosobilRTR22.f_AzSize = 0;
    oSubPosobilRTR22.f_EpsSize = 0;
    oSubPosobilRTR22.f_maxBand = 0;
    oSubPosobilRTR22.f_minBand = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSubPosobilRTR22);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oSubPosobilRTR22, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x825:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void RTMModule::sendBearingAndRoute827() {
    this->genericHeader.packType = 0x827;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    QString jsonStr = "{'Number':12685126,'DateTime':'2018-09-28 13:11:37','StartPoint':{'Latitude':55.12345,'Longitude':38.12345,'Altitude':155.12},'TypeBLA':{'InfoName':'无人机类型','InfoValue':'phantom-3'}";
    QByteArray byteArray = jsonStr.toUtf8();
    qDebug() << "send 0x827:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
}

void RTMModule::sendForbiddenIRIList828() {
    this->genericHeader.packType = 0x828;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OBanIRI oBanIRI;
    oBanIRI.n_Numb = 0;
    oBanIRI.rezerv = 0;
    oBanIRI.f_Freq = 0;
    oBanIRI.f_DelFreq = 0;
    

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OBanIRI);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oBanIRI, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x828:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

void RTMModule::sendWirelessEnvInfo829() {
    this->genericHeader.packType = 0x829;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OSubRadioTime oSubRadioTime;
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    oSubRadioTime.time1 = reqTimestamp & 0xFFFFFFFF;
    oSubRadioTime.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    
    oSubRadioTime.n_Num = 0;
    oSubRadioTime.n_Type = 0;
    
    oSubRadioTime.f_FrBegin = 0;
    oSubRadioTime.f_FrStep = 0;
    oSubRadioTime.n_Cnt = 0;

    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSubRadioTime);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oSubRadioTime, len2);
    QByteArray byteArray(data, len1 + len2);
    qDebug() << "send 0x829:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}
}