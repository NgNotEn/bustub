/**
 * test_main.cpp
 * * 这是一个针对 Bustub CMU 15-445 Project 1 LRU-K Replacer 的高强度测试套件。
 * 包含：逻辑验证、边界测试、随机压力测试、多线程并发测试。
 */

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <cassert>
#include <chrono>

// 引入你的头文件
#include "../../src/include/buffer/lru_k_replacer.h"
using namespace bustub;

// ============================================================================
// 测试辅助工具
// ============================================================================
#define RED   "\033[31m"
#define GREEN "\033[32m"
#define BLUE  "\033[34m"
#define RESET "\033[0m"

std::atomic<int> g_failure_count{0};

void LogInfo(const std::string& msg) {
    std::cout << BLUE << "[INFO] " << msg << RESET << std::endl;
}

void LogPass(const std::string& test_name) {
    std::cout << GREEN << "[PASS] " << test_name << RESET << std::endl;
}

void LogFail(const std::string& test_name, const std::string& reason) {
    std::cout << RED << "[FAIL] " << test_name << ": " << reason << RESET << std::endl;
    g_failure_count++;
}

#define EXPECT_EQ(val1, val2, msg) \
    if ((val1) != (val2)) { \
        LogFail(__func__, std::string(msg) + " Expected " + std::to_string(val1) + " but got " + std::to_string(val2)); \
        return; \
    }

#define EXPECT_TRUE(cond, msg) \
    if (!(cond)) { \
        LogFail(__func__, std::string(msg)); \
        return; \
    }

// ============================================================================
// 测试用例 1: 基础逻辑测试 (Pin, Size, Basic Evict)
// ============================================================================
void LRUK_Basic_Test() {
    LogInfo("Running Basic Test...");
    LRUKReplacer lru_replacer(7, 2);

    // 1. 初始状态
    EXPECT_EQ(0, lru_replacer.Size(), "Initial size should be 0");

    // 2. 访问 Frame 1-6
    for (int i = 1; i <= 6; ++i) {
        lru_replacer.RecordAccess(i);
        lru_replacer.SetEvictable(i, true);
    }
    EXPECT_EQ(6, lru_replacer.Size(), "Size should be 6 after adding 6 frames");

    // 3. Pin Frame 1
    lru_replacer.SetEvictable(1, false);
    EXPECT_EQ(5, lru_replacer.Size(), "Size should be 5 after pinning Frame 1");

    // 4. Evict 一个 (此时 1 被 pin, 剩下 2-6 都是 +inf, 2 最早)
    int value;
    bool success = lru_replacer.Evict(&value);
    EXPECT_TRUE(success, "Evict should return true");
    EXPECT_EQ(2, value, "Should evict Frame 2 (earliest +inf)");
    EXPECT_EQ(4, lru_replacer.Size(), "Size should decrease after eviction");

    // 5. Re-add Frame 2 and Pin it
    lru_replacer.RecordAccess(2);
    lru_replacer.SetEvictable(2, false);
    EXPECT_EQ(4, lru_replacer.Size(), "Adding a pinned frame shouldn't increase size");

    // 6. Unpin Frame 1
    lru_replacer.SetEvictable(1, true);
    EXPECT_EQ(5, lru_replacer.Size(), "Unpinning should increase size");

    LogPass("LRUK_Basic_Test");
}

