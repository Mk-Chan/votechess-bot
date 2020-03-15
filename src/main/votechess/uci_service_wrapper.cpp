#include <memory>

#include "uci_service_wrapper.h"

#include "spdlog/spdlog.h"

#include "libchess/Position.h"

namespace votechess {

uci_service_wrapper::uci_service_wrapper(vote_service& vote_service, irc_client& irc_client)
    : stop_voting_signal_mutex_(),
      io_(),
      cv_(),
      vote_service_(vote_service),
      irc_client_(irc_client) {
    uci_service_ =
        std::make_unique<libchess::UCIService>("IRC VoteChess Bridge", "Manik Charan", io_, io_);
    auto position_handler_func =
        std::bind(&uci_service_wrapper::position_handler, this, std::placeholders::_1);
    auto go_handler_func = std::bind(&uci_service_wrapper::go_handler, this, std::placeholders::_1);
    auto stop_handler_func = std::bind(&uci_service_wrapper::stop_handler, this);
    uci_service_->register_position_handler(position_handler_func);
    uci_service_->register_go_handler(go_handler_func);
    uci_service_->register_stop_handler(stop_handler_func);
}

boost::asio::ip::tcp::iostream& uci_service_wrapper::io_stream() {
    return io_;
}

std::optional<uci_service_wrapper::error_code> uci_service_wrapper::run() {
    if (uci_service_ == nullptr) {
        return error_code::UNINITIALIZED;
    }
    uci_service_->run();
    return std::nullopt;
}

void uci_service_wrapper::position_handler(
    const libchess::UCIPositionParameters& position_parameters) {
    libchess::Position pos = libchess::Position{position_parameters.fen()};
    if (position_parameters.move_list()) {
        for (auto& move_str : position_parameters.move_list()->move_list()) {
            pos.make_move(*libchess::Move::from(move_str));
        }
    }
    vote_service_.position(pos);
}

void uci_service_wrapper::go_handler(const libchess::UCIGoParameters& go_parameters) {
    auto err = vote_service_.new_vote();
    if (err == vote_service::error_code::VOTING_ALREADY_ACTIVE) {
        spdlog::error("Received go command when voting already active! Shutting down...");
        irc_client_.close();
        std::exit(1);
    }

    int movetime = [&]() {
        if (go_parameters.movetime()) {
            return *go_parameters.movetime();
        }
        return [&]() {
            if (vote_service_.side_to_move() == libchess::constants::WHITE) {
                return *go_parameters.wtime();
            } else {
                return *go_parameters.btime();
            }
        }() / 35;
    }() / 1000;
    irc_client_.privmsg(fmt::format("New Position: {}", vote_service_.position().fen()));
    irc_client_.privmsg(fmt::format("Voting has begun! Counting votes in {} seconds!", movetime));

    std::unique_lock lock{stop_voting_signal_mutex_};
    cv_.wait_for(lock, boost::asio::chrono::seconds{movetime});

    libchess::Move best_move = vote_service_.coalesce();
    std::string best_move_str = best_move.to_str();
    irc_client_.privmsg(fmt::format("Winning move: {}", best_move_str));
    libchess::UCIService::bestmove(best_move.to_str(), std::nullopt, io_stream());
}

void uci_service_wrapper::stop_handler() {
    stop_voting_signal_mutex_.unlock();
    cv_.notify_all();
}

}  // namespace votechess
