#include "type/integer_type.h"
#include <cstring>

namespace bustub {

    // constructor
    IntegerType::IntegerType() : Type(TypeId::INTEGER) {
        // nothing
    }

    auto IntegerType::SerializeTo(const Value& val, char* storage) const ->void {
        int32_t raw_val = val.GetAsInteger();
        memcpy(storage, &raw_val, sizeof(int32_t));
    }

    auto IntegerType::DeserializeFrom(const char* storage) const -> Value {
        int32_t raw_val;
        memcpy(&raw_val, storage, sizeof(int32_t));
        return Value(raw_val);
    }

    auto IntegerType::CompareEquals(const Value& left, const Value& right) const -> bool{
        return left.GetAsInteger() == right.GetAsInteger();
    }
        
    auto IntegerType::CompareLessThan(const Value& left, const Value& right) const -> bool{
        return left.GetAsInteger() < right.GetAsInteger();
    }

    auto IntegerType::ToString(const Value& val) const -> std::string{
        return std::to_string(val.GetAsInteger());
    }

}