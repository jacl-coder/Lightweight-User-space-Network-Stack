#pragma once
#include <string>
#include <fstream>
#include <chrono>

namespace lwip {

class PacketCapture {
public:
    PacketCapture(const std::string& filename);
    ~PacketCapture();
    
    void capture_packet(const void* data, size_t len, bool is_incoming);
    
private:
    struct PcapHeader {
        uint32_t magic_number{0xa1b2c3d4};
        uint16_t version_major{2};
        uint16_t version_minor{4};
        int32_t thiszone{0};
        uint32_t sigfigs{0};
        uint32_t snaplen{65535};
        uint32_t network{1};  // Ethernet
    };
    
    std::ofstream file_;
};

} // namespace lwip
