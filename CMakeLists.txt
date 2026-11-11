cmake_minimum_required(VERSION 3.16)
project(JajaHub VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets Network NetworkAuth)

qt_add_executable(JajaHub
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/LoginDialog.h
    src/LoginDialog.cpp
    src/GitHubAuth.h
    src/GitHubAuth.cpp
    src/GitHubApi.h
    src/GitHubApi.cpp
    resources/app.qrc
)

target_link_libraries(JajaHub PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::NetworkAuth
)