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

        p->rtmCustomizedCfg.serverPort = serverPort;
        p->rtmCustomizedCfg.moduleAddress = relayConfig->value(QString("DetectDev%1/DevIP").arg(i)).toString();
        p->rtmCustomizedCfg.modulePort = relayConfig->value(QString("DetectDev%1/DevPort").arg(i)).toInt();
        p->rtmCustomizedCfg.elev = relayConfig->value(QString("DetectDev%1/elev").arg(i)).toFloat();
        p->rtmCustomizedCfg.range = relayConfig->value(QString("DetectDev%1/range").arg(i)).toFloat();
        p->rtmCustomizedCfg.freqMhz = relayConfig->value(QString("DetectDev%1/freqMhz").arg(i)).toFloat();
        p->rtmCustomizedCfg.dFreqMhz = relayConfig->value(QString("DetectDev%1/dFreqMhz").arg(i)).toFloat();
        p->rtmCustomizedCfg.Pow_dBm = relayConfig->value(QString("DetectDev%1/Pow_dBm").arg(i)).toFloat();
        p->rtmCustomizedCfg.SNR_dB = relayConfig->value(QString("DetectDev%1/SNR_dB").arg(i)).toFloat();
        
        p->isDebugOut = relayConfig->value("Common/debugOut").toBool();
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