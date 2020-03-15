#include "irc_client.h"

#include "boost/algorithm/string.hpp"

#include "spdlog/spdlog.h"

#include "config_service.h"
#include "uci_service_wrapper.h"

using boost::asio::ip::tcp;

namespace votechess {

irc_client::irc_client(boost::asio::io_context& io_context)
    : io_context_(io_context),
      socket_(io_context),
      input_buffer_(),
      vote_service_(new vote_service()) {
}

void irc_client::close() {
    boost::asio::post(io_context_, [this]() { socket_.close(); });
}

void irc_client::writeln(const std::string& message) {
    std::string irc_message{message};
    irc_message.append("\n");
    boost::asio::post(io_context_, [this, irc_message]() {
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(irc_message, irc_message.size()),
            [this, irc_message](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    spdlog::warn("Error while writing: {}", ec.message());
                    socket_.close();
                    return;
                }

                spdlog::info("SEND:{}", irc_message.substr(0, length - 1));
            });
    });
}

void irc_client::privmsg(const std::string& message) {
    config_service* config = config_service::singleton();
    std::string privmsg_message = "PRIVMSG " + config->irc_channel() + " :" + message + "\n";
    boost::asio::post(io_context_, [this, privmsg_message]() {
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(privmsg_message, privmsg_message.size()),
            [this, privmsg_message](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    spdlog::warn("Error while writing: {}", ec.message());
                    socket_.close();
                    return;
                }

                spdlog::info("SEND:{}", privmsg_message.substr(0, length - 1));
            });
    });
}

void irc_client::connect(const tcp::resolver::results_type& endpoints) {
    boost::asio::async_connect(
        socket_, endpoints, [this](boost::system::error_code ec, const tcp::endpoint& endpoint) {
            if (ec) {
                spdlog::warn("Error while connecting: {}", ec.message());
                return;
            }

            spdlog::debug("Connected to {}:{}", endpoint.address().to_string(), endpoint.port());
            login();
            read();
        });
}

void irc_client::login() {
    config_service* config = config_service::singleton();
    std::string nick = config->irc_nick();
    std::string host = config->irc_host();
    writeln(fmt::format("USER {} {} {} :{} {}", nick, host, nick, nick, nick));
    writeln(fmt::format("NICK {}", nick));
}

void irc_client::read() {
    boost::asio::async_read_until(
        socket_,
        input_buffer_,
        "\r\n",
        [this, irc = shared_from_this()](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec) {
                spdlog::warn("Error while reading: {}", ec.message());
                socket_.close();
                return;
            }

            std::istream input_stream{&input_buffer_};
            std::string message;
            std::getline(input_stream, message);

            auto space_pos = message.find(' ');
            if (message.substr(0, space_pos) == "PING") {
                std::string pong = "PONG ";
                pong.append(message.substr(space_pos + 1));
                writeln(pong);
            }

            boost::trim_right_if(message, boost::is_any_of("\n"));
            handle_message(message);
            irc->read();
        });
}

bool is_valid_move_char(char c) {
    return (c >= 'a' && c <= 'h') || (c >= '0' && c <= '8') || (c == 'n') || (c == 'b') ||
           (c == 'r') || (c == 'q');
}

void irc_client::handle_message(const std::string& message) {
    spdlog::info("RECV:{}", message);

    auto delim_pos = message.find('!');
    if (delim_pos == std::string::npos) {
        return;
    }

    std::string_view nick = std::string_view{message.c_str() + 1, delim_pos - 1};

    delim_pos = message.find("PRIVMSG", delim_pos);
    if (delim_pos == std::string::npos) {
        return;
    }

    delim_pos = message.find(':', delim_pos);
    std::string_view privmsg_message{message.c_str() + delim_pos + 1};

    if (privmsg_message.rfind(".start", 0) != std::string::npos) {
        auto session = [&]() {
            while (true) {
                spdlog::info("Waiting for lichess-bot...");
                tcp::acceptor conn_acceptor{io_context_, tcp::endpoint{tcp::v4(), 12345}};
                uci_service_wrapper uci_service{*vote_service_, *this};
                conn_acceptor.accept(*uci_service.io_stream().rdbuf());

                privmsg("Waiting for a game...");
                auto err = uci_service.run();
                if (err == uci_service_wrapper::error_code::UNINITIALIZED) {
                    spdlog::error("Tried to run UCIService while uninitialized!");
                }
                privmsg("Game ended!");
            }
        };
        std::thread{session}.detach();
    } else if (privmsg_message.rfind(".vote", 0) != std::string::npos) {
        std::string_view move_view = privmsg_message.substr(6);
        int move_size = move_view.size();
        if (move_size < 4) {
            privmsg(fmt::format("{}: Unable to parse move {}", nick, move_view));
            return;
        }

        std::string move_str = std::string{move_view.substr(0, 5)};
        boost::trim_right(move_str);
        for (char c : move_str) {
            if (!is_valid_move_char(c)) {
                privmsg(fmt::format("{}: Unable to parse move {}", nick, move_str));
                return;
            }
        }

        std::optional<libchess::Move> move = libchess::Move::from(move_str);
        if (!move) {
            privmsg(fmt::format("{}: Unable to parse move {}", nick, move_str));
            return;
        }

        auto err = vote_service_->cast_vote(std::string{nick}, *move);
        if (err == vote_service::error_code::VOTING_INACTIVE) {
            privmsg("Voting is not active right now!");
        } else if (err == vote_service::error_code::ALREADY_VOTED) {
            privmsg(fmt::format("{} has already voted!", nick));
        } else if (err == vote_service::error_code::ILLEGAL_MOVE) {
            privmsg(fmt::format("{} is an illegal move!", move->to_str()));
        }
    }
}

}  // namespace votechess
