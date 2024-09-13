#ifndef _DEMO_H_
#define _DEMO_H_
#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

class Demo : public QObject{
    Q_OBJECT
 public:
    QTcpSocket* pSocket;
    QTimer* pTimer;
    QTimer* pReconnTimer;
    
    explicit Demo(int a, int b);
    void checkSockState();
    void reconnect();
private:
    int a, b;
};

#endif // _DEMO_H_