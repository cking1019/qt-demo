#include "ModuleCore.hpp"
#include "RTMModule.hpp"
#include "NebulaCommon.hpp"

using namespace NEBULA;

ModuleCore::ModuleCore() {
    auto commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));

    auto serverAddress = commCfg->value("VOI/serverAddress").toString();
    auto serverPort = commCfg->value("VOI/serverPort").toInt();
    qDebug() << QString("TCP Server: %1:%2").arg(serverAddress).arg(serverPort);

    // 侦测设备配置
    auto detectorNum = commCfg->value("common/DetectorNum").toInt();
    for (qint16 id = 1; id <= detectorNum; id++) {
        auto rtmModule = new RTMModule(id);
        auto rtmNebula = new NebulaCommon(id);
        

        auto *workerThreadTcp = new QThread(this);
        auto *workerThreadUdp = new QThread(this);
        rtmModule->moveToThread(workerThreadTcp);
        rtmNebula->moveToThread(workerThreadUdp);

        connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtmModule, &RTMModule::sendTargetMarker822);
        
        connect(workerThreadTcp, &QThread::started, rtmModule, &RTMModule::startup);
        connect(workerThreadUdp, &QThread::started, rtmNebula, &NebulaCommon::startup);
        workerThreadTcp->start();
        workerThreadUdp->start();
        rtmVec.append(rtmModule);
        // QTimer::singleShot(0, rtmNebula, &NebulaCommon::sendDetectTarget2Ctl);
    }

    // 干扰设备配置
    // auto jammerNum = commCfg->value("common/JammerNum").toInt();
    // for (qint16 id = 1; id <= detectorNum; id++) {
    //     auto prue = new PRUEModule(id);
    //     // auto rtmNebula = new NebulaCommon(id);
    //     // connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtmModule, &RTMModule::sendTargetMarker822);
    //     prueVec.append(prue);
    // }
}

ModuleCore::~ModuleCore() {
}

// 启动所有设备
void ModuleCore::init() {
    // for (auto& item : prueVec) item->startup();
    // for (auto& item : rtmVec) item->startup();
}