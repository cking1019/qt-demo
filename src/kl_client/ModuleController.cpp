#include "ModuleController.hpp"

namespace NEBULA {

ModuleController::ModuleController() {
}

ModuleController::~ModuleController() {
}

void ModuleController::init() {
    auto relayConfig = new QSettings(RELAY_PATH, QSettings::IniFormat);
    relayConfig->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = relayConfig->value("Common/serverIP").toString();
    auto serverPort = relayConfig->value("Common/serverPort").toInt();
    qDebug() << "server address is " << serverAddress;
    qDebug() << "server port is " << serverPort;

    // 侦测设备配置
    auto iDetectNum = relayConfig->value("DetectDevNum/DevNum").toInt();
    for (int i = 1; i <= iDetectNum; i++) {
        auto p = new RTMModule();
        p->commCfg.serverAddress = serverAddress;
        p->commCfg.serverPort = serverPort;
        p->commCfg.moduleCfg20 = readJson(relayConfig->value("DetectDev1/DevConfig20").toString());

        p->m_rtmCustomizedCfg.serverPort = serverPort;
        p->m_rtmCustomizedCfg.moduleAddress = relayConfig->value(QString("DetectDev%1/DevIP").arg(i)).toString();
        p->m_rtmCustomizedCfg.modulePort = relayConfig->value(QString("DetectDev%1/DevPort").arg(i)).toInt();
        
        p->isDebugOut = relayConfig->value("Common/debugOut").toBool();
        rtmVec.append(p);
    }

    // 诱骗设备配置
    auto iTrapNum = relayConfig->value("TrapDevNum/DevNum").toInt();
    for (int i = 1; i <= iTrapNum; i++) {
        auto p = new PRUEModule();
        prueVec.append(p);
    }

    // 启动所有设备
    for (auto& item : rtmVec) {
        item->startup();
    }
}

}