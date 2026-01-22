#include "storage/table/table_heap.h"
#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "common/rid.h"
#include "storage/table/table_page.h"
#include "storage/table/tuple.h"
#include <cstdint>

namespace bustub {

    // ====================================
    // ============= Iterator =============
    // ====================================
    TableHeap::TableIterator::TableIterator(TableHeap* table_heap, RID rid)
    :table_heap_(table_heap), rid_(rid) {}


    auto TableHeap::TableIterator::operator*() -> const Tuple {
        if (!tuple_cache_.has_value()) {
            tuple_cache_ = table_heap_->GetTuple(rid_);
        }
        return tuple_cache_.value();
    }

    auto TableHeap::TableIterator::operator->() -> const Tuple* {
        if (!tuple_cache_.has_value()) {
            tuple_cache_ = table_heap_->GetTuple(rid_);
        }
        return &tuple_cache_.value();
    }

    auto TableHeap::TableIterator::operator++() -> TableIterator& {
        page_id_t page_id = rid_.GetPageId();
        uint32_t slot_id = rid_.GetSlotId() + 1;    // next slot
        
        // get TablePage
        while(true) {

            // fetch and pin page
            TablePage* table_page = static_cast<TablePage*>(table_heap_->bpm_->FetchPage(page_id));
            // prepare unppin page
            auto require_to_unpin = page_id;
            if(table_page == nullptr) break;


            TablePage::Header* header = table_page->GetHeader();
            
            for(;slot_id < header->tuple_count_; slot_id++) {
                // get slot
                TablePage::Slot* slot = table_page->GetSlot(slot_id);
                // skip the deleted tuple
                if(slot->storage_size_ == 0) continue;
                
                // else get it
                // alter rid regresh cache
                rid_ = RID{page_id, slot_id};
                tuple_cache_ = table_page->GetTuple(rid_);
                // unpin the page
                table_heap_->bpm_->UnpinPage(require_to_unpin, false);
                return *this;
            }

            // not in this page
            auto next_page_id = header->next_page_id_ ;
            // unpin the page
            table_heap_->bpm_->UnpinPage(require_to_unpin, false);
            
            // has next page
            if(next_page_id != INVALID_PAGE_ID) {
                // next page & refresh slot id
                page_id = next_page_id;
                slot_id = 0;
                
            } else {
                break;
            }

        }
        // end
        rid_ = RID();
        tuple_cache_ = std::nullopt;
        return *this;
    }

    auto TableHeap::TableIterator::operator++(int) -> TableIterator {

        TableIterator to_ret(table_heap_, rid_);
        ++(*this);
        return to_ret;
    }

    auto TableHeap::TableIterator::operator==(const TableIterator &itr) const -> bool{
        return this->table_heap_ == itr.table_heap_ && this->rid_ == itr.rid_;
    }

    auto TableHeap::TableIterator::operator!=(const TableIterator &itr) const -> bool{
        return this->table_heap_ != itr.table_heap_  || this->rid_ != itr.rid_;
    }




    // ====================================
    // ============ TableHeap =============
    // ====================================
    
    // iterator
    auto TableHeap::Begin() -> TableIterator {
        auto fetch_page_id = first_page_id_;
        
        while(true) {
            if(fetch_page_id == INVALID_PAGE_ID) break;
            TablePage* page = static_cast<TablePage*>(bpm_->FetchPage(fetch_page_id));

            if(page == nullptr) {
                return End();
            }
            TablePage::Header* header = page->GetHeader();
            for(uint32_t slot_id = 0; slot_id < header->tuple_count_; slot_id++) {
                TablePage::Slot* slot = page->GetSlot(slot_id);
                // get it
                if(slot->storage_size_ != 0) {
                    bpm_->UnpinPage(fetch_page_id, false);
                    return TableIterator(this, {fetch_page_id, slot_id});
                }
            }
            auto next_page_id = header->next_page_id_;
            bpm_->UnpinPage(fetch_page_id, false);
            fetch_page_id = next_page_id;
        }

        return End();
    }
    auto TableHeap::End() -> TableIterator {
        return TableIterator(this, RID());
    }

