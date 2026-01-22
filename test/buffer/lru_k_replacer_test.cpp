#include <random>
#include <thread>
#include <vector>

#include "buffer/lru_k_replacer.h"
#include "gtest/gtest.h"

namespace bustub {

// ============================================================================
// 测试用例 1: 基础逻辑与 Pin/Unpin 切换
// ============================================================================
TEST(LRUKReplacerTest, BasicTest) {
  LRUKReplacer replacer(7, 2);

  // 初始检查
  EXPECT_EQ(0, replacer.Size());

  // 场景：插入 1-6，设为可驱逐
  for (int i = 1; i <= 6; ++i) {
    replacer.RecordAccess(i);
    replacer.SetEvictable(i, true);
  }
  EXPECT_EQ(6, replacer.Size());

  // 驱逐测试：1 最早进入，且只有 1 次访问 (+inf)，应该最先被驱逐
  frame_id_t value;
  EXPECT_TRUE(replacer.Evict(&value));
  EXPECT_EQ(1u, value);
  EXPECT_EQ(5, replacer.Size());

  // Pin 操作：将 2 设为不可驱逐
  replacer.SetEvictable(2, false);
  EXPECT_EQ(4, replacer.Size());

  // 再次驱逐：此时 2 被跳过，3 是剩下的最早 +inf
  EXPECT_TRUE(replacer.Evict(&value));
  EXPECT_EQ(3u, value);
  EXPECT_EQ(3, replacer.Size());
}

// ============================================================================
// 测试用例 2: 核心 K-Distance 逻辑 (Tie-breaker)
// ============================================================================
TEST(LRUKReplacerTest, KDistanceLogicTest) {
  // K = 3
  size_t k = 3;
  LRUKReplacer replacer(10, k);

  // 制造两类 Frame：
  // 1. +inf 组 (访问不满 3 次)
  replacer.RecordAccess(1); // T1: Frame 1 (最早)
  replacer.RecordAccess(2); // T2: Frame 2
  replacer.RecordAccess(2); // T3: Frame 2 (第2次访问)
  
  // 2. Finite 组 (访问满 3 次)
  // Frame 3: 很久以前就满了 3 次 (K-distance 很大)
  replacer.RecordAccess(3); // T4
  replacer.RecordAccess(3); // T5
  replacer.RecordAccess(3); // T6 (倒数第3次是 T4)
  
  // Frame 4: 刚刚才满 3 次 (K-distance 较小)
  replacer.RecordAccess(4); // T7
  replacer.RecordAccess(4); // T8
  replacer.RecordAccess(4); // T9 (倒数第3次是 T7)

  // 全部设为可驱逐
  for (int i = 1; i <= 4; ++i) replacer.SetEvictable(i, true);

  frame_id_t victim;
  // 第一驱逐：+inf 组里最早的 Frame 1
  EXPECT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(1u, victim);

  // 第二驱逐：+inf 组里剩下的 Frame 2
  EXPECT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(2u, victim);

  // 第三驱逐：Finite 组比较。3 的 K-distance (Now - T4) > 4 的 (Now - T7)
  // 所以 3 应该先被驱逐
  EXPECT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(3u, victim);

  // 第四驱逐：最后剩下的 4
  EXPECT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(4u, victim);
}

// ============================================================================
// 测试用例 3: 历史记录清理测试 (防止 Evict 后残留数据)
// ============================================================================
TEST(LRUKReplacerTest, EvictCleanupTest) {
  LRUKReplacer replacer(10, 2);

  // Frame 1 访问 100 次，此时它是 Finite 状态且优先级很低
  for (int i = 0; i < 100; ++i) replacer.RecordAccess(1);
  replacer.SetEvictable(1, true);

  // 驱逐它
  frame_id_t victim;
  EXPECT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(1u, victim);

  // 关键：再次访问 1。它必须被当作一个全新的、只有 1 次记录的 +inf 节点
  replacer.RecordAccess(1);
  replacer.SetEvictable(1, true);

  // 制造一个已经在里面、访问过 2 次的 Frame 2 (Finite 状态)
  replacer.RecordAccess(2);
  replacer.RecordAccess(2);
  replacer.SetEvictable(2, true);

  // 此时驱逐：由于 1 是 +inf，2 是 Finite，1 必须先死
  // 如果你没清理历史，1 会带着“前世”的 100 次记录，变成 Finite 导致 2 被驱逐
  EXPECT_TRUE(replacer.Evict(&victim));
  EXPECT_EQ(1u, victim);
}

// ============================================================================
// 测试用例 4: 并发安全性与 Size 一致性测试
// ============================================================================
TEST(LRUKReplacerTest, ConcurrencyTest) {
  const int num_frames = 100;
  const int num_threads = 8;
  const int ops_per_thread = 1000;
  LRUKReplacer replacer(num_frames, 2);

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&replacer, num_frames, ops_per_thread, i]() {
      std::mt19937 gen(i);
      std::uniform_int_distribution<> dis(0, num_frames - 1);
      for (int j = 0; j < ops_per_thread; ++j) {
        int fid = dis(gen);
        replacer.RecordAccess(fid);
        replacer.SetEvictable(fid, (j % 2 == 0));
        if (j % 5 == 0) {
          frame_id_t victim;
          replacer.Evict(&victim);
        }
      }
    });
  }

  for (auto &t : threads) t.join();

  // 只要不出死锁或 Segment Fault，且 Size 不超过总数，即视为初步通过
  EXPECT_LE(replacer.Size(), static_cast<size_t>(num_frames));
}

}  // namespace bustub