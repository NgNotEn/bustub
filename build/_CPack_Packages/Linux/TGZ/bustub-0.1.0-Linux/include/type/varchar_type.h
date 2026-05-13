#pragma once
#include "type/type.h"
#include "type/value.h"

namespace bustub {

class VarCharType : public Type {
 public:
  VarCharType();

  // value 2 byte, 不仅仅是数据，还有元数据(4byte的长度头)
  void SerializeTo(const Value &val, char *storage) const override;
  // byte 2 value, 不仅仅是数据，还有元数据(4byte的长度头)
  Value DeserializeFrom(const char *storage) const override;

  bool CompareEquals(const Value &left,
                     const Value &right) const override;

  bool CompareLessThan(const Value &left,
                       const Value &right) const override;

  std::string ToString(const Value &val) const override;
};

}  // namespace bustub