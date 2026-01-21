#pragma once

#include <cstdint>
#include <string>
#include <sstream>

namespace bustub {

// 定义 PageId 的类型，通常是 int32_t
using page_id_t = int32_t; 

class RID {
public:
    // 默认构造函数 (初始化为无效值)
    RID() : page_id_(-1), slot_id_(0) {}

    // 参数构造函数
    RID(page_id_t page_id, uint32_t slot_num) 
        : page_id_(page_id), slot_id_(slot_num) {}

    // Getters
    inline page_id_t GetPageId() const { return page_id_; }
    inline uint32_t GetSlotId() const { return slot_id_; }

    // Setters
    inline void Set(page_id_t page_id, uint32_t slot_num) {
        page_id_ = page_id;
        slot_id_ = slot_num;
    }

    // 5. 重载比较运算符 (非常重要！否则没法判断两个 RID 是否指向同一行)
    bool operator==(const RID &other) const {
        return page_id_ == other.page_id_ && slot_id_ == other.slot_id_;
    }

    bool operator!=(const RID &other) const {
        return !(*this == other);
    }

    // 6. 调试用的 ToString
    std::string ToString() const {
        std::stringstream os;
        os << "RID(Page=" << page_id_ << ", Slot=" << slot_id_ << ")";
        return os.str();
    }

private:
    page_id_t page_id_;   // 页
    uint32_t slot_id_;   // 槽位
};

} // namespace bustub