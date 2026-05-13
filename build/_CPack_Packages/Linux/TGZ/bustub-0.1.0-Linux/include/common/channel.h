/*
  线程安全阻塞队列
*/


#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
namespace bustub {
template <typename Tp_>
class Channel {
 public:
  // producer
  void Put(Tp_ elem) {
    std::unique_lock<std::mutex> lock(mutex_);  // lock the queue
    queue_.push(std::move(elem));               // some class could not copy
    cv_.notify_one();
  }
  // consummer
  Tp_ Get() {
    std::unique_lock<std::mutex> lock(mutex_);          // lock
    cv_.wait(lock, [&]() { return !queue_.empty(); });  // wait until true
    Tp_ elem = std::move(queue_.front());
    queue_.pop();
    return elem;  // auto move / NRVO
  }

 private:
  std::queue<Tp_> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
};
}  // namespace bustub