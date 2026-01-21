// test/storage/disk_scheduler_test.cpp
#include <chrono>
#include <cmath>
#include <future>
#include <iostream>
#include <vector>
#include <random>
#include <filesystem>
#include <memory> 

#include "gtest/gtest.h"
#include "common/config.h"
#include "storage/disk/disk_manager.h"
#include "storage/disk/disk_scheduler.h"

namespace bustub {

// 辅助类：用于计时
class ScopedTimer {
 public:
  ScopedTimer(const std::string &name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
  ~ScopedTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
    std::cout << "[ BENCHMARK ] " << name_ << " took " << duration << " ms" << std::endl;
  }
  
  auto GetDuration() -> double {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
  }

 private:
  std::string name_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class DiskBenchmarkTest : public ::testing::Test {
 protected:
  void SetUp() override {
    if (std::filesystem::exists(db_file_)) {
      std::filesystem::remove(db_file_);
    }
  }

  void TearDown() override {
    if (std::filesystem::exists(db_file_)) {
      std::filesystem::remove(db_file_);
    }
  }

  const std::string db_file_ = "benchmark_test.db";
  char data_[PAGE_SIZE] = {0}; 
};

// =========================================================================
// 测试 1: DiskManager 直接写 (Baseline)
// =========================================================================
TEST_F(DiskBenchmarkTest, DiskManagerThroughput) {
  DiskManager dm(db_file_);
  const int num_pages = 10000; 

  std::cout << "--- Starting DiskManager Baseline ---" << std::endl;
  
  ScopedTimer timer("DiskManager Direct Write");
  for (int i = 0; i < num_pages; ++i) {
    dm.WritePage(i, data_);
  }
  
  double elapsed_ms = timer.GetDuration();
  double iops = num_pages / (elapsed_ms / 1000.0);
  double bandwidth = (static_cast<double>(num_pages) * PAGE_SIZE) / 1024 / 1024 / (elapsed_ms / 1000.0);

  std::cout << "Results: " << iops << " IOPS, " << bandwidth << " MB/s" << std::endl;
}

// =========================================================================
// 测试 2: DiskScheduler 调度开销 (适配 unique_ptr)
// =========================================================================
TEST_F(DiskBenchmarkTest, SchedulerOverhead) {
  auto dm = std::make_unique<DiskManager>(db_file_);
  
  // [关键修改] 使用 std::move() 传递 unique_ptr 所有权
  // 适配你的: DiskScheduler(std::unique_ptr<DiskManager> disk_manager)
  DiskScheduler scheduler(std::move(dm));
  
  const int num_pages = 10000;

  std::vector<std::future<bool>> futures;
  futures.reserve(num_pages);

  std::cout << "--- Starting DiskScheduler Sequential Write ---" << std::endl;

  {
    ScopedTimer timer("DiskScheduler Submit & Wait");

    for (int i = 0; i < num_pages; ++i) {
      std::promise<bool> promise;
      futures.push_back(promise.get_future());

      DiskRequest request(true, data_, static_cast<page_id_t>(i), std::move(promise));
      scheduler.Schedule(std::move(request));
    }

    for (auto &f : futures) {
      ASSERT_TRUE(f.get());
    }
  }
}

// =========================================================================
// 测试 3: 多线程随机读写 (适配 unique_ptr)
// =========================================================================
TEST_F(DiskBenchmarkTest, MultiThreadRandomStress) {
  auto dm = std::make_unique<DiskManager>(db_file_);
  
  // [关键修改] 使用 std::move() 传递 unique_ptr 所有权
  DiskScheduler scheduler(std::move(dm));
  
  const int num_threads = 4;
  const int requests_per_thread = 2000;
  
  std::vector<std::thread> threads;
  std::cout << "--- Starting Multi-Threaded Random Stress Test ---" << std::endl;

  ScopedTimer timer("Multi-Thread Random I/O");

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&scheduler, requests_per_thread]() {
      char local_data[PAGE_SIZE];
      std::vector<std::future<bool>> local_futures;
      
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<page_id_t> page_dist(0, 5000);
      std::uniform_int_distribution<> op_dist(0, 1);

      for (int i = 0; i < requests_per_thread; ++i) {
        page_id_t page_id = page_dist(gen);
        bool is_write = (op_dist(gen) == 1);

        std::promise<bool> promise;
        local_futures.push_back(promise.get_future());

        DiskRequest request(is_write, local_data, page_id, std::move(promise));
        scheduler.Schedule(std::move(request));
      }

      for (auto &f : local_futures) {
        f.get();
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }
}

} // namespace bustub