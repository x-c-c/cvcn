#ifndef CHATWINDOW_H
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

/**
 * @file ChatWindow.h
 * @brief Главное окно чата: список чатов, история, поле ввода.
 *
 * Не выполняет валидацию данных и не решает, что показать пользователю —
 * за это отвечает MainController. ChatWindow только эмитит намерения
 * и отображает то, что ему прикажут.
 */
class ChatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWindow(QWidget* parent = nullptr);
    ~ChatWindow();

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
    uint32_t currentChatID_ = 0;
};

#endif // CHATWINDOW_H
