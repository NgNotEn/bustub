#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <random>
#include <string>
#include <set>
#include <filesystem>

#include "buffer/buffer_pool_manager.h"
#include "storage/table/table_heap.h"
#include "storage/table/tuple.h"
#include "catalog/schema.h"
#include "catalog/column.h"
#include "type/value.h"

namespace bustub {

class TableHeapTest : public ::testing::Test {
 protected:
  void SetUp() override {
    if (std::filesystem::exists(db_file_)) {
      std::filesystem::remove(db_file_);
    }
    bpm_ = new BufferPoolManager(50, 2, db_file_);
    
    std::vector<Column> cols;
    cols.emplace_back("id", TypeId::INTEGER);
    cols.emplace_back("name", TypeId::VARCHAR, 64);
    schema_ = new Schema("test_table", cols);
  }

  void TearDown() override {
    delete bpm_;
    delete schema_;
    if (std::filesystem::exists(db_file_)) {
      std::filesystem::remove(db_file_);
    }
  }

  Tuple MakeTuple(int id, const std::string &name) {
    Value v_id(id);
    Value v_name(name);
    return Tuple({v_id, v_name}, schema_);
  }

  const std::string db_file_ = "test_table_heap.db";
  BufferPoolManager *bpm_{nullptr};
  Schema *schema_{nullptr};
};

// ============================================================================
// 测试 1: 基本 CRUD 操作
// ============================================================================
TEST_F(TableHeapTest, BasicCRUDTest) {
  TableHeap heap(bpm_);
  
  // Insert
  std::vector<RID> rids;
  for (int i = 0; i < 100; ++i) {
    Tuple tuple = MakeTuple(i, "user_" + std::to_string(i));
    RID rid = heap.InsertTuple(tuple);
    ASSERT_NE(rid.GetPageId(), INVALID_PAGE_ID) << "Insert failed at " << i;
    rids.push_back(rid);
  }
  
  // Read & Verify
  for (int i = 0; i < 100; ++i) {
    Tuple tuple = heap.GetTuple(rids[i]);
    EXPECT_EQ(tuple.GetValue(schema_, 0).GetAsInteger(), i);
    EXPECT_EQ(tuple.GetValue(schema_, 1).ToString(), "user_" + std::to_string(i));
  }
  
  // Update
  for (int i = 0; i < 50; ++i) {
    Tuple new_tuple = MakeTuple(i + 1000, "updated_" + std::to_string(i));
    bool ok = heap.UpdateTuple(new_tuple, rids[i]);
    EXPECT_TRUE(ok) << "Update failed at " << i;
  }
  
  // Verify Update
  for (int i = 0; i < 50; ++i) {
    Tuple tuple = heap.GetTuple(rids[i]);
    EXPECT_EQ(tuple.GetValue(schema_, 0).GetAsInteger(), i + 1000);
    EXPECT_EQ(tuple.GetValue(schema_, 1).ToString(), "updated_" + std::to_string(i));
  }
  
  // Delete
  for (int i = 50; i < 100; ++i) {
    bool ok = heap.MarkDeleted(rids[i]);
    EXPECT_TRUE(ok) << "Delete failed at " << i;
  }
  
  // Verify Delete
  for (int i = 50; i < 100; ++i) {
    Tuple tuple = heap.GetTuple(rids[i]);
    EXPECT_EQ(tuple.GetStorageSize(), 0) << "Deleted tuple should be empty";
  }
}

// ============================================================================
// 测试 2: 迭代器测试
// ============================================================================
TEST_F(TableHeapTest, IteratorTest) {
  TableHeap heap(bpm_);
  
  // 插入数据
  const int num_tuples = 200;
  for (int i = 0; i < num_tuples; ++i) {
    Tuple tuple = MakeTuple(i, "iter_test_" + std::to_string(i));
    heap.InsertTuple(tuple);
  }
  
  // 使用迭代器遍历并计数
  int count = 0;
  for (auto it = heap.Begin(); it != heap.End(); ++it) {
    EXPECT_GT((*it).GetStorageSize(), 0);
    count++;
  }
  EXPECT_EQ(count, num_tuples);
  
  // 删除部分数据后再遍历
  auto it = heap.Begin();
  std::vector<RID> to_delete;
  int del_count = 0;
  for (; it != heap.End() && del_count < 50; ++it, ++del_count) {
    to_delete.push_back((*it).GetRid());
  }
  for (const auto &rid : to_delete) {
    heap.MarkDeleted(rid);
  }
  
  // 重新遍历，确认跳过了删除的记录
  count = 0;
  for (auto it2 = heap.Begin(); it2 != heap.End(); ++it2) {
    count++;
  }
  EXPECT_EQ(count, num_tuples - 50);
}

// ============================================================================
// 测试 3: 跨页插入测试 (强制分配多个页)
// ============================================================================
TEST_F(TableHeapTest, MultiPageTest) {
  TableHeap heap(bpm_);
  
  // 插入大量数据，强制跨多个页
  const int num_tuples = 500;
  std::vector<RID> rids;
  
  for (int i = 0; i < num_tuples; ++i) {
    // 使用适当长度的字符串（不超过 64 - 4 = 60 字节）
    std::string name = "user_" + std::to_string(i);
    Tuple tuple = MakeTuple(i, name);
    RID rid = heap.InsertTuple(tuple);
    ASSERT_NE(rid.GetPageId(), INVALID_PAGE_ID);
    rids.push_back(rid);
  }
  
  // 确认确实跨了多个页
  std::set<page_id_t> pages_used;
  for (const auto &rid : rids) {
    pages_used.insert(rid.GetPageId());
  }
  EXPECT_GT(pages_used.size(), 1) << "Should span multiple pages";
  std::cout << "-> Used " << pages_used.size() << " pages for " << num_tuples << " tuples" << std::endl;
  
  // 验证所有数据都能正确读取
  for (int i = 0; i < num_tuples; ++i) {
    Tuple tuple = heap.GetTuple(rids[i]);
    EXPECT_EQ(tuple.GetValue(schema_, 0).GetAsInteger(), i);
  }
}

// ============================================================================
// 测试 4: 持久化与恢复测试
// ============================================================================
TEST_F(TableHeapTest, PersistenceTest) {
  page_id_t first_page_id;
  std::vector<RID> rids;
  
  // 第一阶段：插入数据
  {
    TableHeap heap(bpm_);
    first_page_id = heap.InsertTuple(MakeTuple(0, "dummy")).GetPageId();
    
    for (int i = 0; i < 50; ++i) {
      Tuple tuple = MakeTuple(i, "persist_" + std::to_string(i));
      rids.push_back(heap.InsertTuple(tuple));
    }
  }
  
  // 销毁 BPM 触发刷盘
  delete bpm_;
  
  // 第二阶段：重新打开并验证
  bpm_ = new BufferPoolManager(50, 2, db_file_);
  {
    TableHeap heap(bpm_, first_page_id);
    
    // 验证数据
    for (int i = 0; i < 50; ++i) {
      Tuple tuple = heap.GetTuple(rids[i]);
      EXPECT_EQ(tuple.GetValue(schema_, 0).GetAsInteger(), i);
      EXPECT_EQ(tuple.GetValue(schema_, 1).ToString(), "persist_" + std::to_string(i));
    }
  }
}

// ============================================================================
// 测试 5: 空表测试
// ============================================================================
TEST_F(TableHeapTest, EmptyTableTest) {
  TableHeap heap(bpm_);
  
  // 空表迭代器
  auto begin = heap.Begin();
  auto end = heap.End();
  EXPECT_EQ(begin, end);
  
  // 插入一条然后删除
  Tuple tuple = MakeTuple(1, "temp");
  RID rid = heap.InsertTuple(tuple);
  heap.MarkDeleted(rid);
  
  // 应该还是空的
  begin = heap.Begin();
  EXPECT_EQ(begin, heap.End());
}

}  // namespace bustub
