#pragma once
#include <cstdint>
#include <memory>
#include "tcp_packet.hpp"

namespace lwip {

enum class TCPState {
    CLOSED,
    LISTEN,
    SYN_SENT,
    SYN_RECEIVED,
    ESTABLISHED,
    FIN_WAIT_1,
    FIN_WAIT_2,
    CLOSE_WAIT,
    CLOSING,
    LAST_ACK,
    TIME_WAIT
};

class TCPConnection {
public:
    TCPConnection();
    
    bool connect(uint32_t remote_ip, uint16_t remote_port);
    bool accept();
    bool send(const std::vector<uint8_t>& data);
    bool receive(std::vector<uint8_t>& data);
    void close();

private:
    TCPState state_;
    uint32_t local_seq_num_;
    uint32_t remote_seq_num_;
    uint16_t local_port_;
    uint16_t remote_port_;
    uint32_t remote_ip_;
};

} // namespace lwip
