#pragma once

#include "buffer/buffer_pool_manager.h"
#include "catalog/catalog_meta.h"
#include "catalog/table_info.h"
#include "common/config.h"
#include "storage/table/table_heap.h"

namespace bustub {

class CatalogManager {
 public:
  explicit CatalogManager(BufferPoolManager* bpm, DiskManager* disk_manager,
                          const std::string& meta_path);
  ~CatalogManager();

  // Create table: allocate page, create heap, register
  TableInfo* CreateTable(const std::string& name, const Schema& schema);

  // Get table
  TableInfo* GetTable(const std::string& name);
  TableInfo* GetTable(table_id_t tb_id);

  // Drop table
  bool DropTable(const std::string& name);
  bool DropTable(table_id_t tid);

  // List all tables
  std::vector<std::string> GetTableNames() const;

  BufferPoolManager* GetBPM() const { return bpm_; }

 private:
  BufferPoolManager* bpm_;
  DiskManager* disk_manager_;
  CatalogMeta* catalog_meta_;
};

}  // namespace bustub