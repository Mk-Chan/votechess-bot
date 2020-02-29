#include <iostream>
#include <thread>

#include "boost/asio.hpp"

#include "spdlog/spdlog.h"

#include "irc_client.h"

int main() {
    spdlog::set_level(spdlog::level::level_enum::debug);
    boost::asio::io_context io_context;

    boost::asio::signal_set signal_set{io_context, SIGTERM, SIGINT};
    signal_set.async_wait([&](auto, auto) { io_context.stop(); });

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve("chat.freenode.net", "6667");
    votechess::irc_client irc_client{io_context, endpoints};

    std::thread irc_client_thread{[&io_context]() { io_context.run(); }};

    constexpr static int maxlength = 2048;
    char message[maxlength];
    while (std::cin.getline(message, maxlength)) {
        irc_client.writeln(message);
    }

    irc_client.close();
    irc_client_thread.join();
    return 0;
}
