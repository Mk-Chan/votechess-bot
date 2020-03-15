#include "rng_service.h"

namespace votechess {

std::unique_ptr<rng_service> rng_service::instance_ = nullptr;

rng_service::rng_service() : rng_(std::random_device{}()) {
}

rng_service* rng_service::singleton() {
    if (instance_ == nullptr) {
        instance_ = std::make_unique<rng_service>();
    }
    return instance_.get();
}

std::uint32_t rng_service::rand_uint32(std::uint32_t low, std::uint32_t high) {
    std::uniform_int_distribution<std::uint32_t> dist(low, high);
    return dist(rng_);
}

}  // namespace votechess
