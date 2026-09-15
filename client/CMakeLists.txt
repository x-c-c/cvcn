cmake_minimum_required(VERSION 3.16)
project(ClientOnQt VERSION 0.1 LANGUAGES CXX)

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
    ui/AccountDialog.cpp
    ui/AccountDialog.h
    ui/AccountDialog.ui
    app/Model.cpp
    app/Model.h
    app/Controller.cpp
    app/Controller.h
    utils/Logger.cpp
    utils/Logger.h
    protocol/ByteWriter.cpp
    protocol/ByteWriter.h
    protocol/ByteReader.cpp
    protocol/ByteReader.h
    protocol/PacketBuilder.cpp
    protocol/PacketBuilder.h
    protocol/PacketParser.cpp
    protocol/PacketParser.h
    protocol/PacketData.h

)

add_executable(client ${PROJECT_SOURCES} )
target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    fmt::fmt
    spdlog::spdlog
)

set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
