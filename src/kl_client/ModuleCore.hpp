#ifndef _MODULECONTROLLER_H_
#define _MODULECONTROLLER_H_

#include "RTMModule.hpp"
#include "PRUEModule.hpp"

namespace NEBULA {

class ModuleCore : public QObject
{
Q_OBJECT
 private:
    QVector<RTMModule*> rtmVec;
    QVector<PRUEModule*> prueVec;
 public:
    ModuleCore(/* args */);
    ~ModuleCore();
    void init();
};

}  // namespace NEBULA

#endif // _MODULECONTROLLER_H_