#include "catalog/table_info.h"

#include "catalog/schema.h"

namespace bustub {

TableInfo::TableInfo(table_id_t tid, std::string name, Schema schema,
                     page_id_t first_page_id)
  : tid_(tid),
    name_(std::move(name)),
    schema_(schema),
    first_page_id_(first_page_id) {}

table_id_t TableInfo::GetId() const {
  return tid_;
}

const std::string& TableInfo::GetName() const {
  return name_;
}

const Schema& TableInfo::GetSchema() const {
  return schema_;
}

page_id_t TableInfo::GetFirstPageId() const {
  return first_page_id_;
}

}  // namespace bustub