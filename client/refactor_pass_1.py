#!/usr/bin/env python3
"""
refactor_pass_1.py

Pass 1: Foundation.
  - config/AppConfig.h с константами клиента.
  - app/main.cpp использует config, отправляет DisconnectRequest при выходе.
  - utils/Logger использует config, дублирующие константы класса удалены.
  - utils/Validator использует config для лимитов.
  - ui/ChatWindow получает showInformation / showWarning.
  - logic/MainController заменяет QMessageBox на вызовы ChatWindow,
    добавлен slotAboutToQuit.
  - protocol/ProtocolClient получает sendDisconnectRequest.
  - net/Connection добавляет flush() после write.
  - CMakeLists.txt дополняется config/.

Запускать из ~/cvcn/client/ в чистом git-репозитории.
Откат: git reset --hard HEAD && git clean -fd
"""

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path.cwd()


def read(p): return p.read_text(encoding="utf-8")
def write(p, t): p.write_text(t, encoding="utf-8")


def check_cwd():
    required = ["CMakeLists.txt", "app/main.cpp", "logic/MainController.cpp",
                "protocol/ProtocolClient.cpp", "utils/Logger.h"]
    missing = [f for f in required if not (ROOT / f).exists()]
    if missing:
        print(f"ОШИБКА: запустите из ~/cvcn/client/. Не найдены: {missing}",
              file=sys.stderr)
        sys.exit(1)


def check_git_clean():
    for args in (["git", "diff", "--quiet"], ["git", "diff", "--cached", "--quiet"]):
        try:
            r = subprocess.run(args, cwd=ROOT, capture_output=True, check=False)
            if r.returncode != 0:
                print("ОШИБКА: незакоммиченные изменения. git reset --hard HEAD.",
                      file=sys.stderr)
                sys.exit(1)
        except FileNotFoundError:
            break


# ============================================================
# Содержимое файлов
# ============================================================

APP_CONFIG_H = '''#pragma once
#include <cstddef>
#include <cstdint>

/**
 * @file AppConfig.h
 * @brief Константы клиента: сетевые параметры, логирование, лимиты.
 *
 * Все «магические числа» и строки собраны в одном месте.
 * Меняется здесь — действует по всему клиенту.
 */
namespace config
{
    // --- Сеть ---
    inline constexpr const char* DEFAULT_SERVER_HOST = "127.0.0.1";
    inline constexpr uint16_t    DEFAULT_SERVER_PORT = 55550;

    // --- Логи ---
    inline constexpr const char* LOG_FILE_PATH      = "logs/client.log";
    inline constexpr const char* LOGGER_NAME        = "client_logger";
    inline constexpr const char* LOG_PATTERN        = "[%Y-%m-%d %H:%M:%S.%e] [%-8l] %v";
    inline constexpr std::size_t LOG_MAX_FILE_SIZE  = 5 * 1024 * 1024;
    inline constexpr std::size_t LOG_MAX_FILE_COUNT = 3;

    // --- UI ---
    inline constexpr const char* CHAT_TIME_FORMAT = "HH:mm";

    // --- Валидация ---
    inline constexpr std::size_t MAX_USERNAME_LENGTH = 64;
    inline constexpr std::size_t MAX_PASSWORD_LENGTH = 127;
    inline constexpr std::size_t MAX_MESSAGE_LENGTH  = 4096;
}
'''


MAIN_CPP = '''#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"
#include "MainController.h"
#include "Logger.h"
#include "AppConfig.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LOG_INFO("Client starting up");

    ProtocolClient protocolClient;
    AccountDialog accountDialog;
    ChatWindow chatWindow;
    MainController mainController(protocolClient, accountDialog, chatWindow);

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &mainController, &MainController::slotAboutToQuit);

    accountDialog.show();
    mainController.connectToServer(
        QString::fromUtf8(config::DEFAULT_SERVER_HOST),
        config::DEFAULT_SERVER_PORT);

    return app.exec();
}
'''


