#pragma once
#include "type/type.h"
#include "type/value.h"

namespace bustub {

    class IntegerType: public Type {
    public:
        IntegerType();

        // int 2 byte
        auto SerializeTo(const Value& val, char* storage) const ->void override;
        // byte 2 int
        auto DeserializeFrom(const char* storage) const -> Value override;

        auto CompareEquals(const Value& left, const Value& right) const -> bool override;
        
        auto CompareLessThan(const Value& left, const Value& right) const -> bool override;

        auto ToString(const Value& val) const -> std::string override;

    };

}