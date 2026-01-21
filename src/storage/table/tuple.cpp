#include "storage/table/tuple.h"
#include "common/rid.h"
#include <cstdint>
#include <cstring>
#include <vector>
#include <cassert>

namespace bustub {

    Tuple::Tuple()
    :is_allocated_(false), storage_size_(0), rid_(RID()), data_(nullptr){}

    Tuple::Tuple(RID rid, char* data, uint32_t size)
    :is_allocated_(true), rid_(rid), storage_size_(size){
        data_ = new char[storage_size_];
        std::memcpy(data_, data, storage_size_);
    }

    Tuple::Tuple(std::vector<Value> values, Schema* schema) {
        
        is_allocated_ = true;

        assert(values.size() == schema->GetColumnCount());

        // == caculate storage ==
        uint32_t num_col = schema->GetColumnCount();
        
        // caculate bitmap size
        uint32_t bitmap_size = (num_col + 7) / 8;
        // data size
        uint32_t data_size = schema->GetStorageSize();
        // storage size
        storage_size_ = bitmap_size + data_size;


        // == allocate and inital ==
        data_ = new char[storage_size_];

        // clear
        std::memset(data_, 0, storage_size_);

        // == serialize ==

        char* data_ptr = data_ + bitmap_size;   // skip bitmap

        for(uint32_t i = 0; i < num_col; ++i) {
            auto& col = schema->GetColumn(i);
            auto& val = values[i];

            if (col.GetType() == TypeId::VARCHAR) {
                assert(val.GetStorageSize() <= col.GetStorageSize() && "Varchar data too long for column definition!");
            }

            if(val.IsNull()) {
                data_[i >> 3] |= (1 << (i % 8));
            }
            else {
                val.SerializeTo(data_ptr + col.GetOffset());
            }

        }
    }

    Tuple::Tuple(Tuple&& other) 
    :is_allocated_(other.is_allocated_), storage_size_(other.storage_size_), 
    rid_(other.rid_), data_(other.data_){
        other.data_ = nullptr;
    }

    auto Tuple::operator=(Tuple&& other) -> Tuple& {
        is_allocated_ = other.is_allocated_;
        storage_size_ = other.storage_size_;
        rid_ = other.rid_;
        data_ = other.data_;
        other.data_ = nullptr;
        return *this;
    }

    Tuple::~Tuple() {
        if (is_allocated_ && data_ != nullptr) {
            delete[] data_;
            data_ = nullptr;
        }
    }

    Tuple::Tuple(const Tuple& other) {
        is_allocated_= other.is_allocated_;
        storage_size_ = other.storage_size_;
        rid_ = other.rid_;
        data_ = new char[storage_size_];
        memcpy(data_, other.data_,  storage_size_);
    }

    auto Tuple::operator=(const Tuple& other) -> Tuple& {
        is_allocated_= other.is_allocated_;
        storage_size_ = other.storage_size_;
        rid_ = other.rid_;
        data_ = new char[storage_size_];
        memcpy(data_, other.data_,  storage_size_);
        return *this;
    }


    auto Tuple::GetValue(const Schema *schema, uint32_t column_idx) const -> Value {
        assert(schema != nullptr);
        assert(data_ != nullptr);

        // 获取列的定义
        const Column &col = schema->GetColumn(column_idx);
        
        // 检查 Null Bitmap
        // Bitmap 位于 data_ 的最开头
        // 每一位对应一列，1 表示 NULL
        // 计算这一列对应的字节索引和位索引
        uint32_t is_null = (data_[column_idx / 8] & (1 << (column_idx % 8)));
        
        if (is_null) {
            // 如果是 NULL，返回对应类型的空值
            return Value(col.GetType());
        }

        // 计算数据区的起始位置
        // Bitmap 大小 = (列数 + 7) / 8
        uint32_t bitmap_size = (schema->GetColumnCount() + 7) / 8;
        
        // 数据指针 = data_ + bitmap_size + 列的偏移量
        // col.GetOffset() 返回的是相对于数据区起点的偏移
        const char *val_ptr = data_ + bitmap_size + col.GetOffset();

        // 反序列化
        return Value::DeserializeFrom(val_ptr, col.GetType());
    }
}