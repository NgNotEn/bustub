#pragma once

#include <vector>

#include "execution/executor.h"
#include "type/value.h"

namespace bustub {

/**
 * UpdateExecutor 更新表中的行。
 * 简化版：使用新值更新表中的所有行（无 WHERE 过滤）。
 */
class UpdateExecutor : public Executor {
 public:
  UpdateExecutor(table_id_t table_id, std::vector<Value> new_values)
    : table_id_(table_id), new_values_(new_values) {}

  void Init(ExecutionContext* exec_ctx) override;

  bool Next(Tuple* tuple) override;

 private:
  table_id_t table_id_;
  std::vector<Value> new_values_;
  bool updated_ = false;
};

}  // namespace bustub
