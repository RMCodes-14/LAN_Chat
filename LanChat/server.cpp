#include "server.h"
#include <QDateTime>
#include <QMessageBox>
#include <QTimer>

ChatServer::ChatServer(QObject* parent) : QTcpServer(parent) {}

void ChatServer::startServer(quint16 port)
{
    // Double Server Control Validation Block
    if (listen(QHostAddress::Any, port)) {
        emit logMessage("Server started on port " + QString::number(port));
    } else {
        emit logMessage("Server failed to start!");
        QMessageBox::critical(nullptr, "Server Error",
                              "Server failed to start! Port " + QString::number(port) + " is already in use by another instance.");
        exit(EXIT_FAILURE);
    }
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);
    m_clients.append(client);

    connect(client, &QTcpSocket::readyRead, this, &ChatServer::OnReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &ChatServer::onClientDisconnected);

    emit logMessage("New raw connection from: " + client->peerAddress().toString());
}

void ChatServer::OnReadyRead()
{
    QTcpSocket* sender = qobject_cast<QTcpSocket*>(QObject::sender());
    while (sender->canReadLine()) {
        QByteArray line = sender->readLine().trimmed();
        if (line.isEmpty()) continue;

        Message msg = Message::fromBytes(line);
        if (msg.type.isEmpty()) continue;

        if (msg.type == "join") {
            if (m_usernames.contains(sender)) continue;

            m_usernames[sender] = msg.sender;
            emit logMessage(msg.sender + " has joined the chat.");

            sendHistory(sender);

            if (m_history.size() >= 20) m_history.removeFirst();
            m_history.append(msg);

            broadcastMessage(line + "\n", nullptr);
            broadcastUserList();
            emit userListChanged(m_usernames.values());
        }
        else if (msg.type == "leave") {
            emit logMessage(msg.sender + " voluntarily left the chat.");

            if (m_history.size() >= 20) m_history.removeFirst();
            m_history.append(msg);

            broadcastMessage(line + "\n", nullptr);
            m_usernames.remove(sender);
            broadcastUserList();
            emit userListChanged(m_usernames.values());
        }
        else if (msg.type == "chat") {
            emit logMessage("[" + msg.sender + "]: " + msg.content);
            if (m_history.size() >= 20) m_history.removeFirst();
            m_history.append(msg);

            broadcastMessage(line + "\n", sender);
        }
        else if (msg.type == "typing") {
            broadcastMessage(line + "\n", sender);
        }
        else if (msg.type == "file") {
            emit logMessage("[" + msg.sender + " shared a file]: " + msg.content.split('|').first());
            if (m_history.size() >= 20) m_history.removeFirst();
            m_history.append(msg);

            broadcastMessage(line + "\n", sender);
        }
        else if (msg.type == "delete") {
            emit logMessage(msg.sender + " deleted message ID: " + msg.content);

            // History cleanup sync logic loop
            for (int i = 0; i < m_history.size(); ++i) {
                if (m_history[i].id == msg.content) {
                    m_history.removeAt(i);
                    break;
                }
            }
            // Broadcast the exact network packet to sync other users' screens
            broadcastMessage(line + "\n", nullptr);
        }
    }
}

void ChatServer::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    if (m_usernames.contains(client)) {
        QString username = m_usernames[client];
        emit logMessage(username + " disconnected/lost connection.");

        Message leaveMsg;
        leaveMsg.type = "leave";
        leaveMsg.sender = username;
        leaveMsg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

        if (m_history.size() >= 20) m_history.removeFirst();
        m_history.append(leaveMsg);
        broadcastMessage(leaveMsg.toBytes(), client);
        m_usernames.remove(client);
        broadcastUserList();
        emit userListChanged(m_usernames.values());
    }
    m_clients.removeOne(client);
    client->deleteLater();
}

void ChatServer::kickUser(const QString& username)
{
    QTcpSocket* targetSocket = nullptr;

    // 🔥 BUG FIX: map/hash iterator ko safe tarike se read karein taake container detach na ho aur crash bache
    for (auto it = m_usernames.cbegin(); it != m_usernames.cend(); ++it) {
        if (it.value() == username) {
            targetSocket = it.key();
            break;
        }
    }

    if (targetSocket) {
        emit logMessage("Admin kicked user: " + username);
   Message kickMsg;
        kickMsg.type = "kick";
        kickMsg.sender = "SYSTEM/ADMIN";
        kickMsg.content = username;
        kickMsg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

        broadcastMessage(kickMsg.toBytes(), nullptr);
   QTimer::singleShot(200, this, [targetSocket]() {
            if (targetSocket && targetSocket->state() == QAbstractSocket::ConnectedState) {
                targetSocket->abort();
            }
        });
    }
}
void ChatServer::sendGlobalAnnouncement(const QString& text)
{
    Message msg;
    msg.type = "chat";
    msg.sender = "SYSTEM/ADMIN";
    msg.content = "⚠️ " + text;
    msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

    emit logMessage("[ANNOUNCEMENT]: " + text);

    if (m_history.size() >= 20) m_history.removeFirst();
    m_history.append(msg);

    broadcastMessage(msg.toBytes(), nullptr);
}

void ChatServer::broadcastMessage(const QByteArray& data, QTcpSocket* except)
{
    for (QTcpSocket* client : m_clients) {
        if (client != except && client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}

void ChatServer::broadcastUserList()
{
    Message msg;
    msg.type = "userlist";
    msg.sender = "server";
    msg.content = QStringList(m_usernames.values()).join(",");
    broadcastMessage(msg.toBytes(), nullptr);
}

void ChatServer::sendHistory(QTcpSocket* client)
{
    for (const Message& msg : m_history) {
        client->write(msg.toBytes());
    }
}

void ChatServer::stopServer()
{
    emit logMessage("Shutting down server...");

    Message shutdownMsg;
    shutdownMsg.type = "leave";
    shutdownMsg.sender = "SYSTEM/ADMIN";
    shutdownMsg.content = "Server has been closed by the Admin.";
    shutdownMsg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

    QByteArray data = shutdownMsg.toBytes();
    broadcastMessage(data, nullptr);

    for (QTcpSocket* client : m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->disconnectFromHost();
            client->waitForDisconnected(500);
        }
    }

    this->close();
}