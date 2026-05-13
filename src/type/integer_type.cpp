#include "type/integer_type.h"

#include <cstring>

namespace bustub {

// constructor
IntegerType::IntegerType() : Type(TypeId::INTEGER) {
  // nothing
}

// 获取Value内对应整数值，将整数值ccopy到对应内存中
void IntegerType::SerializeTo(const Value &val, char *storage) const {
  _integer raw_val = val.GetAsInteger();
  memcpy(storage, &raw_val, sizeof(_integer));
}

// 将对应内存的值封装为Value
Value IntegerType::DeserializeFrom(const char *storage) const {
  _integer raw_val;
  memcpy(&raw_val, storage, sizeof(_integer));
  return Value(raw_val);
}

bool IntegerType::CompareEquals(const Value &left,
                                const Value &right) const {
  return left.GetAsInteger() == right.GetAsInteger();
}

bool IntegerType::CompareLessThan(const Value &left,
                                  const Value &right) const {
  return left.GetAsInteger() < right.GetAsInteger();
}

std::string IntegerType::ToString(const Value &val) const {
  return std::to_string(val.GetAsInteger());
}

}  // namespace bustub