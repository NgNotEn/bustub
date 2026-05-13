#include "execution/insert_executor.h"

#include "storage/table/table_heap.h"

namespace bustub {

void InsertExecutor::Init(ExecutionContext* exec_ctx) {
  Executor::Init(exec_ctx);
}

bool InsertExecutor::Next(Tuple* tuple) {
  if (inserted_) {
    return false;
  }

  auto table_info = exec_ctx_->catalog_->GetTable(table_id_);
  if (!table_info) {
    return false;
  }

  TableHeap table_heap(exec_ctx_->catalog_->GetBPM(), table_id_,
                       table_info->GetFirstPageId());

  // 创建 tuple 并插入
  Tuple insert_tuple(values_, const_cast<Schema*>(&table_info->GetSchema()));
  RID rid = table_heap.InsertTuple(insert_tuple);

  if (tuple) {
    *tuple = insert_tuple;
  }

  inserted_ = true;
  return true;
}

}  // namespace bustub
