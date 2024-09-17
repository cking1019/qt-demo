#include "ClassB.hpp"
#include <QTimer>

ClassB::ClassB() {
    QTimer* m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &ClassB::sendDataToClassA);
    m_timer->start(1000);
}

void ClassB::sendDataToClassA()
{
    // 发送数据给 ClassA
    emit dataReady("Hello from ClassB");
}