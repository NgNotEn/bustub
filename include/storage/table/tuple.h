/*
  一条记录的运行时对象
*/

#pragma once
#include <cstdint>

#include "catalog/schema.h"
#include "common/rid.h"
#include "type/value.h"

namespace bustub {
class Tuple {
 public:
  // defalt
  Tuple();
  // cp
  Tuple(std::vector<Value> values, Schema *schema);
  Tuple(RID rid, char *data, uint32_t size);
  Tuple(const Tuple &other);
  Tuple& operator=(const Tuple &other);
  // move
  Tuple(Tuple &&other);
  Tuple& operator=(Tuple &&other);

  ~Tuple();

  Value GetValue(const Schema *schema, uint32_t column_idx) const;

  inline RID GetRid() const { return rid_; }
  inline void SetRid(RID rid) { rid_ = rid; }
  inline uint32_t GetStorageSize() const { return storage_size_; };
  inline char *GetData() const { return data_; }

 private:
  bool is_allocated_;
  uint32_t storage_size_;
  RID rid_;
  char *data_;    // 所有数据的字节流
};
}  // namespace bustub