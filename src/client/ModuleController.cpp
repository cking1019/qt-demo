#include "ModuleController.hpp"

namespace NEBULA {

QString readJson(QString Dev0x20Config) {
    QFile file(Dev0x20Config);
    QString jsonContent;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        jsonContent = in.readAll();
        file.close();
    }
    return jsonContent;
}

ModuleController::ModuleController() {
}

ModuleController::~ModuleController() {
}

void ModuleController::init() {
    auto relayConfig = new QSettings(RELAY_PATH, QSettings::IniFormat);
    relayConfig->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = relayConfig->value("ControlInfo/ControlIP").toString();
    auto serverPort = relayConfig->value("ControlInfo/ControlPort").toInt();
    qDebug() << "server address is " << serverAddress;
    qDebug() << "server port is " << serverPort;

    // 侦测设备配置
    auto iDetectNum = relayConfig->value("DetectDevNum/DevNum").toInt();
    for (int i = 1; i <= iDetectNum; i++) {
        auto moduleAddress = relayConfig->value(QString("DetectDev%1/DevIP").arg(i)).toString();
        auto modulePort = relayConfig->value(QString("DetectDev%1/DevPort").arg(i)).toInt();
        auto Dev0x20Config = relayConfig->value("DetectDev1/Dev0x20Config").toString();
        auto p = new RTMModule();
        p->cfg.serverAddress = serverAddress;
        p->cfg.serverPort = serverPort;
        p->cfg.moduleAddress = moduleAddress;
        p->cfg.modulePort = modulePort;
        p->cfg.module0x20Cfg = readJson(Dev0x20Config);
        this->rtmVec.append(p);
    }

    // 诱骗设备配置
    auto iTrapNum = relayConfig->value("TrapDevNum/DevNum").toInt();
    for (int i = 1; i <= iTrapNum; i++) {
        auto p = new PRUEModule();
        this->prueVec.append(p);
    }

    // 启动所有设备
    for (auto& item : this->rtmVec) {
        item->startup();
    }
}

}