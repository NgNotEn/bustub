#include <chrono>
#include <iostream>
#include <cstring>
#include "gtest/gtest.h"
#include "type/value.h"
#include "type/type.h"
#include "type/type_id.h"

namespace bustub {

// 简单的计时器
class ScopedTimer {
 public:
  ScopedTimer(const std::string &name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
  ~ScopedTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
    std::cout << "[ BENCHMARK ] " << name_ << " took " << duration << " ms" << std::endl;
  }
 private:
  std::string name_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class TypeBenchmark : public ::testing::Test {
 protected:
  // 模拟一个磁盘页的 buffer (4KB)
  char buffer_[4096];
};

// =========================================================================
// 测试 1: Integer 吞吐量测试
// =========================================================================
TEST_F(TypeBenchmark, IntegerStressTest) {
  const int iterations = 1000000; // 100万次
  std::cout << "--- Starting Integer Benchmark (" << iterations << " ops) ---" << std::endl;

  {
    ScopedTimer timer("Integer Serialize/Deserialize");
    
    for (int i = 0; i < iterations; ++i) {
      // 1. 创建 Value (内存态)
      Value v_in(i); 

      // 2. 序列化 (写入 buffer)
      v_in.SerializeTo(buffer_);

      // 3. 反序列化 (读出 buffer)
      Value v_out = Value::DeserializeFrom(buffer_, TypeId::INTEGER);

      // 4. 验证正确性 (防止编译器优化掉整个循环)
      if (!v_in.CompareEquals(v_out)) {
        FAIL() << "Integer Mismatch at " << i;
      }
    }
  }
}

// =========================================================================
// 测试 2: Varchar 吞吐量测试 (包含堆内存分配压力)
// =========================================================================
TEST_F(TypeBenchmark, VarcharStressTest) {
  const int iterations = 1000000; // 100万次
  std::string base_str = "BusTub_Is_Awesome_Type_System_Test_";
  std::cout << "--- Starting Varchar Benchmark (" << iterations << " ops) ---" << std::endl;

  {
    ScopedTimer timer("Varchar Serialize/Deserialize");

    for (int i = 0; i < iterations; ++i) {
      // 构造不同的字符串，防止缓存作弊
      // std::to_string 会产生 heap allocation
      std::string current_str = base_str + std::to_string(i); 
      
      // 1. 创建 Value (会 new char[])
      Value v_in(current_str);

      // 2. 序列化 (写入 buffer: [Len][Data])
      v_in.SerializeTo(buffer_);

      // 3. 反序列化 (会再次 new char[])
      Value v_out = Value::DeserializeFrom(buffer_, TypeId::VARCHAR);

      // 4. 比较 (memcmp)
      if (!v_in.CompareEquals(v_out)) {
        FAIL() << "Varchar Mismatch: " << v_in.ToString() << " vs " << v_out.ToString();
      }
      
      // 循环结束，v_in 和 v_out析构，delete[] 被调用
    }
  }
}

// =========================================================================
// 测试 3: 比较性能测试 (CPU 密集型)
// =========================================================================
TEST_F(TypeBenchmark, ComparisonStressTest) {
  const int iterations = 5000000; // 500万次
  std::cout << "--- Starting Comparison Benchmark (" << iterations << " ops) ---" << std::endl;

  Value v1(100);
  Value v2(200);
  Value s1("HelloDatabase");
  Value s2("HelloDatabasf"); // 最后一个字母不同，迫使 memcmp 扫到底

  {
    ScopedTimer timer("Integer Compare");
    volatile bool result = false; 
    for (int i = 0; i < iterations; ++i) {
      result = v1.CompareLessThan(v2);
    }
  }

  {
    ScopedTimer timer("Varchar Compare");
    volatile bool result = false;
    for (int i = 0; i < iterations; ++i) {
      result = s1.CompareLessThan(s2);
    }
  }
}

} // namespace bustub