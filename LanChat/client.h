#pragma once

#include <QObject>
#include <QTcpSocket>
#include "protocol.h"

class ChatClient : public QObject
{
    Q_OBJECT

public:
    explicit ChatClient(QObject* parent = nullptr);
    void connectToServer(const QString& host, quint16 port = 5050);
    void sendMessage(const QString& text, const QString& username);
    void disconnectFromServer();
    void sendRaw(const QByteArray& data);


signals:
    void messageReceived(const Message& msg);
    void connectedToServer();
    void disconnectedFromServer();
    void connectionError(const QString& error);  // ADD

private slots:
    void onReadyRead();

private:
    QTcpSocket* m_socket;
};