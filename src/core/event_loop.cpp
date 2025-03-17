#include "event_loop.hpp"
#include "../utils/logger.hpp"

namespace lwip {

EventLoop::EventLoop() : running_(false) {}

void EventLoop::run() {
    running_ = true;
    thread_ = std::make_unique<std::thread>([this]() {
        while (running_) {
            EventCallback callback;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!event_queue_.empty()) {
                    callback = std::move(event_queue_.front());
                    event_queue_.pop();
                }
            }
            if (callback) {
                callback();
            }
        }
    });
}

void EventLoop::post(EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    event_queue_.push(std::move(callback));
}

void EventLoop::stop() {
    running_ = false;
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

} // namespace lwip
