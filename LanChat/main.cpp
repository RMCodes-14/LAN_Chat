#include <QApplication>
#include "connectionwindow.h"
#include "chatwindow.h"
#include "server.h"
#include "serverwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConnectionWindow* connWindow = new ConnectionWindow();
    connWindow->show();

    // Client mode
    QObject::connect(connWindow, &ConnectionWindow::startAsClient,
                     [&](const QString& host, quint16 port, const QString& username) {
                         connWindow->hide();
                         ChatWindow* chat = new ChatWindow(username, host, port, false);
                         chat->show();
                     });


    // ── SERVER MODE ──
    QObject::connect(connWindow, &ConnectionWindow::startAsServer,
                     [&](const QString& username, quint16 port) {
                         Q_UNUSED(username);


                         ServerWindow* serverUI = new ServerWindow(port);
                         serverUI->show();


                         connWindow->hide();


                     });

    return a.exec();
}