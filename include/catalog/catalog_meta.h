#pragma once

#include <fstream>
#include <memory>
#include <unordered_map>

#include "catalog/table_info.h"
#include "common/config.h"

namespace bustub {

class CatalogMeta {
 public:
  explicit CatalogMeta(const std::string& meta_path);

  bool AddTable(std::unique_ptr<TableInfo> table_info);
  TableInfo* GetTable(table_id_t tid);
  TableInfo* GetTable(const std::string& name);
  bool RemoveTable(table_id_t tid);
  bool RemoveTable(const std::string& name);

  bool SerializeTo(std::ofstream& out) const;
  bool DeserializeFrom(std::ifstream& in);

  bool SaveToDisk() const;
  bool LoadFromDisk();
  std::vector<std::string> GetTableNames() const;

  table_id_t GetNextTableId() const;

 private:
  bool AddTableInternal(std::unique_ptr<TableInfo> table_info);

  std::string meta_path_;
  std::unordered_map<table_id_t, std::unique_ptr<TableInfo>> tid2tbinfo_;
  std::unordered_map<std::string, table_id_t> tname2tid_;
  table_id_t next_table_id_{0};
  static constexpr uint32_t CATALOG_VERSION = 1;
};

}  // namespace bustub