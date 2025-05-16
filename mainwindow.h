#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include "networkmanager.h"
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartServer();
    void onConnectToServer();
    void onSendMessage();
    void onMessageReceived(const QString &message);
    void onConnected();
    void onDisconnected();
    void onError(const QString &errorMessage);
    void onShowDatabase();

private:
    Ui::MainWindow *ui;
    NetworkManager *m_networkManager;
    DatabaseManager *m_databaseManager;
    int m_serverPort;
    QString m_clientAddress;
    int m_clientPort;

    void displayMessage(const QString &message, bool incoming);
    void processDatabasePath();
};
#endif // MAINWINDOW_H
