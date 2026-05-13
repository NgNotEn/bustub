/*
  数据容器,所有类型都用这个存，使用union复用内存
*/

#pragma once
#include <cstdint>
#include <cstring>
#include <string>

#include "type/type.h"
#include "type/type_id.h"



namespace bustub {
class Type;
class Value {
 public:
  // 整数构造
  Value(const int32_t integer);
  // 字符串构造
  Value(const std::string str);
  // destructor
  ~Value();

  // 拷贝构造
  Value(const Value &other);
  Value & operator=(const Value &other);

  // = data access =
  inline int32_t GetAsInteger() const { return value_.integer_; }
  inline const char *GetAsVarChar() const { return value_.varchar_; }
  inline uint32_t GetLogicLength() const { return logic_len_; }   // 字符串长度
  inline TypeId GetTypeId() const { return type_id_; }            
  inline uint32_t GetStorageSize() const { return storage_size_; }  // 可变长数据的最大上限(初始分配)
  inline bool IsNull() const { return is_null_; }

  // == core apis ==
  // trans value to storage
  void SerializeTo(char *storage) const;
  // trans storage to value
  static Value DeserializeFrom(const char *storage, TypeId type_id);
  // compare
  bool CompareEquals(const Value &other) const;
  bool CompareLessThan(const Value &other) const;
  // debug
  std::string ToString() const;

 private:
  TypeId type_id_;
  uint32_t storage_size_;  // bytes
  uint32_t logic_len_;     // for varchar
  bool is_null_;

  union Val {
    int32_t integer_;
    char *varchar_;  // varchar pointer
  } value_;
};
}  // namespace bustub