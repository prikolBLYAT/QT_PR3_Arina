#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serverPort(8080)
    , m_clientPort(8080)
{
    ui->setupUi(this);
    
    // Initialize network manager
    m_networkManager = new NetworkManager(this);
    connect(m_networkManager, &NetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(m_networkManager, &NetworkManager::connected, this, &MainWindow::onConnected);
    connect(m_networkManager, &NetworkManager::disconnected, this, &MainWindow::onDisconnected);
    connect(m_networkManager, &NetworkManager::error, this, &MainWindow::onError);
    
    // Initialize database manager
    m_databaseManager = new DatabaseManager(this);
    
    // Process database path from command line arguments
    processDatabasePath();
    
    // Connect UI signals to slots
    connect(ui->startServerButton, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectToServer);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(ui->messageEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    
    // Set window title
    setWindowTitle("Simple Chat");
}

MainWindow::~MainWindow()
{
    if (m_networkManager) {
        m_networkManager->closeConnections();
    }
    
    delete ui;
}

void MainWindow::processDatabasePath()
{
    QStringList args = QCoreApplication::arguments();
    QString dbPath = "chat.db"; // Default database path
    
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--db" && i + 1 < args.size()) {
            dbPath = args[i + 1];
            break;
        }
    }
    
    if (!m_databaseManager->openDatabase(dbPath)) {
        QMessageBox::critical(this, "Database Error", "Failed to open database. Messages will not be logged.");
    }
}

void MainWindow::onStartServer()
{
    bool ok;
    int port = QInputDialog::getInt(this, "Start Server", "Port:", m_serverPort, 1024, 65535, 1, &ok);
    
    if (ok) {
        m_serverPort = port;
        if (m_networkManager->startServer(port)) {
            ui->statusbar->showMessage("Server started on port " + QString::number(port));
        } else {
            ui->statusbar->showMessage("Failed to start server");
        }
    }
}

void MainWindow::onConnectToServer()
{
    QString address = QInputDialog::getText(this, "Connect to Server", "Address:", QLineEdit::Normal, m_clientAddress);
    if (address.isEmpty()) return;
    
    bool ok;
    int port = QInputDialog::getInt(this, "Connect to Server", "Port:", m_clientPort, 1024, 65535, 1, &ok);
    
    if (ok) {
        m_clientAddress = address;
        m_clientPort = port;
        
        if (m_networkManager->connectToServer(address, port)) {
            ui->statusbar->showMessage("Connecting to " + address + ":" + QString::number(port) + "...");
        } else {
            ui->statusbar->showMessage("Failed to connect");
        }
    }
}

void MainWindow::onSendMessage()
{
    QString message = ui->messageEdit->text().trimmed();
    if (message.isEmpty()) return;
    
    m_networkManager->sendMessage(message);
    displayMessage("You: " + message, false);
    
    ui->messageEdit->clear();
}

void MainWindow::onMessageReceived(const QString &message)
{
    displayMessage("Them: " + message, true);
}

void MainWindow::displayMessage(const QString &message, bool incoming)
{
    // Add message to chat display
    QString timeStamp = QDateTime::currentDateTime().toString("[hh:mm:ss]");
    ui->chatDisplay->appendPlainText(timeStamp + " " + message);
    
    // Log message to database
    m_databaseManager->logMessage(message, incoming);
}

void MainWindow::onConnected()
{
    ui->statusbar->showMessage("Connected");
}

void MainWindow::onDisconnected()
{
    ui->statusbar->showMessage("Disconnected");
}

void MainWindow::onError(const QString &errorMessage)
{
    ui->statusbar->showMessage("Error: " + errorMessage);
}
