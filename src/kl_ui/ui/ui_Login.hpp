/********************************************************************************
** Form generated from reading UI file 'Login.ui'
**
** Created by: Qt User Interface Compiler version 5.15.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *userLogin;
    QFormLayout *formLayout;
    QLabel *accountL;
    QLineEdit *accountEd;
    QLabel *passwdL;
    QLineEdit *passwdEd;
    QHBoxLayout *horizontalLayout;
    QPushButton *loginBtn;
    QPushButton *registerBtn;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(646, 466);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayoutWidget = new QWidget(centralwidget);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(110, 50, 361, 272));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(20);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(20, 20, 20, 20);
        userLogin = new QLabel(verticalLayoutWidget);
        userLogin->setObjectName(QString::fromUtf8("userLogin"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        userLogin->setFont(font);
        userLogin->setLayoutDirection(Qt::LeftToRight);
        userLogin->setStyleSheet(QString::fromUtf8("font-size: 30px"));
        userLogin->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(userLogin);

        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setHorizontalSpacing(0);
        formLayout->setVerticalSpacing(20);
        formLayout->setContentsMargins(10, 10, 10, 10);
        accountL = new QLabel(verticalLayoutWidget);
        accountL->setObjectName(QString::fromUtf8("accountL"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Adobe Heiti Std"));
        font1.setPointSize(14);
        accountL->setFont(font1);

        formLayout->setWidget(0, QFormLayout::LabelRole, accountL);

        accountEd = new QLineEdit(verticalLayoutWidget);
        accountEd->setObjectName(QString::fromUtf8("accountEd"));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Adobe Heiti Std"));
        font2.setPointSize(12);
        accountEd->setFont(font2);
        accountEd->setAutoFillBackground(false);

        formLayout->setWidget(0, QFormLayout::FieldRole, accountEd);

        passwdL = new QLabel(verticalLayoutWidget);
        passwdL->setObjectName(QString::fromUtf8("passwdL"));
        passwdL->setFont(font1);

        formLayout->setWidget(1, QFormLayout::LabelRole, passwdL);

        passwdEd = new QLineEdit(verticalLayoutWidget);
        passwdEd->setObjectName(QString::fromUtf8("passwdEd"));
        passwdEd->setFont(font2);
        passwdEd->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(1, QFormLayout::FieldRole, passwdEd);


        verticalLayout->addLayout(formLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(10, 10, 10, 10);
        loginBtn = new QPushButton(verticalLayoutWidget);
        loginBtn->setObjectName(QString::fromUtf8("loginBtn"));
        loginBtn->setStyleSheet(QString::fromUtf8("font-size: 24px"));
        loginBtn->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(loginBtn);

        registerBtn = new QPushButton(verticalLayoutWidget);
        registerBtn->setObjectName(QString::fromUtf8("registerBtn"));
        registerBtn->setStyleSheet(QString::fromUtf8("font-size: 24px"));

        horizontalLayout->addWidget(registerBtn);


        verticalLayout->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 646, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        userLogin->setText(QCoreApplication::translate("MainWindow", "\347\224\250\346\210\267\347\231\273\345\275\225", nullptr));
        accountL->setText(QCoreApplication::translate("MainWindow", "\350\264\246\345\217\267\357\274\232", nullptr));
        passwdL->setText(QCoreApplication::translate("MainWindow", "\345\257\206\347\240\201\357\274\232", nullptr));
        passwdEd->setInputMask(QString());
        loginBtn->setText(QCoreApplication::translate("MainWindow", "\347\231\273\345\275\225", nullptr));
        registerBtn->setText(QCoreApplication::translate("MainWindow", "\346\263\250\345\206\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGIN_H
