#pragma once

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include "server.h"

class ServerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ServerWindow(quint16 port, QWidget* parent = nullptr);

private slots:
    void onKickRequested();
    void onAnnounceRequested();
    void appendLog(const QString& message);
    void updateUserList(const QStringList& users);

private:
    ChatServer* m_server;

    QListWidget* m_userListWidget;
    QTextEdit* m_logView;
    QLineEdit* m_announcementInput;
    QPushButton* m_kickBtn;
    QPushButton* m_announceBtn;

    void setupUI(quint16 port);
    void applyStyles();


protected:
    void closeEvent(QCloseEvent* event) override;
};