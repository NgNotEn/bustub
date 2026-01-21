#pragma once

#include "common/rid.h"
#include <cstdint>
#include "storage/table/tuple.h"
#include "storage/page/page.h"

namespace bustub {
    class TablePage: public Page {
        struct Header{
            page_id_t page_id_;
            page_id_t prev_page_id_;
            page_id_t next_page_id_;
            uint32_t tuple_count_;
            uint32_t free_space_ptr_;
        };

        struct Slot {
            uint32_t offset_;
            uint32_t storage_size_;
        };

    public:
        auto Init(page_id_t page_id, page_id_t prev_page_id = INVALID_PAGE_ID, page_id_t next_page_id = INVALID_PAGE_ID) -> void;
        auto GetFreeSpaceRemaining() -> uint32_t;
        auto InsertTuple(const Tuple& tuple) -> RID;
        

        auto GetTuple(RID rid) -> Tuple;
        auto MarkDelete(RID rid) -> void;
        auto UpdateTuple(const Tuple& new_tuple, Tuple old_tuple, RID rid) -> bool;
    private:
        // return offset of new tuple
        auto MoveInsertTuple(const Tuple& tuple) -> uint32_t;

        auto GetHeader() -> Header*;
        auto GetSlot(uint32_t slot_idx) -> Slot*;
    };
}