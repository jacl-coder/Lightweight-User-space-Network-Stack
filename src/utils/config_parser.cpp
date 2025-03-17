#include "config_parser.hpp"
#include <fstream>
#include <sstream>

namespace lwip {

bool ConfigParser::parse_file(const std::string& filename, StackConfig& config) {
    std::ifstream file(filename);
    if (!file) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_string(buffer.str(), config);
}

bool ConfigParser::parse_string(const std::string& content, StackConfig& config) {
    auto values = parse_key_values(content);
    return apply_config(values, config);
}

std::map<std::string, std::string> ConfigParser::parse_key_values(const std::string& content) {
    std::map<std::string, std::string> values;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            values[key] = value;
        }
    }
    return values;
}

bool ConfigParser::apply_config(const std::map<std::string, std::string>& values, StackConfig& config) {
    try {
        for (const auto& [key, value] : values) {
            if (key == "mtu") {
                config.mtu = std::stoi(value);
            } else if (key == "tcp_timeout_ms") {
                config.tcp_timeout_ms = std::stoul(value);
            } else if (key == "buffer_pool_size") {
                config.buffer_pool_size = std::stoul(value);
            } else if (key == "max_connections") {
                config.max_connections = std::stoi(value);
            } else if (key == "local_address") {
                config.local_address = InetAddress(value);
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse config: " + std::string(e.what()));
        return false;
    }
}

} // namespace lwip
