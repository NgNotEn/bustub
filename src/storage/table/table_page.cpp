#include "storage/table/table_page.h"
#include "common/config.h"
#include "common/rid.h"
#include "storage/table/tuple.h"
#include <cstdint>
#include <cstring>

namespace bustub {

    auto TablePage::GetHeader() -> Header*{
        return reinterpret_cast<Header*>(data_);
    }

    auto TablePage::GetSlot(uint32_t slot_idx) -> Slot*{
        return reinterpret_cast<Slot*>(data_ + sizeof(Header) + slot_idx * sizeof(Slot));
    }

    auto TablePage::Init(page_id_t page_id, page_id_t prev_page_id, page_id_t next_page_id) -> void{
        Header* header = GetHeader();
        header->page_id_ = page_id;
        header->prev_page_id_ = prev_page_id;
        header->next_page_id_ = next_page_id;
        header->tuple_count_ = 0;
        header->free_space_ptr_ = PAGE_SIZE;
    }

    auto TablePage::GetFreeSpaceRemaining() -> uint32_t {
        Header* header = GetHeader();

        uint32_t used_header_slot_space = sizeof(Header) + sizeof(Slot) * header->tuple_count_;
        
        // 保护：如果数据区起始点 跑到了 Header 区域前面，说明页面坏了
        if (header->free_space_ptr_ < used_header_slot_space) {
            return 0; 
        }
    
    return header->free_space_ptr_ - used_header_slot_space;
}

    auto TablePage::InsertTuple(const Tuple& tuple) -> RID{
        // enough size
        if(GetFreeSpaceRemaining() >= tuple.GetStorageSize() + sizeof(Slot)) {
            Header* header = GetHeader();
            
            // save data
            uint32_t data_size = tuple.GetStorageSize();
            uint32_t data_offset = header->free_space_ptr_ - data_size;
            std::memcpy(data_ + data_offset, tuple.GetData(), data_size);
            header->free_space_ptr_ = data_offset;

            // set slot
            
            Slot slot{data_offset, data_size};
            std::memcpy(data_ + sizeof(Header) + sizeof(Slot)*(header->tuple_count_), &slot, sizeof(Slot));
            RID rid(page_id_, (header->tuple_count_)++);
            return rid;
        }
        else {
            return RID();
        }
    }

    auto TablePage::GetTuple(RID rid) -> Tuple {
        Header* head = GetHeader();
        if (rid.GetSlotId() >= head->tuple_count_) {
            return Tuple();
        }
        Slot* slot = GetSlot(rid.GetSlotId());
        // is delete?
        if (slot->storage_size_ == 0) {
            return Tuple();
        }

        
        Tuple ret_tuple(rid, data_ + slot->offset_, slot->storage_size_);
        return ret_tuple;
    }

    auto TablePage::MarkDelete(RID rid) -> void {
        Header* head = GetHeader();
        if (rid.GetSlotId() >= head->tuple_count_) {
            return ;
        }
        Slot* slot = GetSlot(rid.GetSlotId());
        slot->storage_size_ = 0;
    }

    auto TablePage::MoveInsertTuple(const Tuple& tuple) -> uint32_t {
        if(GetFreeSpaceRemaining() >= tuple.GetStorageSize()) {
            Header* header = GetHeader();
            
            // save data
            uint32_t data_size = tuple.GetStorageSize();
            uint32_t data_offset = header->free_space_ptr_ - data_size;
            std::memcpy(data_ + data_offset, tuple.GetData(), data_size);
            header->free_space_ptr_ = data_offset;

            return data_offset;
        } else {
            return INVALID_OFFSET;
        }
    }


    auto TablePage::UpdateTuple(const Tuple& new_tuple, Tuple old_tuple, RID rid) -> bool {
        // just memcpy
        if(new_tuple.GetStorageSize() <= old_tuple.GetStorageSize()) {
            Slot* slot = GetSlot(rid.GetSlotId());
            uint32_t offset = slot->offset_;
            memcpy(data_ + offset, new_tuple.GetData(), new_tuple.GetStorageSize());
            slot->storage_size_ = new_tuple.GetStorageSize();
            return true;
        }
        // insert
        else {
            uint32_t offset = MoveInsertTuple(new_tuple);
            if(offset != INVALID_OFFSET) {
                // alter slot
                Slot* slot = GetSlot(rid.GetSlotId());
                slot->offset_ = offset;
                slot->storage_size_ = new_tuple.GetStorageSize();
                return true;
            }
            else return false;
        }
    }


}