LOGGER_H = '''#pragma once
#include <string>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <fmt/format.h>

namespace spdlog { class logger; }

/**
 * @file Logger.h
 * @brief Единая точка логирования с автоматическим указанием компонента.
 *
 * Формат вывода:
 *   [YYYY-MM-DD HH:MM:SS.mmm] [level    ] [Component        ] message
 *
 * Компонент берётся из имени файла, откуда вызван макрос.
 *
 * Все параметры (пути, имя логгера, размеры ротации) читаются из
 * config/AppConfig.h, в этом файле не дублируются.
 */
class Logger
{
private:
    Logger();
    ~Logger();

    std::shared_ptr<spdlog::logger> logger_;

public:
    static Logger& instance();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(const char* file,
             spdlog::level::level_enum level,
             const std::string& message);

    template<typename... Args>
    void log(const char* file, spdlog::level::level_enum level,
             const std::string& format, Args&&... args)
    {
        log(file, level, fmt::format(format, std::forward<Args>(args)...));
    }
};

#define LOG_TRACE(...)    Logger::instance().log(__FILE__, spdlog::level::trace,    __VA_ARGS__)
#define LOG_DEBUG(...)    Logger::instance().log(__FILE__, spdlog::level::debug,    __VA_ARGS__)
#define LOG_INFO(...)     Logger::instance().log(__FILE__, spdlog::level::info,     __VA_ARGS__)
#define LOG_WARN(...)     Logger::instance().log(__FILE__, spdlog::level::warn,     __VA_ARGS__)
#define LOG_ERROR(...)    Logger::instance().log(__FILE__, spdlog::level::err,      __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::instance().log(__FILE__, spdlog::level::critical, __VA_ARGS__)
'''


LOGGER_CPP = '''#include "Logger.h"
#include "AppConfig.h"

Logger::Logger()
{
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::info);

    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        config::LOG_FILE_PATH, config::LOG_MAX_FILE_SIZE, config::LOG_MAX_FILE_COUNT);
    fileSink->set_level(spdlog::level::trace);

    std::vector<spdlog::sink_ptr> sinks = {consoleSink, fileSink};
    logger_ = std::make_shared<spdlog::logger>(config::LOGGER_NAME,
                                               sinks.begin(), sinks.end());
    logger_->set_level(spdlog::level::trace);
    logger_->set_pattern(config::LOG_PATTERN);
    logger_->flush_on(spdlog::level::info);
}

Logger::~Logger()
{
    if (logger_)
    {
        logger_->flush();
        spdlog::drop(logger_->name());
    }
}

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

namespace {
    std::string componentFromFile(const char* file)
    {
        std::string path = file;
        const auto slash = path.find_last_of("/\\\\");
        if (slash != std::string::npos)
            path = path.substr(slash + 1);

        const auto dot = path.find_last_of('.');
        if (dot != std::string::npos)
            path = path.substr(0, dot);

        return path;
    }
}

void Logger::log(const char* file,
                 spdlog::level::level_enum level,
                 const std::string& message)
{
    if (!logger_)
        return;

    const std::string component = componentFromFile(file);
    logger_->log(level, "[{:<18}] {}", component, message);
}
'''


VALIDATOR_H = '''#ifndef VALIDATOR_H
#define VALIDATOR_H

#pragma once
#include <string>
#include <cstddef>
#include <cstdint>
#include "AppConfig.h"

/**
 * @file Validator.h
 * @brief Валидация входящих данных.
 *
 * Используется и на клиенте, перед отправкой, и на сервере, перед обработкой.
 * Правила и лимиты берутся из config/AppConfig.h, чтобы клиент и сервер
 * не разошлись.
 */
class Validator
{
public:
    static constexpr size_t MAX_USERNAME_LENGTH = config::MAX_USERNAME_LENGTH;
    static constexpr size_t MAX_PASSWORD_LENGTH = config::MAX_PASSWORD_LENGTH;
    static constexpr size_t MAX_MESSAGE_LENGTH  = config::MAX_MESSAGE_LENGTH;

    static bool validateUsername(const std::string& username);
    static bool validatePassword(const std::string& password);
    static bool validateMessage(const std::string& message);
    static bool validateChatID(uint32_t chatID);

private:
    static bool isCredentialChar(char symbol);
    static bool isMessageChar(char symbol);
};

#endif // VALIDATOR_H
'''


