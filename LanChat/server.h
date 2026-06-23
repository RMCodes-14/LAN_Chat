#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>
#include "protocol.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject* parent = nullptr);
    void startServer(quint16 port = 5050);

    // UI se handle karne ke liye public functions
    void kickUser(const QString& username);
    void sendGlobalAnnouncement(const QString& text);
    QStringList getConnectedUsers() const;

    void stopServer();

signals:
    // Yeh signals Server UI ko update karenge
    void logMessage(const QString& log);
    void userListChanged(const QStringList& users);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void OnReadyRead();
    void onClientDisconnected();

private:
    void broadcastMessage(const QByteArray& data, QTcpSocket* except = nullptr);
    void broadcastUserList();
    void sendHistory(QTcpSocket* client);

    QList<QTcpSocket*> m_clients;
    QMap<QTcpSocket*, QString> m_usernames;
    QList<Message> m_history;
};