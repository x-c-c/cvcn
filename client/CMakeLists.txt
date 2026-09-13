cmake_minimum_required(VERSION 3.16)
project(client VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

set(PROJECT_SOURCES
    app/main.cpp

    config/AppConfig.h

    ui/AccountDialog.cpp
    ui/AccountDialog.h
    ui/AccountDialog.ui

    ui/ChatWindow.cpp
    ui/ChatWindow.h
    ui/ChatWindow.ui

    net/Connection.cpp
    net/Connection.h

    logic/MainController.cpp
    logic/MainController.h

    protocol/ProtocolClient.cpp
    protocol/ProtocolClient.h
    protocol/PacketData.h
    protocol/PacketBuilder.cpp
    protocol/PacketBuilder.h
    protocol/PacketParser.cpp
    protocol/PacketParser.h

    utils/Logger.cpp
    utils/Logger.h
    utils/Validator.cpp
    utils/Validator.h
    utils/ByteReader.cpp
    utils/ByteReader.h
    utils/ByteWriter.cpp
    utils/ByteWriter.h
)

add_executable(client ${PROJECT_SOURCES})

target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_CURRENT_SOURCE_DIR}/logic
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    fmt::fmt
    spdlog::spdlog
)

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
