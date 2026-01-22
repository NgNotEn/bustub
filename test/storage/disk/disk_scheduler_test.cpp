#include <chrono>
#include <future>
#include <random>
#include <vector>
#include <filesystem>
#include <memory>
#include <thread>
#include <cstring>

#include "gtest/gtest.h"
#include "storage/disk/disk_scheduler.h"
#include "storage/disk/disk_manager.h"
#include "common/config.h"

namespace bustub {

class DiskSchedulerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    if (std::filesystem::exists(db_file_)) std::filesystem::remove(db_file_);
  }
  void TearDown() override {
    if (std::filesystem::exists(db_file_)) std::filesystem::remove(db_file_);
  }

  const std::string db_file_ = "disk_test.db";
  
  // 生成随机填充数据的辅助函数
  void FillRandom(char *data, size_t size, uint32_t seed) {
    std::mt19937 gen(seed);
    for (size_t i = 0; i < size; ++i) data[i] = static_cast<char>(gen() % 256);
  }
};

// =========================================================================
// 1. 数据一致性测试：写完立即读，验证数据准确性
// =========================================================================
TEST_F(DiskSchedulerTest, ReadWriteConsistency) {
  auto dm = std::make_unique<DiskManager>(db_file_);
  DiskScheduler scheduler(std::move(dm));

  const int num_pages = 500;
  std::vector<std::vector<char>> write_data(num_pages, std::vector<char>(PAGE_SIZE));
  std::vector<std::vector<char>> read_data(num_pages, std::vector<char>(PAGE_SIZE));

  // 第一步：批量异步写入
  std::vector<std::future<bool>> write_futures;
  for (int i = 0; i < num_pages; ++i) {
    FillRandom(write_data[i].data(), PAGE_SIZE, i);
    std::promise<bool> promise;
    write_futures.push_back(promise.get_future());
    scheduler.Schedule({true, write_data[i].data(), static_cast<page_id_t>(i), std::move(promise)});
  }
  for (auto &f : write_futures) ASSERT_TRUE(f.get());

  // 第二步：批量异步读取并校验
  std::vector<std::future<bool>> read_futures;
  for (int i = 0; i < num_pages; ++i) {
    std::promise<bool> promise;
    read_futures.push_back(promise.get_future());
    scheduler.Schedule({false, read_data[i].data(), static_cast<page_id_t>(i), std::move(promise)});
  }
  for (int i = 0; i < num_pages; ++i) {
    ASSERT_TRUE(read_futures[i].get());
    ASSERT_EQ(std::memcmp(write_data[i].data(), read_data[i].data(), PAGE_SIZE), 0) 
              << "Data mismatch at page " << i;
  }
}

// =========================================================================
// 2. 超大规模高负载吞吐量 (10万次 I/O)
// =========================================================================
TEST_F(DiskSchedulerTest, MassiveThroughputStress) {
  auto dm = std::make_unique<DiskManager>(db_file_);
  DiskScheduler scheduler(std::move(dm));
  
  const int total_requests = 100000;
  char buf[PAGE_SIZE];
  
  std::cout << "[ INFO ] Submitting " << total_requests << " sequential requests..." << std::endl;
  
  auto start = std::chrono::high_resolution_clock::now();
  
  // 采用分批等待策略，模拟 BufferPool 的并发刷盘
  const int batch_size = 1000;
  for (int i = 0; i < total_requests / batch_size; ++i) {
    std::vector<std::future<bool>> futures;
    for (int j = 0; j < batch_size; ++j) {
      std::promise<bool> promise;
      futures.push_back(promise.get_future());
      scheduler.Schedule({true, buf, static_cast<page_id_t>(i * batch_size + j), std::move(promise)});
    }
    for (auto &f : futures) f.get();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  
  double iops = (total_requests * 1000.0) / ms;
  std::cout << "[ BENCHMARK ] Throughput: " << iops << " IOPS" << std::endl;
}

// =========================================================================
// 3. 多线程极速冲突测试：多个线程竞争调度同一批页面
// =========================================================================
TEST_F(DiskSchedulerTest, MultithreadedRaceStress) {
  auto dm = std::make_unique<DiskManager>(db_file_);
  DiskScheduler scheduler(std::move(dm));

  const int num_threads = 16;
  const int requests_per_thread = 1000;
  
  // 先预写入所有页面，确保后续读取不会越界
  char init_buf[PAGE_SIZE] = {0};
  for (int i = 0; i < requests_per_thread; ++i) {
    std::promise<bool> promise;
    auto future = promise.get_future();
    scheduler.Schedule({true, init_buf, static_cast<page_id_t>(i), std::move(promise)});
    future.get();
  }
  
  std::vector<std::thread> threads;

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&scheduler, requests_per_thread, t]() {
      char local_buf[PAGE_SIZE];
      std::vector<std::future<bool>> futures;
      for (int i = 0; i < requests_per_thread; ++i) {
        std::promise<bool> promise;
        futures.push_back(promise.get_future());
        // 多个线程故意操作重叠的 page_id 来增加锁竞争
        scheduler.Schedule({(i % 2 == 0), local_buf, static_cast<page_id_t>(i), std::move(promise)});
      }
      for (auto &f : futures) f.get();
    });
  }

  for (auto &t : threads) t.join();
  std::cout << "[ INFO ] Multithreaded race test finished without deadlocks." << std::endl;
}

// =========================================================================
// 4. 析构安全性：在后台线程忙碌时销毁调度器
// =========================================================================
TEST_F(DiskSchedulerTest, RapidDestruction) {
  std::promise<bool> p;
  auto f = p.get_future();
  char buf[PAGE_SIZE];

  {
    auto dm = std::make_unique<DiskManager>(db_file_);
    DiskScheduler scheduler(std::move(dm));
    
    // 提交一个极大的请求列表然后立即销毁 scheduler
    for (int i = 0; i < 1000; ++i) {
      std::promise<bool> temp_p;
      scheduler.Schedule({true, buf, static_cast<page_id_t>(i), std::move(temp_p)});
    }
    // scheduler 在此处走出作用域并析构
  }
  
  // 如果你的析构函数处理得当（比如发送了 std::nullopt 到队列），这里不应该挂死
  std::cout << "[ INFO ] Scheduler destroyed while busy. Success." << std::endl;
}

}  // namespace bustub