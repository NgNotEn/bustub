#include "catalog/column.h"
#include "type/type_id.h"


namespace bustub {
    Column::Column(std::string name, TypeId type)
    :name_(std::move(name)), type_(type) { 
        if (type_ == TypeId::INTEGER) {
            storage_size_ = 4; // int32_t
        } else {
            // futrue case: BIGINT or others
            type_ = TypeId::INVALID;
            storage_size_ = 0; 
        }
        // 偏移量初始化为0，稍后由 Schema 计算
        column_offset_ = 0;
    }

    Column::Column(std::string name, TypeId type, std::size_t storage_size)
    :name_(std::move(name)), type_(type){
        if(type_ == TypeId::VARCHAR) {
            storage_size_ = storage_size;
        } else {
            // other case
            type_ =   TypeId::INVALID;
            storage_size_ = 0;
        }

        column_offset_ = 0;
    }
}