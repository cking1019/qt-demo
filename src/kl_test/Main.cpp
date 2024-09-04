#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <iostream>
#include "Demo.hpp"
#include "../kl_client/CommonHeader.hpp"

 class Base {
    public:
   virtual void doSomething(int i) const {
     std::cout << "This is from Base with " << i << std::endl;
   }
 };
 
 class Derivied : public Base {
    public:
   void doSomething(int i) {
     std::cout << "This is from Derived with " << i << std::endl;
   }
 };



int main(int argc, char *argv[])
{
    Base* pB = new Base();
    Base* dB = new Derivied();
    pB->doSomething(1);
    dB->doSomething(2);
    return 0;
}