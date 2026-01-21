#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include "type/type_id.h"


namespace bustub {
    class Column {
    friend class Schema;

    public:
    // fixed constructor
    Column(std::string col_name, TypeId type_id);
    
    // vary constructor
    Column(std::string col_name, TypeId type, std::size_t storage_size);

    // Getters...
    std::string GetName() const { return name_; }
    TypeId GetType() const { return type_; }
    uint32_t GetStorageSize() const { return storage_size_; }
    uint32_t GetOffset() const { return column_offset_; }


    private:
        std::string name_;
        TypeId type_;
        uint32_t column_offset_;
        uint32_t storage_size_;
    };
}