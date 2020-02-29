#include "spdlog/spdlog.h"

#include "irc_client.h"

using boost::asio::ip::tcp;

namespace votechess {

irc_client::irc_client(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints)
    : io_context_(io_context), socket_(io_context), input_buffer_() {
    connect(endpoints);
}

void irc_client::close() {
    boost::asio::post(io_context_, [this]() { socket_.close(); });
}

void irc_client::writeln(std::string message) {
    message.append("\n");
    boost::asio::post(io_context_, [this, message]() {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(message, message.size()),
                                 [this, message](boost::system::error_code ec, std::size_t length) {
                                     if (ec) {
                                         spdlog::warn("Error while writing: {}", ec.message());
                                         socket_.close();
                                         return;
                                     }

                                     spdlog::info("SEND:{}", message.substr(0, length - 1));
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
    writeln("USER duderino_ chat.freenode.net duderino_ :duderino_ duderino_");
    writeln("NICK duderino_");
}

void irc_client::read() {
    boost::asio::async_read_until(socket_,
                                  input_buffer_,
                                  "\r\n",
                                  [this](boost::system::error_code ec, std::size_t /*length*/) {
                                      if (ec) {
                                          spdlog::warn("Error while reading: {}", ec.message());
                                          socket_.close();
                                          return;
                                      }

                                      std::istream input_stream{&input_buffer_};
                                      std::string message;
                                      std::getline(input_stream, message);

                                      spdlog::info("RECV:{}", message);

                                      auto space_pos = message.find(' ');
                                      if (message.substr(0, space_pos) == "PING") {
                                          std::string pong = "PONG ";
                                          pong.append(message.substr(space_pos + 1));
                                          writeln(pong);
                                          return;
                                      }

                                      read();
                                  });
}

}  // namespace votechess
