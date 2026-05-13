#pragma once

#include "execution/executor.h"

namespace bustub {

/**
 * DeleteExecutor 删除表中满足 WHERE 条件的行。
 * 支持：
 * - DELETE FROM table          （删除所有行，无 WHERE）
 * - DELETE FROM table WHERE ... （删除符合条件的行）
 *
 * 简化版：暂时仅支持无条件删除（expr == nullptr）。
 * WHERE 条件解析需要完整的表达式求值器，后续实现。
 */
class DeleteExecutor : public Executor {
 public:
  explicit DeleteExecutor(table_id_t table_id) : table_id_(table_id) {}

  void Init(ExecutionContext* exec_ctx) override;

  bool Next(Tuple* tuple) override;

 private:
  table_id_t table_id_;
  bool deleted_ = false;
};

}  // namespace bustub
