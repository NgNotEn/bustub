#include "execution/select_executor.h"

namespace bustub {

void SelectExecutor::Init(ExecutionContext* exec_ctx) {
  Executor::Init(exec_ctx);
  source_->Init(exec_ctx);
}

bool SelectExecutor::Next(Tuple* tuple) {
  return source_->Next(tuple);
}

}  // namespace bustub
