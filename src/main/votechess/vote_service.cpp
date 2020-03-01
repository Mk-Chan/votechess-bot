#include "vote_service.h"

namespace votechess {

VoteService* VoteService::instance_ = nullptr;
std::mutex VoteService::stop_voting_signal_mutex_ = {};

VoteService* VoteService::get_instance() {
    if (instance_ == nullptr) {
        instance_ = new VoteService{};
    }
    return instance_;
}

std::mutex& VoteService::get_stop_voting_signal_mutex() {
    return stop_voting_signal_mutex_;
}

std::optional<VoteService::ErrorType> VoteService::new_vote(const libchess::Position& pos) {
    if (is_active_) {
        return ErrorType::VOTING_ALREADY_ACTIVE;
    }

    std::lock_guard lock{mutex_};

    if (is_active_) {
        return ErrorType::VOTING_ALREADY_ACTIVE;
    }

    is_active_ = true;
    voter_set_.clear();
    move_list_ = pos.legal_move_list();
    move_vote_hist_.clear();
    move_vote_hist_.reserve(move_list_.size());
    move_vote_hist_.resize(move_list_.size(), 0);
    return std::nullopt;
}

std::optional<VoteService::ErrorType> VoteService::cast_vote(const std::string& voter,
                                                             libchess::Move move) {
    std::lock_guard lock{mutex_};

    if (!is_active_) {
        return ErrorType::VOTING_INACTIVE;
    }

    if (voter_set_.find(voter) != voter_set_.end()) {
        return ErrorType::ALREADY_VOTED;
    }

    auto move_ptr = std::find(move_list_.begin(), move_list_.end(), move);
    if (move_ptr == move_list_.end()) {
        return ErrorType::ILLEGAL_MOVE;
    }

    int move_index = move_ptr - move_list_.begin();
    ++move_vote_hist_[move_index];
    voter_set_.insert(voter);
    return std::nullopt;
}

libchess::Move VoteService::coalesce() {
    // FIXME: Collect a list of max-voted moves and pick randomly from within.
    //        This will pick random move when no votes are cast as well.
    std::lock_guard lock{mutex_};

    is_active_ = false;
    auto max_element_ptr = std::max_element(move_vote_hist_.begin(), move_vote_hist_.end());
    if (max_element_ptr == move_vote_hist_.end()) {
        return move_list_.values().at(0);
    }

    std::size_t max_element_index = max_element_ptr - move_vote_hist_.begin();
    return move_list_.values().at(max_element_index);
}

VoteService::VoteService()
    : is_active_(false), mutex_(), move_vote_hist_(), voter_set_(), move_list_() {
}

}  // namespace votechess
