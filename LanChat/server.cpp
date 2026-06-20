#include "server.h"
#include <QDebug>

ChatServer::ChatServer(QObject* parent): QTcpServer(parent) {}

void ChatServer ::startServer(quint16 port){
    if(listen(QHostAddress :: Any , port)){
        qDebug() <<"Server started on port" << port;
        }
    else{
        qDebug() <<"Server Failed To start!";
        }
}

void ChatServer :: incomingConnection(qintptr socketDescriptor){
    QTcpSocket* client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);
    m_clients.append(client);

    connect(client , &QTcpSocket :: readyRead , this , &ChatServer::OnReadyRead);
    connect(client , &QTcpSocket :: disconnected, this , &ChatServer::onClientDisconnected);
    qDebug() << "New client connected:" << client->peerAddress().toString();
}

void ChatServer::OnReadyRead()
{
    QTcpSocket* sender = qobject_cast<QTcpSocket*>(QObject::sender());
    while (sender->canReadLine()) {
        QByteArray line = sender->readLine().trimmed();
        Message msg = Message::fromBytes(line);

        if (msg.type == "chat" || msg.type == "typing") {
            broadcastMessage(line + "\n", sender); // sender ko mat bhejo
        } else {
            broadcastMessage(line + "\n", nullptr); // join/leave sabko
        }
    }
}
void ChatServer::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    m_clients.removeOne(client);
    client->deleteLater();
    qDebug() << "Client disconnected";
}

void ChatServer::broadcastMessage(const QByteArray& data, QTcpSocket* except)
{
    for (QTcpSocket* client : m_clients) {
        if (client != except)
            client->write(data);
    }
}
