#include "MainWin.hpp"

MainWin::MainWin(QWidget* parent) : QWidget(parent) {
    form.setupUi(this);
    this->setWindowTitle("stuinfo windows");

    int nCount = 0;
    for(int i = 0; i < 20; i++) {
        int nCount = form.tableWidget->rowCount();
        form.tableWidget->insertRow(nCount);
        form.tableWidget->setItem(i, 0, new QTableWidgetItem(QString("%1").arg(i + 1)));
        form.tableWidget->setItem(i, 1, new QTableWidgetItem("Demo"));
        form.tableWidget->setItem(i, 2, new QTableWidgetItem(QString("%1").arg(25)));
        form.tableWidget->setItem(i, 3, new QTableWidgetItem(QDate::currentDate().toString()));
    }
}

MainWin::~MainWin() {

}