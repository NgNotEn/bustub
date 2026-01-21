#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "catalog/column.h"

namespace bustub {

    class Schema {
    public:
        Schema(std::string name, std::vector<Column> columns);

        const Column &GetColumn(uint32_t col_idx) const { return columns_[col_idx]; }

        // 获取某一列的偏移量
        uint32_t GetColOffset(uint32_t col_idx) const { return columns_[col_idx].GetOffset(); }

        // 获取列的数量
        uint32_t GetColumnCount() const { return static_cast<uint32_t>(columns_.size()); }

        // 获取总长度
        uint32_t GetStorageSize() const { return storage_size_; }

        // 是否全定长
        bool IsInlined() const { return is_inlined_; }

    private:
        std::string name_;
        std::vector<Column> columns_;
        uint32_t storage_size_;  // 需要的总长度
        bool is_inlined_;        // 是否全为定长列
    };

}

