#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
    // RTM的发送者标识
    this->genericHeader.sender = 0x50454C;
    this->pCurrentSettingTimer = new QTimer();
    this->pCurrentFunctionTimer = new QTimer();
    this->pCurrentStatusTimer = new QTimer();

    // 实时接收来自服务器的数据,使用基类的socket指针来接收数据，保证基类与派生类都可发送数据，但只允许派生类接收数据，然后发送给基类。
    connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &RTMModule::onRecvData);

    // 发送RTM设置定时器
    connect(this->pCurrentSettingTimer, &QTimer::timeout, this, &RTMModule::sendRTMSettings);
    // 发送RTM功能定时器
    connect(this->pCurrentFunctionTimer, &QTimer::timeout, this, &RTMModule::sendRTMFunction);

    // 查看RTM状态检查定时器
    connect(this->pCurrentStatusTimer, &QTimer::timeout, this, &RTMModule::checkStatus);
    this->pCurrentStatusTimer->start();
    // 接收包类型
    this->pkgsRTM = {0x561, 0x563, 0x564};
}

RTMModule::~RTMModule() {
    if (this->pCurrentSettingTimer == nullptr)  delete this->pCurrentSettingTimer;
    if (this->pCurrentFunctionTimer == nullptr) delete this->pCurrentFunctionTimer;
    if (this->pCurrentStatusTimer == nullptr)   delete this->pCurrentStatusTimer;
}

// 查看当前设备状态
void RTMModule::checkStatus() {
    if (!this->isConnected) {
        // 重新尝试连接
        if (!this->pReconnectTimer->isActive())  this->pReconnectTimer->start(1000);
        // 断开连接后，停止发送时间请求0x01
        if (this->pRequestTimer->isActive()) this->pRequestTimer->stop();
        // 断开连接后，停止发送CP与NP021&0x22
        if (this->pCPandNPTimer->isActive()) this->pCPandNPTimer->stop();
        // 断开连接后，停止发送模块状态0x24
        if (this->pModuleStatueTimer->isActive()) this->pModuleStatueTimer->start(1000);

        if (this->pCurrentFunctionTimer->isActive()) this->pCurrentFunctionTimer->stop();
        if (this->pCurrentSettingTimer->isActive())  this->pCurrentSettingTimer->stop();

        if (this->isModuleConfigure) this->isModuleConfigure = false;
        if (this->isModuleLocation)  this->isModuleLocation  = false;
        return;
    };


    if (this->isRegister) {
        // 定时发送当前设置,0x823
        if (!this->pCurrentSettingTimer->isActive())  this->pCurrentSettingTimer->start(1000);
        // 定时发送当前功能,0x825
        if (!this->pCurrentFunctionTimer->isActive()) this->pCurrentFunctionTimer->start(1000);
        // 注册成功后发起心跳机制,每秒发送一次,0x3
        if (!this->pRequestTimer->isActive())  this->pRequestTimer->start(1000);
        // 注册成功后发送NP与CP,0x21&0x22
        if (!this->pCPandNPTimer->isActive())  this->pCPandNPTimer->start(5000);
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

// 494f56ff 02020000 00000010 46000000
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
        case 0x561: this->recvChangingRTMSettings(buff); break;
        case 0x563: this->recvRequestForbiddenIRIList(buff); break;
        case 0x564: this->recvSettingForbiddenIRIList(buff); break;
        default: {
            QString msg = "this is unknown pkg 0x" + QString::number(this->genericHeader.packType, 16);
            this->sendLogMsg(msg);
            this->sendNote2Operator(msg);
            break;
        }
    }
}

// 0x561,收到更改RTM设置
void RTMModule::recvChangingRTMSettings(const QByteArray& buff) {
    this->sendLogMsg("recv changing RTM settings");
    OUpdateRTMSetting oUpdateRTMSetting;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OUpdateRTMSetting);
    memcpy(&oUpdateRTMSetting, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oUpdateRTMSetting), len2);
    qDebug() << "0x561 msg body: " << byteArray.toHex();
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder(code);
}

// 0x563,请求禁止IRI列表
void RTMModule::recvRequestForbiddenIRIList(const QByteArray& buff) {
    this->sendLogMsg("recv request forbidden IRI list");
    // 执行操作代码响应0x23
    uint8_t code = 0;
    this->sendControlledOrder(code);
    // 如果响应代码正常，则发送0x828作为响应
    if (code == 0) this->sendForbiddenIRIList();
}

// 0x564,设置禁止IRI列表
void RTMModule::recvSettingForbiddenIRIList(const QByteArray& buff) {
    this->sendLogMsg("recv setting forbidden IRI list");
    OSetBanIRIlist oSetBanIRIlist;
    uint8_t len1 = sizeof(GenericHeader);
    uint8_t len2 = sizeof(OSetBanIRIlist);
    memcpy(&oSetBanIRIlist, buff.data() + len1, len2);
    QByteArray byteArray(reinterpret_cast<char*>(&oSetBanIRIlist), len2);
    qDebug() << "0x564 msg body: " << byteArray.toHex();
    // 执行操作代码响应0x23,默认返回0为执行成功
    uint8_t code = 0;
    this->sendControlledOrder(code);
}

// 0x822,发送方位标记,需要接收地图传来的数据，作为Nebula的槽函数，并由Nebula信号emit出去
void RTMModule::sendBearingMarker() {
    this->genericHeader.packType = 0x822;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    // this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);
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
    qDebug() << "send 0x822: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

template <class T>
void printStruct(const T& s)
{
    const char* structName = typeid(T).name(); // 获取结构体名称
    qDebug() << "Printing information for struct: " << structName;

    // 使用位操作获取位字段的值
    for (int i = 0; i < sizeof(T); i += sizeof(int)) {
        int memberValue = 0;
        memcpy(&memberValue, reinterpret_cast<const char*>(&s) + i, sizeof(int));
        qDebug() << "Member" << i << ":" << memberValue;
    }
}

// 0x823,发送当前RTM设置
void RTMModule::sendRTMSettings() {
    this->genericHeader.packType = 0x823;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0x823: " << byteArray.toHex();
    printStruct(oSubRezhRTR20);
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 0x825,发送当前RTM功能
void RTMModule::sendRTMFunction() {
    this->genericHeader.packType = 0x825;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0x825: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 0x827,发送方位路线信息
void RTMModule::sendBearingAndRoute() {
    this->genericHeader.packType = 0x827;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

    QString jsonStr = "{'Number':12685126,'DateTime':'2018-09-28 13:11:37','StartPoint':{'Latitude':55.12345,'Longitude':38.12345,'Altitude':155.12},'TypeBLA':{'InfoName':'无人机类型','InfoValue':'phantom-3'}";
    QByteArray byteArray = jsonStr.toUtf8();
    qDebug() << "send 0x827: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
}

// 0x828,发送禁止IRI列表
void RTMModule::sendForbiddenIRIList() {
    this->genericHeader.packType = 0x828;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0x828: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

// 0x829,发送无线电环境信息
void RTMModule::sendWirelessEnvInfo() {
    this->genericHeader.packType = 0x829;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);

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
    qDebug() << "send 0x829: " << byteArray.toHex();
    this->pTcpSocket->write(byteArray);
    this->pTcpSocket->flush();
    free(data);
}

}