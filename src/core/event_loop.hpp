#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace lwip {

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    
    void post(std::function<void()> task);
    void run();
    void stop();

private:
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> running_{false};
};

} // namespace lwip
