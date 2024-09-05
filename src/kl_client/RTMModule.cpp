#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    this->pkgsRTM = {0x561, 0x563, 0x564};
    this->genericHeader = {0x50454C, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};

    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    this->myModuleTimeControl0x3.time1 = reqTimestamp & 0xFFFFFFFF;
    this->myModuleTimeControl0x3.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    this->myModuleGeoLocation0x5.typeData = 1;
    this->myModuleGeoLocation0x5.isValid = 1;
    this->myModuleGeoLocation0x5.reserve = 0;
    this->myModuleGeoLocation0x5.xLat = 10;
    this->myModuleGeoLocation0x5.yLong = 20;
    this->myModuleGeoLocation0x5.zHeight = 30;

    this->pStateMachineTimer =       new QTimer();
    this->pCurrentTargetTimer822 =   new QTimer();
    this->pCurrentSettingTimer823 =  new QTimer();
    this->pCurrentFunctionTimer825 = new QTimer();

    connect(this->pTcpSocket,         &QTcpSocket::readyRead, this, &RTMModule::onRecvData);
    connect(this->pStateMachineTimer,       &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(this->pCurrentTargetTimer822,   &QTimer::timeout, this, &RTMModule::sendBearingMarker822);
    connect(this->pCurrentSettingTimer823,  &QTimer::timeout, this, &RTMModule::sendRTMSettings823);
    connect(this->pCurrentFunctionTimer825, &QTimer::timeout, this, &RTMModule::sendRTMFunction825);
    
    // 启动状态机定时器
    this->pStateMachineTimer->start();

    this->oSubRezhRTR0x823.n_Cnt = 1;
    this->oSubRezhRTR0x823.n_Reserv = 0;
    this->oSubRezhRTR0x823.f_CurAz = -1;
    this->oSubRezhRTR0x823.f_Curfm = 5850.5;
    this->oSubRezhRTR0x823.f_Curband = 120.0;

    this->oSetBanIRIlist0x564And0x828.n_Numb = 1;
    this->oSetBanIRIlist0x564And0x828.reserve = 0;
    this->oSetBanIRIlist0x564And0x828.f_Freq1 = 0;
    this->oSetBanIRIlist0x564And0x828.f_DelFreq1 = 0;
    this->oSetBanIRIlist0x564And0x828.f_Freq2 = 0;
    this->oSetBanIRIlist0x564And0x828.f_DelFreq2 = 0;   
    this->oSetBanIRIlist0x564And0x828.f_Freq3 = 0;
    this->oSetBanIRIlist0x564And0x828.f_DelFreq3 = 0;  
}