// ============================================================================
// 测试用例 2: +inf 和平局打破逻辑 (The Tie Breaker)
// ============================================================================
void LRUK_TieBreaker_Test() {
    LogInfo("Running Tie Breaker Test...");
    // K = 3
    LRUKReplacer replacer(10, 3);

    // A. 制造 3 个“一等公民” (+inf, 访问不足 3 次)
    // Frame 1: 访问 1 次 (最早)
    replacer.RecordAccess(1); 
    replacer.SetEvictable(1, true);
    
    // Frame 2: 访问 2 次 (中间)
    replacer.RecordAccess(2); 
    replacer.RecordAccess(2); 
    replacer.SetEvictable(2, true);

    // Frame 3: 访问 1 次 (最晚)
    replacer.RecordAccess(3); 
    replacer.SetEvictable(3, true);

    // B. 制造 1 个“二等公民” (普通, 访问满 3 次)
    replacer.RecordAccess(4);
    replacer.RecordAccess(4);
    replacer.RecordAccess(4); // 满了 K=3
    replacer.SetEvictable(4, true);

    int victim;
    
    // 第 1 次驱逐: 必须在 +inf 组里选 (1, 2, 3)。
    // 规则: 选最早访问的 (history.front 最小)。
    // Frame 1 是第一个 RecordAccess 的，所以是 1。
    EXPECT_TRUE(replacer.Evict(&victim), "Should evict");
    EXPECT_EQ(1, victim, "Victim should be 1 (earliest +inf)");

    // 第 2 次驱逐: 剩 2, 3 (+inf) 和 4 (finite)。选 +inf 里的最早。
    // Frame 2 的第一次访问比 Frame 3 早。
    EXPECT_TRUE(replacer.Evict(&victim), "Should evict");
    EXPECT_EQ(2, victim, "Victim should be 2 (next earliest +inf)");

    // 第 3 次驱逐: 剩 3 (+inf) 和 4 (finite)。
    // 尽管 4 很久以前就被访问了，但它满足了 K 次，所以它是二等公民。
    // 3 依然是 +inf，优先被杀。
    EXPECT_TRUE(replacer.Evict(&victim), "Should evict");
    EXPECT_EQ(3, victim, "Victim should be 3 (last +inf)");

    // 第 4 次驱逐: 只剩 4。
    EXPECT_TRUE(replacer.Evict(&victim), "Should evict");
    EXPECT_EQ(4, victim, "Victim should be 4");

    LogPass("LRUK_TieBreaker_Test");
}

// ============================================================================
// 测试用例 3: 复杂 Remove 测试 (Anti-Memory Leak)
// ============================================================================
void LRUK_Pin_Remove_Test() {
    LogInfo("Running Remove Test...");
    LRUKReplacer replacer(10, 2);

    // 1. 加两个 frame
    replacer.RecordAccess(1);
    replacer.SetEvictable(1, true);
    replacer.RecordAccess(2);
    replacer.SetEvictable(2, false); // 2 是不可驱逐的

    EXPECT_EQ(1, replacer.Size(), "Initial size error");

    // 2. Remove 可驱逐的 (1)
    replacer.Remove(1);
    EXPECT_EQ(0, replacer.Size(), "Size should be 0 after removing evictable");

    // 3. Remove 不可驱逐的 (2)
    // 此时 Size 本来就是 0，Remove 后不应该变成 (unsigned)-1
    replacer.Remove(2);
    EXPECT_EQ(0, replacer.Size(), "Size should remain 0 after removing non-evictable");

    // 4. 重新加回来，确保还能正常工作
    replacer.RecordAccess(1);
    replacer.SetEvictable(1, true);
    EXPECT_EQ(1, replacer.Size(), "Should be able to re-add removed frame");

    // 5. Remove 不存在的
    // 5. Remove 不存在的 (但在合法范围内)
    // 容量是 10，ID 5 是合法的，但是我们前面只用了 1 和 2。
    // 所以 map 里没有 5。逻辑应该是“直接返回”，不报错。
    try {
        replacer.Remove(5); 
    } catch (...) {
        LogFail("LRUK_Pin_Remove_Test", "Remove(5) threw exception but shouldn't!");
        return;
    }
    EXPECT_EQ(1, replacer.Size(), "Removing non-existent frame should do nothing");
    LogPass("LRUK_Pin_Remove_Test");
}

