#ifndef _NEBULACONTROLLER_H_
#define _NEBULACONTROLLER_H_

#include <QObject>
#include <QDebug>

class NebulaController : public QObject
{
  Q_OBJECT
private:
  /* data */
public:
  NebulaController(/* args */);
  ~NebulaController();
public slots:
    void slots_dosomething(int n);
};

#endif // _NEBULACONTROLLER_H_
