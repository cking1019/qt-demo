#include "ModuleController.hpp"

namespace NEBULA {

ModuleController::ModuleController() {
}

ModuleController::~ModuleController() {
}

void ModuleController::init() {
    auto commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    auto rtmCfg = new QSettings(RTM_CFG, QSettings::IniFormat);
    auto prueCfg = new QSettings(PRUE_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));
    rtmCfg->setIniCodec(QTextCodec::codecForName("utf-8"));
    prueCfg->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = commCfg->value("common/serverIP").toString();
    auto serverPort = commCfg->value("common/serverPort").toInt();
    qDebug() << "server address is " << serverAddress;
    qDebug() << "server port is " << serverPort;

    // 侦测设备配置
    auto iDetectNum = rtmCfg->value("DetectDevNum/num").toInt();
    for (int i = 1; i <= iDetectNum; i++) {
        auto p = new RTMModule();
        p->m_commCfg.serverAddress  = serverAddress;
        p->m_commCfg.serverPort     = serverPort;
        rtmVec.append(p);
    }

    // 诱骗设备配置
    auto iTrapNum = prueCfg->value("JamDevNum/num").toInt();
    for (int i = 1; i <= iTrapNum; i++) {
        auto p = new PRUEModule();
        p->m_commCfg.serverAddress  = serverAddress;
        p->m_commCfg.serverPort     = serverPort;
        prueVec.append(p);
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