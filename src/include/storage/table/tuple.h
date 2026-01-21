#pragma once
#include "common/rid.h"
#include "type/value.h"
#include "catalog/schema.h"
#include <cstdint>


namespace bustub {
    class Tuple {
    public:
        // defalt
        Tuple(); 
        // cp 
        Tuple(std::vector<Value> values, Schema* schema);  
        Tuple(RID rid, char* data, uint32_t size);
        Tuple(const Tuple& other);
        auto operator=(const Tuple& other) -> Tuple&;
        // move
        Tuple(Tuple&& other);
        auto operator=(Tuple&& other) -> Tuple&;

        ~Tuple();  


        auto GetValue(const Schema *schema, uint32_t column_idx) const -> Value;

        inline RID GetRid() const { return rid_; }
        inline void SetRid(RID rid) { rid_ = rid; }
        inline uint32_t GetStorageSize() const { return storage_size_; };
        inline char* GetData() const { return data_; }

    private:
        bool is_allocated_;
        uint32_t storage_size_;
        RID rid_;
        char* data_;
    };
}