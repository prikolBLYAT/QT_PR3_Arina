#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>

/*
 * Класс для управления сетевым взаимодействием
 * Выступает как в роли сервера, так и в роли клиента,
 * обеспечивая двустороннюю связь между приложениями чата
 */
class NetworkManager : public QObject
{
    Q_OBJECT
public:
    // Конструктор класса
    explicit NetworkManager(QObject *parent = nullptr);
    // Деструктор класса
    ~NetworkManager();

    // Запускает сервер на указанном порту
    bool startServer(int port);
    // Подключается к серверу по адресу и порту
    bool connectToServer(const QString &address, int port);
    // Отправляет сообщение через активные соединения
    void sendMessage(const QString &message);
    // Закрывает все активные соединения
    void closeConnections();

signals:
    // Сигнал о получении нового сообщения
    void messageReceived(const QString &message);
    // Сигнал об успешном подключении
    void connected();
    // Сигнал о разрыве соединения
    void disconnected();
    // Сигнал об ошибке в сети
    void error(const QString &errorMessage);

private slots:
    // Обработчик нового подключения к серверу
    void onNewConnection();
    // Обработчик получения данных
    void onReadyRead();
    // Обработчик отключения клиента
    void onClientDisconnected();
    // Обработчик успешного подключения к серверу
    void onConnected();
    // Обработчик ошибок сокета
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    // Объект сервера TCP
    QTcpServer *m_server;
    // Сокет для входящего подключения (когда мы сервер)
    QTcpSocket *m_clientSocket;
    // Сокет для исходящего подключения (когда мы клиент)
    QTcpSocket *m_socket;
    // Флаг состояния подключения
    bool m_isConnected;
};

#endif // NETWORKMANAGER_H 
