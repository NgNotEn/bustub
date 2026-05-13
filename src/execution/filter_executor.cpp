#include "execution/filter_executor.h"

#include "catalog/schema.h"
#include "sql/Expr.h"
#include "storage/table/tuple.h"

namespace bustub {

void FilterExecutor::Init(ExecutionContext* exec_ctx) {
  Executor::Init(exec_ctx);
  child_->Init(exec_ctx);
}

bool FilterExecutor::Next(Tuple* tuple) {
  while (child_->Next(tuple)) {
    if (EvaluateFilter(*tuple)) {
      return true;
    }
  }
  return false;
}

bool FilterExecutor::EvaluateFilter(const Tuple& tuple) {
  if (filter_expr_ == nullptr) {
    return true;  // 无过滤条件，接受所有行
  }

  // 简化版：仅支持基本的二元操作符 (=, <, >, <=, >=, !=)
  if (filter_expr_->type != hsql::kExprOperator) {
    return true;  // 非操作符表达式，暂不支持
  }

  // 期望形式：列 op 值（或值 op 列）
  hsql::Expr* left = filter_expr_->expr;
  hsql::Expr* right = filter_expr_->expr2;

  if (left == nullptr || right == nullptr) {
    return true;  // 不完整的表达式
  }

  // 提取列索引和值
  int col_idx = -1;
  Value compare_val(0);  // 默认整数值
  bool is_left_column = false;

  // 检查左边是否是列引用
  if (left->type == hsql::kExprColumnRef && left->name != nullptr) {
    is_left_column = true;
    // 查找列索引
    for (uint32_t i = 0; i < schema_->GetColumnCount(); i++) {
      if (strcmp(schema_->GetColumn(i).GetName().c_str(), left->name) == 0) {
        col_idx = i;
        break;
      }
    }
    // 评估右边值
    if (right->type == hsql::kExprLiteralInt) {
      compare_val = Value(static_cast<int32_t>(right->ival));
    } else if (right->type == hsql::kExprLiteralString &&
               right->name != nullptr) {
      compare_val = Value(std::string(right->name));
    } else {
      return true;  // 无法评估右边值
    }
  }
  // 检查右边是否是列引用
  else if (right->type == hsql::kExprColumnRef && right->name != nullptr) {
    is_left_column = false;
    // 查找列索引
    for (uint32_t i = 0; i < schema_->GetColumnCount(); i++) {
      if (strcmp(schema_->GetColumn(i).GetName().c_str(), right->name) == 0) {
        col_idx = i;
        break;
      }
    }
    // 评估左边值
    if (left->type == hsql::kExprLiteralInt) {
      compare_val = Value(static_cast<int32_t>(left->ival));
    } else if (left->type == hsql::kExprLiteralString &&
               left->name != nullptr) {
      compare_val = Value(std::string(left->name));
    } else {
      return true;  // 无法评估左边值
    }
  } else {
    return true;  // 不支持的表达式形式
  }

  if (col_idx < 0) {
    return true;  // 找不到列
  }

  Value col_val = tuple.GetValue(schema_, col_idx);

  // 比较
  switch (filter_expr_->opType) {
    case hsql::kOpEquals:
      return col_val.CompareEquals(compare_val);
    case hsql::kOpNotEquals:
      return !col_val.CompareEquals(compare_val);
    case hsql::kOpLess:
      return col_val.CompareLessThan(compare_val);
    case hsql::kOpLessEq:
      return col_val.CompareLessThan(compare_val) ||
             col_val.CompareEquals(compare_val);
    case hsql::kOpGreater:
      return !col_val.CompareLessThan(compare_val) &&
             !col_val.CompareEquals(compare_val);
    case hsql::kOpGreaterEq:
      return !col_val.CompareLessThan(compare_val);
    default:
      return true;  // 不支持的操作符
  }
}

}  // namespace bustub
