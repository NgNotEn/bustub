#pragma once

#include <vector>

#include "execution/executor.h"
#include "type/value.h"

namespace bustub {

/**
 * InsertExecutor 向表中插入一行。
 */
class InsertExecutor : public Executor {
 public:
  InsertExecutor(table_id_t table_id, std::vector<Value> values)
    : table_id_(table_id), values_(values) {}

  void Init(ExecutionContext* exec_ctx) override;

  bool Next(Tuple* tuple) override;

 private:
  table_id_t table_id_;
  std::vector<Value> values_;
  bool inserted_ = false;
};

}  // namespace bustub
