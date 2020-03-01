#ifndef VOTECHESS_IRC_CLIENT_H
#define VOTECHESS_IRC_CLIENT_H

#include "boost/asio.hpp"

using boost::asio::ip::tcp;

namespace votechess {

class irc_client {
   public:
    irc_client(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints);
    void writeln(const std::string& message);
    void privmsg(const std::string& message);
    void close();

   protected:
    void connect(const tcp::resolver::results_type& endpoints);
    void read();
    void login();
    void handle_message(const std::string& message);

   private:
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    boost::asio::streambuf input_buffer_;
};

}  // namespace votechess

#endif  // VOTECHESS_IRC_CLIENT_H
