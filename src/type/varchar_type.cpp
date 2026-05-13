#include "type/varchar_type.h"

#include <algorithm>
#include <cstring>

namespace bustub {

// constructor
VarCharType::VarCharType() : Type(TypeId::VARCHAR) {
  // nothing
}

auto VarCharType::SerializeTo(const Value &val, char *storage) const -> void {
  auto len = val.GetLogicLength();
  memcpy(storage, &len, sizeof(len));

  memcpy(storage + sizeof(len), val.GetAsVarChar(), val.GetLogicLength());
}

auto VarCharType::DeserializeFrom(const char *storage) const -> Value {
  uint32_t len;
  memcpy(&len, storage, sizeof(uint32_t));           // 读出长度头
  std::string str(storage + sizeof(uint32_t), len);  // 根据长度读出数据
  return Value(str);
}

auto VarCharType::CompareEquals(const Value &left,
                               const Value &right) const -> bool {
  uint32_t len1 = left.GetLogicLength();
  uint32_t len2 = right.GetLogicLength();

  if (len1 != len2) {
    return false;
  }
  return std::memcmp(left.GetAsVarChar(), right.GetAsVarChar(), len1) == 0;
}

auto VarCharType::CompareLessThan(const Value &left,
                                 const Value &right) const -> bool {
  uint32_t len1 = left.GetLogicLength();
  uint32_t len2 = right.GetLogicLength();
  const char *str1 = left.GetAsVarChar();
  const char *str2 = right.GetAsVarChar();

  // 找出最短长度，只比较公共部分
  uint32_t min_len = std::min(len1, len2);
  int cmp_result = std::memcmp(str1, str2, min_len);

  if (cmp_result != 0) {
    return cmp_result < 0;
  }

  // 此时长度短的那个更小
  return len1 < len2;
}

auto VarCharType::ToString(const Value &val) const -> std::string {
  return std::string(val.GetAsVarChar(), val.GetLogicLength());
}

}  // namespace bustub