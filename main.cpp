#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set application info
    QApplication::setApplicationName("SimpleChat");
    QApplication::setApplicationVersion("1.0");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Simple Chat Application");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add database path option
    QCommandLineOption dbOption(QStringList() << "db", 
                              "Specify database path", "database");
    parser.addOption(dbOption);
    
    parser.process(a);
    
    MainWindow w;
    w.show();
    return a.exec();
}