CHAT_WINDOW_H = '''#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QString>
#include "PacketData.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ChatWindow;
}
QT_END_NAMESPACE

class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget* parent = nullptr);
    ~ChatWindow();

    void setCurrentUser(const QString& username);
    void appendMessageInHistory(const QString& sender, const QString& text);
    void setChatList(const std::vector<ChatListEntry>& chats);
    void addChat(uint32_t chatID, const QString& peerUsername);
    uint32_t currentChatID() const { return currentChatID_; }
    void appendIncomingMessage(uint32_t chatID, const QString& sender, const QString& text);

    /** @brief Показать информационное окно. Вызывается контроллером. */
    void showInformation(const QString& title, const QString& text);

    /** @brief Показать предупреждение. Вызывается контроллером. */
    void showWarning(const QString& title, const QString& text);

signals:
    void signalMessageSendRequested(uint32_t currentChatID, const QString& text);
    void signalFindUserRequested(const QString& query);
    void signalCreateChatRequested(const QString& peerUsername);
    void signalChatSelected(uint32_t chatID);

private slots:
    void slotClickedSendButton();
    void slotClickedFindButton();
    void slotChatSelectionChanged();

private:
    Ui::ChatWindow* ui;
    QString currentUser_;
    uint32_t currentChatID_ = 0;
};

#endif // CHATWINDOW_H
'''


CHAT_WINDOW_CPP = '''#include "ChatWindow.h"
#include "ui_ChatWindow.h"
#include "Validator.h"
#include "AppConfig.h"
#include <QMessageBox>
#include <QTime>

ChatWindow::ChatWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    connect(ui->sendPushButton, &QPushButton::clicked, this, &ChatWindow::slotClickedSendButton);
    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &ChatWindow::slotClickedSendButton);
    connect(ui->findPushButton, &QPushButton::clicked, this, &ChatWindow::slotClickedFindButton);
    connect(ui->chatsListWidget, &QListWidget::itemSelectionChanged, this, &ChatWindow::slotChatSelectionChanged);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setCurrentUser(const QString& username)
{
    currentUser_ = username;
}

void ChatWindow::appendMessageInHistory(const QString& sender, const QString& text)
{
    const QString time = QTime::currentTime()
        .toString(QString::fromUtf8(config::CHAT_TIME_FORMAT));
    ui->messagesTextBrowser->append(QStringLiteral("[%1] %2: %3").arg(time, sender, text));
}

void ChatWindow::showInformation(const QString& title, const QString& text)
{
    QMessageBox::information(this, title, text);
}

void ChatWindow::showWarning(const QString& title, const QString& text)
{
    QMessageBox::warning(this, title, text);
}

void ChatWindow::slotClickedSendButton()
{
    if (currentChatID_ == 0)
    {
        appendMessageInHistory(QStringLiteral("system"), QStringLiteral("Select a chat first"));
        return;
    }
    const QString text = ui->messageLineEdit->text().trimmed();
    if (text.isEmpty())
    {
        return;
    }
    if (!Validator::validateMessage(text.toStdString()))
    {
        appendMessageInHistory(QStringLiteral("system"), QStringLiteral("Message rejected locally: invalid format"));
        return;
    }

    ui->messageLineEdit->clear();

    const QString displayName = currentUser_.isEmpty() ? QStringLiteral("me") : currentUser_;

    appendMessageInHistory(displayName, text);
    emit signalMessageSendRequested(currentChatID_, text);
}

void ChatWindow::slotClickedFindButton()
{
    const QString query = ui->searchLineEdit->text().trimmed();
    if (query.isEmpty())
        return;
    emit signalFindUserRequested(query);
}

void ChatWindow::slotChatSelectionChanged()
{
    auto* item = ui->chatsListWidget->currentItem();
    if (!item)
        return;
    const uint32_t id = item->data(Qt::UserRole).toUInt();
    currentChatID_ = id;
    ui->messagesTextBrowser->clear();
    emit signalChatSelected(id);
}

void ChatWindow::setChatList(const std::vector<ChatListEntry>& chats)
{
    ui->chatsListWidget->clear();
    for (const auto& e : chats)
        addChat(e.chatID, QString::fromStdString(e.peerUsername));
}

void ChatWindow::addChat(uint32_t chatID, const QString& peerUsername)
{
    // Проверяем, нет ли уже такого чата
    for (int i = 0; i < ui->chatsListWidget->count(); ++i)
    {
        auto* existing = ui->chatsListWidget->item(i);
        if (existing->data(Qt::UserRole).toUInt() == chatID)
            return;
    }
    auto* item = new QListWidgetItem(peerUsername);
    item->setData(Qt::UserRole, chatID);
    ui->chatsListWidget->addItem(item);
}

void ChatWindow::appendIncomingMessage(uint32_t chatID, const QString& sender, const QString& text)
{
    if (chatID == currentChatID_)
        appendMessageInHistory(sender, text);
}
'''


