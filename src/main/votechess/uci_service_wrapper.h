#ifndef VOTECHESS_UCI_SERVICE_PROVIDER_H
#define VOTECHESS_UCI_SERVICE_PROVIDER_H

#include "boost/asio.hpp"

#include "libchess/UCIService.h"

#include "irc_client.h"
#include "vote_service.h"

namespace votechess {

class uci_service_wrapper {
   public:
    enum class error_code
    {
        UNINITIALIZED,
    };

    explicit uci_service_wrapper(vote_service& vote_service, irc_client& irc_client);

    boost::asio::ip::tcp::iostream& io_stream();

    std::optional<error_code> run();

   protected:
    void position_handler(const libchess::UCIPositionParameters& position_parameters);
    void go_handler(const libchess::UCIGoParameters& go_parameters);
    void stop_handler();

   private:
    std::mutex stop_voting_signal_mutex_;
    boost::asio::ip::tcp::iostream io_;
    std::condition_variable cv_;
    std::unique_ptr<libchess::UCIService> uci_service_;
    vote_service& vote_service_;
    irc_client& irc_client_;
};

}  // namespace votechess

#endif  // VOTECHESS_UCI_SERVICE_PROVIDER_H
