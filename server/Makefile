# Makefile для сервера мессенджера.

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -O2
LDFLAGS  :=
LDLIBS   := -lsqlite3 -lspdlog -lfmt -lpthread

TARGET    := server
BUILD_DIR := build

INCLUDES := \
	-Iapp -Inet -Iprotocol -Iservices -Istorage -Iutils -Iconfig

SRCS := \
	app/main.cpp \
	app/PortSelector.cpp \
	app/ShutdownSignal.cpp \
	net/ListeningSocket.cpp \
	net/EventPoller.cpp \
	net/ClientSession.cpp \
	net/PacketAssembler.cpp \
	net/PacketSender.cpp \
	net/SessionManager.cpp \
	net/SessionRegistry.cpp \
	protocol/PacketBuilder.cpp \
	protocol/PacketParser.cpp \
	protocol/PacketDispatcher.cpp \
	services/AuthService.cpp \
	services/ChatService.cpp \
	services/MessageService.cpp \
	storage/Database.cpp \
	storage/UserRepository.cpp \
	storage/ChatRepository.cpp \
	storage/MessageRepository.cpp \
	utils/Logger.cpp \
	utils/Validator.cpp \
	utils/ByteReader.cpp \
	utils/ByteWriter.cpp

HEADERS := \
	app/PortSelector.h \
	app/ShutdownSignal.h \
	net/ListeningSocket.h \
	net/EventPoller.h \
	net/ClientSession.h \
	net/PacketAssembler.h \
	net/PacketSender.h \
	net/SessionManager.h \
	net/SessionRegistry.h \
	protocol/PacketData.h \
	protocol/PacketBuilder.h \
	protocol/PacketParser.h \
	protocol/PacketDispatcher.h \
	services/AuthService.h \
	services/ChatService.h \
	services/MessageService.h \
	storage/Database.h \
	storage/UserRepository.h \
	storage/ChatRepository.h \
	storage/MessageRepository.h \
	utils/Logger.h \
	utils/Validator.h \
	utils/ByteReader.h \
	utils/ByteWriter.h \
	config/ServerConfig.h

OBJS := $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

.PHONY: all clean rebuild run

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

run: all
	./$(BUILD_DIR)/$(TARGET)