    // construct with new table
    TableHeap::TableHeap(BufferPoolManager* bpm)
    :bpm_(bpm){
        TablePage* table_page = static_cast<TablePage*>(bpm_->NewPage());
        if(table_page == nullptr) {
            first_page_id_ = INVALID_PAGE_ID;
            last_page_id_ = INVALID_PAGE_ID;
            return;
        }
        first_page_id_ = table_page->GetPageId();
        last_page_id_ = first_page_id_;
        table_page->Init(first_page_id_);
        bpm_->UnpinPage(first_page_id_, true);
    }

    // open a table
    TableHeap::TableHeap(BufferPoolManager* bpm, page_id_t first_page_id)
    :bpm_(bpm), first_page_id_(first_page_id){
        // transerval the delist to get the last_page_id
        auto fetch_page_id = first_page_id;
        while(true) {
            TablePage* page = static_cast<TablePage*>(bpm_->FetchPage(fetch_page_id));
            if (page == nullptr) break;
            TablePage::Header* header = page->GetHeader();
            auto next_page_id = header->next_page_id_;

            bpm_->UnpinPage(fetch_page_id, false);

            if(next_page_id == INVALID_PAGE_ID) {
                last_page_id_ = fetch_page_id;
                break;
            }
            fetch_page_id = next_page_id;
        }
    }


    // Logic fuctions
    auto TableHeap::InsertTuple(const Tuple& tuple) -> RID {
        // prepare to unpin
        auto fetch_page_id = last_page_id_;
        RID ret_rid{};

        // open the last page
        TablePage* last_page = static_cast<TablePage*>(bpm_->FetchPage(fetch_page_id));
        if(last_page == nullptr) return ret_rid;
        // try insert
        ret_rid= last_page->InsertTuple(tuple);

        // has not enough room
        if(ret_rid.GetPageId() == INVALID_PAGE_ID) {
            TablePage* new_page = static_cast<TablePage*>(bpm_->NewPage());
            // success allocate
            if(new_page != nullptr) {
                auto new_page_id = new_page->GetPageId();
                // to be next
                new_page->Init(new_page_id, last_page_id_);  // push
                // to be prev
                TablePage::Header* header = last_page->GetHeader();
                header->next_page_id_ = new_page_id;
                // change last_page_id
                last_page_id_ = new_page_id;
                // insert in new page
                ret_rid = new_page->InsertTuple(tuple);
                // unpin new page and fetched page
                bpm_->UnpinPage(new_page_id, true);
                bpm_->UnpinPage(fetch_page_id, true);
            } 
            // failure allocate
            else bpm_->UnpinPage(fetch_page_id, false);

        } 
        else bpm_->UnpinPage(fetch_page_id, true);
        
        return ret_rid;
    }

    auto TableHeap::MarkDeleted(const RID rid) -> bool {
        // prepare to unpin
        auto fetch_page_id = rid.GetPageId();
        TablePage* page = static_cast<TablePage*>(bpm_->FetchPage(fetch_page_id));
        if(page != nullptr) {
            if(page->MarkDeleted(rid)) {
                bpm_->UnpinPage(fetch_page_id, true);
                return true;
            }
            bpm_->UnpinPage(fetch_page_id, false);
        }
        return false;
    }
    auto TableHeap::UpdateTuple(const Tuple& new_tuple, RID rid) -> bool {
        // prepare to unpin
        auto fetch_page_id = rid.GetPageId();
        TablePage* page = static_cast<TablePage*>(bpm_->FetchPage(fetch_page_id));
        if(page != nullptr) {
            // ok to insert 
            if(page->UpdateTuple(new_tuple, rid)) {
                bpm_->UnpinPage(fetch_page_id, true);
                return true;
            }
            bpm_->UnpinPage(fetch_page_id, false);
        }
        return false;
    }
    auto TableHeap::GetTuple(const RID& rid) -> Tuple {
        auto fetch_page_id = rid.GetPageId();
        TablePage* page = static_cast<TablePage*>(bpm_->FetchPage(fetch_page_id));
        if(page != nullptr) {
            Tuple tuple = page->GetTuple(rid);
            bpm_->UnpinPage(fetch_page_id, false);
            return tuple;
        }
        return  Tuple();
    }
}