#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>

class ConnectionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWindow(QWidget* parent = nullptr);

signals:


    void startAsServer(const QString& username, quint16 port);
    void startAsClient(const QString& host, quint16 port, const QString& username);

private slots:
    void onServerClicked();
    void onClientClicked();

private:
    void setupUI();
    void applyStyles();


    // Main layout
    QStackedWidget* m_stack;

    // Mode selection page
    QWidget*     m_modePage;
    QPushButton* m_serverBtn;
    QPushButton* m_clientBtn;

    // Server page
    QWidget*     m_serverPage;
    QLineEdit*   m_portInput;
    QPushButton* m_startServerBtn;
    QLineEdit* m_serverUsernameInput;  // add karo private mein

    // Client page
    QWidget*     m_clientPage;
    QLineEdit*   m_hostInput;
    QLineEdit*   m_userInput;
    QLineEdit*   m_portInput2;
    QPushButton* m_connectBtn;
};