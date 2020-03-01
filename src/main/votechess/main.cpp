#include <iostream>
#include <thread>

#include "boost/asio.hpp"

#include "spdlog/spdlog.h"

#include "libchess/Position.h"
#include "libchess/UCIService.h"

#include "irc_client.h"
#include "vote_service.h"

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cout.setf(std::ios::unitbuf);
    spdlog::set_level(spdlog::level::level_enum::debug);

    boost::asio::io_context io_context;

    boost::asio::signal_set signal_set{io_context, SIGTERM, SIGINT};
    signal_set.async_wait([&](auto, auto) { io_context.stop(); });

    libchess::Position pos{libchess::constants::STARTPOS_FEN};
    std::condition_variable cv{};

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve("chat.freenode.net", "6667");
    votechess::irc_client irc_client{io_context, endpoints};

    auto position_handler =
        [&pos, &irc_client](const libchess::UCIPositionParameters& position_parameters) {
            irc_client.privmsg(fmt::format("New Position: {}", pos.fen()));
            pos = libchess::Position{position_parameters.fen()};
            if (!position_parameters.move_list()) {
                return;
            }
            for (auto& move_str : position_parameters.move_list()->move_list()) {
                pos.make_move(*libchess::Move::from(move_str));
            }
        };
    auto go_handler = [&pos, &irc_client, &cv](const libchess::UCIGoParameters& go_parameters) {
        votechess::VoteService* vote_service = votechess::VoteService::get_instance();
        auto err = vote_service->new_vote(pos);
        if (err == votechess::VoteService::ErrorType::VOTING_ALREADY_ACTIVE) {
            spdlog::error("Received go command when voting already active! Shutting down...");
            irc_client.close();
            std::exit(1);
        }

        int movetime = *go_parameters.movetime() / 1000;  // FIXME: Use better timing mechanisms
        irc_client.privmsg(
            fmt::format("Voting has begun! Counting votes in {} seconds!", movetime));
        std::unique_lock lock{votechess::VoteService::get_stop_voting_signal_mutex()};

        // FIXME: Wrap in a predicate loop to prevent spurious wakeup issues
        cv.wait_for(lock, boost::asio::chrono::seconds{movetime});

        libchess::Move best_move = vote_service->coalesce();
        std::string best_move_str = best_move.to_str();
        irc_client.privmsg(fmt::format("Winning move: {}", best_move_str));
        libchess::UCIService::bestmove(best_move.to_str());
    };
    auto stop_handler = [&cv]() {
        votechess::VoteService::get_stop_voting_signal_mutex().unlock();
        cv.notify_all();
    };

    libchess::UCIService uci_service{"IRC VoteChess Bridge", "Manik Charan"};
    uci_service.register_position_handler(position_handler);
    uci_service.register_go_handler(go_handler);
    uci_service.register_stop_handler(stop_handler);

    std::thread irc_client_thread{[&io_context]() { io_context.run(); }};

    constexpr static int maxlength = 2048;
    char line_data[maxlength];
    while (std::cin.getline(line_data, maxlength)) {
        std::string line{line_data};
        if (line == "uci") {
            uci_service.run();
        } else if (line == "quit") {
            break;
        } else {
            spdlog::error("Supported Protocols: uci");
        }
    }

    irc_client.close();
    irc_client_thread.join();
    return 0;
}
