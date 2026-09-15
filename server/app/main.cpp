#include "Logger.h"
#include "ServerConfig.h"
#include "PortSelector.h"
#include "ListeningSocket.h"
#include "ShutdownSignal.h"
#include "Database.h"
#include "UserRepository.h"
#include "ChatRepository.h"
#include "MessageRepository.h"
#include "SessionRegistry.h"
#include "AuthService.h"
#include "ChatService.h"
#include "MessageService.h"
#include "PacketDispatcher.h"
#include "EventPoller.h"
#include "SessionManager.h"
#include "ClientSession.h"
#include "ThreadPool.h"
#include "ResultQueue.h"
#include <chrono>
#include <thread>
#include <exception>
#include <cstdlib>
#include <iostream>

namespace {

void terminateHandler()
{
    std::cerr << "[fatal] Unhandled exception, terminating" << std::endl;
    std::abort();
}

} // namespace

int main()
{
    std::set_terminate(terminateHandler);

    try
    {
        ShutdownSignal::setup();
        LOG_INFO("Server starting up");

        Database db("chat.db");
        UserRepository userRepository(db.getHandle());
        ChatRepository chatRepository(db.getHandle());
        MessageRepository messageRepository(db.getHandle());

        SessionRegistry sessionRegistry;
        AuthService authService(&userRepository, &sessionRegistry);
        ChatService chatService(&userRepository, &chatRepository);
        MessageService messageService(&messageRepository, &chatRepository, &sessionRegistry);

        PacketDispatcher dispatcher(&authService, &chatService, &messageService);

        ServerConfig config;
        const int chosenPort = promptForPort(config.getPort());
        if (chosenPort == -1)
        {
            LOG_INFO("Shutdown requested during port selection");
            return 0;
        }
        config.setPort(chosenPort);

        ListeningSocket listener;
        listener.startListening(config);

        ResultQueue resultQueue;
        ThreadPool threadPool(std::thread::hardware_concurrency());

        EventPoller eventPoller;
        SessionManager sessionManager(&dispatcher, &sessionRegistry,
                                      &eventPoller, &threadPool, &resultQueue);

        eventPoller.setNewConnectionCallback(
            [&sessionManager](int fd){ sessionManager.onNewConnection(fd); });
        eventPoller.setWriteEventCallback(
            [&sessionManager](int fd){ sessionManager.onWrite(fd); });
        eventPoller.setErrorEventCallback(
            [&sessionManager](int fd, uint32_t ev){ sessionManager.onError(fd, ev); });

        eventPoller.setReadEventCallback([&](int fd) {
            if (fd == resultQueue.eventFd())
            {
                auto commands = resultQueue.drain();
                for (auto& cmd : commands)
                {
                    switch (cmd.type)
                    {
                    case CommandType::SendRaw:
                    {
                        ClientSession* s = sessionManager.getSession(cmd.fd);
                        if (s && !s->isClosed())
                            s->sendRawDirect(cmd.data);
                        break;
                    }
                    case CommandType::Close:
                        sessionManager.requestCloseClient(cmd.fd);
                        break;
                    case CommandType::TaskDone:
                        sessionManager.onTaskDone(cmd.fd);
                        break;
                    }
                }
            }
            else
            {
                sessionManager.onRead(fd);
            }
        });

        eventPoller.addFileDescriptor(resultQueue.eventFd(), EPOLLIN);
        eventPoller.startEventLoop(listener.fileDescriptor());

        // Graceful shutdown: дать воркерам завершить работу.
        LOG_INFO("Draining worker tasks...");
        while (threadPool.hasPendingTasks())
        {
            auto commands = resultQueue.drain();
            for (auto& cmd : commands)
            {
                switch (cmd.type)
                {
                case CommandType::SendRaw:
                {
                    ClientSession* s = sessionManager.getSession(cmd.fd);
                    if (s && !s->isClosed())
                        s->sendRawDirect(cmd.data);
                    break;
                }
                case CommandType::Close:
                    sessionManager.requestCloseClient(cmd.fd);
                    break;
                case CommandType::TaskDone:
                    sessionManager.onTaskDone(cmd.fd);
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        threadPool.stop();

        LOG_INFO("Server shutdown");
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[fatal] " << e.what() << std::endl;
        LOG_CRITICAL("Fatal exception: {}", e.what());
        return 1;
    }
    catch (...)
    {
        std::cerr << "[fatal] Unknown exception" << std::endl;
        LOG_CRITICAL("Fatal unknown exception");
        return 1;
    }
}
