#pragma once
#include "type/type_id.h"
#include "type/type.h"
#include <cstdint>
#include <string>
#include <cstring>

class Type;

namespace bustub {

    class Value {
    public:
        // integer construct
        Value(const int32_t integer);
        // string construct
        Value(const std::string str);
        // destructor
        ~Value();

        // deep copy
        Value(const Value& other);
        auto operator=(const Value& other) -> Value&;

        // = data access =
        inline int32_t GetAsInteger() const { return value_.integer_; }
        inline const char* GetData() const { return value_.varlen_; }
        inline uint32_t GetLogicLength() const { return logic_len_; }
        inline TypeId GetTypeId() const { return type_id_; }
        inline uint32_t GetStorageSize() const { return storage_size_; }
        inline bool IsNull() const { return is_null_; }


        // == core apis ==
        // trans value to storage
        auto SerializeTo(char* storage) const -> void;
        // trans storage to value
        static auto DeserializeFrom(const char *storage, TypeId type_id) -> Value;
        // compare 
        auto CompareEquals(const Value &other) const -> bool;
        auto CompareLessThan(const Value &other) const -> bool;
        // debug
        auto ToString() const -> std::string;
    private:
        TypeId type_id_;
        uint32_t storage_size_; // bytes
        uint32_t logic_len_;  // for varchar
        bool is_null_;

        union Val {
            int32_t integer_;
            char *varlen_;  // varchar pointer
        } value_;
    };
}