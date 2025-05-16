#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    bool startServer(int port);
    bool connectToServer(const QString &address, int port);
    void sendMessage(const QString &message);
    void closeConnections();

signals:
    void messageReceived(const QString &message);
    void connected();
    void disconnected();
    void error(const QString &errorMessage);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
    void onConnected();

private:
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket;
    QTcpSocket *m_socket;
    bool m_isConnected;
};

#endif // NETWORKMANAGER_H 
