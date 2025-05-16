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

/*
 * Класс для управления базой данных сообщений
 * Отвечает за подключение к базе данных, создание таблиц и журналирование сообщений
 */
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    // Конструктор класса
    explicit DatabaseManager(QObject *parent = nullptr);
    // Деструктор класса
    ~DatabaseManager();

    // Открывает соединение с базой данных по указанному пути
    bool openDatabase(const QString &dbPath);
    // Записывает сообщение в журнал (базу данных)
    void logMessage(const QString &message, bool incoming);
    // Получает все сообщения из базы данных
    QList<QPair<QString, QPair<QString, bool>>> getMessages();

private:
    // Объект подключения к базе данных
    QSqlDatabase m_database;
    // Создает необходимые таблицы в базе данных
    bool createTables();
};

#endif // DATABASEMANAGER_H 