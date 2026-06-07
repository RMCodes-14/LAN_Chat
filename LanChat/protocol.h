#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>


struct Message{
    QString type;
    QString sender;
    QString content;
    QString timestamp;
    QByteArray toBytes() const {

        QJsonObject obj;
        obj["type"] = type;
        obj["sender"] = sender;
        obj["content"] = content;
        obj["timestamp"] = timestamp;

        return QJsonDocument(obj).toJson(QJsonDocument::Compact) +"\n";
    }
    static Message fromBytes(const QByteArray& data ){
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        Message msg;
        msg.type = obj["type"].toString();
        msg.sender = obj["sender"].toString();
        msg.content = obj["content"].toString();
        msg.timestamp = obj["timestamp"].toString();


        return msg;
    }

};

