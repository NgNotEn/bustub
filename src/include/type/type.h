#pragma once
#include "type/type_id.h"
#include <string>

namespace bustub {
    class Value;
    // basic type
    class Type {
    public:
        virtual ~Type() = default;

        // 单例工厂：根据 TypeId 拿到对应的 Type 指针 (比如 IntegerType*)
        static auto GetInstance(TypeId type_id) -> Type*;


        // de/serialize
        // 把 val 序列化后 放进 storage 里
        virtual auto SerializeTo(const Value& val, char* storage)  const-> void = 0;
        // 从 storage 取出数据还原成 al
        virtual auto DeserializeFrom(const char* storage) const -> Value = 0;

        //compare
        virtual auto CompareEquals(const Value& left, const Value& right) const -> bool = 0;
        virtual auto CompareLessThan(const Value& left, const Value& right) const -> bool = 0;// for b+tree
        
        // debug
        virtual auto ToString(const Value& val) const -> std::string = 0;

        inline auto IsVariableLength() const -> bool  { return type_id_ == VARCHAR; }
        inline auto GetTypeId() const -> TypeId { return type_id_; }
    protected:
        TypeId type_id_;
        // 构造函数 protected，强制单例，不允许外部随便 new Type()
        explicit Type(TypeId type_id) : type_id_(type_id) {}
    };

}