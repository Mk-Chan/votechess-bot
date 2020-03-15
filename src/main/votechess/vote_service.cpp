#include "vote_service.h"

#include <queue>

#include "rng_service.h"

namespace votechess {

vote_service::vote_service()
    : pos_(libchess::constants::STARTPOS_FEN),
      is_active_(false),
      mutex_(),
      move_vote_hist_(),
      voter_set_(),
      move_list_() {
}

libchess::Color vote_service::side_to_move() {
    return pos_.side_to_move();
}

void vote_service::position(const libchess::Position& pos) {
    pos_ = pos;
    move_list_ = pos.legal_move_list();
    move_vote_hist_.clear();
    move_vote_hist_.reserve(move_list_.size());
    move_vote_hist_.resize(move_list_.size(), 0);
}

const libchess::Position& vote_service::position() const {
    return pos_;
}

std::optional<vote_service::error_code> vote_service::new_vote() {
    if (is_active_) {
        return error_code::VOTING_ALREADY_ACTIVE;
    }

    std::lock_guard lock{mutex_};

    if (is_active_) {
        return error_code::VOTING_ALREADY_ACTIVE;
    }

    is_active_ = true;
    voter_set_.clear();
    return std::nullopt;
}

std::optional<vote_service::error_code> vote_service::cast_vote(const std::string& voter,
                                                                libchess::Move move) {
    std::lock_guard lock{mutex_};

    if (!is_active_) {
        return error_code::VOTING_INACTIVE;
    }

    if (voter_set_.find(voter) != voter_set_.end()) {
        return error_code::ALREADY_VOTED;
    }

    auto move_ptr = std::find(move_list_.begin(), move_list_.end(), move);
    if (move_ptr == move_list_.end()) {
        return error_code::ILLEGAL_MOVE;
    }

    int move_index = move_ptr - move_list_.begin();
    ++move_vote_hist_[move_index];
    voter_set_.insert(voter);
    return std::nullopt;
}

libchess::Move vote_service::coalesce() {
    std::lock_guard lock{mutex_};
    is_active_ = false;

    struct vote_data {
        int move_index;
        int votes;
    };
    auto vote_data_cmp = [](const vote_data& left, const vote_data& right) {
        return left.votes < right.votes;
    };
    std::priority_queue<vote_data, std::vector<vote_data>, decltype(vote_data_cmp)> vote_heap{
        vote_data_cmp};

    for (int i = 0; i < move_list_.size(); ++i) {
        vote_heap.push(vote_data{i, move_vote_hist_.at(i)});
    }

    std::vector<int> top_voted_move_indices;
    auto top_vote_data = vote_heap.top();
    int max_vote_count_for_move = top_vote_data.votes;
    top_voted_move_indices.push_back(top_vote_data.move_index);
    vote_heap.pop();
    while (!vote_heap.empty()) {
        auto next_vote_data = vote_heap.top();
        if (next_vote_data.votes == max_vote_count_for_move) {
            top_voted_move_indices.push_back(next_vote_data.move_index);
            vote_heap.pop();
        } else {
            break;
        }
    }

    rng_service* rng_service = rng_service::singleton();
    std::uint32_t rand_top_move_index =
        rng_service->rand_uint32(0, top_voted_move_indices.size() - 1);
    return move_list_.values().at(top_voted_move_indices.at(rand_top_move_index));
}

}  // namespace votechess
