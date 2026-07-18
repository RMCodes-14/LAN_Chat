# LAN_Chat

# LAN_Chat

A desktop LAN chat application built with **C++** and **Qt6**. One machine hosts a server, other machines connect as clients over TCP — no internet connection required, just a shared local network.

## Features

- **Host or join** — launch a window to either start a server or connect to one as a client
- **Real-time group chat** over TCP sockets, with a JSON-based message protocol
- **File attachments** — send files to everyone in the chat
- **Message history** — new clients receive prior messages on joining
- **Delete messages** — right-click a message for a context menu with delete support
- **User list** — sidebar showing who's currently connected
- **Server admin controls** — kick users and send global announcements from the server window
- **Notification sounds** on new messages
- **Cross-platform** — builds on Windows, macOS, and Linux via Qt6 and CMake

## Tech Stack

- C++17
- Qt6 (Core, Gui, Widgets, Network, Multimedia)
- CMake (3.16+)
- TCP sockets (`QTcpServer` / `QTcpSocket`) with a lightweight JSON message protocol

## Project Structure

```
LanChat/
├── main.cpp                # App entry point — launches connection window
├── connectionwindow.*       # Choose to host or join, enter host/port/username
├── server.*                 # ChatServer: accepts connections, broadcasts messages, tracks history
├── client.*                 # ChatClient: connects to server, sends/receives messages
├── chatwindow.*              # Main chat UI — messages, file attachments, user list
├── serverwindow.*             # Server-side admin UI — logs, connected users, kick/announce
├── protocol.h                  # Message struct + JSON (de)serialization
├── sounds/                     # Notification sound assets
└── CMakeLists.txt
```

## Getting Started

### Prerequisites

- Qt6 (Core, Gui, Widgets, Network, Multimedia modules)
- CMake 3.16+
- A C++17-capable compiler

### Build

```bash
git clone https://github.com/RMCodes-14/LAN_Chat.git
cd LAN_Chat/LanChat
mkdir build && cd build
cmake ..
cmake --build .
```

### Run

1. Launch the app on one machine and choose **Start Server** (default port `5050`).
2. Launch the app on other machines on the same LAN and choose **Join**, entering the host machine's local IP and port.
3. Chat, share files, and see who's online in the sidebar.

## License

No license specified yet.
