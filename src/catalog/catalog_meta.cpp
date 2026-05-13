#include "catalog/catalog_meta.h"

#include <cassert>
#include <cstring>
#include <filesystem>

#include "catalog/column.h"
#include "catalog/schema.h"
#include "catalog/table_info.h"

namespace bustub {

CatalogMeta::CatalogMeta(const std::string& meta_path)
  : meta_path_(meta_path) {}

bool CatalogMeta::AddTable(std::unique_ptr<TableInfo> table_info) {
  if (!AddTableInternal(std::move(table_info))) {
    return false;
  }
  // Flush to disk immediately
  return SaveToDisk();
}

bool CatalogMeta::AddTableInternal(std::unique_ptr<TableInfo> table_info) {
  if (table_info == nullptr) {
    return false;
  }

  table_id_t tid = table_info->GetId();
  const std::string& name = table_info->GetName();

  // Check for duplicate name
  if (tname2tid_.find(name) != tname2tid_.end()) {
    return false;
  }

  // Update next_table_id if necessary
  if (tid >= next_table_id_) {
    next_table_id_ = tid + 1;
  }

  tid2tbinfo_[tid] = std::move(table_info);
  tname2tid_[name] = tid;
  return true;
}

TableInfo* CatalogMeta::GetTable(table_id_t tid) {
  auto it = tid2tbinfo_.find(tid);
  if (it == tid2tbinfo_.end()) {
    return nullptr;
  }
  return it->second.get();
}

TableInfo* CatalogMeta::GetTable(const std::string& name) {
  auto it = tname2tid_.find(name);
  if (it == tname2tid_.end()) {
    return nullptr;
  }
  return GetTable(it->second);
}

table_id_t CatalogMeta::GetNextTableId() const {
  return next_table_id_;
}

bool CatalogMeta::RemoveTable(table_id_t tid) {
  auto it = tid2tbinfo_.find(tid);
  if (it == tid2tbinfo_.end()) {
    return false;
  }

  const std::string& name = it->second->GetName();
  tname2tid_.erase(name);
  tid2tbinfo_.erase(it);

  // Flush to disk immediately
  return SaveToDisk();
}

bool CatalogMeta::RemoveTable(const std::string& name) {
  auto it = tname2tid_.find(name);
  if (it == tname2tid_.end()) {
    return false;
  }

  table_id_t tid = it->second;
  tname2tid_.erase(it);
  tid2tbinfo_.erase(tid);

  // Flush to disk immediately
  return SaveToDisk();
}

bool CatalogMeta::SerializeTo(std::ofstream& out) const {
  if (!out.is_open()) {
    return false;
  }

  // Write version
  uint32_t version = CATALOG_VERSION;
  out.write(reinterpret_cast<const char*>(&version), sizeof(version));

  // Write next_table_id
  out.write(reinterpret_cast<const char*>(&next_table_id_),
            sizeof(next_table_id_));

  // Write number of tables
  uint32_t num_tables = static_cast<uint32_t>(tid2tbinfo_.size());
  out.write(reinterpret_cast<const char*>(&num_tables), sizeof(num_tables));

  // Write each table
  for (const auto& [tid, table_info] : tid2tbinfo_) {
    // Write table id
    table_id_t table_id = table_info->GetId();
    out.write(reinterpret_cast<const char*>(&table_id), sizeof(table_id));

    // Write table name
    const std::string& table_name = table_info->GetName();
    uint32_t name_len = static_cast<uint32_t>(table_name.length());
    out.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
    out.write(table_name.c_str(), name_len);

    // Write first page id
    page_id_t first_page_id = table_info->GetFirstPageId();
    out.write(reinterpret_cast<const char*>(&first_page_id),
              sizeof(first_page_id));

    // Write schema
    const Schema& schema = table_info->GetSchema();

    // Write schema name
    const std::string& schema_name = schema.GetName();
    uint32_t schema_name_len = static_cast<uint32_t>(schema_name.length());
    out.write(reinterpret_cast<const char*>(&schema_name_len),
              sizeof(schema_name_len));
    out.write(schema_name.c_str(), schema_name_len);

    // Write column count
    uint32_t col_count = schema.GetColumnCount();
    out.write(reinterpret_cast<const char*>(&col_count), sizeof(col_count));

    // Write each column
    for (uint32_t i = 0; i < col_count; i++) {
      const Column& col = schema.GetColumn(i);

      // Write column name
      const std::string& col_name = col.GetName();
      uint32_t col_name_len = static_cast<uint32_t>(col_name.length());
      out.write(reinterpret_cast<const char*>(&col_name_len),
                sizeof(col_name_len));
      out.write(col_name.c_str(), col_name_len);

      // Write column type
      uint32_t col_type = static_cast<uint32_t>(col.GetType());
      out.write(reinterpret_cast<const char*>(&col_type), sizeof(col_type));

      // Write column storage size
      uint32_t col_storage_size = col.GetStorageSize();
      out.write(reinterpret_cast<const char*>(&col_storage_size),
                sizeof(col_storage_size));
    }
  }

  return out.good();
}

bool CatalogMeta::DeserializeFrom(std::ifstream& in) {
  if (!in.is_open()) {
    return false;
  }

  // Read version
  uint32_t version;
  in.read(reinterpret_cast<char*>(&version), sizeof(version));
  if (version != CATALOG_VERSION) {
    return false;
  }

  // Read next_table_id
  in.read(reinterpret_cast<char*>(&next_table_id_), sizeof(next_table_id_));

  // Read number of tables
  uint32_t num_tables;
  in.read(reinterpret_cast<char*>(&num_tables), sizeof(num_tables));

  if (!in.good()) {
    return false;
  }

  // Read each table
  for (uint32_t t = 0; t < num_tables; t++) {
    // Read table id
    table_id_t table_id;
    in.read(reinterpret_cast<char*>(&table_id), sizeof(table_id));

    // Read table name
    uint32_t name_len;
    in.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    std::string table_name(name_len, '\0');
    in.read(&table_name[0], name_len);

    // Read first page id
    page_id_t first_page_id;
    in.read(reinterpret_cast<char*>(&first_page_id), sizeof(first_page_id));

    // Read schema
    // Read schema name
    uint32_t schema_name_len;
    in.read(reinterpret_cast<char*>(&schema_name_len), sizeof(schema_name_len));
    std::string schema_name(schema_name_len, '\0');
    in.read(&schema_name[0], schema_name_len);

    // Read column count
    uint32_t col_count;
    in.read(reinterpret_cast<char*>(&col_count), sizeof(col_count));

    if (!in.good()) {
      return false;
    }

    // Read each column
    std::vector<Column> columns;
    for (uint32_t i = 0; i < col_count; i++) {
      // Read column name
      uint32_t col_name_len;
      in.read(reinterpret_cast<char*>(&col_name_len), sizeof(col_name_len));
      std::string col_name(col_name_len, '\0');
      in.read(&col_name[0], col_name_len);

      // Read column type
      uint32_t col_type_val;
      in.read(reinterpret_cast<char*>(&col_type_val), sizeof(col_type_val));
      TypeId col_type = static_cast<TypeId>(col_type_val);

      // Read column storage size
      uint32_t col_storage_size;
      in.read(reinterpret_cast<char*>(&col_storage_size),
              sizeof(col_storage_size));

      // Create column
      if (col_type == TypeId::INTEGER) {
        columns.emplace_back(col_name, col_type);
      } else if (col_type == TypeId::VARCHAR) {
        columns.emplace_back(col_name, col_type, col_storage_size);
      } else {
        return false;
      }
    }

    if (!in.good()) {
      return false;
    }

    // Create schema and table info
    Schema schema(schema_name, columns);
    auto table_info = std::make_unique<TableInfo>(table_id, table_name, schema,
                                                  first_page_id);

    // Add to maps without flushing (we'll flush once at the end)
    if (!AddTableInternal(std::move(table_info))) {
      return false;
    }
  }

  return in.good();
}

bool CatalogMeta::SaveToDisk() const {
  // Create data directory if not exists
  std::filesystem::create_directories(
      std::filesystem::path(meta_path_).parent_path());

  // Open file for writing
  std::ofstream out(meta_path_, std::ios::binary | std::ios::trunc);
  if (!out.is_open()) {
    return false;
  }

  bool result = SerializeTo(out);
  out.close();
  return result;
}

bool CatalogMeta::LoadFromDisk() {
  std::ifstream in(meta_path_, std::ios::binary);
  if (!in.is_open()) {
    return false;
  }

  bool result = DeserializeFrom(in);
  in.close();
  return result;
}

std::vector<std::string> CatalogMeta::GetTableNames() const {
  std::vector<std::string> names;
  for (const auto& [name, _] : tname2tid_) {
    names.push_back(name);
  }
  return names;
}

}  // namespace bustub