#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    this->pkgsRTM = {0x561, 0x563, 0x564};
    this->genericHeader = {0x50454C, 0xff, 0x02, 0x02, 0x0000, 0x000000, 0x01, 0x0000, 0x0000};

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

    this->oBearingMark0x822.idxCeilVOI = 0xffff;
    this->oBearingMark0x822.idxCeilSPP = 1;
    this->oBearingMark0x822.idxPoint = 0;
    this->oBearingMark0x822.typeCeilSPP = 4;
    this->oBearingMark0x822.typeChannel = 0;
    this->oBearingMark0x822.typeSignal = 0;
    this->oBearingMark0x822.azim = 180;
    this->oBearingMark0x822.elev = 10.5;
    this->oBearingMark0x822.range = 1060;
    this->oBearingMark0x822.freqMhz = 5840;
    this->oBearingMark0x822.dFreqMhz = 10;
    this->oBearingMark0x822.Pow_dBm = -80;
    this->oBearingMark0x822.SNR_dB = 15;

    this->oSubRezhRTR0x823.N = 1;
    this->oSubRezhRTR0x823.curAz = -1;
    this->oSubRezhRTR0x823.freqs.append({5850.5, 120.5});

    this->oSubPosobilRTR0x825.isRotate = 0;
    this->oSubPosobilRTR0x825.maxTasks = 5;
    this->oSubPosobilRTR0x825.numDiap = 1;
    this->oSubPosobilRTR0x825.dAz = -1;
    this->oSubPosobilRTR0x825.dElev = -1;
    this->oSubPosobilRTR0x825.minFreqRTR = 300;
    this->oSubPosobilRTR0x825.maxFreqRTR = 6000;

    this->oSetBanIRIlist0x828.NIRI = 1;
    this->oSetBanIRIlist0x828.freqs.append({5850.5, 120.5});

    this->oSubRadioTime0x829.taskNum = 0;
    this->oSubRadioTime0x829.powType = 0;
    this->oSubRadioTime0x829.freqBegin = 0;
    this->oSubRadioTime0x829.freqStep = 0;
    this->oSubRadioTime0x829.N = 1;
    this->oSubRadioTime0x829.pow1 = 100;
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
    memcpy(&genericHeader2, buff.data(), HEADER_LEN);
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
    if(buff.length() > HEADER_LEN) {
        this->oSubRezhRTR0x823.N = buff.at(HEADER_LEN + 0);
    }
    // 4 + 8 * n
    this->oSubRezhRTR0x823.freqs.clear();
    for(qint8 i = HEADER_LEN + 4; i < buff.length(); i += 8) {
        float freq1, dFreq1;
        memcpy(&freq1,  buff.mid(i, 8).constData(), 4);
        memcpy(&dFreq1, buff.mid(i, 8).constData() + 4, 4);
        this->oSubRezhRTR0x823.freqs.append({freq1, dFreq1});
    }
    this->sendControlledOrder23(0);
    this->sendRTMSettings823();
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x561:" << buff.toHex()
             << "N:"          << this->oSubRezhRTR0x823.N;
    for(auto const &item : this->oSubRezhRTR0x823.freqs) {
        qDebug() << item[0] << "," << item[1];
    }
}

// 563->828
void RTMModule::recvRequestForbiddenIRIList563(const QByteArray& buff) {
    this->sendControlledOrder23(0);
    this->sendForbiddenIRIList828();
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x563:" << buff.toHex();
}

//564修改IRI的中心评率
void RTMModule::recvSettingForbiddenIRIList564(const QByteArray& buff) {
    if(buff.length() > HEADER_LEN) {
        this->oSetBanIRIlist0x828.NIRI = buff.at(HEADER_LEN + 0);
    }
    this->oSetBanIRIlist0x828.freqs.clear();
    for(qint8 i = HEADER_LEN + 4; i < buff.length(); i += 8) {
        float freq1, dFreq1;
        memcpy(&freq1,  buff.mid(i, 8).constData(), 4);
        memcpy(&dFreq1, buff.mid(i, 8).constData() + 4, 4);
        this->oSetBanIRIlist0x828.freqs.append({freq1, dFreq1});
    }

    this->sendControlledOrder23(0);
    /* ------------------------------------------------------------------------ */
    qDebug() << "recv 0x564:" << buff.toHex()
             << "NIRI:"       << this->oSetBanIRIlist0x828.NIRI;
    for(auto const &item : this->oSetBanIRIlist0x828.freqs) {
        qDebug() << item[0] << "," << item[1];
    }
}