MAIN_CONTROLLER_H = '''#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H
#include <QObject>
#include "ProtocolClient.h"
#include "AccountDialog.h"
#include "ChatWindow.h"

class MainController : public QObject
{
    Q_OBJECT
public:
    MainController(ProtocolClient& protocolClient,
                   AccountDialog& accountDialog,
                   ChatWindow& chatWindow);
    void connectToServer(const QString& host, quint16 port);

public slots:
    /** @brief Отправить DisconnectRequest перед выходом. */
    void slotAboutToQuit();

private:
    ProtocolClient& protocolClient_;
    AccountDialog& accountDialog_;
    ChatWindow& chatWindow_;
    QString currentUsername_;

private slots:
    void slotAuthRequested(const QString& username, const QString& password);
    void slotRegRequested(const QString& username, const QString& password);
    void slotDelRequested(const QString& username, const QString& password);
    void slotMessageSendRequested(uint32_t chatID, const QString& text);
    void slotFindUserRequested(const QString& query);
    void slotCreateChatRequested(const QString& peerUsername);
    void slotChatSelected(uint32_t chatID);
    void slotUsersFound(const std::vector<std::string>& usernames);
    void slotChatCreated(bool success, uint32_t chatID, const QString& peerUsername);
    void slotChatListReceived(const std::vector<ChatListEntry>& chats);

    void slotRegistrationFinished(bool success);
    void slotAuthFinished(bool success, uint32_t sessionID);
    void slotDeleteFinished(bool success);
    void slotError(const QString& errorString);

    void slotMessageReceived(const QString& senderUsername,
                             uint32_t chatID,
                             const QString& text);
};

#endif // MAINCONTROLLER_H
'''


