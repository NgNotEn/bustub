#include "catalog/catalog_manager.h"

#include <string>
#include <vector>

#include "buffer/buffer_pool_manager.h"
#include "catalog/catalog_meta.h"
#include "catalog/schema.h"
#include "catalog/table_info.h"
#include "storage/disk/disk_manager.h"
#include "storage/table/table_heap.h"
#include "storage/table/table_page.h"

namespace bustub {

CatalogManager::CatalogManager(BufferPoolManager* bpm,
                               DiskManager* disk_manager,
                               const std::string& meta_path)
  : bpm_(bpm), disk_manager_(disk_manager) {
  catalog_meta_ = new CatalogMeta(meta_path);
  // Load existing tables from disk
  catalog_meta_->LoadFromDisk();

  // Open all table files in disk manager
  auto table_names = catalog_meta_->GetTableNames();
  for (const auto& table_name : table_names) {
    TableInfo* info = catalog_meta_->GetTable(table_name);
    if (info) {
      disk_manager_->OpenTableFile(info->GetId(), table_name);
    }
  }
}

CatalogManager::~CatalogManager() {
  delete catalog_meta_;
}

TableInfo* CatalogManager::CreateTable(const std::string& name,
                                       const Schema& schema) {
  // Check if table already exists
  if (catalog_meta_->GetTable(name) != nullptr) {
    return nullptr;
  }

  // Allocate new table id
  table_id_t table_id = catalog_meta_->GetNextTableId();

  // Open table file in disk manager
  if (!disk_manager_->OpenTableFile(table_id, name)) {
    return nullptr;
  }

  // Create first page for the table
  page_id_t first_page_id;
  Page* first_page = bpm_->NewPage(table_id, &first_page_id);
  if (!first_page) {
    disk_manager_->DeleteTableFile(table_id, name);
    return nullptr;
  }

  // Initialize table page
  TablePage* table_page = reinterpret_cast<TablePage*>(first_page);
  table_page->Init(first_page_id);

  bpm_->UnpinPage(table_id, first_page_id, true);

  // Create table info
  auto table_info =
      std::make_unique<TableInfo>(table_id, name, schema, first_page_id);
  TableInfo* info_ptr = table_info.get();

  // Add to catalog (will auto-save to disk)
  if (!catalog_meta_->AddTable(std::move(table_info))) {
    // Cleanup on failure
    bpm_->DeletePage(table_id, first_page_id);
    disk_manager_->DeleteTableFile(table_id, name);
    return nullptr;
  }

  return info_ptr;
}

TableInfo* CatalogManager::GetTable(const std::string& name) {
  return catalog_meta_->GetTable(name);
}

TableInfo* CatalogManager::GetTable(table_id_t tb_id) {
  return catalog_meta_->GetTable(tb_id);
}

bool CatalogManager::DropTable(const std::string& name) {
  TableInfo* info = catalog_meta_->GetTable(name);
  if (!info) {
    return false;
  }

  table_id_t table_id = info->GetId();

  // TODO: Reclaim all pages of the table from TableHeap

  // Remove from catalog (will auto-save to disk)
  if (!catalog_meta_->RemoveTable(name)) {
    return false;
  }

  // Delete table file
  disk_manager_->DeleteTableFile(table_id, name);
  return true;
}

bool CatalogManager::DropTable(table_id_t tid) {
  TableInfo* info = catalog_meta_->GetTable(tid);
  if (!info) {
    return false;
  }

  return DropTable(info->GetName());
}

std::vector<std::string> CatalogManager::GetTableNames() const {
  return catalog_meta_->GetTableNames();
}

}  // namespace bustub