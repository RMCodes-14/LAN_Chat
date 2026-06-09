#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ChatServer server;
    server.startServer(5050);

    return a.exec();
}