MAIN_CONTROLLER_CPP = '''#include "MainController.h"
#include "Validator.h"
#include "Logger.h"
#include <QInputDialog>

MainController::MainController(ProtocolClient& protocolClient,
                               AccountDialog& accountDialog,
                               ChatWindow& chatWindow):
    QObject(nullptr),
    protocolClient_(protocolClient),
    accountDialog_(accountDialog),
    chatWindow_(chatWindow)
{
    connect(&accountDialog_, &AccountDialog::signalAuthRequested, this, &MainController::slotAuthRequested);
    connect(&accountDialog_, &AccountDialog::signalRegRequested,  this, &MainController::slotRegRequested);
    connect(&accountDialog_, &AccountDialog::signalDelRequested,  this, &MainController::slotDelRequested);

    connect(&chatWindow_, &ChatWindow::signalMessageSendRequested, this, &MainController::slotMessageSendRequested);
    connect(&chatWindow_, &ChatWindow::signalFindUserRequested,    this, &MainController::slotFindUserRequested);
    connect(&chatWindow_, &ChatWindow::signalCreateChatRequested,  this, &MainController::slotCreateChatRequested);
    connect(&chatWindow_, &ChatWindow::signalChatSelected,         this, &MainController::slotChatSelected);

    connect(&protocolClient_, &ProtocolClient::signalRegistrationFinished, this, &MainController::slotRegistrationFinished);
    connect(&protocolClient_, &ProtocolClient::signalAuthFinished,         this, &MainController::slotAuthFinished);
    connect(&protocolClient_, &ProtocolClient::signalDeleteFinished,       this, &MainController::slotDeleteFinished);
    connect(&protocolClient_, &ProtocolClient::signalErrorOccurred,        this, &MainController::slotError);
    connect(&protocolClient_, &ProtocolClient::signalUsersFound,           this, &MainController::slotUsersFound);
    connect(&protocolClient_, &ProtocolClient::signalChatCreated,          this, &MainController::slotChatCreated);
    connect(&protocolClient_, &ProtocolClient::signalChatListReceived,     this, &MainController::slotChatListReceived);
    connect(&protocolClient_, &ProtocolClient::signalMessageReceived,      this, &MainController::slotMessageReceived);
}

void MainController::connectToServer(const QString& host, quint16 port)
{
    protocolClient_.connectToServer(host, port);
}

void MainController::slotAboutToQuit()
{
    LOG_INFO("Client shutting down, sending DisconnectRequest");
    protocolClient_.sendDisconnectRequest();
}

void MainController::slotAuthRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Auth rejected locally: invalid username or password format");
        return;
    }
    currentUsername_ = username;
    protocolClient_.sendAuthRequest(username, password);
}

void MainController::slotRegRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Register rejected locally: invalid username or password format");
        return;
    }
    protocolClient_.sendRegRequest(username, password);
}

void MainController::slotDelRequested(const QString& username, const QString& password)
{
    const std::string u = username.toStdString();
    const std::string p = password.toStdString();
    if (!Validator::validateUsername(u) || !Validator::validatePassword(p))
    {
        LOG_WARN("Delete rejected locally: invalid username or password format");
        return;
    }
    protocolClient_.sendDeleteRequest(username, password);
}

void MainController::slotMessageSendRequested(uint32_t chatID, const QString& text)
{
    protocolClient_.sendMessage(chatID, text);
}

void MainController::slotRegistrationFinished(bool success)
{
    LOG_INFO("Registration {}", success ? "OK" : "FAILED");
}

void MainController::slotAuthFinished(bool success, uint32_t sessionID)
{
    LOG_INFO("Auth {} sessionID = {}", success ? "OK" : "FAILED", sessionID);
    if (success)
    {
        chatWindow_.setCurrentUser(currentUsername_);
        accountDialog_.hide();
        chatWindow_.show();
        protocolClient_.sendChatListRequest();
    }
}

void MainController::slotDeleteFinished(bool success)
{
    LOG_INFO("Delete {}", success ? "OK" : "FAILED");
}

void MainController::slotError(const QString& errorString)
{
    LOG_ERROR("{}", errorString.toStdString());
}

void MainController::slotFindUserRequested(const QString& query)
{
    protocolClient_.sendFindUserRequest(query);
}

void MainController::slotCreateChatRequested(const QString& peerUsername)
{
    protocolClient_.sendCreateChatRequest(peerUsername);
}

void MainController::slotChatSelected(uint32_t chatID)
{
    Q_UNUSED(chatID);
    // В следующей итерации: запрос истории сообщений
}

void MainController::slotUsersFound(const std::vector<std::string>& usernames)
{
    if (usernames.empty())
    {
        chatWindow_.showInformation(QStringLiteral("Search"),
                                    QStringLiteral("No users found"));
        return;
    }
    QStringList list;
    for (const auto& u : usernames)
        list << QString::fromStdString(u);
    bool ok = false;
    const QString chosen = QInputDialog::getItem(&chatWindow_, QStringLiteral("Found users"),
                                                 QStringLiteral("Select user:"),
                                                 list, 0, false, &ok);
    if (ok && !chosen.isEmpty())
        protocolClient_.sendCreateChatRequest(chosen);
}

void MainController::slotChatCreated(bool success, uint32_t chatID, const QString& peerUsername)
{
    if (success)
        chatWindow_.addChat(chatID, peerUsername);
    else
        chatWindow_.showWarning(QStringLiteral("Chat"),
                                QStringLiteral("Failed to create chat"));
}

void MainController::slotChatListReceived(const std::vector<ChatListEntry>& chats)
{
    chatWindow_.setChatList(chats);
}

void MainController::slotMessageReceived(const QString& senderUsername,
                                         uint32_t chatID,
                                         const QString& text)
{
    chatWindow_.appendIncomingMessage(chatID, senderUsername, text);
}
'''


