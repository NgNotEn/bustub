#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "storage/table/tuple.h"
#include "catalog/schema.h"
#include "catalog/column.h"
#include "type/value.h"

namespace bustub {

class TupleTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::vector<Column> cols;
    cols.emplace_back("id", TypeId::INTEGER);
    cols.emplace_back("name", TypeId::VARCHAR, 32);
    schema_ = new Schema("test", cols);
  }

  void TearDown() override {
    delete schema_;
  }

  Schema *schema_{nullptr};
};

// ============================================================================
// 测试 1: 基本构造与序列化/反序列化
// ============================================================================
TEST_F(TupleTest, BasicSerializationTest) {
  Value v_id(42);
  Value v_name("Alice");
  
  Tuple tuple({v_id, v_name}, schema_);
  
  // 验证反序列化
  EXPECT_EQ(tuple.GetValue(schema_, 0).GetAsInteger(), 42);
  EXPECT_EQ(tuple.GetValue(schema_, 1).ToString(), "Alice");
}

// ============================================================================
// 测试 2: 拷贝构造与赋值
// ============================================================================
TEST_F(TupleTest, CopyTest) {
  Value v_id(123);
  Value v_name("Charlie");
  
  Tuple original({v_id, v_name}, schema_);
  
  // 拷贝构造
  Tuple copy1(original);
  EXPECT_EQ(copy1.GetValue(schema_, 0).GetAsInteger(), 123);
  EXPECT_EQ(copy1.GetValue(schema_, 1).ToString(), "Charlie");
  
  // 拷贝赋值
  Tuple copy2;
  copy2 = original;
  EXPECT_EQ(copy2.GetValue(schema_, 0).GetAsInteger(), 123);
  EXPECT_EQ(copy2.GetValue(schema_, 1).ToString(), "Charlie");
  
  // 确认深拷贝
  EXPECT_NE(original.GetData(), copy1.GetData());
  EXPECT_NE(original.GetData(), copy2.GetData());
}

// ============================================================================
// 测试 3: 移动构造与赋值
// ============================================================================
TEST_F(TupleTest, MoveTest) {
  Value v_id(999);
  Value v_name("David");
  
  Tuple original({v_id, v_name}, schema_);
  char *original_data = original.GetData();
  
  // 移动构造
  Tuple moved(std::move(original));
  EXPECT_EQ(moved.GetData(), original_data);
  EXPECT_EQ(original.GetData(), nullptr);
  
  EXPECT_EQ(moved.GetValue(schema_, 0).GetAsInteger(), 999);
  EXPECT_EQ(moved.GetValue(schema_, 1).ToString(), "David");
  
  // 移动赋值
  Tuple another({Value(1), Value("temp")}, schema_);
  char *moved_data = moved.GetData();
  another = std::move(moved);
  EXPECT_EQ(another.GetData(), moved_data);
  EXPECT_EQ(moved.GetData(), nullptr);
}

// ============================================================================
// 测试 4: 空 Tuple 处理
// ============================================================================
TEST_F(TupleTest, EmptyTupleTest) {
  Tuple empty;
  EXPECT_EQ(empty.GetStorageSize(), 0);
  EXPECT_EQ(empty.GetData(), nullptr);
  
  // 拷贝空 Tuple
  Tuple copy(empty);
  EXPECT_EQ(copy.GetStorageSize(), 0);
  EXPECT_EQ(copy.GetData(), nullptr);
  
  // 赋值空 Tuple
  Tuple assigned;
  assigned = empty;
  EXPECT_EQ(assigned.GetStorageSize(), 0);
  EXPECT_EQ(assigned.GetData(), nullptr);
  
  // 移动空 Tuple
  Tuple moved(std::move(empty));
  EXPECT_EQ(moved.GetStorageSize(), 0);
}

// ============================================================================
// 测试 5: 自赋值安全性
// ============================================================================
TEST_F(TupleTest, SelfAssignmentTest) {
  Value v_id(111);
  Value v_name("Eve");
  
  Tuple tuple({v_id, v_name}, schema_);
  char *data_before = tuple.GetData();
  
  tuple = tuple;
  
  EXPECT_EQ(tuple.GetData(), data_before);
  EXPECT_EQ(tuple.GetValue(schema_, 0).GetAsInteger(), 111);
  EXPECT_EQ(tuple.GetValue(schema_, 1).ToString(), "Eve");
}

// ============================================================================
// 测试 6: 不同长度 VARCHAR
// ============================================================================
TEST_F(TupleTest, VarcharLengthTest) {
  Value v_id1(1);
  Value v_name1("A");
  Tuple tuple1({v_id1, v_name1}, schema_);
  EXPECT_EQ(tuple1.GetValue(schema_, 1).ToString(), "A");
  
  Value v_id2(2);
  Value v_name2("Hello World");
  Tuple tuple2({v_id2, v_name2}, schema_);
  EXPECT_EQ(tuple2.GetValue(schema_, 1).ToString(), "Hello World");
  
  // 注意：VARCHAR(32) 的存储大小包含 4 字节长度前缀，实际字符串最多 28 字节
  Value v_id3(3);
  Value v_name3("ABCDEFGHIJKLMNOPQRSTUVWXYZ");  // 26 chars
  Tuple tuple3({v_id3, v_name3}, schema_);
  EXPECT_EQ(tuple3.GetValue(schema_, 1).ToString(), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

// ============================================================================
// 测试 7: RID 设置与获取
// ============================================================================
TEST_F(TupleTest, RidTest) {
  Value v_id(1);
  Value v_name("Test");
  
  Tuple tuple({v_id, v_name}, schema_);
  
  RID default_rid = tuple.GetRid();
  EXPECT_EQ(default_rid.GetPageId(), INVALID_PAGE_ID);
  
  RID new_rid(10, 5);
  tuple.SetRid(new_rid);
  EXPECT_EQ(tuple.GetRid().GetPageId(), 10);
  EXPECT_EQ(tuple.GetRid().GetSlotId(), 5);
}

// ============================================================================
// 测试 8: 多 Tuple 场景
// ============================================================================
TEST_F(TupleTest, MultipleTuplesTest) {
  std::vector<Tuple> tuples;
  
  for (int i = 0; i < 100; ++i) {
    Value v_id(i);
    Value v_name("user_" + std::to_string(i));
    tuples.emplace_back(Tuple({v_id, v_name}, schema_));
  }
  
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(tuples[i].GetValue(schema_, 0).GetAsInteger(), i);
    EXPECT_EQ(tuples[i].GetValue(schema_, 1).ToString(), "user_" + std::to_string(i));
  }
}

}  // namespace bustub
