#include "ModuleCore.hpp"
#include "RTMModule.hpp"
#include "NebulaCommon.hpp"

using namespace NEBULA;

ModuleCore::ModuleCore() {
    auto commCfg = new QSettings(COMM_CFG, QSettings::IniFormat);
    commCfg->setIniCodec(QTextCodec::codecForName("utf-8"));
    qDebug() << QString("VOI    Server: %1:%2").arg(commCfg->value("VOI/serverAddress").toString())
                                               .arg(commCfg->value("VOI/serverPort").toInt());
    qDebug() << QString("Nebula Server: %1:%2").arg(commCfg->value("Nebula/nebulaAddress").toString())
                                               .arg(commCfg->value("Nebula/nebulaPort").toInt());
    
    // 侦测设备配置
    for (qint16 id = 1; id <= commCfg->value("Common/DetectorNum").toInt(); id++) {
        auto rtmModule = new RTMModule(id);
        auto rtmNebula = new NebulaCommon(id);
        auto *workerThreadTcp = new QThread(this);
        auto *workerThreadUdp = new QThread(this);
        rtmModule->moveToThread(workerThreadTcp);
        rtmNebula->moveToThread(workerThreadUdp);

        // Qt::BlockingQueuedConnection，槽函数会在信号发射的线程上阻塞执行，直到槽函数执行完毕。
        connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtmModule, &RTMModule::sendTarget822, Qt::BlockingQueuedConnection);
        connect(workerThreadTcp, &QThread::started, rtmModule, &RTMModule::startup);
        connect(workerThreadUdp, &QThread::started, rtmNebula, &NebulaCommon::startup);
        // m_threadVec.append(workerThreadTcp);
        // m_threadVec.append(workerThreadUdp);
    }

    // 干扰设备配置
    for (qint16 id = 1; id <= commCfg->value("Common/JammerNum").toInt(); id++) {
        auto prueModule = new PRUEModule(id);
        // auto rtmNebula = new NebulaCommon(id);
        auto *workerThreadTcp = new QThread(this);
        prueModule->moveToThread(workerThreadTcp);
        connect(workerThreadTcp, &QThread::started, prueModule, &PRUEModule::startup);
        m_threadVec.append(workerThreadTcp);
    }

    for(auto& item : m_threadVec) 
    {
        item->start();
        QThread::msleep(1000);
    }
}

ModuleCore::~ModuleCore() {
}