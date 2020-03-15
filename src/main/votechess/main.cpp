#include <iostream>
#include <thread>

#include "boost/asio.hpp"

#include "spdlog/spdlog.h"

#include "config_service.h"
#include "irc_client.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: ./votechess <path-to-config>\n";
        return 1;
    }

    votechess::config_service* config = votechess::config_service::singleton();
    config->parse(argv[1]);

    std::ios_base::sync_with_stdio(false);
    std::cout.setf(std::ios::unitbuf);
    spdlog::set_level(spdlog::level::level_enum::debug);

    boost::asio::io_context irc_io_context;
    tcp::resolver resolver(irc_io_context);
    auto endpoints = resolver.resolve(config->irc_host(), "6667");
    auto irc_client = std::make_shared<votechess::irc_client>(irc_io_context);
    irc_client->connect(endpoints);

    std::thread irc_client_thread{[&irc_io_context]() { irc_io_context.run(); }};

    constexpr static int maxlength = 2048;
    char line_data[maxlength];
    while (std::cin.getline(line_data, maxlength)) {
        std::string line{line_data};
        if (line == "join") {
            irc_client->writeln("JOIN " + config->irc_channel());
        } else if (line == "quit") {
            break;
        } else {
            spdlog::error("Unknown command: {}", line);
        }
    }

    irc_client->close();
    irc_client_thread.join();
    return 0;
}
