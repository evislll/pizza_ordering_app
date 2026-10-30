#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    MainWindow w;
    w.resize(980, 760);
    w.setMinimumSize(860, 640);
    w.show();

    return a.exec();
}
