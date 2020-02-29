#ifndef VOTECHESS_IRC_CLIENT_H
#define VOTECHESS_IRC_CLIENT_H

#include "boost/asio.hpp"

using boost::asio::ip::tcp;

namespace votechess {

class irc_client {
   public:
    irc_client(boost::asio::io_context& io_context, tcp::resolver::results_type& endpoints);
    void writeln(std::string message);
    void close();

   protected:
    void connect(const tcp::resolver::results_type& endpoints);
    void read();
    void login();

   private:
    constexpr static int max_read_size_ = 2048;
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    boost::asio::streambuf input_buffer_;
};

}  // namespace votechess

#endif  // VOTECHESS_IRC_CLIENT_H
