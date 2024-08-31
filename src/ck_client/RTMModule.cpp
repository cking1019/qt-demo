#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    this->pkgsRTM = {0x561, 0x563, 0x564};
    this->genericHeader = {0x50454C, 0xff, 0x2, 0x2, 0x0, 0x0, 0x1, 0x0, 0x0};
    this->pStateMachineTimer = new QTimer();
    this->pCurrentSettingTimer823 = new QTimer();
    this->pCurrentFunctionTimer825 = new QTimer();

    connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &RTMModule::onRecvData);
    connect(this->pStateMachineTimer, &QTimer::timeout, this, &RTMModule::stateMachine);
    connect(this->pCurrentSettingTimer823, &QTimer::timeout, this, &RTMModule::sendRTMSettings823);
    connect(this->pCurrentFunctionTimer825, &QTimer::timeout, this, &RTMModule::sendRTMFunction825);
    
    this->pStateMachineTimer->start();
}

RTMModule::~RTMModule() {
    if (this->pStateMachineTimer == nullptr)       delete this->pStateMachineTimer;
    if (this->pCurrentSettingTimer823 == nullptr)  delete this->pCurrentSettingTimer823;
    if (this->pCurrentFunctionTimer825 == nullptr) delete this->pCurrentFunctionTimer825;
}

// 查看当前设备状态
void RTMModule::stateMachine() {
    switch (this->connStatus)
    {
    case unConnected:
        if (!this->pReconnectTimer->isActive())  this->pReconnectTimer->start(1000);

        if (this->pRequestTimer03->isActive())   this->pRequestTimer03->stop();
        if (this->isModuleLocation05)  this->isModuleLocation05  = false;
        if (this->isModuleConfigure20) this->isModuleConfigure20 = false;
        
        if (this->isNPStatus21)        this->isNPStatus21 = false;
        if (this->isCPStatus22)        this->isCPStatus22 = false;
        break;
    case connecting:
        break;
    case connected:
        if (this->pReconnectTimer->isActive())  this->pReconnectTimer->stop();
        break;
    default:
        break;
    }
    // 注册状态
    switch (this->registerStatus)
    {
        case unRegister:
            if (this->pNPTimer21->isActive())               this->pNPTimer21->stop();
            if (this->pCPTimer22->isActive())               this->pCPTimer22->stop();
            if (this->pModuleStatueTimer24->isActive())     this->pModuleStatueTimer24->stop();
            if (this->pCurrentSettingTimer823->isActive())  this->pCurrentSettingTimer823->stop();
            if (this->pCurrentFunctionTimer825->isActive()) this->pCurrentFunctionTimer825->stop();
            break;
        case registering:
            break;
        case registered:
            if (!this->isModuleLocation05)           this->sendModuleLocation05();
            if (!this->isModuleConfigure20)          this->sendModuleFigure20();
            if (!this->pNPTimer21->isActive())               this->pNPTimer21->start(5000);
            if (!this->pCPTimer22->isActive())               this->pCPTimer22->start(5000);
            if (!this->pModuleStatueTimer24->isActive())     this->pModuleStatueTimer24->start(1000);
            if (!this->pCurrentSettingTimer823->isActive())  this->pCurrentSettingTimer823->start(1000);
            if (!this->pCurrentFunctionTimer825->isActive()) this->pCurrentFunctionTimer825->start(1000);
            break;
        default:
            break;
    }
}

// 接收数据统一接口
void RTMModule::onRecvData() {
    QByteArray buff = this->pTcpSocket->readAll();
    memcpy(&this->genericHeader, buff.data(), sizeof(GenericHeader));
    qDebug("===================================================================");
    qDebug() << "received data from server: " << buff.toHex();
    qDebug("the size of pkg: %d", buff.size());
    qDebug("the type of pkg: %x", this->genericHeader.packType);
    qDebug("the type of pkg: %x", this->genericHeader.sender);
    qDebug("===================================================================");
    // 策略模式，根据包类型决定转发至哪个函数
    if (this->pkgsComm.contains(this->genericHeader.packType)) {
        this->onReadCommData(buff);
    }
    if (this->pkgsRTM.contains(this->genericHeader.packType)) {
        this->onReadRTMData(buff);
    }
}

// 从服务器中读取RTM数据
void RTMModule::onReadRTMData(QByteArray& buff) {
    switch (genericHeader.packType) {
        case 0x561: this->recvChangingRTMSettings561(buff); break;
        case 0x563: this->recvRequestForbiddenIRIList563(buff); break;
        case 0x564: this->recvSettingForbiddenIRIList564(buff); break;
        default: {
            // QString msg = "this is unknown pkg 0x" + QString::number(this->genericHeader.packType, 16);
            // this->sendLogMsg25(msg);
            // this->sendNote2Operator26(msg);
            break;
        }
    }
}

// 0x561,收到更改RTM设置
void RTMModule::recvChangingRTMSettings561(const QByteArray& buff) {
    // this->sendLogMsg25("recv changing RTM settings");
    OUpdateRTMSetting oUpdateRTMSetting;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OUpdateRTMSetting);
    memcpy(&oUpdateRTMSetting, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oUpdateRTMSetting), len2);
    qDebug() << "recv 0x561:" << byteArray.toHex();
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

// 0x563,请求禁止IRI列表
void RTMModule::recvRequestForbiddenIRIList563(const QByteArray& buff) {
    // this->sendLogMsg25("recv request forbidden IRI list");
    // 响应0x23
    uint8_t code = 0;
    this->sendControlledOrder23(code);
    // 如果响应代码正常，则发送0x828作为响应
    if (code == 0) this->sendForbiddenIRIList828();
}

// 0x564,设置禁止IRI列表
void RTMModule::recvSettingForbiddenIRIList564(const QByteArray& buff) {
    // this->sendLogMsg25("recv setting forbidden IRI list");
    OSetBanIRIlist oSetBanIRIlist;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OSetBanIRIlist);
    memcpy(&oSetBanIRIlist, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oSetBanIRIlist), len2);
    qDebug() << "recv 0x564:" << byteArray.toHex();
    // 响应0x23
    uint8_t code = 0;
    this->sendControlledOrder23(code);
}

// 0x822,发送方位标记,需要接收地图传来的数据，作为Nebula的槽函数，并由Nebula信号emit出去
void RTMModule::sendBearingMarker822() {
    this->genericHeader.packType = 0x822;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OBearingMark oBearingMark;
    oBearingMark.idxCeilSPP = 0xffff;
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
    // 消息头
    oBearingMark.header = this->genericHeader;

    quint8 len1 = sizeof(OBearingMark);
    char* data = (char*)malloc(len1);
    memcpy(data, &oBearingMark, len1);
    QByteArray byteArray(data, len1);
    qDebug() << "send 0x822:" << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 0x823,发送当前RTM设置
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

// 0x825,发送当前RTM功能
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

// 0x827,发送方位路线信息
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

// 0x828,发送禁止IRI列表
void RTMModule::sendForbiddenIRIList828() {
    this->genericHeader.packType = 0x828;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OBanIRI oBanIRI;
    oBanIRI.f_Freq = 0;
    oBanIRI.f_DelFreq = 0;
    oBanIRI.n_Numb = 0;

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

// 0x829,发送无线电环境信息
void RTMModule::sendWirelessEnvInfo829() {
    this->genericHeader.packType = 0x829;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum(reinterpret_cast<char*>(&this->genericHeader), sizeof(GenericHeader) - 2);

    // 消息体
    OSubRadioTime oSubRadioTime;
    oSubRadioTime.n_Time = 0;
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