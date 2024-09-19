#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QtDebug>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    QWidget window;
    window.resize(720, 480);
    window.setWindowTitle("Login Window");

    QLineEdit *usernameEdit = new QLineEdit(&window);
    QLineEdit *passwordEdit = new QLineEdit(&window);
    passwordEdit->setEchoMode(QLineEdit::Password);

    QPushButton *loginButton = new QPushButton("Login", &window);
    QObject::connect(loginButton, &QPushButton::clicked, [&](){
        QString username = usernameEdit->text();
        QString password = passwordEdit->text();

        if(username == "admin" && password == "123456") {
            QMessageBox::information(&window, "Login", "Login successful!");
        } else {
            QMessageBox::critical(&window, "Login", "Invalid username or password.");
        }
    });

    QFormLayout *layout = new QFormLayout;
    layout->addRow("Username:", usernameEdit);
    layout->addRow("Password:", passwordEdit);
    layout->addWidget(loginButton);

    window.setLayout(layout);

    window.show();
    return a.exec();
}
