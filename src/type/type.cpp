#include "type/type.h"
#include "type/integer_type.h"
#include "type/type_id.h"
#include "type/varlen_type.h"

namespace bustub {
    auto Type::GetInstance(TypeId type_id) -> Type* {
        static IntegerType k_integer_type;
        static VarlenType k_varlen_type;

        switch (type_id) {
            case TypeId::INTEGER:
                return &k_integer_type;
            case TypeId::VARCHAR:
                return &k_varlen_type;
            default:
                return nullptr;
        }
    }
}

