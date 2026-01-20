#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace bustub {

// 异常类型枚举
enum class ExceptionType {
  INVALID = 0,           // 无效类型
  OUT_OF_RANGE = 1,      // 越界
  CONVERSION = 2,        // 类型转换错误
  UNKNOWN_TYPE = 3,      // 未知类型
  DECIMAL = 4,           // Decimal 类型错误
  MISMATCH_TYPE = 5,     // 类型不匹配
  DIVIDE_BY_ZERO = 6,    // 除以零
  OBJECT_SIZE = 7,       // 对象大小错误
  INCOMPLETE = 8,        // 未完成功能
  NOT_IMPLEMENTED = 9,   // 未实现
  EXECUTION = 10,        // 执行错误
  CATALOG = 11,          // Catalog 错误
  BLOCKER = 12           // 阻塞
};

class Exception : public std::runtime_error {
 public:
  // 构造函数 1: 仅消息
  explicit Exception(const std::string &message) 
      : std::runtime_error(message), type_(ExceptionType::INVALID) {
    std::cerr << "Exception: " << message << std::endl;
  }

  // 构造函数 2: 类型 + 消息
  Exception(ExceptionType type, const std::string &message)
      : std::runtime_error(message), type_(type) {
    std::cerr << "Exception: " << ExceptionTypeToString(type) << " : " << message << std::endl;
  }

  // 获取异常类型的字符串表示
  static std::string ExceptionTypeToString(ExceptionType type) {
    switch (type) {
      case ExceptionType::INVALID: return "Invalid";
      case ExceptionType::OUT_OF_RANGE: return "Out of Range";
      case ExceptionType::CONVERSION: return "Conversion";
      case ExceptionType::UNKNOWN_TYPE: return "Unknown Type";
      case ExceptionType::DECIMAL: return "Decimal";
      case ExceptionType::MISMATCH_TYPE: return "Mismatch Type";
      case ExceptionType::DIVIDE_BY_ZERO: return "Divide by Zero";
      case ExceptionType::OBJECT_SIZE: return "Object Size";
      case ExceptionType::INCOMPLETE: return "Incomplete";
      case ExceptionType::NOT_IMPLEMENTED: return "Not Implemented";
      case ExceptionType::EXECUTION: return "Execution";
      case ExceptionType::CATALOG: return "Catalog";
      case ExceptionType::BLOCKER: return "Blocker";
      default: return "Unknown";
    }
  }

 private:
  ExceptionType type_;
};

} // namespace bustub