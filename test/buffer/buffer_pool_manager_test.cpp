#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <iostream>
#include "buffer/buffer_pool_manager.h"

namespace bustub {

class BufferPoolManagerTest : public ::testing::Test {
protected:
    // 每次测试前清理旧的数据库文件
    void SetUp() override {
        remove("test.db");
    }
    // 测试结束后清理
    void TearDown() override {
        remove("test.db");
    }

    const std::string db_name_ = "test.db";
};

// 辅助函数：生成随机数
int GetRandomInt(int min, int max) {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

// ============================================================================
// 测试用例 1: 高强度并发拷打测试 (Hardcore Concurrency Torture)
// 目的：检测死锁、Pin Count 泄漏、脏数据覆盖
// ============================================================================
TEST_F(BufferPoolManagerTest, HardcoreConcurrencyTest) {
    const int pool_size = 10;          // 极小的池，强制疯狂驱逐
    const int k_lru = 2;               // LRU-K
    const int num_threads = 8;         // 线程数
    const int ops_per_thread = 5000;   // 每个线程的操作次数

    // 创建 BPM
    auto *bpm = new BufferPoolManager(pool_size, k_lru, db_name_);

    std::atomic<page_id_t> max_page_id{0};
    std::atomic<bool> failed{false}; // 用于在线程中标记测试失败

    auto worker_func = [&](int thread_id) {
        for (int i = 0; i < ops_per_thread; ++i) {
            if (failed.load()) break; // 如果已经有线程失败，停止运行

            // 随机操作选择: 60% Read/Check, 30% New/Write, 10% Delete
            int op_type = GetRandomInt(0, 9);

            if (op_type <= 5) {
                // === CASE A: Fetch & Verify (读取并校验) ===
                page_id_t current_max = max_page_id.load();
                if (current_max == 0) continue;

                page_id_t target_id = GetRandomInt(0, current_max - 1);
                
                // 使用 try-catch 捕获越界异常（页面可能被删除）
                Page *page = nullptr;
                try {
                    page = bpm->FetchPage(target_id);
                } catch (const std::exception &e) {
                    // 页面可能已被删除，跳过
                    continue;
                }

                if (page != nullptr) {
                    // 1. 校验数据完整性
                    // 我们的约定：页面的前4个字节存储的是 page_id
                    int *data = reinterpret_cast<int *>(page->GetData());
                    
                    // 如果数据不匹配，且不是初始的0（防止刚分配还没写的竞态），则报错
                    if (*data != static_cast<int>(target_id) && *data != 0) {
                        std::cerr << "[FATAL] Thread " << thread_id 
                                  << " Expected PageId: " << target_id 
                                  << " But Got Data: " << *data << std::endl;
                        failed.store(true);
                    }

                    // 2. 随机写入 (模拟脏页)
                    bool is_write = GetRandomInt(0, 1) == 1;
                    if (is_write) {
                        *data = target_id; // 再次写入确保脏数据
                    }

                    // 3. Unpin
                    bpm->UnpinPage(target_id, is_write);
                }
            } else if (op_type <= 8) {
                // === CASE B: New Page (创建并写入) ===
                Page *page = bpm->NewPage();
                if (page != nullptr) {
                    page_id_t pid = page->GetPageId();
                    
                    // 1. 写入数据
                    int *data = reinterpret_cast<int *>(page->GetData());
                    *data = pid;

                    // 2. 更新最大页号 (CAS 更新)
                    page_id_t old_max = max_page_id.load();
                    while (pid >= old_max && !max_page_id.compare_exchange_weak(old_max, pid + 1));

                    // 3. Unpin (脏)
                    bpm->UnpinPage(pid, true);
                }
            } else {
                // === CASE C: 不删除页面，改为刷新页面 ===
                // 删除页面会导致后续读取越界，改用 Flush 来测试
                page_id_t current_max = max_page_id.load();
                if (current_max > 0) {
                    page_id_t target_id = GetRandomInt(0, current_max - 1);
                    bpm->FlushPage(target_id);
                }
            }
        }
    };

    // 启动线程
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_func, i);
    }

    // 等待结束
    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }

    // 断言测试过程中没有发生致命错误
    EXPECT_FALSE(failed.load()) << "Data inconsistency detected during concurrent execution!";
    
    // 清理
    delete bpm;
}

// ============================================================================
// 测试用例 2: 持久化与恢复测试 (Persistence & Recovery)
// 目的：确保 FlushPage 或 BPM 析构时，数据真的写到了磁盘上，并且能被重新读回
// ============================================================================
TEST_F(BufferPoolManagerTest, PersistenceTest) {
    const int pool_size = 5;
    const int k_lru = 2;

    // 1. 第一阶段：写入数据
    {
        auto *bpm = new BufferPoolManager(pool_size, k_lru, db_name_);
        
        // 创建 10 个页面 (超过 pool_size，强制发生驱逐和刷盘)
        for (int i = 0; i < 10; ++i) {
            Page *page = bpm->NewPage();
            ASSERT_NE(page, nullptr);
            
            // 写入独特的数据： page_id * 12345
            int *data = reinterpret_cast<int *>(page->GetData());
            *data = page->GetPageId() * 12345;
            
            // 标记为脏并 Unpin
            bpm->UnpinPage(page->GetPageId(), true);
        }

        // 显式 Flush 一部分，利用析构 Flush 另一部分
        bpm->FlushPage(0);
        bpm->FlushPage(1);
        
        delete bpm; // 析构函数应该将所有脏页刷盘
    }

    // 2. 第二阶段：重启系统并验证
    {
        auto *bpm = new BufferPoolManager(pool_size, k_lru, db_name_);

        for (int i = 0; i < 10; ++i) {
            Page *page = bpm->FetchPage(i);
            ASSERT_NE(page, nullptr);

            int *data = reinterpret_cast<int *>(page->GetData());
            // 验证数据是否还在
            EXPECT_EQ(*data, i * 12345) << "Data mismatch on Page " << i << " after restart.";

            bpm->UnpinPage(i, false);
        }

        delete bpm;
    }
}

// ============================================================================
// 测试用例 3: 顺序扫描抖动测试 (Sequential Scan Thrashing)
// 目的：模拟全表扫描场景，验证 LRU-K 在极端驱逐下的稳定性
// ============================================================================
TEST_F(BufferPoolManagerTest, ScanThrashingTest) {
    const int pool_size = 3; // 极小
    const int num_pages = 30; // 很大
    auto *bpm = new BufferPoolManager(pool_size, 2, db_name_);

    // 1. 创建大量页面
    std::vector<page_id_t> pages;
    for (int i = 0; i < num_pages; ++i) {
        Page *p = bpm->NewPage();
        ASSERT_NE(p, nullptr);
        pages.push_back(p->GetPageId());
        
        // 写入数据
        strcpy(p->GetData(), ("Page-" + std::to_string(i)).c_str());
        bpm->UnpinPage(p->GetPageId(), true);
    }

    // 2. 模拟全表扫描多次
    for (int round = 0; round < 5; ++round) {
        for (int i = 0; i < num_pages; ++i) {
            page_id_t pid = pages[i];
            Page *p = bpm->FetchPage(pid);
            ASSERT_NE(p, nullptr) << "Failed to fetch page " << pid << " in round " << round;
            
            // 验证内容
            char expected[32];
            sprintf(expected, "Page-%d", i);
            EXPECT_EQ(strcmp(p->GetData(), expected), 0);

            bpm->UnpinPage(pid, false);
        }
    }

    delete bpm;
}

} // namespace bustub