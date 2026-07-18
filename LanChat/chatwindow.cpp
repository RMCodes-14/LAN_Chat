#include "chatwindow.h"
#include <QScrollBar>
#include <QDateTime>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

// Constructor
ChatWindow::ChatWindow(const QString& username,
                       const QString& host,
                       quint16 port,
                       bool isServer,
                       QWidget* parent)
    : QWidget(parent), m_username(username), m_isServer(isServer)
{
    setupUI();
    applyStyles();

    m_client = new ChatClient(this);
    m_messagesWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_messagesWidget, &QWidget::customContextMenuRequested,
            this, &ChatWindow::showContextMenu);

    connect(m_client, &ChatClient::messageReceived,
            this,     &ChatWindow::onMessageReceived);
    connect(m_client, &ChatClient::connectedToServer,
            this,     &ChatWindow::onConnected);
    connect(m_client, &ChatClient::disconnectedFromServer,
            this,     &ChatWindow::onDisconnected);
    connect(m_client, &ChatClient::connectionError,
            this, [this](const QString& err) {
                QMessageBox::critical(this, "Connection Error",
                                      "Could not connect:\n" + err);
                close();
            });

    m_client->connectToServer(host, port);
    m_notifSound = new QSoundEffect(this);
    m_notifSound->setSource(QUrl("qrc:/sounds/notif.wav"));
    m_notifSound->setVolume(0.6f);

    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    connect(m_typingTimer, &QTimer::timeout, this, [this]() {
        m_typingLabel->clear();
    });
}

// UI Setup
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

    QLabel* onlineSec = new QLabel("ONLINE");
    onlineSec->setObjectName("sectionLabel");
    onlineSec->setContentsMargins(18, 14, 18, 5);
    sideLayout->addWidget(onlineSec);

    QWidget* usersContainer = new QWidget();
    m_usersLayout = new QVBoxLayout(usersContainer);
    m_usersLayout->setContentsMargins(0, 0, 0, 0);
    m_usersLayout->setSpacing(0);
    m_usersLayout->addStretch();
    sideLayout->addWidget(usersContainer);
    sideLayout->addStretch();

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

    m_leaveBtn = new QPushButton("Leave");
    m_leaveBtn->setObjectName("leaveBtn");

    headerLayout->addWidget(chatAv);
    headerLayout->addLayout(chatInfo);
    headerLayout->addStretch();
    headerLayout->addWidget(lanBadge);
    headerLayout->addWidget(m_leaveBtn);
    mainLayout->addWidget(header);

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

    m_typingLabel = new QLabel();
    m_typingLabel->setObjectName("typingLabel");
    m_typingLabel->setContentsMargins(22, 4, 22, 4);
    mainLayout->addWidget(m_typingLabel);

    // 🎨 Premium Message Input Bar Layout (Bina Kisi Change Ke Same)
    QWidget* inputBar = new QWidget();
    inputBar->setObjectName("inputBar");
    QHBoxLayout* inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(16, 12, 16, 12);
    inputLayout->setSpacing(10);

    QPushButton* m_attachBtn = new QPushButton("📎");
    m_attachBtn->setFixedSize(44, 44);
    m_attachBtn->setObjectName("attachBtn");

    m_inputField = new QLineEdit();
    m_inputField->setObjectName("inputField");
    m_inputField->setPlaceholderText("Message...");
    m_inputField->setFixedHeight(44);

    m_sendBtn = new QPushButton("➤");
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setFixedSize(44, 44);

    inputLayout->addWidget(m_attachBtn);
    inputLayout->addWidget(m_inputField);
    inputLayout->addWidget(m_sendBtn);

    mainLayout->addWidget(inputBar);
    root->addWidget(mainArea);

    // Signals Connections
    connect(m_sendBtn,    &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(m_inputField, &QLineEdit::returnPressed, this, &ChatWindow::onSendClicked);
    connect(m_attachBtn,  &QPushButton::clicked, this, &ChatWindow::onAttachFileClicked);

    connect(m_inputField, &QLineEdit::textChanged, this, [this]() {
        Message msg;
        msg.type      = "typing";
        msg.sender    = m_username;
        msg.content   = "";
        msg.timestamp = "";
        m_client->sendRaw(msg.toBytes());
        m_typingTimer->start(2500);
    });

    connect(m_leaveBtn, &QPushButton::clicked, this, [this]() {
        Message msg;
        msg.type      = "leave";
        msg.sender    = m_username;
        msg.content   = "";
        msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");
        m_client->sendRaw(msg.toBytes());

        QTimer::singleShot(100, this, [this]() {
            m_client->disconnectFromServer();
            close();
        });
    });
}

