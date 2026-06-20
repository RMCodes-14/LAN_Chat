#include "chatwindow.h"
#include <QScrollBar>
#include <QDateTime>

// ─────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────
ChatWindow::ChatWindow(const QString& username,
                       const QString& host,
                       quint16 port,
                       QWidget* parent)
    : QWidget(parent), m_username(username)
{
    setupUI();
    applyStyles();


    m_client = new ChatClient(this);

    connect(m_client, &ChatClient::messageReceived,
            this,     &ChatWindow::onMessageReceived);
    connect(m_client, &ChatClient::connectedToServer,
            this,     &ChatWindow::onConnected);
    connect(m_client, &ChatClient::disconnectedFromServer,
            this,     &ChatWindow::onDisconnected);

    m_client->connectToServer(host, port);

    // Typing timer — clears label after 2.5s
    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    connect(m_typingTimer, &QTimer::timeout, this, [this]() {
        m_typingLabel->clear();
    });

}

// ─────────────────────────────────────────
//  UI Setup
// ─────────────────────────────────────────
void ChatWindow::setupUI()
{
    setWindowTitle("LanChat");
    setMinimumSize(860, 560);

    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── SIDEBAR ──────────────────────────
    m_sidebar = new QWidget();
    m_sidebar->setObjectName("sidebar");
    m_sidebar->setFixedWidth(220);

    QVBoxLayout* sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->setSpacing(0);

    // Logo
    QWidget* logoArea = new QWidget();
    logoArea->setObjectName("logoArea");
    QVBoxLayout* logoLayout = new QVBoxLayout(logoArea);
    logoLayout->setContentsMargins(18, 20, 18, 16);

    QLabel* logo = new QLabel("Lan<span style='color:#8b5cf6'>Chat</span>");
    logo->setTextFormat(Qt::RichText);
    logo->setObjectName("logo");

    QLabel* serverTag = new QLabel("● Connected");
    serverTag->setObjectName("serverTag");

    logoLayout->addWidget(logo);
    logoLayout->addWidget(serverTag);
    sideLayout->addWidget(logoArea);

    // Section label
    QLabel* onlineSec = new QLabel("ONLINE");
    onlineSec->setObjectName("sectionLabel");
    onlineSec->setContentsMargins(18, 14, 18, 5);
    sideLayout->addWidget(onlineSec);

    // Users list
    QWidget* usersContainer = new QWidget();
    m_usersLayout = new QVBoxLayout(usersContainer);
    m_usersLayout->setContentsMargins(0, 0, 0, 0);
    m_usersLayout->setSpacing(0);
    m_usersLayout->addStretch();
    sideLayout->addWidget(usersContainer);
    sideLayout->addStretch();

    // My profile
    QWidget* myProfile = new QWidget();
    myProfile->setObjectName("myProfile");
    QHBoxLayout* profileLayout = new QHBoxLayout(myProfile);
    profileLayout->setContentsMargins(18, 14, 18, 14);

    QLabel* myAv = new QLabel(m_username.left(2).toUpper());
    myAv->setObjectName("myAvatar");
    myAv->setFixedSize(32, 32);
    myAv->setAlignment(Qt::AlignCenter);

    QVBoxLayout* myInfo = new QVBoxLayout();
    myInfo->setSpacing(1);
    QLabel* myName = new QLabel(m_username);
    myName->setObjectName("myName");
    QLabel* myStatus = new QLabel("You");
    myStatus->setObjectName("myStatus");
    myInfo->addWidget(myName);
    myInfo->addWidget(myStatus);

    profileLayout->addWidget(myAv);
    profileLayout->addLayout(myInfo);
    sideLayout->addWidget(myProfile);

    root->addWidget(m_sidebar);

    // ── MAIN AREA ─────────────────────────
    QWidget* mainArea = new QWidget();
    mainArea->setObjectName("mainArea");
    QVBoxLayout* mainLayout = new QVBoxLayout(mainArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QWidget* header = new QWidget();
    header->setObjectName("chatHeader");
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(22, 14, 22, 14);

    QLabel* chatAv = new QLabel("GC");
    chatAv->setObjectName("chatAvatar");
    chatAv->setFixedSize(36, 36);
    chatAv->setAlignment(Qt::AlignCenter);

    QVBoxLayout* chatInfo = new QVBoxLayout();
    chatInfo->setSpacing(1);
    QLabel* chatTitle = new QLabel("Group Chat");
    chatTitle->setObjectName("chatTitle");
    QLabel* chatSub = new QLabel("LAN Network");
    chatSub->setObjectName("chatSub");
    chatInfo->addWidget(chatTitle);
    chatInfo->addWidget(chatSub);

    QLabel* lanBadge = new QLabel("LAN · 192.168.1.x");
    lanBadge->setObjectName("lanBadge");

    // ── LEAVE BUTTON ──
    m_leaveBtn = new QPushButton("Leave");
    m_leaveBtn->setObjectName("leaveBtn");

    headerLayout->addWidget(chatAv);
    headerLayout->addLayout(chatInfo);
    headerLayout->addStretch();
    headerLayout->addWidget(lanBadge);
    headerLayout->addWidget(m_leaveBtn);
    mainLayout->addWidget(header);

    // Scroll area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setObjectName("scrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->verticalScrollBar()->setObjectName("scrollBar");

    m_messagesWidget = new QWidget();
    m_messagesWidget->setObjectName("messagesWidget");
    m_messagesLayout = new QVBoxLayout(m_messagesWidget);
    m_messagesLayout->setContentsMargins(22, 18, 22, 18);
    m_messagesLayout->setSpacing(3);
    m_messagesLayout->addStretch();

    m_scrollArea->setWidget(m_messagesWidget);
    mainLayout->addWidget(m_scrollArea);

    // Typing label
    m_typingLabel = new QLabel();
    m_typingLabel->setObjectName("typingLabel");
    m_typingLabel->setContentsMargins(22, 4, 22, 4);
    mainLayout->addWidget(m_typingLabel);

    // Input bar
    QWidget* inputBar = new QWidget();
    inputBar->setObjectName("inputBar");
    QHBoxLayout* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(16, 12, 16, 12);
    inputLayout->setSpacing(10);

    m_inputField = new QLineEdit();
    m_inputField->setObjectName("inputField");
    m_inputField->setPlaceholderText("Message...");
    m_inputField->setFixedHeight(44);

    m_sendBtn = new QPushButton("➤");
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setFixedSize(44, 44);

    inputLayout->addWidget(m_inputField);
    inputLayout->addWidget(m_sendBtn);
    mainLayout->addWidget(inputBar);

    root->addWidget(mainArea);

    // ── CONNECT SIGNALS ───────────────────
    connect(m_sendBtn,    &QPushButton::clicked,
            this,         &ChatWindow::onSendClicked);
    connect(m_inputField, &QLineEdit::returnPressed,
            this,         &ChatWindow::onSendClicked);

    // Typing broadcast
    connect(m_inputField, &QLineEdit::textChanged, this, [this]() {
        Message msg;
        msg.type      = "typing";
        msg.sender    = m_username;
        msg.content   = "";
        msg.timestamp = "";
        m_client->sendRaw(msg.toBytes());
        m_typingTimer->start(2500);
    });

    // Leave button
    connect(m_leaveBtn, &QPushButton::clicked, this, [this]() {
        // Pehle leave message broadcast karo
        Message msg;
        msg.type      = "leave";
        msg.sender    = m_username;
        msg.content   = "";
        msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");
        m_client->sendRaw(msg.toBytes());

        // 100ms baad disconnect karo taake message jaaye pehle
        QTimer::singleShot(100, this, [this]() {
            m_client->disconnectFromServer();
            close();
        });
    });
}

// ─────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────
void ChatWindow::onSendClicked()
{
    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) return;

    // Apna message locally add karo — right side
    Message msg;
    msg.type      = "chat";
    msg.sender    = m_username;
    msg.content   = text;
    msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");
    addBubble(msg);

    // Server ko bhejo — doosron ko jaega
    m_client->sendMessage(text, m_username);
    m_inputField->clear();
}

