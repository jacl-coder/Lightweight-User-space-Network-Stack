#pragma once
#include "network_interface.hpp"
#include "../core/event_loop.hpp"

namespace lwip {

class NonBlockingInterface : public NetworkInterface {
public:
    bool init() override;
    bool send_frame(const std::vector<uint8_t>& data) override;
    bool receive_frame(std::vector<uint8_t>& data) override;
    uint64_t get_mac_address() const override;
    void set_mac_address([[maybe_unused]] uint64_t mac) override;
    void process_io();

protected:
    virtual void on_frame_received([[maybe_unused]] const std::vector<uint8_t>& frame);

private:
    int fd_{-1};
    EventLoop event_loop_;
};

} // namespace lwip
