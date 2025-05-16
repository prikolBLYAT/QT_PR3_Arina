#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QPair>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase(const QString &dbPath);
    void logMessage(const QString &message, bool incoming);
    QList<QPair<QString, QPair<QString, bool>>> getMessages();

private:
    QSqlDatabase m_database;
    bool createTables();
};

#endif // DATABASEMANAGER_H 