#include "type/type.h"

#include "type/integer_type.h"
#include "type/type_id.h"
#include "type/varchar_type.h"

namespace bustub {
  // 根据类型 ID 派出对应的类型处理器(处理器单例复用即可)
Type* Type::GetInstance(TypeId type_id) {
  static IntegerType kIntegerType;
  static VarCharType kVarcharType;

  switch (type_id) {
    case TypeId::INTEGER:
      return &kIntegerType;
    case TypeId::VARCHAR:
      return &kVarcharType;
    default:
      return nullptr;
  }
}
}  // namespace bustub
