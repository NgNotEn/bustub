#pragma once

#include <memory>

#include "execution/executor.h"
#include "storage/table/table_heap.h"

namespace bustub {

/**
 * TableScanExecutor 实现整个表的无条件顺次扶描。
 * 它使用 TableHeap 的迭代器遚到每一条记录。
 */
class TableScanExecutor : public Executor {
 public:
  /**
   * @param table_id 要扶描的表 ID
   */
  explicit TableScanExecutor(table_id_t table_id) : table_id_(table_id) {}

  void Init(ExecutionContext* exec_ctx) override;

  bool Next(Tuple* tuple) override;

 private:
  table_id_t table_id_;
  std::unique_ptr<TableHeap> table_heap_;
  std::unique_ptr<TableHeap::TableIterator> iter_;
};

}  // namespace bustub
