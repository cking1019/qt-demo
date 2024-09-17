#include "ClassA.hpp"

ClassA::ClassA() {

}

void ClassA::processData(const QString& data)
{
    // 处理接收到的数据
    qDebug() << "Data processed in ClassA: " << data;
}