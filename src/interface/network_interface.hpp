#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace lwip {

class NetworkInterface {
public:
    virtual ~NetworkInterface() = default;
    virtual bool init() = 0;
    virtual bool send_frame(const std::vector<uint8_t>& data) = 0;
    virtual bool receive_frame(std::vector<uint8_t>& data) = 0;
    virtual uint64_t get_mac_address() const = 0;
    virtual void set_mac_address([[maybe_unused]] uint64_t mac) = 0;
    virtual bool start() = 0;  // 改名为start
    virtual void shutdown() = 0;  // 改名为shutdown
};

class TunInterface : public NetworkInterface {
public:
    ~TunInterface() override;
    bool init() override;
    bool send_frame(const std::vector<uint8_t>& data) override;
    bool receive_frame(std::vector<uint8_t>& data) override;
    uint64_t get_mac_address() const override;
    void set_mac_address(uint64_t mac) override;
    bool start() override { return true; }  // 改名为start
    void shutdown() override;  // 改名为shutdown

private:
    int fd_{-1};
};

class TapInterface : public NetworkInterface {
public:
    ~TapInterface() override;
    bool init() override;
    bool send_frame(const std::vector<uint8_t>& data) override;
    bool receive_frame(std::vector<uint8_t>& data) override;
    uint64_t get_mac_address() const override;
    void set_mac_address(uint64_t mac) override;

private:
    bool configure_interface(const char* name);
    int fd_{-1};
};

} // namespace lwip
