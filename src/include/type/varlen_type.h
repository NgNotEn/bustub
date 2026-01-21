#pragma once
#include "type/type.h"
#include "type/value.h"

namespace bustub {

    class VarlenType: public Type {
    public:
        VarlenType();

        // value 2 byte, 不仅仅是数据，还有元数据(4byte的长度头)
        auto SerializeTo(const Value& val, char* storage) const ->void override;
        // byte 2 value, 不仅仅是数据，还有元数据(4byte的长度头)
        auto DeserializeFrom(const char* storage) const -> Value override;

        auto CompareEquals(const Value& left, const Value& right) const -> bool override;
        
        auto CompareLessThan(const Value& left, const Value& right) const -> bool override;

        auto ToString(const Value& val) const -> std::string override;

    };

}