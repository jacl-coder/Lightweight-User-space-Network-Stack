#include "non_blocking_interface.hpp"
#include <fcntl.h>
#include <unistd.h>
#include "../utils/logger.hpp"

#define MAX_FRAME_SIZE 1518

namespace lwip {

bool NonBlockingInterface::init() {
    TunInterface tun;
    if (!tun.init()) {
        return false;
    }
    
    // 设置非阻塞模式
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    
    if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        return false;
    }
    
    event_loop_.run();
    return true;
}

bool NonBlockingInterface::send_frame(const std::vector<uint8_t>& data) {
    if (fd_ < 0) return false;
    ssize_t written = write(fd_, data.data(), data.size());
    return written == static_cast<ssize_t>(data.size());
}

bool NonBlockingInterface::receive_frame(std::vector<uint8_t>& data) {
    if (fd_ < 0) return false;
    data.resize(MAX_FRAME_SIZE);
    ssize_t n = read(fd_, data.data(), data.size());
    if (n > 0) {
        data.resize(n);
        return true;
    }
    return false;
}

uint64_t NonBlockingInterface::get_mac_address() const {
    return 0;
}

void NonBlockingInterface::set_mac_address([[maybe_unused]] uint64_t mac) {}

void NonBlockingInterface::process_io() {
    std::vector<uint8_t> buffer(MAX_FRAME_SIZE);
    ssize_t n;
    
    while ((n = read(fd_, buffer.data(), buffer.size())) > 0) {
        buffer.resize(n);
        on_frame_received(buffer);
    }
    
    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        LOG_ERROR("Failed to read from interface");
    }
}

void NonBlockingInterface::on_frame_received([[maybe_unused]] const std::vector<uint8_t>& frame) {}

} // namespace lwip