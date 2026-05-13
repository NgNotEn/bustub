/*
  整数类型实现
*/

#pragma once
#include <cstdint>
#include "type/type.h"
#include "type/value.h"

namespace bustub {

class IntegerType : public Type {
  using _integer = int32_t;
 public:
  IntegerType();

  // int 2 byte
  void SerializeTo(const Value &val, char *storage) const override;
  // byte 2 int
  Value DeserializeFrom(const char *storage) const override;

  bool CompareEquals(const Value &left,
                     const Value &right) const override;

  bool CompareLessThan(const Value &left,
                       const Value &right) const override;

  std::string ToString(const Value &val) const override;
};

}  // namespace bustub