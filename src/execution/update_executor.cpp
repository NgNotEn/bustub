#include "execution/update_executor.h"

#include "storage/table/table_heap.h"

namespace bustub {

void UpdateExecutor::Init(ExecutionContext* exec_ctx) {
  Executor::Init(exec_ctx);
}

bool UpdateExecutor::Next(Tuple* tuple) {
  if (updated_) {
    return false;
  }

  auto table_info = exec_ctx_->catalog_->GetTable(table_id_);
  if (!table_info) {
    return false;
  }

  TableHeap table_heap(exec_ctx_->catalog_->GetBPM(), table_id_,
                       table_info->GetFirstPageId());

  // 遍历所有行并更新
  auto iter = table_heap.Begin();
  auto end_iter = table_heap.End();

  while (iter != end_iter) {
    Tuple old_tuple = *iter;
    Tuple new_tuple(new_values_, const_cast<Schema*>(&table_info->GetSchema()));
    new_tuple.SetRid(old_tuple.GetRid());
    table_heap.UpdateTuple(new_tuple, old_tuple.GetRid());
    ++iter;
  }

  updated_ = true;
  return true;
}

}  // namespace bustub
