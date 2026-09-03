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
    main.cpp
    AccountDialog.cpp
    AccountDialog.h
    AccountDialog.ui
    Model.cpp
    Model.h
    Controller.cpp
    Controller.h
    Logger.cpp
    Logger.h
    ByteWriter.cpp
    ByteWriter.h
    ByteReader.cpp
    ByteReader.h
    PacketBuilder.cpp
    PacketBuilder.h
    PacketDeserializer.cpp
    PacketDeserializer.h
    PacketData.h

)

add_executable(client ${PROJECT_SOURCES} )
target_link_libraries(client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    fmt::fmt
    spdlog::spdlog
)

set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
