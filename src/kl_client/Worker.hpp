#ifndef _WORKER_H_
#define _WORKER_H_
#include <QThread>
namespace NEBULA {
class Worker : public QThread
{
private:
    /* data */
public:
    Worker(/* args */);
    ~Worker();
    void run() override;
protected:
    
};


}

#endif // _WORKER_H_