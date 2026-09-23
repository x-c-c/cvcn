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

	net/ListeningSocket.cpp
	net/EventPoller.cpp
	net/PortSelector.cpp
	net/ClientSession.cpp
	net/SessionManager.cpp

	protocol/ByteReader.cpp
	protocol/ByteWriter.cpp
	protocol/PacketBuilder.cpp
	protocol/PacketParser.cpp
	protocol/PacketAssembler.cpp
	protocol/PacketSender.cpp

	storage/Database.cpp

	utils/Logger.cpp
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
