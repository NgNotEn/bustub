#include "catalog/schema.h"
#include "type/type_id.h"
#include <cstdint>

namespace bustub {
    Schema::Schema(std::string name, std::vector<Column> columns)
    :name_(std::move(name)), columns_(std::move(columns)){
        storage_size_ = 0;
        is_inlined_ = true;
        uint32_t offset = 0;
        for(auto& col: columns_) {
            if(col.GetType() != TypeId::INTEGER) {
                is_inlined_ = false;
            }

            col.column_offset_ = offset;
            offset += col.GetStorageSize();
        }
        storage_size_ = offset;
    }
}