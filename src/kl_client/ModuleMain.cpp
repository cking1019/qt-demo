#include <QCoreApplication>
#include <QDir>
#include <QMutex>
#include "ModuleCommon.hpp"
#include "ModuleController.hpp"
// using namespace CUR_NAMESPACE;

// 写日志功能
void MessWriteLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //定义日志文件存放路径
    QString logDir = QCoreApplication::applicationDirPath() + "\\RunLog\\" + QDateTime::currentDateTime().toString("yyyy_MM_dd")+"\\";
    QString logFile;
    QDir curDir(logDir);
    if(!curDir.exists(logDir))//如果路径不存在则创建
    {
        curDir.mkpath(logDir);
        qDebug() << logDir;
    }
    static QMutex mutex;
    mutex.lock();
    QString contextType;
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
        qDebug().noquote() << msg;
        break;
    case QtCriticalMsg:
        logFile = logDir + "Critical";
        contextType = QString("Critical");
        qDebug().noquote() << msg;
        break;
    case QtFatalMsg:
        logFile = logDir + "Fatal";
        contextType = QString("Fatal");
        qDebug().noquote() << msg;
    }
    QString contextInfo = QString("[%1: %2]").arg(QString(context.file)).arg(context.line);//代码所在文件及行数
    QString contextTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.mmm");
 
    QString mess = QString("%1: [%2] %3: %4").arg(contextTime).arg(contextType).arg(contextInfo).arg(msg);
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
    NEBULA::ModuleController moduleController;
    moduleController.init();

    return a.exec();
}