#include "server.h"
#include <QDebug>

ChatServer::ChatServer(QObject* parent)
    : QTcpServer(parent)
{}

void ChatServer::startServer(quint16 port)
{
    if (listen(QHostAddress::Any, port)) {
        qDebug() << "Server started on port" << port;
    } else {
        qDebug() << "Server failed to start!";
    }
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);
    m_clients.append(client);

    connect(client, &QTcpSocket::readyRead,
            this,   &ChatServer::OnReadyRead);
    connect(client, &QTcpSocket::disconnected,
            this,   &ChatServer::onClientDisconnected);

    qDebug() << "New client:" << client->peerAddress().toString();
}

void ChatServer::OnReadyRead()
{
    QTcpSocket* sender = qobject_cast<QTcpSocket*>(QObject::sender());
    while (sender->canReadLine()) {
        QByteArray line = sender->readLine().trimmed();

        // Validate — empty ya invalid ignore karo
        if (line.isEmpty()) continue;

        Message msg = Message::fromBytes(line);

        // Invalid message ignore karo
        if (msg.type.isEmpty()) continue;

        if (msg.type == "join") {
            // Duplicate check — agar already registered hai toh ignore
            if (m_usernames.contains(sender)) continue;

            m_usernames[sender] = msg.sender;
            sendHistory(sender);
            broadcastMessage(line + "\n", nullptr);
            broadcastUserList();
        }
        else if (msg.type == "leave") {
            broadcastMessage(line + "\n", nullptr);
            m_usernames.remove(sender);
            broadcastUserList();

        } else if (msg.type == "chat") {
            // History mein add karo — max 20 messages
            if (m_history.size() >= 20)
                m_history.removeFirst();
            m_history.append(msg);

            // Sirf doosron ko bhejo
            broadcastMessage(line + "\n", sender);

        } else if (msg.type == "typing") {
            // Sirf doosron ko bhejo
            broadcastMessage(line + "\n", sender);
        }
    }
}

void ChatServer::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());

    // Agar username tha — leave broadcast karo
    if (m_usernames.contains(client)) {
        QString username = m_usernames[client];

        Message leaveMsg;
        leaveMsg.type      = "leave";
        leaveMsg.sender    = username;
        leaveMsg.content   = "";
        leaveMsg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

        QByteArray data = leaveMsg.toBytes();
        // trimmed without \n since toBytes already adds it
        broadcastMessage(data, client);

        m_usernames.remove(client);
        broadcastUserList();
    }

    m_clients.removeOne(client);
    client->deleteLater();
}

void ChatServer::broadcastMessage(const QByteArray& data, QTcpSocket* except)
{
    for (QTcpSocket* client : m_clients) {
        if (client != except)
            client->write(data);
    }
}

void ChatServer::broadcastUserList()
{
    // Saare online usernames comma se join karo
    QStringList users;
    for (const QString& name : m_usernames.values())
        users << name;

    Message msg;
    msg.type    = "userlist";
    msg.sender  = "server";
    msg.content = users.join(",");
    msg.timestamp = "";

    broadcastMessage(msg.toBytes(), nullptr);
}

void ChatServer::sendHistory(QTcpSocket* client)
{
    for (const Message& msg : m_history) {
        client->write(msg.toBytes());
    }
}