void ChatWindow::onSendClicked()
{
    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) return;

    Message msg;
    msg.id = m_username + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    msg.type = "chat";
    msg.sender = m_username;
    msg.content = text;
    msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

    m_client->sendRaw(msg.toBytes());
    addBubble(msg);
    m_inputField->clear();
}

void ChatWindow::onAttachFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select File", "", "All Files (*.*)");
    if (filePath.isEmpty()) return;

    QFileInfo fileInfo(filePath);
    qint64 fileSize = fileInfo.size();
    QString ext = fileInfo.suffix().toLower();

    if (ext == "mp4" || ext == "mkv" || ext == "avi" || ext == "mov") {
        QMessageBox::critical(this, "Error", "Videos are not allowed as they are too heavy!");
        return;
    }

    qint64 maxLimit = 5 * 1024 * 1024; // 5 MB
    if (fileSize > maxLimit) {
        QMessageBox::critical(this, "Error", "File is too heavy! Maximum allowed size is 5MB.");
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray fileData = file.readAll();

        Message msg;
        msg.id = m_username + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
        msg.type = "file";
        msg.sender = m_username;
        // Filename aur Base64 ko content stream mein pack kiya
        msg.content = fileInfo.fileName() + "|" + QString(fileData.toBase64());
        msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");

        m_client->sendRaw(msg.toBytes());
        addBubble(msg);
    }
}

void ChatWindow::onMessageReceived(const Message& msg)
{
    if (msg.type == "chat" || msg.type == "file") {
        if (msg.sender != m_username) {
            addBubble(msg);
            m_notifSound->play();
        }
    }
    else if (msg.type == "history") {
        addBubble(msg);
    }
    else if (msg.type == "join") {
        if (msg.sender == m_username)
            addSystemMessage("You joined the chat ✓");
        else
            addSystemMessage(msg.sender + " joined the chat");
    }
    else if (msg.type == "leave") {
        addSystemMessage(msg.sender + " left the chat");
    }
    else if (msg.type == "kick") {
        addSystemMessage("❌ " + msg.content + " was kicked out by Admin!");
        if (msg.content == m_username) {
            m_client->disconnectFromServer();
            QTimer::singleShot(500, this, [this]() {
                QMessageBox::warning(this, "Kicked", "You have been kicked from the chat room.");
                close();
            });
        }
    }
    else if (msg.type == "userlist") {
        updateUserList(msg.content.split(",", Qt::SkipEmptyParts));
    }
    else if (msg.type == "typing") {
        if (msg.sender != m_username) {
            m_typingLabel->setText(msg.sender + " is typing...");
            m_typingTimer->start(2500);
        }
    }
    else if (msg.type == "delete") {
        // 🔥 UPDATE: Jab network se delete message aaye to sahi target row ko update karein
        QString targetId = msg.content;
        QWidget* targetRow = m_messagesWidget->findChild<QWidget*>(targetId);
        if (targetRow) {
            QList<QLabel*> labels = targetRow->findChildren<QLabel*>();
            bool textLabelFound = false;
            for (QLabel* label : labels) {
                if (label->property("isMessageText").toBool() || label->objectName() == "msgTextLabel") {
                    if (msg.sender == m_username) {
                        label->setText("<i>🚫 You deleted this message</i>");
                    } else {
                        label->setText("<i>🚫 This message was deleted from " + msg.sender + "'s side</i>");
                    }
                    label->setStyleSheet("color: #a89bb8; font-style: italic;");
                    textLabelFound = true;
                    break;
                }
            }

            if (!textLabelFound) {
                QWidget* bubbleWidget = targetRow->findChild<QWidget*>();
                if (bubbleWidget) {
                    QLayout* bLayout = bubbleWidget->layout();
                    if (bLayout) {
                        QLayoutItem* item;
                        while ((item = bLayout->takeAt(0)) != nullptr) {
                            if (item->widget()) item->widget()->deleteLater();
                            delete item;
                        }
                        QLabel* delNotice = new QLabel(msg.sender == m_username ? "<i>🚫 You deleted this file</i>" : "<i>🚫 This file was deleted</i>");
                        delNotice->setStyleSheet("color: #a89bb8; font-style: italic; font-size: 12px;");
                        bLayout->addWidget(delNotice);
                    }
                }
            }
        }
    }
}

void ChatWindow::onConnected()
{
    if (m_joinSent) return;
    m_joinSent = true;
    int delay = m_isServer ? 100 : 0;
    QTimer::singleShot(delay, this, [this]() {
        Message msg;
        msg.type      = "join";
        msg.sender    = m_username;
        msg.content   = "";
        msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");
        m_client->sendRaw(msg.toBytes());
    });
}

void ChatWindow::onDisconnected()
{
    addSystemMessage("Disconnected from server");
}