// ============================================================================
// 测试用例 4: 复杂场景下的 LRU-K 逻辑验证
// ============================================================================
void LRUK_Complex_Scenario_Test() {
    LogInfo("Running Complex Scenario Test (K=2)...");
    LRUKReplacer replacer(100, 2);

    // 场景模拟：
    // Frame 1: 很久以前极其活跃，最近不活跃了
    // Frame 2: 最近非常活跃
    // Frame 3: 偶尔访问

    // T=1~5: Frame 1 访问 5 次
    for(int i=0; i<5; i++) replacer.RecordAccess(1);
    
    // T=6~10: Frame 2 访问 5 次
    for(int i=0; i<5; i++) replacer.RecordAccess(2);

    // T=11: Frame 3 访问 1 次
    replacer.RecordAccess(3);

    // 全部设为可驱逐
    replacer.SetEvictable(1, true);
    replacer.SetEvictable(2, true);
    replacer.SetEvictable(3, true);

    // 分析：
    // Frame 3: 只有 1 次访问 -> +inf 距离 -> 优先级最高。
    // Frame 1: 倒数第 2 次访问是在 T=4。
    // Frame 2: 倒数第 2 次访问是在 T=9。
    // 比较 1 和 2:
    // Distance(1) = Now - 4  (很大)
    // Distance(2) = Now - 9  (较小)
    // 所以 Frame 1 的距离更大，比 Frame 2 优先被踢。

    int victim;
    
    // 第一杀: 3 (+inf)
    replacer.Evict(&victim);
    EXPECT_EQ(3, victim, "First evict should be +inf frame (3)");

    // 第二杀: 1 (Oldest K-th access)
    replacer.Evict(&victim);
    EXPECT_EQ(1, victim, "Second evict should be frame with largest K-distance (1)");

    // 第三杀: 2
    replacer.Evict(&victim);
    EXPECT_EQ(2, victim, "Last evict should be 2");

    LogPass("LRUK_Complex_Scenario_Test");
}

// ============================================================================
// 测试用例 5: 多线程并发测试 (Thread Safety Check)
// ============================================================================
void LRUK_Concurrency_Test() {
    LogInfo("Running Concurrency Stress Test...");
    LRUKReplacer replacer(100, 2); // 100 frames
    
    std::atomic<bool> running{true};
    std::vector<std::thread> threads;

    // 任务 A: 疯狂 RecordAccess (写操作)
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<> dis(0, 99);
            while (running) {
                int fid = dis(gen);
                replacer.RecordAccess(fid);
                // 偶尔设为可驱逐，增加混乱度
                // 【修复】加上 try-catch，防止该 Frame 在这两行代码之间被其他线程 Evict 掉
                if (fid % 2 == 0) {
                    try {
                        replacer.SetEvictable(fid, true);
                    } catch (...) {} 
                }
            }
        });
    }

    // 任务 B: 疯狂 Evict (删除操作)
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&]() {
            while (running) {
                int victim;
                replacer.Evict(&victim);
                // 休息极短时间，避免 CPU 空转太快导致无法 Record
                std::this_thread::yield(); 
            }
        });
    }

    // 任务 C: 疯狂切换 Evictable 状态 (状态翻转)
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&]() {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<> dis(0, 99);
            while (running) {
                int fid = dis(gen);
                bool state = (fid % 3 == 0);
                try {
                    // 这里可能会抛异常(如果frame没找到)，我们要捕获它
                    replacer.SetEvictable(fid, state);
                } catch (...) {}
            }
        });
    }

    // 运行 2 秒钟
    std::this_thread::sleep_for(std::chrono::seconds(2));
    running = false;

    for (auto& t : threads) {
        t.join();
    }

    // 如果能活到这里没 Crash，说明线程安全基本合格
    LogPass("LRUK_Concurrency_Test (Survived)");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  LRU-K REPLACER STRICT SUITE  " << std::endl;
    std::cout << "========================================" << std::endl;

    LRUK_Basic_Test();
    LRUK_TieBreaker_Test();
    LRUK_Pin_Remove_Test();
    LRUK_Complex_Scenario_Test();
    LRUK_Concurrency_Test();

    std::cout << "========================================" << std::endl;
    if (g_failure_count == 0) {
        std::cout << GREEN << "  ALL TESTS PASSED! PERFECT!  " << RESET << std::endl;
        return 0;
    } else {
        std::cout << RED << "  FAILED " << g_failure_count << " TESTS. KEEP DEBUGGING.  " << RESET << std::endl;
        return 1;
    }
}