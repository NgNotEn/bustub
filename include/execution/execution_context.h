#pragma once

#include "catalog/catalog_manager.h"

namespace bustub {

/**
 * ExecutionContext 是每个 SQL 执行过程中的上下文。
 * 它为 Executor 提供库表管理器及其他执行时所需的资源。
 *
 * 作用：
 * - catalog_: 库表元数据管理，用于获取表 schema、流水号等信息
 */
struct ExecutionContext {
  CatalogManager* catalog_;

  explicit ExecutionContext(CatalogManager* catalog) : catalog_(catalog) {}
};

}  // namespace bustub
