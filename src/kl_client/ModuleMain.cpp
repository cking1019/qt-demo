#include <QCoreApplication>
#include "ModuleBase.hpp"
#include "NebulaCommon.hpp"
#include "RTMModule.hpp"
#include "PRUEModule.hpp"
// using namespace CUR_NAMESPACE;
using namespace NEBULA;

QVector<QThread*> m_threadVec;

// 写日志功能
void MessWriteLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString logDir = QCoreApplication::applicationDirPath() + "\\logs\\" + QDateTime::currentDateTime().toString("yyyy_MM_dd")+"\\";
    QDir curDir(logDir);
    if(!curDir.exists(logDir))//如果路径不存在则创建
    {
        curDir.mkpath(logDir);
        qDebug() << logDir;
    }
    static QMutex mutex;
    mutex.lock();
    QString contextType;
    QString logFile;
    switch(type)
    {
    case QtDebugMsg:
        logFile = logDir + "Debug";
        contextType = QString("Debug");
        qDebug().noquote() << msg;
        break;
    case QtWarningMsg:
        logFile = logDir + "Warning";
        contextType = QString("Warning");
        break;
    case QtCriticalMsg:
        logFile = logDir + "Critical";
        contextType = QString("Critical");
        break;
    case QtFatalMsg:
        logFile = logDir + "Fatal";
        contextType = QString("Fatal");
    }
    QString contextInfo = QString("[%1:%2]").arg(QString(context.file)).arg(context.line); //代码所在文件及行数
    QString contextTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.mmm");
 
    QString mess = QString("%1:[%2]:%3::%4").arg(contextTime).arg(contextType).arg(contextInfo).arg(msg);
    QFile contextFile(logFile + ".log");
    contextFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream contextStream(&contextFile);
    contextStream << mess << "\r\n";
    contextFile.flush();
    contextFile.close();
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(MessWriteLog);//安装消息处理程序

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
        auto *workerThreadTcp = new QThread();
        auto *workerThreadUdp = new QThread();
        rtmModule->moveToThread(workerThreadTcp);
        rtmNebula->moveToThread(workerThreadUdp);

        // Qt::BlockingQueuedConnection，槽函数会在信号发射的线程上阻塞执行，直到槽函数执行完毕。
        QObject::connect(rtmNebula, &NebulaCommon::signalSendDetectTarget2Ctl, rtmModule, &RTMModule::sendTarget822, Qt::BlockingQueuedConnection);
        QObject::connect(workerThreadTcp, &QThread::started, rtmModule, &RTMModule::startup);
        QObject::connect(workerThreadUdp, &QThread::started, rtmNebula, &NebulaCommon::startup);
        m_threadVec.append(workerThreadTcp);
        m_threadVec.append(workerThreadUdp);
    }

    // 干扰设备配置
    for (qint16 id = 1; id <= commCfg->value("Common/JammerNum").toInt(); id++) {
        auto prueModule = new PRUEModule(id);
        // auto rtmNebula = new NebulaCommon(id);
        auto *workerThreadTcp = new QThread();
        prueModule->moveToThread(workerThreadTcp);
        QObject::connect(workerThreadTcp, &QThread::started, prueModule, &PRUEModule::startup);
        m_threadVec.append(workerThreadTcp);
    }

    for(auto& item : m_threadVec) 
    {
        item->start();
        QThread::msleep(1000);
    }

    return a.exec();
}