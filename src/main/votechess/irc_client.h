#ifndef VOTECHESS_IRC_CLIENT_H
#define VOTECHESS_IRC_CLIENT_H

#include "boost/asio.hpp"

#include "vote_service.h"

using boost::asio::ip::tcp;

namespace votechess {

class irc_client : public std::enable_shared_from_this<irc_client> {
   public:
    explicit irc_client(boost::asio::io_context& io_context);
    void connect(const tcp::resolver::results_type& endpoints);
    void writeln(const std::string& message);
    void privmsg(const std::string& message);
    void close();

   protected:
    void read();
    void login();
    void handle_message(const std::string& message);

   private:
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    boost::asio::streambuf input_buffer_;
    std::unique_ptr<vote_service> vote_service_;
};

}  // namespace votechess

#endif  // VOTECHESS_IRC_CLIENT_H
