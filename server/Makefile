CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -O2
LDFLAGS  :=
LDLIBS   := -lsqlite3 -lspdlog -lfmt -lpthread

TARGET   := server
BUILD_DIR := build

SRCS := \
	main.cpp \
	CheckPort.cpp \
	ClientSession.cpp \
	Database.cpp \
	Epoller.cpp \
	Logger.cpp \
	PacketAssembler.cpp \
	PacketBuilder.cpp \
	PacketDeserializer.cpp \
	ByteReader.cpp \
	ByteWriter.cpp \
	ResponseSender.cpp \
	ServerStartStop.cpp \
	SessionManager.cpp \
	SessionRegistry.cpp \
	SigintHandler.cpp \
	Validator.cpp \
	UserRepository.cpp \
	ChatRepository.cpp \
	MessageRepository.cpp \
	AuthService.cpp \
	ChatService.cpp \
	MessageService.cpp \
	PacketDispatcher.cpp


OBJS := $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

HEADERS := \
	CheckPort.h \
	ClientSession.h \
	Database.h \
	Epoller.h \
	Logger.h \
	PacketAssembler.h \
	PacketBuilder.h \
	PacketData.h \
	PacketDeserializer.h \
	ByteReader.h \
	ByteWriter.h \
	ResponseSender.h \
	ServerConfig.h \
	ServerStartStop.h \
	SessionManager.h \
	SessionRegistry.h \
	SigintHandler.h \
	Validator.h \
	UserRepository.h \
	ChatRepository.h \
	MessageRepository.h \
	AuthService.h \
	ChatService.h \
	MessageService.h \
	PacketDispatcher.h

.PHONY: all clean rebuild run

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

run: all
	./$(BUILD_DIR)/$(TARGET)
