#ifndef VOTECHESS_VOTE_SERVICE_H
#define VOTECHESS_VOTE_SERVICE_H

#include <optional>
#include <set>
#include <string>

#include "spdlog/spdlog.h"

#include "libchess/Move.h"
#include "libchess/Position.h"

namespace votechess {

class vote_service {
   public:
    enum class error_code
    {
        ALREADY_VOTED,
        ILLEGAL_MOVE,
        VOTING_ALREADY_ACTIVE,
        VOTING_INACTIVE,
    };

    explicit vote_service();

    libchess::Color side_to_move();

    void position(const libchess::Position& pos);
    const libchess::Position& position() const;

    std::optional<error_code> new_vote();
    std::optional<error_code> cast_vote(const std::string& voter, libchess::Move move);
    libchess::Move coalesce();

   private:
    libchess::Position pos_;
    bool is_active_;
    std::mutex mutex_;
    std::vector<int> move_vote_hist_;
    std::set<std::string> voter_set_;
    libchess::MoveList move_list_;
};

}  // namespace votechess

#endif  // VOTECHESS_VOTE_SERVICE_H
