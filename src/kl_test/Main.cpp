#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include "Worker.hpp"
#include "Demo.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Demo demo = {1, 2};
    return a.exec();
}
