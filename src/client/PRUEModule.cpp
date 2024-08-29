#include "PRUEModule.hpp"

namespace NEBULA
{
PRUEModule::PRUEModule() {
    this->pCurrentSettingTimer = new QTimer();
    this->pCurrentFunctionTimer = new QTimer();
    this->pCurrentStatusTimer = new QTimer();

    // 实时接收来自服务器的数据,使用基类的socket指针来接收数据，保证基类与派生类都可发送数据，但只允许派生类接收数据，然后发送给基类。
    connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &PRUEModule::onRecvData);

    // 发送RTM设置定时器
    connect(this->pCurrentSettingTimer, &QTimer::timeout, this, &PRUEModule::sendPRUESettings);
    // 发送RTM功能定时器
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
        if (this->pModuleStatueTimer->isActive()) this->pModuleStatueTimer->start(1000);

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

// 接收数据统一接口
void PRUEModule::onRecvData() {
    QByteArray buff = this->pTcpSocket->readAll();
    memcpy(&this->genericHeader, buff.data(), sizeof(GenericHeader));
    qDebug() << "===============================================================";
    qDebug() << "received data from server: " << buff.toHex();
    qDebug() << "the size of pkg: " << buff.size();
    qDebug() << "the type of pkg: " << QString::number(this->genericHeader.packType, 16);
    qDebug() << "the sender is: " << QString::number(this->genericHeader.sender, 16);
    qDebug() << "===============================================================";
    // 策略模式，根据包类型决定转发至哪个函数
    if (this->pkgsComm.contains(this->genericHeader.packType)) {
        this->onReadCommData(buff);
    }
    if (this->pkgsPRUE.contains(this->genericHeader.packType)) {
        this->onReadPRUEData(buff);
    }
}

// 从服务器中读取RTM数据
void PRUEModule::onReadPRUEData(QByteArray& buff) {
    switch (genericHeader.packType) {
        case 0x601: this->recvUpdatePRUESetting(buff); break;
        case 0x201: this->recvSettingBanSector(buff); break;
        case 0x202: this->recvBanRadiation(buff); break;
        default: {
            QString msg = "this is unknown pkg 0x" + this->genericHeader.packType;
            this->sendLogMsg(msg);
            this->sendNote2Operator(msg);
            break;
        }
    }
}
// 发送当前PRUE设置,0xD21
void PRUEModule::sendPRUESettings() {
}
// 发送当前PRUE功能,0xD22
void PRUEModule::sendPRUEFunction() {
}
// 发送已安装的辐射禁止扇区,0xD01
void PRUEModule::sendInstalledBanSector() {
}

// 收到更改PRUE设置,0x601
void PRUEModule::recvUpdatePRUESetting(QByteArray& buff) {
}
// 收到设置辐射禁止扇区,0x201
void PRUEModule::recvSettingBanSector(QByteArray& buff) {
}
// 收到设置辐射禁止,0x202
void PRUEModule::recvBanRadiation(QByteArray& buff) {
}
}  // namespace NEBULA
