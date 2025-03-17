#include "event_loop.hpp"
#include "../utils/logger.hpp"

namespace lwip {

EventLoop::EventLoop() = default;
EventLoop::~EventLoop() = default;

void EventLoop::post(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_.push(std::move(task));
    condition_.notify_one();
}

void EventLoop::run() {
    running_ = true;
    while (running_) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { 
                return !tasks_.empty() || !running_; 
            });
            if (!running_) break;
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

void EventLoop::stop() {
    running_ = false;
    condition_.notify_all();
}

} // namespace lwip
