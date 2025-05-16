#include "databasemanager.h"

// Конструктор класса
DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

// Деструктор класса: закрывает соединение с базой данных
DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

// Открывает соединение с базой данных SQLite по указанному пути
bool DatabaseManager::openDatabase(const QString &dbPath)
{
    // Создаем подключение к SQLite
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);
    
    // Пытаемся открыть соединение
    if (!m_database.open()) {
        qDebug() << "Ошибка открытия базы данных:" << m_database.lastError().text();
        return false;
    }
    
    // Создаем необходимые таблицы
    return createTables();
}

// Создает нужные таблицы в базе данных, если они еще не существуют
bool DatabaseManager::createTables()
{
    QSqlQuery query;
    // Создаем таблицу сообщений с полями id, timestamp, message и direction
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME,"
        "message TEXT,"
        "direction TEXT"
        ")"
    );
    
    if (!success) {
        qDebug() << "Ошибка создания таблиц:" << query.lastError().text();
        return false;
    }
    
    return true;
}

// Записывает сообщение в журнал с указанием входящее оно или исходящее
void DatabaseManager::logMessage(const QString &message, bool incoming)
{
    QSqlQuery query;
    // Подготавливаем SQL-запрос для вставки записи
    query.prepare("INSERT INTO messages (timestamp, message, direction) VALUES (?, ?, ?)");
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(message);
    query.addBindValue(incoming ? "incoming" : "outgoing");
    
    // Выполняем запрос
    if (!query.exec()) {
        qDebug() << "Ошибка журналирования сообщения:" << query.lastError().text();
    }
}

// Получает все сообщения из базы данных и возвращает их в структурированном виде
QList<QPair<QString, QPair<QString, bool>>> DatabaseManager::getMessages()
{
    QList<QPair<QString, QPair<QString, bool>>> messages;
    
    // Запрашиваем все сообщения, отсортированные по времени
    QSqlQuery query("SELECT timestamp, message, direction FROM messages ORDER BY timestamp");
    
    // Обрабатываем результаты запроса
    while (query.next()) {
        QString timestamp = query.value(0).toString();
        QString message = query.value(1).toString();
        bool isIncoming = query.value(2).toString() == "incoming";
        
        // Добавляем сообщение в список результатов
        messages.append(qMakePair(timestamp, qMakePair(message, isIncoming)));
    }
    
    return messages;
} 