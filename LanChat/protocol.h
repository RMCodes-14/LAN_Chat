#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>


struct Message {
    QString id;        // ADD THIS: Unique identifier for each message
    QString type;      // "chat", "join", "leave", "delete", etc.
    QString sender;
    QString content;
    QString timestamp;
    QString fileName;
    QByteArray fileData;

    QByteArray toBytes() const {
        QJsonObject obj;
        obj["id"] = id; // Include ID in JSON
        obj["type"] = type;
        obj["sender"] = sender;
        obj["content"] = content;
        obj["timestamp"] = timestamp;
        return QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
    }

    static Message fromBytes(const QByteArray& data) {
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        Message msg;
        msg.id = obj["id"].toString(); //  Read ID from JSON
        msg.type = obj["type"].toString();
        msg.sender = obj["sender"].toString();
        msg.content = obj["content"].toString();
        msg.timestamp = obj["timestamp"].toString();
        return msg;
    }
};

