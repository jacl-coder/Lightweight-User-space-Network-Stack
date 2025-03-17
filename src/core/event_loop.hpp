#pragma once
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <atomic>

namespace lwip {

class EventLoop {
public:
    using EventCallback = std::function<void()>;
    
    EventLoop();
    ~EventLoop();

    void post(EventCallback callback);
    void run();
    void stop();

private:
    std::queue<EventCallback> event_queue_;
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> thread_;
    std::mutex mutex_;
};

} // namespace lwip
