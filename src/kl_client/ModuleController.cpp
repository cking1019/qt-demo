#include "ModuleController.hpp"
#include "RTMModule.hpp"
#include "NebulaCommon.hpp"

namespace NEBULA {


ModuleController::ModuleController() {
}

ModuleController::~ModuleController() {
}

void ModuleController::init() {
    auto commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = commCfg->value("common/serverIP").toString();
    auto serverPort = commCfg->value("common/serverPort").toInt();
    qDebug() << QString("server address is %1:%2").arg(serverAddress).arg(serverPort);

    // 侦测设备配置
    auto num = commCfg->value("Detector/num").toInt();
    for (int i = 0; i < num; i++) {
        auto rtm = new RTMModule();
        rtm->m_commCfg.serverAddress  = serverAddress;
        rtm->m_commCfg.serverPort     = serverPort;
        // 连接地图服务器来读取数据
        auto rtmNebula = new NebulaCommon();
        connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtm, &RTMModule::sendTargetMarker822);
        rtmVec.append(rtm);
    }

    // 启动所有设备
    for (auto& item : rtmVec) {
        item->startup();
    }
    // for (auto& item : prueVec) {
    //     item->startup();
    // }
}

}