#include "ModuleCore.hpp"
#include "RTMModule.hpp"
#include "NebulaCommon.hpp"

using namespace NEBULA;

ModuleCore::ModuleCore() {
    auto commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = commCfg->value("VOI/serverAddress").toString();
    auto serverPort = commCfg->value("VOI/serverPort").toInt();
    qDebug() << QString("TCP Server Address and Port is %1:%2").arg(serverAddress).arg(serverPort);

    // auto rtmNebula = new NebulaCommon();
    // 侦测设备配置
    auto detectorNum = commCfg->value("common/DetectorNum").toInt();
    for (qint16 i = 1; i <= detectorNum; i++) {
        auto rtm = new RTMModule(i);
        auto rtmNebula = new NebulaCommon(i);
        connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtm, &RTMModule::sendTargetMarker822);
        rtmVec.append(rtm);
    }
}

ModuleCore::~ModuleCore() {
}

void ModuleCore::init() {
    // for (auto& item : prueVec) item->startup();
    // 启动所有设备
    for (auto& item : rtmVec) item->startup();

}