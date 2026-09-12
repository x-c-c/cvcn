#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QString>

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

    // Устанавливает имя текущего пользователя для отображения своих сообщений
    void setCurrentUser(const QString& username);

    // Добавляет сообщение в историю
    void appendMessageInHistory(const QString& sender, const QString& text);

signals:
    void signalMessageSendRequested(const QString& text);

private slots:
    void slotClickedSendButton();

private:
    Ui::ChatWindow* ui;
    QString currentUser_;
};

#endif // CHATWINDOW_H
