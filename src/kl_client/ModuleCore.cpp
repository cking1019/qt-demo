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
 
    // 侦测设备配置
    auto detectorNum = commCfg->value("common/DetectorNum").toInt();
    for (qint16 id = 1; id <= detectorNum; id++) {
        auto rtm = new RTMModule(id);
        // auto rtmNebula = new NebulaCommon(id);
        // connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtm, &RTMModule::sendTargetMarker822);
        rtmVec.append(rtm);
    }

    // 干扰设备配置
    auto jammerNum = commCfg->value("common/JammerNum").toInt();
    for (qint16 id = 1; id <= detectorNum; id++) {
        auto prue = new PRUEModule(id);
        // auto rtmNebula = new NebulaCommon(id);
        // connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtm, &RTMModule::sendTargetMarker822);
        prueVec.append(prue);
    }
}

ModuleCore::~ModuleCore() {
}

// 启动所有设备
void ModuleCore::init() {
    for (auto& item : prueVec) item->startup();
    // for (auto& item : rtmVec) item->startup();
}