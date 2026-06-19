#include "connectionwindow.h"

ConnectionWindow::ConnectionWindow(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    applyStyles();
}

void ConnectionWindow::setupUI()
{
    setFixedSize(420, 500);
    setWindowTitle("LanChat");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_stack = new QStackedWidget(this);
    mainLayout->addWidget(m_stack);

    // ── MODE PAGE ──
    m_modePage = new QWidget();
    QVBoxLayout* modeLayout = new QVBoxLayout(m_modePage);
    modeLayout->setAlignment(Qt::AlignCenter);
    modeLayout->setSpacing(16);

    QLabel* logo = new QLabel("LanChat");
    logo->setAlignment(Qt::AlignCenter);
    logo->setObjectName("logo");

    QLabel* sub = new QLabel("Choose how to continue");
    sub->setAlignment(Qt::AlignCenter);
    sub->setObjectName("subLabel");

    m_serverBtn = new QPushButton("Start as Server");
    m_serverBtn->setObjectName("primaryBtn");
    m_serverBtn->setFixedHeight(48);

    m_clientBtn = new QPushButton("Join as Client");
    m_clientBtn->setObjectName("secondaryBtn");
    m_clientBtn->setFixedHeight(48);

    modeLayout->addStretch();
    modeLayout->addWidget(logo);
    modeLayout->addWidget(sub);
    modeLayout->addSpacing(32);
    modeLayout->addWidget(m_serverBtn);
    modeLayout->addWidget(m_clientBtn);
    modeLayout->addStretch();

    // ── SERVER PAGE ──
    m_serverPage = new QWidget();
    QVBoxLayout* serverLayout = new QVBoxLayout(m_serverPage);
    serverLayout->setAlignment(Qt::AlignCenter);
    serverLayout->setSpacing(12);

    QLabel* serverTitle = new QLabel("Start as Server");
    serverTitle->setObjectName("pageTitle");
    serverTitle->setAlignment(Qt::AlignCenter);

    m_serverUsernameInput = new QLineEdit();
    m_serverUsernameInput->setPlaceholderText("Your username");
    m_serverUsernameInput->setObjectName("inputField");
    m_serverUsernameInput->setFixedHeight(44);



    m_portInput = new QLineEdit();
    m_portInput->setPlaceholderText("Port (default: 5050)");
    m_portInput->setObjectName("inputField");
    m_portInput->setFixedHeight(44);
    m_portInput->setText("5050");

    m_startServerBtn = new QPushButton("Start Server");
    m_startServerBtn->setObjectName("primaryBtn");
    m_startServerBtn->setFixedHeight(48);

    QPushButton* backBtn1 = new QPushButton("← Back");
    backBtn1->setObjectName("ghostBtn");

    serverLayout->addStretch();
    serverLayout->addWidget(serverTitle);
    serverLayout->addSpacing(16);
    // serverLayout mein add karo port se pehle
    serverLayout->addWidget(m_serverUsernameInput);
    serverLayout->addWidget(m_portInput);
    serverLayout->addWidget(m_startServerBtn);
    serverLayout->addWidget(backBtn1);
    serverLayout->addStretch();
    connect(m_startServerBtn, &QPushButton::clicked, this, [this]() {
        QString username = m_serverUsernameInput->text().trimmed();
        quint16 port = m_portInput->text().toUShort();
        if (port == 0) port = 5050;
        if (username.isEmpty()) return;
        emit startAsServer(username, port);
    });

    // ── CLIENT PAGE ──
    m_clientPage = new QWidget();
    QVBoxLayout* clientLayout = new QVBoxLayout(m_clientPage);
    clientLayout->setAlignment(Qt::AlignCenter);
    clientLayout->setSpacing(12);

    QLabel* clientTitle = new QLabel("Join as Client");
    clientTitle->setObjectName("pageTitle");
    clientTitle->setAlignment(Qt::AlignCenter);

    m_hostInput = new QLineEdit();
    m_hostInput->setPlaceholderText("Server IP (e.g. 192.168.1.5)");
    m_hostInput->setObjectName("inputField");
    m_hostInput->setFixedHeight(44);

    m_portInput2 = new QLineEdit();
    m_portInput2->setPlaceholderText("Port (default: 5050)");
    m_portInput2->setObjectName("inputField");
    m_portInput2->setFixedHeight(44);
    m_portInput2->setText("5050");

    m_userInput = new QLineEdit();
    m_userInput->setPlaceholderText("Your username");
    m_userInput->setObjectName("inputField");
    m_userInput->setFixedHeight(44);

    m_connectBtn = new QPushButton("Connect");
    m_connectBtn->setObjectName("primaryBtn");
    m_connectBtn->setFixedHeight(48);

    QPushButton* backBtn2 = new QPushButton("← Back");
    backBtn2->setObjectName("ghostBtn");

    clientLayout->addStretch();
    clientLayout->addWidget(clientTitle);
    clientLayout->addSpacing(16);
    clientLayout->addWidget(m_hostInput);
    clientLayout->addWidget(m_portInput2);
    clientLayout->addWidget(m_userInput);
    clientLayout->addWidget(m_connectBtn);
    clientLayout->addWidget(backBtn2);
    clientLayout->addStretch();

    // ── ADD PAGES ──
    m_stack->addWidget(m_modePage);
    m_stack->addWidget(m_serverPage);
    m_stack->addWidget(m_clientPage);

    // ── CONNECTIONS ──
    connect(m_serverBtn,      &QPushButton::clicked, this, &ConnectionWindow::onServerClicked);
    connect(m_clientBtn,      &QPushButton::clicked, this, &ConnectionWindow::onClientClicked);
    connect(m_startServerBtn, &QPushButton::clicked, this, [this]() {
        QString username = m_serverUsernameInput->text().trimmed();
        quint16 port = m_portInput->text().toUShort();
        if (port == 0) port = 5050;
        if (username.isEmpty()) return;
        emit startAsServer(username, port);
    });
    connect(m_connectBtn, &QPushButton::clicked, this, [this]() {
        QString host = m_hostInput->text().trimmed();
        QString user = m_userInput->text().trimmed();
        quint16 port = m_portInput2->text().toUShort();
        if (port == 0) port = 5050;
        if (host.isEmpty() || user.isEmpty()) return;
        emit startAsClient(host, port, user);
    });
    connect(backBtn1, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
    });
    connect(backBtn2, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
    });
}

