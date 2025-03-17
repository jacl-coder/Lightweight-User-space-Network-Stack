#include "packet_capture.hpp"
#include <cstring>

namespace lwip {

PacketCapture::PacketCapture(const std::string& filename) {
    file_.open(filename, std::ios::binary);
    if (file_.is_open()) {
        PcapHeader header;
        file_.write(reinterpret_cast<const char*>(&header), sizeof(header));
    }
}

PacketCapture::~PacketCapture() {
    if (file_.is_open()) {
        file_.close();
    }
}

void PacketCapture::capture_packet(const void* data, size_t len, bool is_incoming) {
    if (!file_.is_open()) return;

    // PCAP数据包头
    struct {
        uint32_t ts_sec;
        uint32_t ts_usec;
        uint32_t incl_len;
        uint32_t orig_len;
    } pcap_pkthdr;

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    pcap_pkthdr.ts_sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    pcap_pkthdr.ts_usec = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
    pcap_pkthdr.incl_len = len;
    pcap_pkthdr.orig_len = len;

    file_.write(reinterpret_cast<const char*>(&pcap_pkthdr), sizeof(pcap_pkthdr));
    file_.write(reinterpret_cast<const char*>(data), len);
    file_.flush();
}

} // namespace lwip
