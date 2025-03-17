#include "config_parser.hpp"
#include "logger.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace lwip {

bool ConfigParser::parse_file(const std::string& filename, StackConfig& config) {
    std::ifstream file(filename);
    if (!file) {
        LOG_ERROR("Failed to open config file: " + filename);
        return false;
    }

    std::map<std::string, std::string> values;
    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') continue;

        // 解析键值对
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // 去除首尾空格
        while (!key.empty() && std::isspace(key.back())) key.pop_back();
        while (!key.empty() && std::isspace(key.front())) key.erase(0, 1);
        while (!value.empty() && std::isspace(value.back())) value.pop_back();
        while (!value.empty() && std::isspace(value.front())) value.erase(0, 1);

        if (!key.empty() && !value.empty()) {
            values[key] = value;
        }
    }

    return apply_config(values, config);
}

bool ConfigParser::apply_config(const std::map<std::string, std::string>& values, 
                              StackConfig& config) {
    try {
        for (const auto& [key, value] : values) {
            if (key == "interface") {
                config.interface_name = value;
            } else if (key == "ip_address") {
                config.ip_address = value;
            } else if (key == "netmask") {
                config.netmask = value;
            } else if (key == "mtu") {
                config.mtu = std::stoi(value);
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse config: " + std::string(e.what()));
        return false;
    }
}

} // namespace lwip
