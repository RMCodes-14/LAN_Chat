#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include "client.h"

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(const QString& username,
                        const QString& host,
                        quint16 port,
                        QWidget* parent = nullptr);

private slots:
    void onSendClicked();
    void onMessageReceived(const Message& msg);
    void onConnected();
    void onDisconnected();

private:
    void setupUI();
    void applyStyles();
    void addBubble(const Message& msg);
    void addSystemMessage(const QString& text);
    void scrollToBottom();

    // Backend
    ChatClient* m_client;
    QString     m_username;

    // UI
    QWidget*     m_sidebar;
    QVBoxLayout* m_usersLayout;

    QScrollArea* m_scrollArea;
    QWidget*     m_messagesWidget;
    QVBoxLayout* m_messagesLayout;

    QLabel*      m_typingLabel;
    QLineEdit*   m_inputField;
    QPushButton* m_sendBtn;

    QTimer*      m_typingTimer;
};