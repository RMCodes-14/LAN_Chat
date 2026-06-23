#include "server.h"
#include <QDateTime>

ChatServer::ChatServer(QObject* parent) : QTcpServer(parent) {}

void ChatServer::startServer(quint16 port)
{
    if (listen(QHostAddress::Any, port)) {
        emit logMessage("Server started on port " + QString::number(port));
    } else {
        emit logMessage("Server failed to start!");
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

            // 1. Pehle naye user ko pichli saari history bhejein
            sendHistory(sender);

            // BUG FIX: Added "join alert" in historyfor new user to get this msg as well
            if (m_history.size() >= 20) m_history.removeFirst();
            m_history.append(msg);

            // 2. Baqi sab ko inform karein
            broadcastMessage(line + "\n", nullptr);
            broadcastUserList();
            emit userListChanged(m_usernames.values());
        }
        else if (msg.type == "leave") {
            emit logMessage(msg.sender + " voluntarily left the chat.");

            // BUG FIX: added in history
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

// GUI Action: User ko kick karna
void ChatServer::kickUser(const QString& username)
{
    QTcpSocket* targetSocket = nullptr;
    // Username se socket dhoondo
    for (auto it = m_usernames.begin(); it != m_usernames.end(); ++it) {
        if (it.value() == username) {
            targetSocket = it.key();
            break;
        }
    }

    if (targetSocket) {
        emit logMessage("Admin kicked user: " + username);

        // Pehle baqi sab ko leave message bhejo taake unke paas se gayeb ho jaye
        Message leaveMsg;
        leaveMsg.type = "leave";
        leaveMsg.sender = username;
        leaveMsg.content = "Kicked by Admin";
        leaveMsg.timestamp = QDateTime::currentDateTime().toString("hh:mm");
        broadcastMessage(leaveMsg.toBytes(), targetSocket);

        // Socket close kar do, client khud hi disconnect handle kar legi
        targetSocket->disconnectFromHost();
    }
}

void ChatServer::sendGlobalAnnouncement(const QString& text)
{
    Message msg;
    msg.type = "chat"; // Client isay chat samajh kar addBubble karega
    msg.sender = "SYSTEM/ADMIN";
    msg.content = "⚠️ " + text;
    msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

    emit logMessage("[ANNOUNCEMENT]: " + text);

    // 🔥 BUG FIX: Is announcement ko history mein add for new user to get the announcement as well
    if (m_history.size() >= 20) m_history.removeFirst();
    m_history.append(msg);

    broadcastMessage(msg.toBytes(), nullptr);
}

void ChatServer::broadcastMessage(const QByteArray& data, QTcpSocket* except)
{
    for (QTcpSocket* client : m_clients) {
        if (client != except && client->state() == QAbstractSocket::ConnectedState)
            client->write(data);
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
    // Type change karne ki zaroorat nahi, original packet hi bhej dein
    for (const Message& msg : m_history) {
        client->write(msg.toBytes());
    }
}
void ChatServer::stopServer()
{
    emit logMessage("Shutting down server...");

    // Sab clients ko inform karne ke liye system message send
    Message shutdownMsg;
    shutdownMsg.type = "leave";
    shutdownMsg.sender = "SYSTEM/ADMIN";
    shutdownMsg.content = "Server has been closed by the Admin.";
    shutdownMsg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

    QByteArray data = shutdownMsg.toBytes();
    broadcastMessage(data, nullptr);

    // Sab sockets close
    for (QTcpSocket* client : m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->disconnectFromHost();
            client->waitForDisconnected(500); // 500ms wait karein taake packet chala jaye
        }
    }

    this->close(); // QTcpServer ko stop karein
}