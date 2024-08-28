#include "RTMModule.hpp"

namespace NEBULA
{
RTMModule::RTMModule() {
  connect(this->pTcpSocket, &QTcpSocket::readyRead, this, &RTMModule::onReadDataRTM);
}

RTMModule::~RTMModule() {
    
}

void RTMModule::onReadDataRTM() {
    QByteArray buff = this->pTcpSocket->readAll();
    memcpy(&this->genericHeader, buff.data(), sizeof(GenericHeader));
    qDebug() << "=======================================";
    qDebug() << "received data from server: " << buff.toHex();
    qDebug() << "the size of pkg: " << buff.size();
    qDebug() << "the type of pkg: " << QString::number(this->genericHeader.packType, 16);
    qDebug() << "the sender is: " << QString::number(this->genericHeader.sender, 16);
    qDebug() << "=======================================";
    switch (genericHeader.packType) {
        case 0x561: this->recvChangingRTMSettings(buff); break;
        case 0x563: this->recvRequestForbiddenIRIList(buff); break;
        case 0x564: this->recvSettingForbiddenIRIList(buff); break;
        default: {
            qDebug() << "this is unknown pkg 0x" << this->genericHeader.packType;
            this->sendLogMsg("this is unknown pkg");
            this->sendNote2Operator("this is unknown pkg");
            break;
        }
    }
}

// 0x561,收到更改RTM设置
void RTMModule::recvChangingRTMSettings(QByteArray buff){

}
// 0x563,请求禁止IRI列表
void RTMModule::recvRequestForbiddenIRIList(QByteArray buff){

}
// 0x564,设置禁止IRI列表
void RTMModule::recvSettingForbiddenIRIList(QByteArray buff){

}

// 0x822,发送方位标记
void RTMModule::sendBearingMarker() {
    this->genericHeader.packType = 0x3;
    this->genericHeader.dataSize = sizeof(OBearingMark);
    this->genericHeader.packIdx++;
    this->genericHeader.checkSum = calcChcekSum((char*)&this->genericHeader, sizeof(GenericHeader) - 2);
    
}
// 0x823,发送当前RTM设置
void RTMModule::sendRTMSettings() {

}
// 0x825,发送当前RTM功能
void RTMModule::sendRTMFunction() {

}
// 0x827,发送方位路线信息
void RTMModule::sendBearingAndRoute() {

}
// 0x828,发送禁止IRI列表
void RTMModule::sendForbiddenIRIList() {

}
// 0x829,发送无线电环境信息
void RTMModule::sendWirelessEnvInfo() {

}


}