#ifndef _MODULECONTROLLER_H_
#define _MODULECONTROLLER_H_

#include "RTMModule.hpp"
#include "PRUEModule.hpp"
#include <QVector>
#include <QSettings>

namespace NEBULA {

class ModuleController
{
private:
    /* data */
    QVector<RTMModule*> rtmVec;
    QVector<PRUEModule*> prueVec;
public:
    ModuleController(/* args */);
    ~ModuleController();
    void init();
};

}

#endif // _MODULECONTROLLER_H_