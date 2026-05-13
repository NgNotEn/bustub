#pragma once

#include <memory>

#include "execution/executor.h"

namespace hsql {
struct Expr;
}

namespace bustub {

/**
 * FilterExecutor 对来自子执行器的行进行过滤。
 * 简化版：仅支持简单的比较条件（列 = 常数、列 < 常数等）
 * 暂不支持复杂的布尔操作（AND、OR）
 */
class FilterExecutor : public Executor {
 public:
  FilterExecutor(std::unique_ptr<Executor> child, hsql::Expr* filter_expr,
                 const Schema* schema)
    : child_(std::move(child)), filter_expr_(filter_expr), schema_(schema) {}

  void Init(ExecutionContext* exec_ctx) override;

  bool Next(Tuple* tuple) override;

 private:
  // 评估过滤表达式对给定元组是否为真
  bool EvaluateFilter(const Tuple& tuple);

  std::unique_ptr<Executor> child_;
  hsql::Expr* filter_expr_;  // WHERE 条件
  const Schema* schema_;
};

}  // namespace bustub
