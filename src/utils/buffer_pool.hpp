#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>

namespace lwip {

class BufferPool {
public:
    explicit BufferPool(size_t buffer_size, size_t pool_size);
    
    std::unique_ptr<uint8_t[]> acquire();
    void release(std::unique_ptr<uint8_t[]> buffer);
    
private:
    const size_t buffer_size_;
    std::queue<std::unique_ptr<uint8_t[]>> free_buffers_;
    std::mutex mutex_;
};

} // namespace lwip
