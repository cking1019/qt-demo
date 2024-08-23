#include "NebulaController.hpp"


NebulaController::NebulaController(/* args */)
{
  
}

NebulaController::~NebulaController()
{
}

void NebulaController::slots_dosomething(int n) {
  qDebug() << "this is value from signals: " << n;
}