void ChatWindow::updateUserList(const QStringList& users)
{
    QLayoutItem* item;
    while ((item = m_usersLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    QStringList seen;
    for (const QString& username : users) {
        if (seen.contains(username)) continue;
        seen.append(username);

        QWidget* userRow = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(userRow);
        layout->setContentsMargins(18, 7, 18, 7);
        layout->setSpacing(9);

        QLabel* av = new QLabel(username.left(2).toUpper());
        av->setFixedSize(30, 30);
        av->setAlignment(Qt::AlignCenter);
        av->setStyleSheet("background-color:#ede0f8; color:#8b5cf6; border-radius:9px; font-weight:700; font-size:10px;");

        QLabel* name = new QLabel(username);
        name->setStyleSheet("font-size:12px;font-weight:500;color:#3d3450;");

        QLabel* dot = new QLabel();
        dot->setFixedSize(7, 7);
        dot->setStyleSheet("background-color:#86efac;border-radius:3px;");

        layout->addWidget(av);
        layout->addWidget(name);
        layout->addStretch();
        layout->addWidget(dot);

        m_usersLayout->addWidget(userRow);
    }
    m_usersLayout->addStretch();
}

void ChatWindow::showContextMenu(const QPoint& pos)
{
    QWidget* child = m_messagesWidget->childAt(pos);
    if (!child) return;

    QWidget* targetRow = child;
    while (targetRow && targetRow->parentWidget() != m_messagesWidget) {
        targetRow = targetRow->parentWidget();
    }
    if (!targetRow || targetRow->objectName().isEmpty()) return;

    QString messageId = targetRow->objectName();
    if (!messageId.startsWith(m_username + "_")) return; // Sirf apne message delete karne ki permission

    QMenu contextMenu(this);
    QAction* deleteAction = contextMenu.addAction("Delete Message");
    QAction* selectedAction = contextMenu.exec(m_messagesWidget->mapToGlobal(pos));

    if (selectedAction == deleteAction) {
        onDeleteRequested(messageId);
    }
}

// 🔥 UPDATE: Pure network standard par delete packet bejhna taake dono users sync ho sakein
void ChatWindow::onDeleteRequested(const QString& messageId)
{
    Message deletePacket;
    deletePacket.id = messageId;
    deletePacket.type = "delete";
    deletePacket.sender = m_username;
    deletePacket.content = messageId; // content field holds the unique row ID
    deletePacket.timestamp = QDateTime::currentDateTime().toString("hh:mm");
    m_client->sendRaw(deletePacket.toBytes());
}

// UI Bubble Generation Logic
void ChatWindow::addBubble(const Message& msg)
{
    bool isMe = (msg.sender == m_username);

    QWidget* row = new QWidget();
    row->setObjectName(msg.id); // Set standard ID for deletion target lookup

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

    QWidget* bubble = new QWidget();
    bubble->setObjectName(isMe ? "bubbleMe" : "bubbleThem");
    QVBoxLayout* bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(10, 10, 10, 10);

    if (msg.type == "file") {
        QStringList parts = msg.content.split("|");
        QString fileName = parts.first();
        QString base64Data = parts.size() > 1 ? parts.at(1) : "";

        QHBoxLayout* fileLayout = new QHBoxLayout();
        fileLayout->setSpacing(8);

        QLabel* fileIcon = new QLabel("📁");
        fileIcon->setStyleSheet("font-size: 16px;");

        QLabel* fileNameLabel = new QLabel(fileName);
        fileNameLabel->setStyleSheet("font-weight: bold; color: " + QString(isMe ? "white" : "#3d3450") + "; font-size: 13px;");
        fileNameLabel->setWordWrap(true);

        QPushButton* downloadBtn = new QPushButton("Open");
        downloadBtn->setCursor(Qt::PointingHandCursor);
        downloadBtn->setFixedWidth(55);
        downloadBtn->setStyleSheet(
            "background-color: rgba(255, 255, 255, 0.25);"
            "border: 1px solid rgba(255, 255, 255, 0.4);"
            "color: " + QString(isMe ? "white" : "#8b5cf6") + ";"
                                                    "border-radius: 6px; padding: 3px; font-size: 11px; font-weight: bold;"
            );

        // 🔥 Safe Download & Instant Open System Integration
        connect(downloadBtn, &QPushButton::clicked, this, [fileName, base64Data, this]() {
            if (base64Data.isEmpty()) return;

            // Safe standard Windows directory path jo humesha file permission pass karti hai
            QString savePath = QDir::tempPath() + "/" + fileName;
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(QByteArray::fromBase64(base64Data.toUtf8()));
                file.close();

                // Open file with native application launcher automatically
                QDesktopServices::openUrl(QUrl::fromLocalFile(savePath));
            } else {
                QMessageBox::warning(this, "Error", "Failed to decode and save file attachment.");
            }
        });

        fileLayout->addWidget(fileIcon);
        fileLayout->addWidget(fileNameLabel, 1);
        fileLayout->addWidget(downloadBtn);
        bubbleLayout->addLayout(fileLayout);
    } else {
        QLabel* textLabel = new QLabel(msg.content);
        textLabel->setWordWrap(true);
        textLabel->setObjectName("msgTextLabel");
        textLabel->setProperty("isMessageText", true);
        textLabel->setStyleSheet("color: inherit; font-size: 13px;");
        bubbleLayout->addWidget(textLabel);
    }

    bubble->setMaximumWidth(420);

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

// Design Styles (Bina Kisi Change Ke Same)
void ChatWindow::applyStyles()
{
    setStyleSheet(R"(
        QWidget#sidebar { background-color: #ede6f0; border-right: 1px solid #e0d5ec; }
        QWidget#logoArea { border-bottom: 1px solid #e0d5ec; background-color: #ede6f0; }
        QLabel#logo { font-size: 20px; font-weight: 700; color: #3d3450; }
        QLabel#serverTag { font-size: 10px; color: #86efac; }
        QLabel#sectionLabel { font-size: 9px; font-weight: 600; color: #c4b8d4; letter-spacing: 1px; }
        QWidget#myProfile { background-color: #e4daea; border-top: 1px solid #e0d5ec; }
        QLabel#myAvatar { background-color: #ede0f8; color: #8b5cf6; border-radius: 10px; font-weight: 700; font-size: 11px; }
        QLabel#myName { font-size: 12px; font-weight: 600; color: #3d3450; }
        QLabel#myStatus { font-size: 10px; color: #a89bb8; }
        QWidget#mainArea { background-color: #f5f0eb; }
        QWidget#chatHeader { background-color: #ffffff; border-bottom: 1px solid #ede6f0; }
        QLabel#chatAvatar { background-color: #ede0f8; color: #6d28d9; border-radius: 11px; font-weight: 700; font-size: 12px; }
        QLabel#chatTitle { font-size: 14px; font-weight: 600; color: #3d3450; }
        QLabel#chatSub { font-size: 10px; color: #a89bb8; }
        QLabel#lanBadge { background-color: #f5f0fb; border: 1px solid #e8dff0; border-radius: 20px; padding: 3px 12px; font-size: 10px; color: #a89bb8; }
        QPushButton#leaveBtn { background-color: transparent; border: 1px solid #e8dff0; border-radius: 8px; color: #a89bb8; font-size: 11px; padding: 4px 12px; }
        QPushButton#leaveBtn:hover { border-color: #f87171; color: #f87171; }
        QScrollArea#scrollArea { background-color: #f9f6f2; border: none; }
        QWidget#messagesWidget { background-color: #f9f6f2; }
        QScrollBar:vertical { width: 4px; background: transparent; }
        QScrollBar::handle:vertical { background: #e8dff0; border-radius: 2px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        QWidget#bubbleMe { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #8b5cf6, stop:1 #a78bfa); color: white; border-radius: 14px; }
        QWidget#bubbleThem { background-color: #ffffff; color: #3d3450; border: 1px solid #ede6f0; border-radius: 14px; }
        QLabel#senderName { font-size: 9px; color: #c4b8d4; padding-left: 3px; }
        QLabel#msgTime { font-size: 9px; color: #d4cae0; padding-left: 3px; }
        QLabel#systemMsg { font-size: 10px; color: #c4b8d4; padding: 4px 0; }
        QLabel#typingLabel { font-size: 14px; color: #c4b8d4; font-style: italic; background-color: #f9f6f2; }
        QWidget#inputBar { background-color: #ffffff; border-top: 1px solid #ede6f0; }
        QLineEdit#inputField { background-color: #f5f0fb; border: 1px solid #e8dff0; border-radius: 12px; padding: 0 14px; font-size: 13px; color: #3d3450; }
        QLineEdit#inputField:focus { border: 1px solid #8b5cf6; }
        QPushButton#sendBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #8b5cf6, stop:1 #a78bfa); border: none; border-radius: 12px; color: white; font-size: 16px; }
        QPushButton#sendBtn:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #7c3aed, stop:1 #8b5cf6); }
        QLabel#avatarMe { background-color: #ede0f8; color: #8b5cf6; border-radius: 8px; font-weight: 700; font-size: 9px; }
        QLabel#avatarThem { background-color: #dde8f8; color: #4f7ccf; border-radius: 8px; font-weight: 700; font-size: 9px; }
    )");
}