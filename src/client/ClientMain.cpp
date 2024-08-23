#include <QCoreApplication>
#include "RTMController.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    RTMController rtmController;
    rtmController.init();

    return a.exec();
}