#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w(a.arguments());    // decides during construction if it needs to display self or just convay commands

    if(! w.singel_app() ){          // only if there is not already a other instance running
        return a.exec();            // run app
    }
    else {return 0;}                // else just exit after sending commands
}
/*

switch from txtfiles to sqlite db


*/
