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
protected:
    void incomingConnection(qintptr socketDescriptor) override;
private slots:
    void OnReadyRead();
    void onClientDisconnected();
private:
    void broadcastMessage(const QByteArray& data , QTcpSocket* except = nullptr);

    QList<QTcpSocket*> m_clients;
    QMap<QTcpSocket*, QString> m_usernames;  // ADD
    QList<Message> m_history;                // ADD — last 20 msgs

    void broadcastUserList();                // ADD
    void sendHistory(QTcpSocket* client);    // ADD
};

