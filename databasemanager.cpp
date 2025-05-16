#include "databasemanager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::openDatabase(const QString &dbPath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qDebug() << "Error opening database:" << m_database.lastError().text();
        return false;
    }
    
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME,"
        "message TEXT,"
        "direction TEXT"
        ")"
    );
    
    if (!success) {
        qDebug() << "Error creating tables:" << query.lastError().text();
        return false;
    }
    
    return true;
}

void DatabaseManager::logMessage(const QString &message, bool incoming)
{
    QSqlQuery query;
    query.prepare("INSERT INTO messages (timestamp, message, direction) VALUES (?, ?, ?)");
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(message);
    query.addBindValue(incoming ? "incoming" : "outgoing");
    
    if (!query.exec()) {
        qDebug() << "Error logging message:" << query.lastError().text();
    }
} 