PROTOCOL_CLIENT_H = '''#ifndef PROTOCOLCLIENT_H
#define PROTOCOLCLIENT_H

#include <QObject>
#include <QString>
#include <vector>
#include <cstdint>
#include "PacketData.h"

class Connection;

class ProtocolClient : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolClient(QObject* parent = nullptr);
    ~ProtocolClient() = default;

    void connectToServer(const QString& address, quint16 port);

    void sendRegRequest(const QString& username, const QString& password);
    void sendAuthRequest(const QString& username, const QString& password);
    void sendDeleteRequest(const QString& username, const QString& password);
    void sendMessage(uint32_t chatID, const QString& text);
    void sendFindUserRequest(const QString& query);
    void sendCreateChatRequest(const QString& peerUsername);
    void sendChatListRequest();
    void sendDisconnectRequest();

signals:
    void signalConnected();
    void signalDisconnected();
    void signalRegistrationFinished(bool success);
    void signalAuthFinished(bool success, uint32_t sessionID);
    void signalDeleteFinished(bool success);
    void signalErrorOccurred(const QString& errorString);
    void signalUsersFound(const std::vector<std::string>& usernames);
    void signalChatCreated(bool success, uint32_t chatID, const QString& peerUsername);
    void signalChatListReceived(const std::vector<ChatListEntry>& chats);
    void signalMessageReceived(const QString& senderUsername,
                               uint32_t chatID,
                               const QString& text);

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotErrorOccurred(const QString& errorString);
    void slotRawPacketReceived(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);

private:
    Connection* connection_;
    uint32_t messageID_ = 0;
    uint32_t sessionID_ = 0;

    void sendPacket(const std::vector<uint8_t>& packet);
    void increaseMessageID();
    void processIncomingPacket(const PacketHeaderRaw& header, const std::vector<uint8_t>& body);
};

#endif // PROTOCOLCLIENT_H
'''


CMAKE = '''cmake_minimum_required(VERSION 3.16)
project(client VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

set(PROJECT_SOURCES
    app/main.cpp

    config/AppConfig.h

    ui/AccountDialog.cpp
    ui/AccountDialog.h
    ui/AccountDialog.ui

    ui/ChatWindow.cpp
    ui/ChatWindow.h
    ui/ChatWindow.ui

    net/Connection.cpp
    net/Connection.h

    logic/MainController.cpp
    logic/MainController.h

    protocol/ProtocolClient.cpp
    protocol/ProtocolClient.h
    protocol/PacketData.h
    protocol/PacketBuilder.cpp
    protocol/PacketBuilder.h
    protocol/PacketParser.cpp
    protocol/PacketParser.h

    utils/Logger.cpp
    utils/Logger.h
    utils/Validator.cpp
    utils/Validator.h
    utils/ByteReader.cpp
    utils/ByteReader.h
    utils/ByteWriter.cpp
    utils/ByteWriter.h
)

add_executable(client ${PROJECT_SOURCES})

target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    fmt::fmt
    spdlog::spdlog
)

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
'''


# ============================================================
# Дополнительно: добавить sendDisconnectRequest в ProtocolClient.cpp
# ============================================================

