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

/*
 * Класс для управления главным окном приложения чата
 * Обеспечивает взаимодействие пользователя с сетью и базой данных
 * Содержит элементы управления для отправки и получения сообщений
 * Выполняет журналирование всех сообщений в базу данных
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*
     * Конструктор создаёт и настраивает интерфейс приложения
     * Инициализирует менеджеры сети и базы данных
     * Устанавливает соединения между сигналами и слотами
     */
    MainWindow(QWidget *parent = nullptr);
    
    /*
     * Деструктор освобождает ресурсы приложения
     * Закрывает сетевые соединения перед выходом
     */
    ~MainWindow();

private slots:
    /*
     * Слот для запуска серверной части чата
     * Показывает диалог для ввода порта и запускает сервер
     */
    void onStartServer();
    
    /*
     * Слот для подключения к другому экземпляру чата
     * Показывает диалоги для ввода адреса и порта сервера
     */
    void onConnectToServer();
    
    /*
     * Слот для отправки сообщения другому участнику
     * Вызывается при нажатии кнопки "Отправить" или клавиши Enter
     */
    void onSendMessage();
    
    /*
     * Слот для обработки входящих сообщений
     * Отображает сообщение в чате и журналирует его в БД
     */
    void onMessageReceived(const QString &message);
    
    /*
     * Слот вызывается при успешном установлении соединения
     * Обновляет статусную строку и разблокирует отправку сообщений
     */
    void onConnected();
    
    /*
     * Слот вызывается при разрыве соединения
     * Обновляет статусную строку
     */
    void onDisconnected();
    
    /*
     * Слот для обработки сетевых ошибок
     * Отображает сообщение об ошибке в статусной строке
     */
    void onError(const QString &errorMessage);
    
    /*
     * Слот для отображения истории сообщений из базы данных
     * Создаёт диалоговое окно с таблицей всех сообщений
     */
    void onShowDatabase();

private:
    // Указатель на UI-объекты, созданные в Qt Designer
    Ui::MainWindow *ui;
    
    // Объект для управления сетевыми соединениями
    NetworkManager *m_networkManager;
    
    // Объект для работы с базой данных сообщений
    DatabaseManager *m_databaseManager;
    
    // Порт для серверной части приложения (по умолчанию 8080)
    int m_serverPort;
    
    // Последний использованный адрес для клиентского подключения
    QString m_clientAddress;
    
    // Последний использованный порт для клиентского подключения
    int m_clientPort;

    /*
     * Метод отображает сообщение в окне чата
     * Добавляет временную метку и направление сообщения (входящее/исходящее)
     * Записывает сообщение в базу данных через m_databaseManager
     */
    void displayMessage(const QString &message, bool incoming);
    
    /*
     * Метод обрабатывает аргументы командной строки для определения пути к БД
     * Ищет аргумент --db и открывает базу данных по указанному пути
     * При отсутствии аргумента использует базу данных по умолчанию "chat.db"
     */
    void processDatabasePath();
};
#endif // MAINWINDOW_H
