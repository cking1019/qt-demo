#include <QCoreApplication>
#include "CommonModule.hpp"
#include "ModuleController.hpp"
// using namespace CUR_NAMESPACE;


int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);
    NEBULA::ModuleController moduleController;
    moduleController.init();

    return a.exec();
    return 0;
}