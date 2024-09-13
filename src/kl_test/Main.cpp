#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include "Worker.hpp"
#include "Demo.hpp"
#include <iostream>
using namespace std;

// 前向声明类B
class B;

class A {
private:
    int data;

public:
    A(int d) : data(d) {}

    void displayData() {
        std::cout << "Data in class A: " << data << std::endl;
    }

    // 声明类B为友元类
    friend class B;
};

class B {
public:
    void modifyData(A* a, int newData) {
        a->data += 1000;
        a->displayData();
    }
};

void process() {
    while(true) {
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    A a(10);
    a.displayData();
    B b;
    b.modifyData(&a, 100);
    
    QThread th;
    th.tr("hell world");
    return app.exec();
}
