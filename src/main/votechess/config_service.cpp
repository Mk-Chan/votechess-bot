#include "config_service.h"

#include <fstream>

namespace votechess {

std::unique_ptr<config_service> config_service::singleton_ = nullptr;

config_service* config_service::singleton() {
    if (singleton_ == nullptr) {
        singleton_ = std::make_unique<config_service>();
    }
    return singleton_.get();
}

void config_service::parse(const std::string& filepath) {
    std::ifstream config_in{filepath};
    std::string line;
    while (std::getline(config_in, line)) {
        const char* line_data = line.c_str();
        auto delim_pos = line.find('=');
        if (delim_pos == std::string_view::npos) {
            continue;
        }

        std::string_view key{line_data, delim_pos};
        std::string_view value{line_data + delim_pos + 1};

        if (key == "irc.nick") {
            irc_nick_ = std::string{value};
        } else if (key == "irc.channel") {
            irc_channel_ = std::string{value};
        } else if (key == "irc.host") {
            irc_host_ = std::string{value};
        }
    }
}

std::string config_service::irc_nick() const {
    return irc_nick_;
}

std::string config_service::irc_channel() const {
    return irc_channel_;
}

std::string config_service::irc_host() const {
    return irc_host_;
}

}  // namespace votechess
