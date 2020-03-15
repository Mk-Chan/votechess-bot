#ifndef VOTECHESS_RNG_SERVICE_H
#define VOTECHESS_RNG_SERVICE_H

#include <memory>
#include <random>

namespace votechess {

class rng_service {
   public:
    explicit rng_service();

    [[nodiscard]] static rng_service* singleton();

    [[nodiscard]] std::uint32_t rand_uint32(std::uint32_t low, std::uint32_t high);

   private:
    static std::unique_ptr<rng_service> instance_;

    std::mt19937 rng_;
};

}  // namespace votechess

#endif  // VOTECHESS_RNG_SERVICE_H
