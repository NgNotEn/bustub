#include "execution/table_scan_executor.h"

#include "storage/table/table_heap.h"

namespace bustub {

void TableScanExecutor::Init(ExecutionContext* exec_ctx) {
  Executor::Init(exec_ctx);

  // 从 catalog 中获取表信息
  auto table_info = exec_ctx->catalog_->GetTable(table_id_);
  if (!table_info) {
    return;
  }

  // 创建 TableHeap 程文
  table_heap_ = std::make_unique<TableHeap>(
      exec_ctx->catalog_->GetBPM(), table_id_, table_info->GetFirstPageId());

  // 创建迭代器，从表头开始
  iter_ = std::make_unique<TableHeap::TableIterator>(table_heap_->Begin());
}

bool TableScanExecutor::Next(Tuple* tuple) {
  if (!iter_ || !table_heap_) {
    return false;
  }

  // 判断是否到了表尾
  auto end_iter = table_heap_->End();
  if (*iter_ == end_iter) {
    return false;
  }

  // 获取当前记录
  if (tuple) {
    *tuple = **iter_;
  }

  // 移动到下一条
  ++(*iter_);

  return true;
}

}  // namespace bustub