def patch_protocol_client_cpp():
    p = ROOT / "protocol" / "ProtocolClient.cpp"
    text = read(p)
    if "sendDisconnectRequest" in text:
        return
    pattern = r"(void ProtocolClient::sendChatListRequest\(\)\s*\n\s*\{[^}]*\})"
    new_method = r'''void ProtocolClient::sendDisconnectRequest()
{
    DisconnectRequestData payload;
    sendPacket(PacketBuilder::buildPacket(messageID_, sessionID_, payload));
    increaseMessageID();
}

\1'''
    text, n = re.subn(pattern, new_method, text, count=1)
    if n:
        write(p, text)
        print(f"  ProtocolClient.cpp: sendDisconnectRequest добавлен")


# ============================================================
# Дополнительно: добавить flush() в Connection::send
# ============================================================

def patch_connection_cpp():
    p = ROOT / "net" / "Connection.cpp"
    text = read(p)
    if "socket_->flush()" in text:
        return
    old = '''    const qint64 bytesWritten = socket_->write(data);
    if (bytesWritten == -1)
        emit signalErrorOccurred(socket_->errorString());'''
    new = '''    const qint64 bytesWritten = socket_->write(data);
    if (bytesWritten == -1)
        emit signalErrorOccurred(socket_->errorString());
    else
        socket_->flush();'''
    text, n = re.subn(re.escape(old), new, text)
    if n:
        write(p, text)
        print(f"  Connection.cpp: flush() добавлен")


# ============================================================
# Точка входа
# ============================================================

def main():
    check_cwd()
    check_git_clean()

    print("=== Pass 1: Foundation ===")

    print("  Создаю config/AppConfig.h")
    (ROOT / "config").mkdir(exist_ok=True)
    write(ROOT / "config" / "AppConfig.h", APP_CONFIG_H)

    print("  Перезаписываю app/main.cpp")
    write(ROOT / "app" / "main.cpp", MAIN_CPP)

    print("  Перезаписываю utils/Logger.h и .cpp")
    write(ROOT / "utils" / "Logger.h", LOGGER_H)
    write(ROOT / "utils" / "Logger.cpp", LOGGER_CPP)

    print("  Перезаписываю utils/Validator.h")
    write(ROOT / "utils" / "Validator.h", VALIDATOR_H)

    print("  Перезаписываю ui/ChatWindow.h и .cpp")
    write(ROOT / "ui" / "ChatWindow.h", CHAT_WINDOW_H)
    write(ROOT / "ui" / "ChatWindow.cpp", CHAT_WINDOW_CPP)

    print("  Перезаписываю logic/MainController.h и .cpp")
    write(ROOT / "logic" / "MainController.h", MAIN_CONTROLLER_H)
    write(ROOT / "logic" / "MainController.cpp", MAIN_CONTROLLER_CPP)

    print("  Перезаписываю protocol/ProtocolClient.h")
    write(ROOT / "protocol" / "ProtocolClient.h", PROTOCOL_CLIENT_H)

    patch_protocol_client_cpp()
    patch_connection_cpp()

    print("  Перезаписываю CMakeLists.txt")
    write(ROOT / "CMakeLists.txt", CMAKE)

    print()
    print("=== Pass 1 готов ===")
    print("Проверьте:")
    print("  grep -rn 'QMessageBox' logic/       # должен быть пусто")
    print("  grep -rn '\"127.0.0.1\"' app/ logic/ # должен быть пусто")
    print("  grep -rn '55550' app/ logic/        # должен быть пусто")
    print()
    print("Дальше:")
    print("  1. Закройте Qt Creator.")
    print("  2. Удалите CMakeLists.txt.user и build/.")
    print("  3. Откройте проект заново (Qt Creator перечитает CMakeLists).")
    print("  4. Build -> Rebuild.")
    print("  5. Запустите сервер и два клиента, проверьте чат.")
    print("  6. git add . && git commit -m 'Pass 1: config, UI cleanup, graceful shutdown'")
    print()
    print("Что дальше (Pass 2):")
    print("  - Connection выносится из ProtocolClient, передаётся через DI.")
    print("  - Validator убирается из ChatWindow, валидация в MainController.")
    print("  - ErrorKind: транспортные / протокольные / бизнес-ошибки.")


if __name__ == "__main__":
    main()
