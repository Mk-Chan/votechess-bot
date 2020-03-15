#ifndef VOTECHESS_CONFIG_SERVICE_H
#define VOTECHESS_CONFIG_SERVICE_H

#include <memory>

namespace votechess {

class config_service {
   public:
    static config_service* singleton();

    void parse(const std::string& filepath);

    [[nodiscard]] std::string irc_nick() const;
    [[nodiscard]] std::string irc_channel() const;
    [[nodiscard]] std::string irc_host() const;

   private:
    static std::unique_ptr<config_service> singleton_;

    std::string irc_nick_;
    std::string irc_channel_;
    std::string irc_host_;
};

}  // namespace votechess

#endif  // VOTECHESS_CONFIG_SERVICE_H
