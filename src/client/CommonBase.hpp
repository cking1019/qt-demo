#ifndef _COMMONBASE_H_
#define _COMMONBASE_H_

#include <QObject>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QHostAddress>
#include <QThread>

class CommonBase
{
private:
  /* data */
public:
  CommonBase(/* args */);
  ~CommonBase();
  static qint16 calcChcekSum(const char* sMess,int nCnt);
};

#endif // _COMMONBASE_H_
