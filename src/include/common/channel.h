#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <utility>
namespace bustub {
    template<typename Tp_>
    class Channel{
    public:
        // producer
        auto Put(Tp_ elem) -> void {
            std::unique_lock<std::mutex> lock(mutex_);  // lock the queue
            queue_.push(std::move(elem));                   // some class could not copy
            cv_.notify_one();
        }
        // consummer
        auto Get() -> Tp_ {
            std::unique_lock<std::mutex> lock(mutex_);      // lock
            cv_.wait(lock, [&]() { return !queue_.empty(); }); // wait until true
            Tp_ elem = std::move(queue_.front());
            queue_.pop();
            return elem;    // auto move / NRVO
        }

    private:
        std::queue<Tp_> queue_;
        std::mutex mutex_;
        std::condition_variable cv_;
    };
}