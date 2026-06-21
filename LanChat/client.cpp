
#include "client.h"
#include <QDebug>

ChatClient::ChatClient(QObject* parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected,
            this,     &ChatClient::connectedToServer);

    connect(m_socket, &QTcpSocket::disconnected,
            this,     &ChatClient::disconnectedFromServer);

    connect(m_socket, &QTcpSocket::readyRead,
            this,     &ChatClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError) {
                emit connectionError(m_socket->errorString());
            });
}

void ChatClient::connectToServer(const QString& host, quint16 port)
{
    m_socket->connectToHost(host, port);
}

void ChatClient::sendMessage(const QString& text, const QString& username)
{
    Message msg;
    msg.type      = "chat";
    msg.sender    = username;
    msg.content   = text;
    msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

    m_socket->write(msg.toBytes());
}

void ChatClient::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void ChatClient::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine().trimmed();
        Message msg = Message::fromBytes(line);
        emit messageReceived(msg);
    }
}
void ChatClient::sendRaw(const QByteArray& data)
{
    m_socket->write(data);
}