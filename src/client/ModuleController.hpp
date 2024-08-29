#ifndef _MODULECONTROLLER_H_
#define _MODULECONTROLLER_H_

#include "RTMModule.hpp"
#include "PRUEModule.hpp"

namespace NEBULA {

class ModuleController {
 private:
    QVector<RTMModule*> rtmVec;
    QVector<PRUEModule*> prueVec;
 public:
    ModuleController(/* args */);
    ~ModuleController();
    void init();
};

}  // namespace NEBULA

#endif // _MODULECONTROLLER_H_