RTMModule::~RTMModule() {
    if (this->pStateMachineTimer == nullptr)       delete this->pStateMachineTimer;
    if (this->pCurrentTargetTimer822 == nullptr)   delete this->pCurrentTargetTimer822;
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
    default: {
        if(this->isDebugOut) {
            this->sendLogMsg25("the connection state is unknown");
            this->sendNote2Operator26("the connection state is unknown");
        }
        break;
    }
        
    }
    // 注册状态
    switch (this->registerStatus)
    {
    case RegisterStatus::unRegister:
        if (this->pRequestTimer03->isActive())          this->pRequestTimer03->stop();
        if (this->timeStatus == TimeStatus::timed)      this->timeStatus = TimeStatus::unTime;
        break;
    case RegisterStatus::registering:
        break;
    case RegisterStatus::registered:
        if (!this->pRequestTimer03->isActive())         this->pRequestTimer03->start(1000);
        break;
    default:{
        if(this->isDebugOut) {
            this->sendLogMsg25("the regsiter state is unknown");
            this->sendNote2Operator26("the regsiter state is unknown");
        }
        break;
    }
        
    }
    // 对时状态
    switch (this->timeStatus)
    {
    case TimeStatus::unTime: {
        if (this->isModuleLocation05)                   this->isModuleLocation05  = false;
        if (this->isModuleConfigure20)                  this->isModuleConfigure20 = false;
        if (this->pModuleStateTimer21->isActive())               this->pModuleStateTimer21->stop();
        if (this->pCPTimer22->isActive())               this->pCPTimer22->stop();
        if (this->pModuleStatueTimer24->isActive())     this->pModuleStatueTimer24->stop();
        if (this->pNPTimer28->isActive())  this->pNPTimer28->stop();
        if (this->pCurrentTargetTimer822->isActive())   this->pCurrentTargetTimer822->stop();
        if (this->pCurrentSettingTimer823->isActive())  this->pCurrentSettingTimer823->stop();
        if (this->pCurrentFunctionTimer825->isActive()) this->pCurrentFunctionTimer825->stop();
        break;
    }
    case TimeStatus::timing: break;
    case TimeStatus::timed:
    {
        if (!this->isModuleLocation05)                   this->sendModuleLocation05();
        if (!this->isModuleConfigure20)                  this->sendModuleFigure20();
        if (!this->pModuleStateTimer21->isActive())      this->pModuleStateTimer21->start(30000);
        // if (!this->pCPTimer22->isActive())            this->pCPTimer22->start(6000);
        if (!this->pModuleStatueTimer24->isActive())     this->pModuleStatueTimer24->start(10000);
        if (!this->pNPTimer28->isActive())               this->pNPTimer28->start(12000);
        if (!this->pCurrentTargetTimer822->isActive())   this->pCurrentTargetTimer822->start(15000);
        if (!this->pCurrentSettingTimer823->isActive())  this->pCurrentSettingTimer823->start(17000);
        if (!this->pCurrentFunctionTimer825->isActive()) this->pCurrentFunctionTimer825->start(19000);
        break;
    }
    default:{
        if(this->isDebugOut) {
            this->sendLogMsg25("the regsiter state is unknown");
            this->sendNote2Operator26("the regsiter state is unknown");
        }
        break;
    }
    }
}

