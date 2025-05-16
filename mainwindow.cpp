#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QCoreApplication>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QDialog>
#include <QPushButton>
#include <QHeaderView>

/**
 * Конструктор класса MainWindow
 * Выполняет начальную настройку интерфейса, создает объекты менеджеров
 * и устанавливает соединения между сигналами и слотами
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serverPort(8080)  // Порт по умолчанию для сервера
    , m_clientPort(8080)  // Порт по умолчанию для клиента
{
    // Загружаем и настраиваем пользовательский интерфейс из UI-файла
    ui->setupUi(this);
    
    // Создаем и настраиваем менеджер сетевого взаимодействия
    m_networkManager = new NetworkManager(this);
    
    // Подключаем сигналы от сетевого менеджера к соответствующим слотам
    connect(m_networkManager, &NetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(m_networkManager, &NetworkManager::connected, this, &MainWindow::onConnected);
    connect(m_networkManager, &NetworkManager::disconnected, this, &MainWindow::onDisconnected);
    connect(m_networkManager, &NetworkManager::error, this, &MainWindow::onError);
    
    // Создаем менеджер базы данных для журналирования сообщений
    m_databaseManager = new DatabaseManager(this);
    
    // Открываем базу данных, используя путь из аргументов командной строки
    processDatabasePath();
    
    // Связываем события элементов интерфейса с соответствующими слотами
    connect(ui->startServerButton, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectToServer);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(ui->messageEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    connect(ui->showDbButton, &QPushButton::clicked, this, &MainWindow::onShowDatabase);
    
    // Устанавливаем заголовок главного окна
    setWindowTitle("Простой чат");
}

/**
 * Деструктор класса MainWindow
 * Освобождает ресурсы и закрывает соединения перед завершением работы
 */
MainWindow::~MainWindow()
{
    // Закрываем все сетевые соединения
    if (m_networkManager) {
        m_networkManager->closeConnections();
    }
    
    // Освобождаем память, занятую пользовательским интерфейсом
    delete ui;
}

/**
 * Обрабатывает аргументы командной строки для определения пути к базе данных
 * Ищет аргумент --db и открывает базу данных по указанному пути
 * При отсутствии аргумента использует базу данных по умолчанию "chat.db"
 */
void MainWindow::processDatabasePath()
{
    // Получаем список аргументов командной строки
    QStringList args = QCoreApplication::arguments();
    
    // Устанавливаем путь к базе данных по умолчанию
    QString dbPath = "chat.db";
    
    // Ищем аргумент --db среди аргументов командной строки
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--db" && i + 1 < args.size()) {
            // Если найден аргумент --db и за ним следует значение, используем его
            dbPath = args[i + 1];
            break;
        }
    }
    
    // Пытаемся открыть базу данных по указанному пути
    if (!m_databaseManager->openDatabase(dbPath)) {
        // В случае ошибки выводим сообщение пользователю
        QMessageBox::critical(this, "Ошибка базы данных", 
                             "Не удалось открыть базу данных. Сообщения не будут журналироваться.");
    }
}

/**
 * Обрабатывает нажатие на кнопку "Запустить сервер"
 * Запрашивает у пользователя порт и запускает серверную часть чата
 */
void MainWindow::onStartServer()
{
    // Запрашиваем у пользователя порт для сервера через диалоговое окно
    bool ok;
    int port = QInputDialog::getInt(this, "Запуск сервера", "Порт:", 
                                   m_serverPort, 1024, 65535, 1, &ok);
    
    // Если пользователь нажал ОК, пытаемся запустить сервер
    if (ok) {
        // Сохраняем введенный порт для последующего использования
        m_serverPort = port;
        
        // Запускаем сервер на указанном порту
        if (m_networkManager->startServer(port)) {
            // При успешном запуске выводим сообщение в статусной строке
            ui->statusbar->showMessage("Сервер запущен на порту " + QString::number(port));
        } else {
            // При ошибке выводим соответствующее сообщение
            ui->statusbar->showMessage("Не удалось запустить сервер");
        }
    }
}

/**
 * Обрабатывает нажатие на кнопку "Подключиться к серверу"
 * Запрашивает у пользователя адрес и порт, затем устанавливает соединение
 */
void MainWindow::onConnectToServer()
{
    // Запрашиваем у пользователя адрес сервера через диалоговое окно
    QString address = QInputDialog::getText(this, "Подключение к серверу", 
                                          "Адрес:", QLineEdit::Normal, m_clientAddress);
    
    // Если пользователь отменил ввод или оставил поле пустым, выходим
    if (address.isEmpty()) return;
    
    // Запрашиваем у пользователя порт сервера через диалоговое окно
    bool ok;
    int port = QInputDialog::getInt(this, "Подключение к серверу", "Порт:", 
                                   m_clientPort, 1024, 65535, 1, &ok);
    
    // Если пользователь нажал ОК, пытаемся установить соединение
    if (ok) {
        // Сохраняем введенные данные для последующего использования
        m_clientAddress = address;
        m_clientPort = port;
        
        // Пытаемся подключиться к серверу по указанным адресу и порту
        if (m_networkManager->connectToServer(address, port)) {
            // Выводим сообщение о попытке подключения
            ui->statusbar->showMessage("Подключение к " + address + ":" + QString::number(port) + "...");
        } else {
            // При ошибке выводим соответствующее сообщение
            ui->statusbar->showMessage("Не удалось подключиться");
        }
    }
}

