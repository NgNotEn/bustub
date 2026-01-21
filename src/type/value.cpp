#include "type/value.h"
#include "type/type_id.h"
#include <cstring>


namespace bustub {

    // 构造
    Value::Value(const int32_t integer) 
    :value_({integer}), storage_size_(4), type_id_(TypeId::INTEGER), logic_len_(0){}
    Value::Value(const std::string str)
    : type_id_(TypeId::VARCHAR), logic_len_(str.size()){

        value_.varlen_ = new char[logic_len_ + 1];
        std::memcpy(value_.varlen_, str.c_str(), logic_len_);
        value_.varlen_[logic_len_] = '\0';
        storage_size_ = logic_len_ + 4; // 4 bytes union area + allocated heap data
    } 

    // 深拷贝
    Value::Value(const Value& other) {
        if (this == &other) {
            return;
        }
        type_id_ = other.type_id_;
        storage_size_ = other.storage_size_;
        logic_len_ = other.logic_len_;
        if (type_id_ == VARCHAR) {
            value_.varlen_ = new char[logic_len_ + 1];
            memcpy(value_.varlen_, other.value_.varlen_, logic_len_ + 1);
        } else {
            value_.integer_ = other.value_.integer_;
        }
    }
    auto Value::operator=(const Value& other) -> Value& {
        // 防止自赋值 (a = a)
        if (this == &other) {
            return *this;
        }

        // 如果自己之前有堆内存，先释放！(防止内存泄漏)
        if (type_id_ == TypeId::VARCHAR && value_.varlen_ != nullptr) {
            delete[] value_.varlen_;
        }

        // 复制元数据
        type_id_ = other.type_id_;
        storage_size_ = other.storage_size_;
        logic_len_ = other.logic_len_;

        // 深拷贝数据
        if (type_id_ == TypeId::VARCHAR) {
            value_.varlen_ = new char[logic_len_ + 1];
            std::memcpy(value_.varlen_, other.value_.varlen_, logic_len_ + 1);
        } else {
            value_.integer_ = other.value_.integer_;
        }
        return *this;
    }

    // 析构
    Value::~Value() {
        if (type_id_ == VARCHAR && value_.varlen_ != nullptr) {
            delete[] value_.varlen_;
        }
    }

    // === 核心逻辑：分发给单例Type ===
    auto Value::SerializeTo(char* storage) const -> void {
        Type::GetInstance(type_id_)->SerializeTo(*this, storage);
    }

    auto Value::DeserializeFrom(const char *storage, TypeId type_id) -> Value {
        return Type::GetInstance(type_id)->DeserializeFrom(storage);
    }

    auto Value::CompareEquals(const Value &other) const -> bool {
        return Type::GetInstance(type_id_)->CompareEquals(*this, other);
    }

    auto Value::CompareLessThan(const Value &other) const -> bool{
        return Type::GetInstance(type_id_)->CompareLessThan(*this, other);
    }

    auto Value::ToString() const -> std::string {
        return Type::GetInstance(type_id_)->ToString(*this);
    }

}