void ChatWindow::onMessageReceived(const Message& msg)
{
    if (msg.type == "chat") {
        if (msg.sender != m_username) {
            addBubble(msg);
             // sirf doosron ka message aaye toh sound
        }
    }else if (msg.type == "join") {
        // Sabka join dikhao — apna bhi
        addSystemMessage(msg.sender + " joined the chat");

    } else if (msg.type == "leave") {
        addSystemMessage(msg.sender + " left the chat");

    } else if (msg.type == "typing") {
        // Apna typing indicator khud ko mat dikhao
        if (msg.sender != m_username) {
            m_typingLabel->setText(msg.sender + " is typing...");
            m_typingTimer->start(2500);
        }
    }
}

void ChatWindow::onConnected()
{
    // Sirf join broadcast karo — locally mat dikhao
    // Server broadcast karega sabko — tab dikhega
    Message msg;
    msg.type      = "join";
    msg.sender    = m_username;
    msg.content   = "";
    msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");
    m_client->sendRaw(msg.toBytes());
}

void ChatWindow::onDisconnected()
{
    addSystemMessage("Disconnected from server");
}

// ─────────────────────────────────────────
//  UI Helpers
// ─────────────────────────────────────────
void ChatWindow::addBubble(const Message& msg)
{
    bool isMe = (msg.sender == m_username);

    QWidget* row = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 2, 0, 2);
    rowLayout->setSpacing(8);

    QLabel* av = new QLabel(msg.sender.left(2).toUpper());
    av->setFixedSize(26, 26);
    av->setAlignment(Qt::AlignCenter);
    av->setObjectName(isMe ? "avatarMe" : "avatarThem");

    QWidget* bubbleCol = new QWidget();
    QVBoxLayout* colLayout = new QVBoxLayout(bubbleCol);
    colLayout->setContentsMargins(0, 0, 0, 0);
    colLayout->setSpacing(2);

    if (!isMe) {
        QLabel* sender = new QLabel(msg.sender);
        sender->setObjectName("senderName");
        colLayout->addWidget(sender);
    }

    QLabel* bubble = new QLabel(msg.content);
    bubble->setWordWrap(true);
    /*bubble->setMinimumWidth(200); */  // minimum half screen
    bubble->setMaximumWidth(420);
    bubble->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bubble->setObjectName(isMe ? "bubbleMe" : "bubbleThem");

    QLabel* time = new QLabel(msg.timestamp);
    time->setObjectName("msgTime");

    colLayout->addWidget(bubble);
    colLayout->addWidget(time);

    if (isMe) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubbleCol);
        rowLayout->addWidget(av);
        time->setAlignment(Qt::AlignRight);
    } else {
        rowLayout->addWidget(av);
        rowLayout->addWidget(bubbleCol);
        rowLayout->addStretch();
    }

    m_messagesLayout->addWidget(row);

    scrollToBottom();
}

