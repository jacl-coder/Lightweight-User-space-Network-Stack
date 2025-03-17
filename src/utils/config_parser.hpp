#pragma once
#include <string>
#include <map>
#include "../core/config.hpp"

namespace lwip {

class ConfigParser {
public:
    bool parse_file(const std::string& filename, StackConfig& config);
    bool parse_string(const std::string& content, StackConfig& config);

private:
    std::map<std::string, std::string> parse_key_values(const std::string& content);
    bool apply_config(const std::map<std::string, std::string>& values, StackConfig& config);
};

} // namespace lwip
