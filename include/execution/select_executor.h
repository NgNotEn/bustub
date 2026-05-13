#pragma once

#include <memory>

#include "execution/executor.h"

namespace bustub {

/**
 * SelectExecutor 执行 SELECT 语句。
 * 简化版：仅支持 SELECT * FROM table [WHERE ...]
 * 使用 TableScanExecutor + FilterExecutor 的组合
 */
class SelectExecutor : public Executor {
 public:
  explicit SelectExecutor(std::unique_ptr<Executor> source)
    : source_(std::move(source)) {}

  void Init(ExecutionContext* exec_ctx) override;

  bool Next(Tuple* tuple) override;

 private:
  std::unique_ptr<Executor> source_;  // TableScan 或 Filter 包装的执行器
};

}  // namespace bustub
