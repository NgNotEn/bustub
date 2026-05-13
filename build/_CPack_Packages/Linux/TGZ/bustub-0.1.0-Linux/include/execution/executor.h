#pragma once

#include "execution/execution_context.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * Executor 是火山模型中所有算子的基类。
 * 每个算子实现一个Pull-based的流水线：
 *   Init() -> 初始化资源
 *   Next(tuple) -> 获取下一条记录（返回 true/false 表示是否有记录）
 */
class Executor {
 public:
  virtual ~Executor() = default;

  /**
   * 初始化 Executor，传入执行上下文。
   */
  virtual void Init(ExecutionContext* exec_ctx) { exec_ctx_ = exec_ctx; }

  /**
   * 获取下一条记录。
   * @param tuple 会被设置为下一条记录
   * @return true 表示成功获取，false 表示没有更多记录了
   */
  virtual bool Next(Tuple* tuple) = 0;

 protected:
  ExecutionContext* exec_ctx_ = nullptr;
};

}  // namespace bustub