void RTMModule::onRecvData() {
    QByteArray buff = this->pTcpSocket->readAll();
    GenericHeader genericHeader2;
    memcpy(&genericHeader2, buff.data(), sizeof(GenericHeader));
    qint16 pkgID = genericHeader2.packType;
    qDebug().nospace().noquote() << "recv 0x" << QString::number(pkgID, 16) << ": " << buff.toHex();
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

// 修改频率、频段
void RTMModule::recvChangingRTMSettings561(const QByteArray& buff) {
    OUpdateRTMSetting0x561 oUpdateRTMSetting561;
    qint8 len1 = sizeof(GenericHeader);
    qint8 len2 = sizeof(OUpdateRTMSetting0x561);
    memcpy(&oUpdateRTMSetting561, buff.data() + len1, len2);
    this->sendControlledOrder23(0);
    this->oSubRezhRTR0x823.f_Curfm   = oUpdateRTMSetting561.f_Freq;
    this->oSubRezhRTR0x823.f_Curband = oUpdateRTMSetting561.f_DelFreq;
    this->sendRTMSettings823();
    /* ------------------------------------------------------------------------ */
    QByteArray byteArray(reinterpret_cast<char*>(&oUpdateRTMSetting561), len2);
    if(this->isDebugOut) {
        this->sendLogMsg25(QString::fromUtf8(byteArray));
        this->sendNote2Operator26(QString::fromUtf8(byteArray));
    }
    qDebug() << "recv 0x561:" << byteArray.toHex()
                              << oUpdateRTMSetting561.n_range
                              << oUpdateRTMSetting561.rezerv
                              << oUpdateRTMSetting561.f_Freq
                              << oUpdateRTMSetting561.f_DelFreq;
}

// 563->828
void RTMModule::recvRequestForbiddenIRIList563(const QByteArray& buff) {
    this->sendControlledOrder23(0);
    this->sendForbiddenIRIList828();
}

//564修改IRI的中心评率
void RTMModule::recvSettingForbiddenIRIList564(const QByteArray& buff) {
    // 只能接收等于三个频率、频段
    OSetBanIRIlist0x564And0x828 oSetBanIRIlist;
    qint8 len1 = sizeof(GenericHeader);
    qint8 len2 = sizeof(OSetBanIRIlist0x564And0x828);
    memcpy(&oSetBanIRIlist, buff.data() + len1, len2);
    this->oSetBanIRIlist0x564And0x828.n_Numb = oSetBanIRIlist.n_Numb;
    this->oSetBanIRIlist0x564And0x828.reserve = oSetBanIRIlist.reserve;
    this->oSetBanIRIlist0x564And0x828.f_Freq1 = oSetBanIRIlist.f_Freq1;
    this->oSetBanIRIlist0x564And0x828.f_DelFreq1 = oSetBanIRIlist.f_DelFreq1;
    this->oSetBanIRIlist0x564And0x828.f_Freq2 = oSetBanIRIlist.f_Freq2;
    this->oSetBanIRIlist0x564And0x828.f_DelFreq2 = oSetBanIRIlist.f_DelFreq2;
    this->oSetBanIRIlist0x564And0x828.f_Freq3 = oSetBanIRIlist.f_Freq3;
    this->oSetBanIRIlist0x564And0x828.f_DelFreq3 = oSetBanIRIlist.f_DelFreq3;

    this->sendControlledOrder23(0);
    /* ------------------------------------------------------------------------ */
    QByteArray byteArray(reinterpret_cast<char*>(&oSetBanIRIlist), len2);
    qDebug() << "recv 0x564:" << byteArray.toHex()
             << "n_Numb:"     << oSetBanIRIlist.n_Numb;
}

void RTMModule::sendBearingMarker822() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x822;
    this->genericHeader.dataSize = sizeof(OBearingMark0x822);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    OBearingMark0x822 oBearingMark;
    oBearingMark.idxCeilVOI = 0xffff;
    oBearingMark.iReserve = 0;
    oBearingMark.idxCeilSPP = 1;
    oBearingMark.idxPoint++;
    oBearingMark.typeCeilSPP = 4;
    oBearingMark.typeChannel = 0;
    oBearingMark.typeSignal = 0;
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    oBearingMark.timePel1 = reqTimestamp & 0xFFFFFFFF;
    oBearingMark.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    oBearingMark.azim = 180;
    oBearingMark.elev = 10.5;
    oBearingMark.range = 1060;
    oBearingMark.freqMhz = 5840;
    oBearingMark.dFreqMhz = 10;
    oBearingMark.Pow_dBm = -80;
    oBearingMark.SNR_dB = 15;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OBearingMark0x822);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oBearingMark, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    /* ------------------------------------------------------------------------ */
    if(this->isDebugOut) {
        this->sendLogMsg25("a target has being detected");
        this->sendNote2Operator26("a target has being detected");
    }
    qDebug() << "send 0x822:"   << byteArray.toHex() 
             << "checksum:"<< this->genericHeader.checkSum
             << "azim:"    << oBearingMark.azim
             << "elev:"    << oBearingMark.elev
             << "range:"   << oBearingMark.range
             << "freqMhz:" << oBearingMark.freqMhz
             << "dFreqMhz:"<< oBearingMark.dFreqMhz
             << "Pow_dBm:" << oBearingMark.Pow_dBm
             << "SNR_dB:"  << oBearingMark.SNR_dB;
}

void RTMModule::sendRTMSettings823() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x823;
    this->genericHeader.dataSize = sizeof(OSubRezhRTR0x823);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSubRezhRTR0x823);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->oSubRezhRTR0x823, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x823:"     << byteArray.toHex()
             << "n_Cnt:"          << this->oSubRezhRTR0x823.n_Cnt
             << "n_Reserv:"       << this->oSubRezhRTR0x823.n_Reserv
             << "f_CurAz:"        << this->oSubRezhRTR0x823.f_CurAz
             << "f_Curfm:"        << this->oSubRezhRTR0x823.f_Curfm
             << "f_Curband:"      << this->oSubRezhRTR0x823.f_Curband;
}