void ConnectionWindow::onServerClicked()
{
    m_stack->setCurrentIndex(1);
}

void ConnectionWindow::onClientClicked()
{
    m_stack->setCurrentIndex(2);
}

void ConnectionWindow::applyStyles()
{
    setStyleSheet(R"(
        ConnectionWindow {
            background-color: #f5f0eb;
        }
        QLabel#logo {
            font-size: 32px;
            font-weight: 700;
            color: #3d3450;
            letter-spacing: -1px;
        }
        QLabel#logo span {
            color: #8b5cf6;
        }
        QLabel#subLabel {
            font-size: 13px;
            color: #a89bb8;
        }
        QLabel#pageTitle {
            font-size: 20px;
            font-weight: 600;
            color: #3d3450;
        }
        QLineEdit#inputField {
            background-color: #f5f0fb;
            border: 1px solid #e8dff0;
            border-radius: 10px;
            padding: 0 14px;
            font-size: 13px;
            color: #3d3450;
        }
        QLineEdit#inputField:focus {
            border: 1px solid #8b5cf6;
        }
        QPushButton#primaryBtn {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 #8b5cf6, stop:1 #a78bfa
            );
            border: none;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
            color: white;
        }
        QPushButton#primaryBtn:hover {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 #7c3aed, stop:1 #8b5cf6
            );
        }
        QPushButton#secondaryBtn {
            background-color: #fff;
            border: 1px solid #e8dff0;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
            color: #3d3450;
        }
        QPushButton#secondaryBtn:hover {
            border: 1px solid #8b5cf6;
            color: #8b5cf6;
        }
        QPushButton#ghostBtn {
            background: transparent;
            border: none;
            color: #a89bb8;
            font-size: 12px;
        }
        QPushButton#ghostBtn:hover {
            color: #8b5cf6;
        }
    )");
}