/**
 * Обрабатывает отправку сообщения
 * Вызывается при нажатии кнопки "Отправить" или клавиши Enter
 */
void MainWindow::onSendMessage()
{
    // Получаем текст из поля ввода и удаляем пробелы в начале и конце
    QString message = ui->messageEdit->text().trimmed();
    
    // Если сообщение пустое, не отправляем его
    if (message.isEmpty()) return;
    
    // Отправляем сообщение через сетевой менеджер
    m_networkManager->sendMessage(message);
    
    // Отображаем отправленное сообщение в окне чата
    displayMessage("Вы: " + message, false);
    
    // Очищаем поле ввода для следующего сообщения
    ui->messageEdit->clear();
}

/**
 * Обрабатывает получение нового сообщения от собеседника
 * Вызывается при получении сигнала messageReceived от сетевого менеджера
 */
void MainWindow::onMessageReceived(const QString &message)
{
    // Отображаем полученное сообщение в окне чата
    displayMessage("Собеседник: " + message, true);
}

/**
 * Отображает сообщение в окне чата и записывает его в базу данных
 * Добавляет временную метку и форматирует сообщение
 */
void MainWindow::displayMessage(const QString &message, bool incoming)
{
    // Формируем временную метку в формате [ЧЧ:ММ:СС]
    QString timeStamp = QDateTime::currentDateTime().toString("[hh:mm:ss]");
    
    // Добавляем сообщение с временной меткой в окно чата
    ui->chatDisplay->appendPlainText(timeStamp + " " + message);
    
    // Записываем сообщение в базу данных для журналирования
    m_databaseManager->logMessage(message, incoming);
}

/**
 * Обрабатывает событие успешного подключения
 * Вызывается при получении сигнала connected от сетевого менеджера
 */
void MainWindow::onConnected()
{
    // Выводим сообщение об успешном подключении в статусной строке
    ui->statusbar->showMessage("Подключено");
}

/**
 * Обрабатывает событие разрыва соединения
 * Вызывается при получении сигнала disconnected от сетевого менеджера
 */
void MainWindow::onDisconnected()
{
    // Выводим сообщение о разрыве соединения в статусной строке
    ui->statusbar->showMessage("Отключено");
}

/**
 * Обрабатывает сетевые ошибки
 * Вызывается при получении сигнала error от сетевого менеджера
 */
void MainWindow::onError(const QString &errorMessage)
{
    // Выводим сообщение об ошибке в статусной строке
    ui->statusbar->showMessage("Ошибка: " + errorMessage);
}

/**
 * Отображает содержимое базы данных в отдельном диалоговом окне
 * Вызывается при нажатии на кнопку "Показать базу данных"
 */
void MainWindow::onShowDatabase()
{
    // Создаем новое диалоговое окно как дочернее для главного окна
    QDialog *dbDialog = new QDialog(this);
    
    // Устанавливаем заголовок и размеры диалогового окна
    dbDialog->setWindowTitle("Содержимое базы данных");
    dbDialog->resize(600, 400);
    
    // Создаем вертикальную компоновку для размещения элементов в диалоге
    QVBoxLayout *layout = new QVBoxLayout(dbDialog);
    
    // Создаем таблицу для отображения данных из БД
    QTableWidget *tableWidget = new QTableWidget(dbDialog);
    
    // Устанавливаем количество и заголовки столбцов
    tableWidget->setColumnCount(3);
    tableWidget->setHorizontalHeaderLabels({"Время", "Сообщение", "Направление"});
    
    // Настраиваем автоматическое изменение ширины столбцов
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Создаем кнопку закрытия диалога
    QPushButton *closeButton = new QPushButton("Закрыть", dbDialog);
    
    // Подключаем сигнал нажатия на кнопку к закрытию диалога
    connect(closeButton, &QPushButton::clicked, dbDialog, &QDialog::accept);
    
    // Добавляем таблицу и кнопку в компоновку
    layout->addWidget(tableWidget);
    layout->addWidget(closeButton);
    
    // Получаем все сообщения из базы данных
    QList<QPair<QString, QPair<QString, bool>>> messages = m_databaseManager->getMessages();
    
    // Устанавливаем количество строк в таблице равным количеству сообщений
    tableWidget->setRowCount(messages.size());
    
    // Заполняем таблицу данными из базы данных
    for (int i = 0; i < messages.size(); ++i) {
        // Получаем данные текущего сообщения
        QPair<QString, QPair<QString, bool>> messageData = messages.at(i);
        
        // Извлекаем компоненты сообщения
        QString timestamp = messageData.first;  // Временная метка
        QString message = messageData.second.first;  // Текст сообщения
        bool isIncoming = messageData.second.second;  // Направление (входящее/исходящее)
        
        // Создаем элементы таблицы для каждого столбца
        QTableWidgetItem *timestampItem = new QTableWidgetItem(timestamp);
        QTableWidgetItem *messageItem = new QTableWidgetItem(message);
        QTableWidgetItem *directionItem = new QTableWidgetItem(isIncoming ? "Входящее" : "Исходящее");
        
        // Устанавливаем элементы в соответствующие ячейки таблицы
        tableWidget->setItem(i, 0, timestampItem);
        tableWidget->setItem(i, 1, messageItem);
        tableWidget->setItem(i, 2, directionItem);
    }
    
    // Отображаем модальное диалоговое окно
    dbDialog->exec();
}
