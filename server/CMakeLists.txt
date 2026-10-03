cmake_minimum_required(VERSION 3.16)
project(ServerChat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(SQLite3 REQUIRED)

set(SERVER_SOURCES
        app/main.cpp
        app/ShutdownSignal.cpp
        app/ShutdownSignal.h

        config/ServerConfig.h

        net/ListeningSocket.cpp
        net/ListeningSocket.h
        net/EventPoller.cpp
        net/EventPoller.h
        net/PortSelector.cpp
        net/PortSelector.h
        net/ClientSession.cpp
        net/ClientSession.h
        net/SessionManager.cpp
        net/SessionManager.h

        protocol/ByteReader.cpp
        protocol/ByteReader.h
        protocol/ByteWriter.cpp
        protocol/ByteWriter.h
        protocol/PacketBuilder.cpp
        protocol/PacketBuilder.h
        protocol/PacketParser.cpp
        protocol/PacketParser.h
        protocol/PacketAssembler.cpp
        protocol/PacketAssembler.h
        protocol/PacketSender.cpp
        protocol/PacketSender.h
        protocol/PacketData.h
        storage/Database.cpp
        storage/Database.h

        utils/Logger.cpp
        utils/Logger.h
)

add_executable(server ${SERVER_SOURCES})

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(server PRIVATE
    fmt::fmt
    spdlog::spdlog
    SQLite::SQLite3
)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
