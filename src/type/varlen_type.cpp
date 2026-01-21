#include "type/varlen_type.h"
#include <algorithm>
#include <cstring>

namespace bustub {

    // constructor
    VarlenType::VarlenType() : Type(TypeId::VARCHAR) {
        // nothing
    }

    auto VarlenType::SerializeTo(const Value& val, char* storage) const ->void {
        auto len = val.GetLogicLength();
        memcpy(storage, &len, sizeof(len));

        memcpy(storage + sizeof(len), val.GetData(), val.GetLogicLength());
    }

    auto VarlenType::DeserializeFrom(const char* storage) const -> Value {
        uint32_t len;
        memcpy(&len, storage, sizeof(uint32_t));     // 读出长度头
        std::string str(storage + sizeof(uint32_t), len);   // 根据长度读出数据
        return Value(str);
    }

    auto VarlenType::CompareEquals(const Value& left, const Value& right) const -> bool{
        uint32_t len1 = left.GetLogicLength();
        uint32_t len2 = right.GetLogicLength();
        
        if (len1 != len2) {
            return false;
        }
        return std::memcmp(left.GetData(), right.GetData(), len1) == 0;
    }
        
    auto VarlenType::CompareLessThan(const Value& left, const Value& right) const -> bool{
        uint32_t len1 = left.GetLogicLength();
        uint32_t len2 = right.GetLogicLength();
        const char* str1 = left.GetData();
        const char* str2 = right.GetData();

        // 找出最短长度，只比较公共部分
        uint32_t min_len = std::min(len1, len2);
        int cmp_result = std::memcmp(str1, str2, min_len);

        if (cmp_result != 0) {
            return cmp_result < 0;
        }

        // 此时长度短的那个更小
        return len1 < len2;
    }

    auto VarlenType::ToString(const Value& val) const -> std::string{
        return std::string(val.GetData(), val.GetLogicLength());
    }

}