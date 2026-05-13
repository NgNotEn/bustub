#pragma once
#include <string>

#include "catalog/schema.h"
#include "common/config.h"

namespace bustub {
class TableInfo {
 public:
  TableInfo(table_id_t tid, std::string name, Schema schema,
            page_id_t first_page_id);
  table_id_t GetId() const;
  const std::string& GetName() const;
  const Schema& GetSchema() const;
  page_id_t GetFirstPageId() const;

 private:
  table_id_t tid_;
  std::string name_;
  Schema schema_;
  page_id_t first_page_id_;
};
}  // namespace bustub