void RTMModule::sendBearingMarker822() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x822;
    this->genericHeader.dataSize = sizeof(OBearingMark0x822);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    this->oBearingMark0x822.timePel1 = reqTimestamp & 0xFFFFFFFF;
    this->oBearingMark0x822.timePel2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    this->oBearingMark0x822.idxPoint++;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OBearingMark0x822);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->oBearingMark0x822, len2);
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
             << "azim:"    << this->oBearingMark0x822.azim
             << "elev:"    << this->oBearingMark0x822.elev
             << "range:"   << this->oBearingMark0x822.range
             << "freqMhz:" << this->oBearingMark0x822.freqMhz
             << "dFreqMhz:"<< this->oBearingMark0x822.dFreqMhz
             << "Pow_dBm:" << this->oBearingMark0x822.Pow_dBm
             << "SNR_dB:"  << this->oBearingMark0x822.SNR_dB;
}

void RTMModule::sendRTMSettings823() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x823;
    this->genericHeader.dataSize = HEADER_LEN + 8 + this->oSubRezhRTR0x823.freqs.size() * 8;
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubRezhRTR0x823);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->oSubRezhRTR0x823, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x823:"     << byteArray.toHex()
             << "N:"          << this->oSubRezhRTR0x823.N
             << "curAz:"        << this->oSubRezhRTR0x823.curAz;
}

void RTMModule::sendRTMFunction825() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x825;
    this->genericHeader.dataSize = sizeof(OSubPosobilRTR0x825);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubPosobilRTR0x825);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->oSubPosobilRTR0x825, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x825:"   << byteArray.toHex()
             << "numDiap:"      << this->oSubPosobilRTR0x825.numDiap
             << "minFreqRTR:"   << this->oSubPosobilRTR0x825.minFreqRTR
             << "maxFreqRTR:"   << this->oSubPosobilRTR0x825.maxFreqRTR;
}

void RTMModule::sendBearingAndRoute827() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x827;
    this->genericHeader.dataSize = sizeof(OBearingMark0x822);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    QString jsonStr = "{'Number':12685126,'DateTime':'2018-09-28 13:11:37','StartPoint':{'Latitude':55.12345,'Longitude':38.12345,'Altitude':155.12},'TypeBLA':{'InfoName':'无人机类型','InfoValue':'phantom-3'}";
    /* ------------------------------------------------------------------------ */
    QByteArray byteArray = jsonStr.toUtf8();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    qDebug() << "send 0x827:"   << byteArray.toHex()
             << "jsonStr:"      << jsonStr;
}

// 修改频率、频段
void RTMModule::sendForbiddenIRIList828() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x828;
    this->genericHeader.dataSize = HEADER_LEN + 4 + this->oSetBanIRIlist0x828.freqs.size() * 8;
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = 4;
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->oSetBanIRIlist0x828, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x828:"  << byteArray.toHex()
             << "NIRI:"      << this->oSetBanIRIlist0x828.NIRI;
}

void RTMModule::sendWirelessEnvInfo829() {
    /* ------------------------------------------------------------------------ */
    this->genericHeader.packType = 0x829;
    this->genericHeader.dataSize = sizeof(OSubRadioTime0x829);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), HEADER_LEN - 2);
    /* ------------------------------------------------------------------------ */
    qint64 reqTimestamp = QDateTime::currentMSecsSinceEpoch();
    this->oSubRadioTime0x829.time1 = reqTimestamp & 0xFFFFFFFF;
    this->oSubRadioTime0x829.time2 = (reqTimestamp >> 32) & 0xFFFFFFFF;
    /* ------------------------------------------------------------------------ */
    quint8 len1 = HEADER_LEN;
    quint8 len2 = sizeof(OSubRadioTime0x829);
    char* data = (char*)malloc(len1 + len2);
    memcpy(data, &this->genericHeader, len1);
    memcpy(data + len1, &this->oSubRadioTime0x829, len2);
    QByteArray byteArray(data, len1 + len2);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
    qDebug() << "send 0x829:"     << byteArray.toHex()
             << "N:"              << this->oSubRadioTime0x829.N
             << "pow1:"           << this->oSubRadioTime0x829.pow1;
}
}