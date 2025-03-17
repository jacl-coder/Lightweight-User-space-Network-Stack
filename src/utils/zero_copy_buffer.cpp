#include "zero_copy_buffer.hpp"

namespace lwip {

void ZeroCopyBuffer::append(const std::vector<uint8_t>& data) {
    if (data.empty()) return;
    
    auto holder = std::make_shared<std::vector<uint8_t>>(data);
    segments_.push_back({
        holder->data(),
        holder->size(),
        std::move(holder)
    });
}

void ZeroCopyBuffer::append(std::vector<uint8_t>&& data) {
    if (data.empty()) return;
    
    auto holder = std::make_shared<std::vector<uint8_t>>(std::move(data));
    segments_.push_back({
        holder->data(),
        holder->size(),
        std::move(holder)
    });
}

std::vector<ZeroCopyBuffer::BufferSegment> ZeroCopyBuffer::segments() const {
    return segments_;
}

} // namespace lwip
