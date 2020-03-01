#ifndef VOTECHESS_VOTE_SERVICE_H
#define VOTECHESS_VOTE_SERVICE_H

#include <optional>
#include <set>
#include <string>

#include "spdlog/spdlog.h"

#include "libchess/Move.h"
#include "libchess/Position.h"

#include "irc_client.h"

namespace votechess {

class VoteService {
   public:
    enum class ErrorType
    {
        ALREADY_VOTED,
        ILLEGAL_MOVE,
        VOTING_ALREADY_ACTIVE,
        VOTING_INACTIVE,
    };

    [[nodiscard]] static VoteService* get_instance();
    [[nodiscard]] static std::mutex& get_stop_voting_signal_mutex();

    std::optional<ErrorType> new_vote(const libchess::Position& pos);
    std::optional<ErrorType> cast_vote(const std::string& voter, libchess::Move move);
    libchess::Move coalesce();

   private:
    explicit VoteService();

    static VoteService* instance_;
    static std::mutex stop_voting_signal_mutex_;

    bool is_active_;
    std::mutex mutex_;
    std::vector<int> move_vote_hist_;
    std::set<std::string> voter_set_;
    libchess::MoveList move_list_;
};

}  // namespace votechess

#endif  // VOTECHESS_VOTE_SERVICE_H
