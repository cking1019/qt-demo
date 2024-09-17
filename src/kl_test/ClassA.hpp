#ifndef _CLASSA_H_
#define _CLASSA_H_

#include <QObject>
#include <QDebug>


class ClassA : public QObject
{
    Q_OBJECT

public:
    ClassA();

public slots:
    void processData(const QString& data);
};

#endif // _CLASSA_H_