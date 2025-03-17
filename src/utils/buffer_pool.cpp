#include "buffer_pool.hpp"
#include "../utils/logger.hpp"

namespace lwip {

BufferPool::BufferPool(size_t buffer_size, size_t pool_size)
    : buffer_size_(buffer_size) {
    for (size_t i = 0; i < pool_size; ++i) {
        free_buffers_.push(std::make_unique<uint8_t[]>(buffer_size));
    }
    LOG_INFO("Buffer pool initialized with " + std::to_string(pool_size) + " buffers");
}

std::unique_ptr<uint8_t[]> BufferPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (free_buffers_.empty()) {
        return std::make_unique<uint8_t[]>(buffer_size_);
    }
    
    auto buffer = std::move(free_buffers_.front());
    free_buffers_.pop();
    return buffer;
}

void BufferPool::release(std::unique_ptr<uint8_t[]> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    free_buffers_.push(std::move(buffer));
}

} // namespace lwip
