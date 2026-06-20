cmake_minimum_required(VERSION 3.16)
project(LanChat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network )



set(PROJECT_SOURCES
        main.cpp
)

qt_add_executable(LanChat
    MANUAL_FINALIZATION
    main.cpp
    protocol.h
    server.h
    server.cpp
    client.h
    client.cpp
    connectionwindow.h
    connectionwindow.cpp
    chatwindow.h
    chatwindow.cpp

)

target_link_libraries(LanChat PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

set_target_properties(LanChat PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS LanChat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(LanChat)
