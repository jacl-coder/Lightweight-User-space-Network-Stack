#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <future>

namespace lwip {

class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    ~ThreadPool();
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;
    
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_{false};
};

} // namespace lwip