void RTMModule::sendRTMFunction825() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x825;
    this->genericHeader.dataSize = sizeof(OSubPosobilRTR0x825);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    OSubPosobilRTR0x825 oSubPosobilRTR825;
    oSubPosobilRTR825.n_IsRotate = 0;
    oSubPosobilRTR825.n_MaxTask = 5;
    oSubPosobilRTR825.n_MaxSubDiap = 1;
    oSubPosobilRTR825.n_Rezerv = 0;
    oSubPosobilRTR825.f_AzSize = -1;
    oSubPosobilRTR825.f_EpsSize = -1;
    oSubPosobilRTR825.f_fpMin = 300;
    oSubPosobilRTR825.f_fpMax = 6000;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSubPosobilRTR0x825);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oSubPosobilRTR825, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x825:"        << byteArray.toHex()
             << "n_IsRotate:"        << oSubPosobilRTR825.n_IsRotate
             << "n_MaxTask:"         << oSubPosobilRTR825.n_MaxTask
             << "n_MaxSubDiap:"      << oSubPosobilRTR825.n_MaxSubDiap
             << "n_Rezerv:"          << oSubPosobilRTR825.n_Rezerv
             << "f_AzSize:"          << oSubPosobilRTR825.f_AzSize
             << "f_EpsSize:"         << oSubPosobilRTR825.f_EpsSize
             << "f_fpMin:"           << oSubPosobilRTR825.f_fpMin
             << "f_fpMax:"           << oSubPosobilRTR825.f_fpMax;
}

void RTMModule::sendBearingAndRoute827() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x827;
    this->genericHeader.dataSize = sizeof(OBearingMark0x822);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    QString jsonStr = "{'Number':12685126,'DateTime':'2018-09-28 13:11:37','StartPoint':{'Latitude':55.12345,'Longitude':38.12345,'Altitude':155.12},'TypeBLA':{'InfoName':'无人机类型','InfoValue':'phantom-3'}";
    /* ------------------------------------------------------------------------ */
    QByteArray byteArray = jsonStr.toUtf8();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    qDebug() << "send 0x827:"   << byteArray.toHex()
             << "jsonStr:" << jsonStr;
}

// 修改频率、频段
void RTMModule::sendForbiddenIRIList828() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x828;
    this->genericHeader.dataSize = sizeof(OSetBanIRIlist0x564And0x828);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSetBanIRIlist0x564And0x828);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->oSetBanIRIlist0x564And0x828, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x828:"  << byteArray.toHex()
             << "n_Numb:"      << this->oSetBanIRIlist0x564And0x828.n_Numb
             << "f_Freq1:"    << this->oSetBanIRIlist0x564And0x828.f_Freq1
             << "f_DelFreq1:" << this->oSetBanIRIlist0x564And0x828.f_DelFreq1
             << "f_Freq2:"    << this->oSetBanIRIlist0x564And0x828.f_Freq2
             << "f_DelFreq2:" << this->oSetBanIRIlist0x564And0x828.f_DelFreq2
             << "f_Freq3:"    << this->oSetBanIRIlist0x564And0x828.f_Freq3
             << "f_DelFreq3:" << this->oSetBanIRIlist0x564And0x828.f_DelFreq3;
}

void RTMModule::sendWirelessEnvInfo829() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x829;
    this->genericHeader.dataSize = sizeof(OSubRadioTime0x829);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);
    /* ------------------------------------------------------------------------ */
    OSubRadioTime0x829 oSubRadioTime;
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    oSubRadioTime.time1 = reqTimestamp & 0xFFFFFFFF;
    oSubRadioTime.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    oSubRadioTime.n_Num = 0;
    oSubRadioTime.n_Type = 0;
    oSubRadioTime.f_FrBegin = 0;
    oSubRadioTime.f_FrStep = 0;
    oSubRadioTime.n_Cnt = 0;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = sizeof(GenericHeader);
    quint8 len2 = sizeof(OSubRadioTime0x829);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &oSubRadioTime, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x829:"     << byteArray.toHex()
             << "n_Num:"     << oSubRadioTime.n_Num
             << "n_Type:"    << oSubRadioTime.n_Type
             << "f_FrBegin:" << oSubRadioTime.f_FrBegin
             << "f_FrStep:"  << oSubRadioTime.f_FrStep
             << "n_Cnt:"     << oSubRadioTime.n_Cnt;
}
}