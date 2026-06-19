#include <QApplication>
#include "connectionwindow.h"
#include "chatwindow.h"
#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConnectionWindow* connWindow = new ConnectionWindow();
    connWindow->show();

    // Client mode
    QObject::connect(connWindow, &ConnectionWindow::startAsClient,
                     [&](const QString& host, quint16 port, const QString& username) {
                         connWindow->hide();
                         ChatWindow* chat = new ChatWindow(username, host, port);
                         chat->show();
                     });


    // Server mode
    QObject::connect(connWindow, &ConnectionWindow::startAsServer,
                     [&](const QString& username, quint16 port) {
                         ChatServer* server = new ChatServer();
                         server->startServer(port);
                         connWindow->hide();
                         ChatWindow* chat = new ChatWindow(username, "127.0.0.1", port);
                         chat->show();
                     });
    return a.exec();
}