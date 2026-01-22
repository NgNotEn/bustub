#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <filesystem>

#include "buffer/buffer_pool_manager.h"
#include "storage/table/table_page.h"
#include "storage/table/tuple.h"
#include "catalog/schema.h"
#include "catalog/column.h"
#include "type/value.h"

namespace bustub {

class TablePageTest : public ::testing::Test {
 protected:
  // 对应你代码中的“0. 环境准备”
  void SetUp() override {
    if (std::filesystem::exists(db_file_)) {
      std::filesystem::remove(db_file_);
    }
    // 适配你的 BPM 构造函数（10个页，2是某种参数，db_file_路径）
    bpm_ = new BufferPoolManager(10, 2, db_file_);

    // 适配你的 Schema 定义
    std::vector<Column> cols;
    cols.emplace_back("id", TypeId::INTEGER);
    cols.emplace_back("name", TypeId::VARCHAR, 32); 
    schema_ = new Schema("test_table", cols);
  }

  void TearDown() override {
    delete bpm_;
    delete schema_;
    if (std::filesystem::exists(db_file_)) {
      std::filesystem::remove(db_file_);
    }
  }

  const std::string db_file_ = "test_table_page.db";
  BufferPoolManager *bpm_{nullptr};
  Schema *schema_{nullptr};
};

// =========================================================================
// 核心测试：完整迁移你提供的所有 Benchmark 逻辑
// =========================================================================
TEST_F(TablePageTest, ComprehensiveBenchmark) {
  std::cout << "=== [Start] TablePage Comprehensive Benchmark ===" << std::endl;

  // --- Test 1: 初始化与插入 ---
  Page *page = bpm_->NewPage();
  ASSERT_NE(page, nullptr); // 对应 assert(page != nullptr)
  page_id_t page_id = page->GetPageId();

  auto *table_page = reinterpret_cast<TablePage *>(page);
  
  // 完全保留你的 Init 参数类型
  table_page->Init(page_id, INVALID_PAGE_ID, INVALID_PAGE_ID);

  std::vector<RID> rids;
  int tuple_count = 0;

  for (int i = 0; i < 40; ++i) {
    Value v_id(i);
    Value v_name("val_" + std::to_string(i));
    Tuple tuple({v_id, v_name}, schema_); // 保留原参数 &schema (GTest里是成员指针)

    RID rid = table_page->InsertTuple(tuple);
    if (rid.GetPageId() == INVALID_PAGE_ID) {
      std::cout << "Page full unexpectedly at " << i << std::endl;
      break;
    }
    rids.push_back(rid);
    tuple_count++;
  }
  EXPECT_GT(tuple_count, 0);
  std::cout << "-> Inserted " << tuple_count << " tuples. Free space: " << table_page->GetFreeSpaceRemaining() << std::endl;

  // --- Test 2: 读取验证 ---
  for (int i = 0; i < tuple_count; ++i) {
    Tuple t = table_page->GetTuple(rids[i]);
    EXPECT_EQ(t.GetValue(schema_, 0).GetAsInteger(), i);
    EXPECT_EQ(t.GetValue(schema_, 1).ToString(), "val_" + std::to_string(i));
  }
  std::cout << "-> All tuples verified." << std::endl;

  // --- Test 3: 原地更新 ---
  RID update_rid = rids[0];
  Tuple old_tuple = table_page->GetTuple(update_rid);
  
  Value v_id_new(9999);
  Value v_name_new("new_0"); 
  Tuple new_tuple({v_id_new, v_name_new}, schema_);

  bool res = table_page->UpdateTuple(new_tuple, update_rid);
  EXPECT_TRUE(res); // 对应 assert(res == true)

  Tuple updated_t = table_page->GetTuple(update_rid);
  EXPECT_EQ(updated_t.GetValue(schema_, 0).GetAsInteger(), 9999);
  EXPECT_EQ(updated_t.GetValue(schema_, 1).ToString(), "new_0");
  std::cout << "-> Update In-Place success." << std::endl;

  // --- Test 4: 更新较长字符串 ---
  RID move_rid = rids[1];
  Tuple old_tuple_2 = table_page->GetTuple(move_rid);
  Value v_name_large("large_string_update_test"); 
  Tuple larger_tuple({old_tuple_2.GetValue(schema_, 0), v_name_large}, schema_);

  res = table_page->UpdateTuple(larger_tuple, move_rid);
  if (res) {
    Tuple t = table_page->GetTuple(move_rid);
    EXPECT_EQ(t.GetValue(schema_, 1).ToString(), "large_string_update_test");
    std::cout << "-> Update success (Value Updated Correctly)." << std::endl;
  }

  // --- Test 5: 删除 ---
  RID delete_rid = rids[2];
  table_page->MarkDeleted(delete_rid);
  Tuple deleted_t = table_page->GetTuple(delete_rid);
  EXPECT_EQ(deleted_t.GetStorageSize(), 0); 
  std::cout << "-> Delete success." << std::endl;

  // --- Test 6: 持久化验证 ---
  bpm_->UnpinPage(page_id, true);
  bpm_->FlushPage(page_id);

  Page *page_again = bpm_->FetchPage(page_id);
  ASSERT_NE(page_again, nullptr);
  auto *table_page_again = reinterpret_cast<TablePage *>(page_again);
  
  Tuple t_0 = table_page_again->GetTuple(rids[0]);
  EXPECT_EQ(t_0.GetValue(schema_, 1).ToString(), "new_0");

  Tuple t_2 = table_page_again->GetTuple(rids[2]);
  EXPECT_EQ(t_2.GetStorageSize(), 0);

  bpm_->UnpinPage(page_id, false);
  std::cout << "=== [Success] All Benchmark Tests Passed! ===" << std::endl;
}

} // namespace bustub