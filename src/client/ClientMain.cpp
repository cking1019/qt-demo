#include <QCoreApplication>
#include "CommonBase.hpp"
#include "CommonHeader.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CommonBase commonBase;
    commonBase.init();

    return a.exec();
}