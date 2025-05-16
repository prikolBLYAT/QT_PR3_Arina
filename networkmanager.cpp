#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_clientSocket(nullptr)
    , m_socket(nullptr)
    , m_isConnected(false)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);
}

NetworkManager::~NetworkManager()
{
    closeConnections();
}

bool NetworkManager::startServer(int port)
{
    if (!m_server->listen(QHostAddress::Any, port)) {
        emit error("Server could not start: " + m_server->errorString());
        return false;
    }
    return true;
}

bool NetworkManager::connectToServer(const QString &address, int port)
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        delete m_socket;
    }
    
    m_socket = new QTcpSocket(this);
    
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    
    m_socket->connectToHost(address, port);
    return true;
}

void NetworkManager::sendMessage(const QString &message)
{
    QByteArray data = message.toUtf8();
    
    if (m_clientSocket && m_clientSocket->state() == QTcpSocket::ConnectedState) {
        m_clientSocket->write(data);
    }
    
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->write(data);
    }
}

void NetworkManager::closeConnections()
{
    if (m_server) {
        m_server->close();
    }
    
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
    
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    m_isConnected = false;
}

void NetworkManager::onNewConnection()
{
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
    }
    
    m_clientSocket = m_server->nextPendingConnection();
    
    if (m_clientSocket) {
        connect(m_clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        connect(m_clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::onClientDisconnected);
        m_isConnected = true;
        emit connected();
    }
}

void NetworkManager::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);
    
    emit messageReceived(message);
}

void NetworkManager::onClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket == m_clientSocket) {
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
        m_isConnected = false;
        emit disconnected();
    }
}

void NetworkManager::onConnected()
{
    m_isConnected = true;
    emit connected();
}
