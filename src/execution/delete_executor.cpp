#include "execution/delete_executor.h"

#include "storage/table/table_heap.h"

namespace bustub {

void DeleteExecutor::Init(ExecutionContext* exec_ctx) {
  Executor::Init(exec_ctx);
}

bool DeleteExecutor::Next(Tuple* tuple) {
  if (deleted_) {
    return false;
  }

  auto table_info = exec_ctx_->catalog_->GetTable(table_id_);
  if (!table_info) {
    return false;
  }

  TableHeap table_heap(exec_ctx_->catalog_->GetBPM(), table_id_,
                       table_info->GetFirstPageId());

  // 遍历所有行并标记删除
  auto iter = table_heap.Begin();
  auto end_iter = table_heap.End();

  while (iter != end_iter) {
    Tuple current_tuple = *iter;
    table_heap.MarkDeleted(current_tuple.GetRid());
    ++iter;
  }

  deleted_ = true;
  return true;
}

}  // namespace bustub
