#ifndef _CLASSB_H_
#define _CLASSB_H_
#include <QObject>
#include <QDebug>

// 在线程2中的类
class ClassB : public QObject
{
    Q_OBJECT

public:
    ClassB();
signals:
    void dataReady(const QString& data);
public slots:
    void sendDataToClassA();
};

#endif // _CLASSB_H_
