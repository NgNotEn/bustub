/*
  类型操作集，定义行为，不存储数据，使用GetInstance获取对应的类型处理器

  Type系统使用时不直接使用Type，而是使用Value转发
*/

#pragma once
#include <string>

#include "type/type_id.h"

namespace bustub {
class Value;
// basic type
class Type {
 public:
  virtual ~Type() = default;

  // 单例工厂：根据 TypeId 拿到对应的 Type 处理器
  static Type * GetInstance(TypeId type_id);

  /*
    定义type的行为，但不具体实现，将所有行为分发到具体type实现
  */

  // 序列化与反序列化
  // 把 value 序列化后 放进 storage 里
  virtual void SerializeTo(const Value &val, char *storage) const = 0;
  // 从 storage 取出数据还原成 val
  virtual Value DeserializeFrom(const char *storage) const = 0;

  // compare
  virtual bool CompareEquals(const Value &left, const Value &right) const = 0;
  virtual bool CompareLessThan(const Value &left, const Value &right) const = 0;  // for b+tree

  // debug
  virtual std::string ToString(const Value &val) const = 0;

  // 判断是否变长
  inline bool IsVariableLength() const { return type_id_ == VARCHAR; }
  // 获取 TypeId
  inline TypeId GetTypeId() const { return type_id_; }

 protected:
  TypeId type_id_;
  // 构造函数 protected，强制单例，不允许外部随便 new Type()
  explicit Type(TypeId type_id) : type_id_(type_id) {}
};

}  // namespace bustub