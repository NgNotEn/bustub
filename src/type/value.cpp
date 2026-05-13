#include "type/value.h"

#include <cstring>

#include "type/type_id.h"

namespace bustub {

// 构造
Value::Value(const int32_t integer)
    : value_({integer}),
      storage_size_(4),
      type_id_(TypeId::INTEGER),
      logic_len_(0) {}
Value::Value(const std::string str)
    : type_id_(TypeId::VARCHAR), logic_len_(str.size()) {
  value_.varchar_ = new char[logic_len_ + 1];
  std::memcpy(value_.varchar_, str.c_str(), logic_len_);
  value_.varchar_[logic_len_] = '\0';
  storage_size_ = logic_len_ + 4;  // 4 bytes union area + allocated heap data
}

// 深拷贝
Value::Value(const Value &other) {
  if (this == &other) {
    return;
  }
  type_id_ = other.type_id_;
  storage_size_ = other.storage_size_;
  logic_len_ = other.logic_len_;
  if (type_id_ == VARCHAR) {
    value_.varchar_ = new char[logic_len_ + 1];
    memcpy(value_.varchar_, other.value_.varchar_, logic_len_ + 1);
  } else {
    value_.integer_ = other.value_.integer_;
  }
}
Value& Value::operator=(const Value &other) {
  // 防止自赋值 (a = a)
  if (this == &other) {
    return *this;
  }

  // 如果自己之前有堆内存，先释放！(防止内存泄漏)
  if (type_id_ == TypeId::VARCHAR && value_.varchar_ != nullptr) {
    delete[] value_.varchar_;
  }

  // 复制元数据
  type_id_ = other.type_id_;
  storage_size_ = other.storage_size_;
  logic_len_ = other.logic_len_;

  // 深拷贝数据
  if (type_id_ == TypeId::VARCHAR) {
    value_.varchar_ = new char[logic_len_ + 1];
    std::memcpy(value_.varchar_, other.value_.varchar_, logic_len_ + 1);
  } else {
    value_.integer_ = other.value_.integer_;
  }
  return *this;
}

// 析构
Value::~Value() {
  if (type_id_ == VARCHAR && value_.varchar_ != nullptr) {
    delete[] value_.varchar_;
  }
}

// === 核心逻辑：分发给单例Type ===
void Value::SerializeTo(char *storage) const {
  Type::GetInstance(type_id_)->SerializeTo(*this, storage);
}

Value Value::DeserializeFrom(const char *storage, TypeId type_id) {
  return Type::GetInstance(type_id)->DeserializeFrom(storage);
}

bool Value::CompareEquals(const Value &other) const {
  return Type::GetInstance(type_id_)->CompareEquals(*this, other);
}

bool Value::CompareLessThan(const Value &other) const {
  return Type::GetInstance(type_id_)->CompareLessThan(*this, other);
}

std::string Value::ToString() const {
  return Type::GetInstance(type_id_)->ToString(*this);
}

}  // namespace bustub