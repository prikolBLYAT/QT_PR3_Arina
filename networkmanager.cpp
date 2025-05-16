#include "networkmanager.h"

/**
 * Конструктор класса NetworkManager
 * Инициализирует объекты для сетевого взаимодействия
 * и устанавливает необходимые соединения сигналов и слотов
 */
NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_clientSocket(nullptr)
    , m_socket(nullptr)
    , m_isConnected(false)
{
    // Создаем экземпляр TCP-сервера
    m_server = new QTcpServer(this);
    
    // Подключаем сигнал о новом соединении к соответствующему слоту
    connect(m_server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);
}

/**
 * Деструктор класса NetworkManager
 * Закрывает все открытые соединения перед уничтожением объекта
 */
NetworkManager::~NetworkManager()
{
    // Закрываем все соединения
    closeConnections();
}

/**
 * Запускает TCP-сервер на указанном порту
 * 
 * @param port Номер порта для прослушивания (1024-65535)
 * @return true в случае успешного запуска, false в случае ошибки
 */
bool NetworkManager::startServer(int port)
{
    // Пытаемся запустить сервер на указанном порту на всех сетевых интерфейсах
    if (!m_server->listen(QHostAddress::Any, port)) {
        // При ошибке отправляем сигнал с текстом ошибки
        emit error("Невозможно запустить сервер: " + m_server->errorString());
        return false;
    }
    return true;
}

/**
 * Подключается к удаленному серверу по указанному адресу и порту
 * 
 * @param address IP-адрес или имя хоста сервера
 * @param port Номер порта сервера (1024-65535)
 * @return true всегда, т.к. подключение асинхронное
 */
bool NetworkManager::connectToServer(const QString &address, int port)
{
    // Если уже есть сокет для подключения к серверу, закрываем его
    if (m_socket) {
        // Отключаемся от хоста
        m_socket->disconnectFromHost();
        // Освобождаем ресурсы
        delete m_socket;
    }
    
    // Создаем новый сокет для подключения к серверу
    m_socket = new QTcpSocket(this);
    
    // Связываем сигналы сокета с нашими слотами
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkManager::onSocketError);
    
    // Пытаемся установить соединение с сервером
    m_socket->connectToHost(address, port);
    return true;
}

/**
 * Отправляет текстовое сообщение через активные соединения
 * Если активны оба соединения (клиентское и серверное),
 * сообщение отправляется через оба
 * 
 * @param message Текст сообщения для отправки
 */
void NetworkManager::sendMessage(const QString &message)
{
    // Преобразуем текстовое сообщение в массив байтов в кодировке UTF-8
    QByteArray data = message.toUtf8();
    
    // Если есть активное клиентское соединение, отправляем через него
    if (m_clientSocket && m_clientSocket->state() == QTcpSocket::ConnectedState) {
        m_clientSocket->write(data);
    }
    
    // Если есть активное соединение с сервером, отправляем через него
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->write(data);
    }
}

/**
 * Закрывает все активные сетевые соединения
 * Останавливает сервер и отключается от удаленного сервера
 */
void NetworkManager::closeConnections()
{
    // Останавливаем сервер, если он запущен
    if (m_server) {
        m_server->close();
    }
    
    // Закрываем соединение с клиентом, если оно установлено
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
    
    // Закрываем соединение с сервером, если оно установлено
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    // Обновляем флаг состояния подключения
    m_isConnected = false;
}

/**
 * Обработчик сигнала о новом входящем подключении
 * Вызывается, когда к нашему серверу подключается клиент
 */
void NetworkManager::onNewConnection()
{
    // Если у нас уже есть клиентское подключение, закрываем его
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
    }
    
    // Получаем сокет нового подключения
    m_clientSocket = m_server->nextPendingConnection();
    
    if (m_clientSocket) {
        // Связываем сигналы нового сокета с нашими слотами
        connect(m_clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
        connect(m_clientSocket, &QTcpSocket::disconnected, this, &NetworkManager::onClientDisconnected);
        
        // Устанавливаем флаг подключения и отправляем сигнал о подключении
        m_isConnected = true;
        emit connected();
    }
}

/**
 * Обработчик получения данных по сети
 * Вызывается, когда в сокете появляются данные для чтения
 */
void NetworkManager::onReadyRead()
{
    // Определяем, от какого сокета пришли данные
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    // Читаем все доступные данные из сокета
    QByteArray data = socket->readAll();
    
    // Преобразуем данные из UTF-8 в строку QString
    QString message = QString::fromUtf8(data);
    
    // Отправляем сигнал с полученным сообщением
    emit messageReceived(message);
}

/**
 * Обработчик отключения клиента
 * Вызывается, когда клиент разрывает соединение с нашим сервером
 */
void NetworkManager::onClientDisconnected()
{
    // Определяем, какой сокет был отключен
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    
    // Если это клиентский сокет, выполняем очистку
    if (socket == m_clientSocket) {
        // Запланируем удаление сокета
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
        
        // Сбрасываем флаг подключения и отправляем сигнал об отключении
        m_isConnected = false;
        emit disconnected();
    }
}

/**
 * Обработчик успешного подключения к серверу
 * Вызывается, когда наш клиент успешно подключается к удаленному серверу
 */
void NetworkManager::onConnected()
{
    // Устанавливаем флаг подключения
    m_isConnected = true;
    
    // Отправляем сигнал об успешном подключении
    emit connected();
}

/**
 * Обработчик ошибок сокета
 * Вызывается при возникновении ошибки в сетевом соединении
 * 
 * @param socketError Код ошибки сокета
 */
void NetworkManager::onSocketError(QAbstractSocket::SocketError socketError)
{
    // Получаем сокет, в котором произошла ошибка
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    
    // Отправляем сигнал с описанием ошибки
    emit error("Ошибка сети: " + socket->errorString());
}