void ChatWindow::addSystemMessage(const QString& text)
{
    QLabel* sys = new QLabel(text);
    sys->setObjectName("systemMsg");
    sys->setAlignment(Qt::AlignCenter);
    m_messagesLayout->addWidget(sys);
    scrollToBottom();
}

void ChatWindow::scrollToBottom()
{
    QTimer::singleShot(50, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum()
            );
    });
}

// ─────────────────────────────────────────
//  Styles
// ─────────────────────────────────────────
void ChatWindow::applyStyles()
{
    setStyleSheet(R"(
        QWidget#sidebar {
            background-color: #ede6f0;
            border-right: 1px solid #e0d5ec;
        }
        QWidget#logoArea {
            border-bottom: 1px solid #e0d5ec;
            background-color: #ede6f0;
        }
        QLabel#logo {
            font-size: 20px;
            font-weight: 700;
            color: #3d3450;
        }
        QLabel#serverTag {
            font-size: 10px;
            color: #86efac;
        }
        QLabel#sectionLabel {
            font-size: 9px;
            font-weight: 600;
            color: #c4b8d4;
            letter-spacing: 1px;
        }
        QWidget#myProfile {
            background-color: #e4daea;
            border-top: 1px solid #e0d5ec;
        }
        QLabel#myAvatar {
            background-color: #ede0f8;
            color: #8b5cf6;
            border-radius: 10px;
            font-weight: 700;
            font-size: 11px;
        }
        QLabel#myName {
            font-size: 12px;
            font-weight: 600;
            color: #3d3450;
        }
        QLabel#myStatus {
            font-size: 10px;
            color: #a89bb8;
        }
        QWidget#mainArea {
            background-color: #f5f0eb;
        }
        QWidget#chatHeader {
            background-color: #ffffff;
            border-bottom: 1px solid #ede6f0;
        }
        QLabel#chatAvatar {
            background-color: #ede0f8;
            color: #6d28d9;
            border-radius: 11px;
            font-weight: 700;
            font-size: 12px;
        }
        QLabel#chatTitle {
            font-size: 14px;
            font-weight: 600;
            color: #3d3450;
        }
        QLabel#chatSub {
            font-size: 10px;
            color: #a89bb8;
        }
        QLabel#lanBadge {
            background-color: #f5f0fb;
            border: 1px solid #e8dff0;
            border-radius: 20px;
            padding: 3px 12px;
            font-size: 10px;
            color: #a89bb8;
        }
        QPushButton#leaveBtn {
            background-color: transparent;
            border: 1px solid #e8dff0;
            border-radius: 8px;
            color: #a89bb8;
            font-size: 11px;
            padding: 4px 12px;
        }
        QPushButton#leaveBtn:hover {
            border-color: #f87171;
            color: #f87171;
        }
        QScrollArea#scrollArea {
            background-color: #f9f6f2;
            border: none;
        }
        QWidget#messagesWidget {
            background-color: #f9f6f2;
        }
        QScrollBar:vertical {
            width: 4px;
            background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #e8dff0;
            border-radius: 2px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QLabel#bubbleMe {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #8b5cf6, stop:1 #a78bfa);
            color: white;
            border-radius: 14px;
            padding: 9px 13px;
            font-size: 13px;
        }
        QLabel#bubbleThem {
            background-color: #ffffff;
            color: #3d3450;
            border: 1px solid #ede6f0;
            border-radius: 14px;
            padding: 9px 13px;
            font-size: 13px;
        }
        QLabel#senderName {
            font-size: 9px;
            color: #c4b8d4;
            padding-left: 3px;
        }
        QLabel#msgTime {
            font-size: 9px;
            color: #d4cae0;
            padding-left: 3px;
        }
        QLabel#systemMsg {
            font-size: 10px;
            color: #c4b8d4;
            padding: 4px 0;
        }
        QLabel#typingLabel {
            font-size: 14px;
            color: #c4b8d4;
            font-style: italic;
            background-color: #f9f6f2;

        }
        QWidget#inputBar {
            background-color: #ffffff;
            border-top: 1px solid #ede6f0;
        }
        QLineEdit#inputField {
            background-color: #f5f0fb;
            border: 1px solid #e8dff0;
            border-radius: 12px;
            padding: 0 14px;
            font-size: 13px;
            color: #3d3450;
        }
        QLineEdit#inputField:focus {
            border: 1px solid #8b5cf6;
        }
        QPushButton#sendBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #8b5cf6, stop:1 #a78bfa);
            border: none;
            border-radius: 12px;
            color: white;
            font-size: 16px;
        }
        QPushButton#sendBtn:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                stop:0 #7c3aed, stop:1 #8b5cf6);
        }
        QLabel#avatarMe {
            background-color: #ede0f8;
            color: #8b5cf6;
            border-radius: 8px;
            font-weight: 700;
            font-size: 9px;
        }
        QLabel#avatarThem {
            background-color: #dde8f8;
            color: #4f7ccf;
            border-radius: 8px;
            font-weight: 700;
            font-size: 9px;
        }
    )");
}