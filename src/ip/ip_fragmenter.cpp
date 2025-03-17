#include "ip_fragmenter.hpp"
#include "../utils/logger.hpp"
#include <array>
#include "icmp_packet.hpp"

namespace lwip {

std::vector<IPPacket> IPFragmenter::fragment_packet(const IPPacket& packet, uint16_t mtu) {
    std::vector<IPPacket> fragments;
    auto data = packet.serialize();
    
    // IP头部大小
    size_t header_size = sizeof(IPPacket::IPHeader);
    // 每个分片的负载大小
    size_t fragment_size = (mtu - header_size) & ~7;  // 8字节对齐
    
    size_t offset = 0;
    while (offset < data.size()) {
        IPPacket fragment;
        bool is_last = (offset + fragment_size >= data.size());
        size_t current_size = is_last ? (data.size() - offset) : fragment_size;
        
        // 设置分片标志和偏移
        fragment.set_fragment_offset(offset / 8);
        fragment.set_more_fragments(!is_last);
        
        // 复制数据
        std::vector<uint8_t> fragment_data(
            data.begin() + offset,
            data.begin() + offset + current_size
        );
        fragment.set_payload(fragment_data);
        
        fragments.push_back(fragment);
        offset += current_size;
    }
    
    return fragments;
}

bool IPFragmenter::add_fragment(const IPPacket& fragment) {
    uint16_t offset = fragment.get_fragment_offset() * 8;
    fragments_[offset].data = fragment.get_payload();
    fragments_[offset].received = true;
    fragments_[offset].timestamp = std::chrono::steady_clock::now();
    
    return is_complete();
}

bool IPFragmenter::is_complete() const {
    if (fragments_.empty()) {
        return false;
    }
    
    size_t expected_size = 0;
    for (const auto& [offset, info] : fragments_) {
        if (!info.received) {
            return false;
        }
        expected_size = std::max(expected_size, offset + info.data.size());
    }
    return expected_size > 0;
}

void IPFragmenter::cleanup_timeout_fragments() {
    auto now = std::chrono::steady_clock::now();
    for (auto it = fragments_.begin(); it != fragments_.end();) {
        if (now - it->second.timestamp > std::chrono::seconds(60)) {
            it = fragments_.erase(it);
        } else {
            ++it;
        }
    }
}

IPPacket IPFragmenter::reassemble() {
    if (!is_complete()) {
        return IPPacket();
    }

    std::vector<uint8_t> complete_data;
    uint32_t total_size = 0;

    // 按偏移排序并合并数据
    for (const auto& [offset, info] : fragments_) {
        total_size = std::max(total_size, static_cast<uint32_t>(offset + info.data.size()));
    }
    
    complete_data.resize(total_size);
    for (const auto& [offset, info] : fragments_) {
        std::copy(info.data.begin(), info.data.end(), complete_data.begin() + offset);
    }

    IPPacket reassembled;
    reassembled.parse(complete_data);
    fragments_.clear();
    
    return reassembled;
}

void IPFragmenter::cleanup() {
    cleanup_timeout_fragments();
    // 清理过大的分片缓存
    if (fragments_.size() > MAX_FRAGMENTS) {
        fragments_.clear();
    }
}

void IPFragmenter::discover_path_mtu(uint32_t destination_ip) {
    static const std::array<uint16_t, 4> test_sizes = {1500, 1280, 576, 296};
    
    for (uint16_t size : test_sizes) {
        if (try_mtu_size(destination_ip, size)) {
            path_mtu_cache_[destination_ip] = size;
            return;
        }
    }
    
    // 如果所有尺寸都失败，使用最小MTU
    path_mtu_cache_[destination_ip] = 296;
}

bool IPFragmenter::try_mtu_size(uint32_t destination_ip, uint16_t mtu_size) {
    ICMPPacket probe;
    probe.create_echo_request(0, 0);
    // 移除不支持的 set_payload 调用
    // std::vector<uint8_t> payload(mtu_size - sizeof(ICMPPacket::ICMPHeader));
    // probe.set_payload(payload);
    
    return send_probe(destination_ip, probe.serialize());
}

bool IPFragmenter::send_probe(uint32_t destination_ip, const std::vector<uint8_t>& data) {
    // 消除未使用参数警告
    (void)destination_ip;
    (void)data;
    // TODO: 实现探测包发送逻辑
    return false;
}

}
