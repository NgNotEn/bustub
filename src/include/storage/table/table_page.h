#pragma once

#include "common/config.h"
#include "common/rid.h"
#include <cstdint>
#include "storage/table/tuple.h"
#include "storage/page/page.h"

// This is a page_node of table heap (a delist);

namespace bustub {
    class TableHeap;
    class TablePage: public Page {
        friend TableHeap;
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
        

        auto GetTuple(const RID rid) -> Tuple;
        auto MarkDeleted(const RID rid) -> bool;
        auto UpdateTuple(const Tuple& new_tuple, RID rid) -> bool;
    private:
        // return offset of new tuple
        auto MoveInsertTuple(const Tuple& tuple) -> uint32_t;

        auto GetHeader() -> Header*;
        auto GetSlot(uint32_t slot_id) -> Slot*;
    };
}