#include <QCoreApplication>
#include <QDir>
#include <QMutex>
#include "CommonModule.hpp"
#include "ModuleController.hpp"
// using namespace CUR_NAMESPACE;

//写日志功能
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
    static QMutex mutex;//互斥量
    mutex.lock();
    QString contextType;//日志类型
    switch(type)
    {
    case QtDebugMsg:
        logFile=logDir+"Debug";
        contextType = QString("Debug");
        qDebug().noquote() << msg;
        break;
    case QtWarningMsg:
        logFile=logDir+"Warning";
        contextType = QString("Warning");
        qDebug().noquote() << msg;
        break;
    case QtCriticalMsg:
        logFile=logDir+"Critical";
        contextType = QString("Critical");
        qDebug().noquote() << msg;
        break;
    case QtFatalMsg:
        logFile=logDir+"Fatal";
        contextType = QString("Fatal");
        qDebug().noquote() << msg;
    }
    QString contextInfo = QString("[%1: %2]").arg(QString(context.file)).arg(context.line);//代码所在文件及行数
    QString contextTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.mmm");//当前时间
 
    QString mess = QString("%1: [%2] %3: %4").arg(contextTime).arg(contextType).arg(contextInfo).arg(msg);//一行日志信息
    QFile contextFile(logFile+".log");
    contextFile.open(QIODevice::WriteOnly | QIODevice::Append);//以只写或追加方式打开文件
    QTextStream contextStream(&contextFile);//定义文件流
    contextStream << mess << "\r\n";//将文件流写入文件
    contextFile.flush();
    contextFile.close();
    mutex.unlock();//释放互斥量
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(MessWriteLog);//安装消息处理程序
    NEBULA::ModuleController moduleController;
    moduleController.init();

    return a.exec();
}