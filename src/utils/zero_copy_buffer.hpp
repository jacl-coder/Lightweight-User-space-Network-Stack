#pragma once
#include <memory>
#include <vector>

namespace lwip {

class ZeroCopyBuffer {
public:
    struct BufferSegment {
        const uint8_t* data;
        size_t length;
        std::shared_ptr<std::vector<uint8_t>> holder;
    };

    void append(const std::vector<uint8_t>& data);
    void append(std::vector<uint8_t>&& data);
    std::vector<BufferSegment> segments() const;
    
private:
    std::vector<BufferSegment> segments_;
};

} // namespace lwip
