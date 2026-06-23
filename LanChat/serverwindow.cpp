#include "serverwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QCloseEvent>

ServerWindow::ServerWindow(quint16 port, QWidget* parent)
    : QWidget(parent)
{
    setupUI(port);
    applyStyles();

    m_server = new ChatServer(this);

    connect(m_server, &ChatServer::logMessage, this, &ServerWindow::appendLog);
    connect(m_server, &ChatServer::userListChanged, this, &ServerWindow::updateUserList);

    m_server->startServer(port);
}

void ServerWindow::setupUI(quint16 port)
{
    setWindowTitle("LanChat - Admin Server Panel");
    setMinimumSize(700, 450);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ── LEFT SIDE: Connected Users ──
    QVBoxLayout* leftLayout = new QVBoxLayout();
    QLabel* userLabel = new QLabel("Connected Clients");
    userLabel->setObjectName("panelTitle");

    m_userListWidget = new QListWidget();
    m_userListWidget->setObjectName("userList");

    m_kickBtn = new QPushButton("Kick Selected User");
    m_kickBtn->setObjectName("kickBtn");
    m_kickBtn->setFixedHeight(36);


    QPushButton* stopServerBtn = new QPushButton("Stop Server", this);
    stopServerBtn->setObjectName("kickBtn"); // Red styling ke liye
    stopServerBtn->setFixedHeight(36);

    leftLayout->addWidget(userLabel);
    leftLayout->addWidget(m_userListWidget);
    leftLayout->addWidget(m_kickBtn);
    leftLayout->addWidget(stopServerBtn);
    mainLayout->addLayout(leftLayout, 2);

    // ── RIGHT SIDE: Server Logs & Announcement ──
    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* logLabel = new QLabel("Live Server Logs (Port: " + QString::number(port) + ")");
    logLabel->setObjectName("panelTitle");

    m_logView = new QTextEdit();
    m_logView->setReadOnly(true);
    m_logView->setObjectName("logView");

    // Announcement Area
    QHBoxLayout* announceLayout = new QHBoxLayout();
    m_announcementInput = new QLineEdit();
    m_announcementInput->setPlaceholderText("Type system announcement here...");
    m_announcementInput->setObjectName("inputField");
    m_announcementInput->setFixedHeight(36);

    m_announceBtn = new QPushButton("Broadcast");
    m_announceBtn->setObjectName("primaryBtn");
    m_announceBtn->setFixedHeight(36);

    announceLayout->addWidget(m_announcementInput);
    announceLayout->addWidget(m_announceBtn);

    rightLayout->addWidget(logLabel);
    rightLayout->addWidget(m_logView);
    rightLayout->addLayout(announceLayout);
    mainLayout->addLayout(rightLayout, 3);

    // Connect local button slots
    connect(m_kickBtn, &QPushButton::clicked, this, &ServerWindow::onKickRequested);
    connect(m_announceBtn, &QPushButton::clicked, this, &ServerWindow::onAnnounceRequested);
    connect(m_announcementInput, &QLineEdit::returnPressed, this, &ServerWindow::onAnnounceRequested);


    // Button click coneected to close window
    connect(stopServerBtn, &QPushButton::clicked, this, &QWidget::close);
}

void ServerWindow::onKickRequested()
{
    QListWidgetItem* item = m_userListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Kick Error", "Please select a user to kick.");
        return;
    }
    QString username = item->text();
    m_server->kickUser(username);
}

void ServerWindow::onAnnounceRequested()
{
    QString text = m_announcementInput->text().trimmed();
    if (text.isEmpty()) return;

    m_server->sendGlobalAnnouncement(text);
    m_announcementInput->clear();
}

void ServerWindow::appendLog(const QString& message)
{
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logView->append("<b>[" + timeStr + "]</b> " + message);
}

void ServerWindow::updateUserList(const QStringList& users)
{
    m_userListWidget->clear();
    m_userListWidget->addItems(users);
}

void ServerWindow::closeEvent(QCloseEvent* event)
{
    // Server close and all users out
    m_server->stopServer();
    event->accept(); // Window close
}

void ServerWindow::applyStyles()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #f5f0eb;
            color: #3d3450;
            font-family: 'Segoe UI', sans-serif;
        }
        QLabel#panelTitle {
            font-size: 13px;
            font-weight: 600;
            color: #8b5cf6;
        }
        QListWidget#userList, QTextEdit#logView {
            background-color: #ffffff;
            border: 1px solid #e0d5ec;
            border-radius: 8px;
            padding: 5px;
            font-size: 12px;
        }
        QLineEdit#inputField {
            background-color: #ffffff;
            border: 1px solid #e8dff0;
            border-radius: 8px;
            padding: 0 10px;
        }
        QLineEdit#inputField:focus {
            border: 1px solid #8b5cf6;
        }
        QPushButton#primaryBtn {
            background: #8b5cf6;
            color: white;
            border: none;
            border-radius: 8px;
            font-weight: 600;
            padding: 0 15px;
        }
        QPushButton#primaryBtn:hover { background: #7c3aed; }
        QPushButton#kickBtn {
            background-color: #fff;
            border: 1px solid #f87171;
            color: #f87171;
            border-radius: 8px;
            font-weight: 600;
        }
        QPushButton#kickBtn:hover {
            background-color: #f87171;
            color: white;
        }
    )");
}