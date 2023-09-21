#pragma once
#include <cstdint>

namespace NextFlix
{
    class Server
    {
    public:
        Server() = delete;
        Server(uint16_t port);
        ~Server();

    private:
        uint16_t port;

        void advertiseServer();
    };
}