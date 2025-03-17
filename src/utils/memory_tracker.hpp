#pragma once
#include <cstddef>
#include <map>
#include <mutex>
#include <string>

namespace lwip {

class MemoryTracker {
public:
    static MemoryTracker& instance();
    
    void track_allocation(void* ptr, size_t size, const char* file, int line);
    void track_deallocation(void* ptr);
    void dump_leaks() const;
    
private:
    struct AllocationInfo {
        size_t size;
        std::string file;
        int line;
    };
    
    std::map<void*, AllocationInfo> allocations_;
    std::mutex mutex_;
    std::atomic<size_t> total_allocated_{0};
};

#define TRACK_NEW(ptr, size) \
    MemoryTracker::instance().track_allocation(ptr, size, __FILE__, __LINE__)
#define TRACK_DELETE(ptr) \
    MemoryTracker::instance().track_deallocation(ptr)

} // namespace lwip
