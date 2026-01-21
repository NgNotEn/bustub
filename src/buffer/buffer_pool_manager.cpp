#include "buffer/buffer_pool_manager.h"
#include "buffer/lru_k_replacer.h"
#include "common/config.h"
#include "storage/disk/disk_manager.h"
#include "storage/disk/disk_scheduler.h"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <future>
#include <mutex>

namespace bustub {

    // == constructor and destructor ==
    BufferPoolManager::BufferPoolManager(std::size_t num_pages, std::size_t lru_k, const std::filesystem::path& db_file)
    :pool_size_(num_pages) {
        pages_ = new Page[num_pages];
        replacer_ = std::make_unique<LRUKReplacer>(pool_size_, lru_k);
        auto disk_manager = std::make_unique<DiskManager>(db_file);
        next_page_id_ = disk_manager->GetNumPages();
        disk_scheduler_ = std::make_unique<DiskScheduler>(std::move(disk_manager));
        for (size_t i = 0; i < pool_size_; ++i) {
            pages_[i].ResetMemory();
            free_list_.emplace_back(static_cast<frame_id_t>(i));
        }
    };
    BufferPoolManager::~BufferPoolManager(){
        delete [] pages_;
    }

    // == pin page ==
    inline auto BufferPoolManager::PinPage(frame_id_t frame_id) -> void {
        pages_[frame_id].Pin();
        replacer_->RecordAccess(frame_id);
        replacer_->SetEvictable(frame_id,  false);
    }


    // == flush page == 
    auto BufferPoolManager::FlushPage(page_id_t page_id) -> void{
        if (page_id == INVALID_PAGE_ID) return;
        
        std::lock_guard<std::mutex> lock(latch_);
        // find page
        auto it = page_table_.find(page_id);
        if(it == page_table_.end()) return;
        frame_id_t frame_id = it->second;
        Page* page = &pages_[frame_id];

        // construct promsise-futrue
        DiskScheduler::DiskSchedulerPromise promise;
        auto future = promise.get_future();
        disk_scheduler_->Schedule(DiskRequest{true, page->GetData(),page_id,  std::move(promise)});
        if(future.get()) {
            page -> SetDirty(false);
        }
    }

    auto BufferPoolManager::FlushPageInternal(page_id_t page_id) -> void{
        if (page_id == INVALID_PAGE_ID) return;
        // find page
        auto it = page_table_.find(page_id);
        if(it == page_table_.end()) return;
        frame_id_t frame_id = it->second;
        Page* page = &pages_[frame_id];

        // construct promsise-futrue
        DiskScheduler::DiskSchedulerPromise promise;
        auto future = promise.get_future();
        disk_scheduler_->Schedule(DiskRequest{true, page->GetData(),page_id,  std::move(promise)});
        if(future.get()) {
            page -> SetDirty(false);
        }
    }

    auto BufferPoolManager::FlushAllPages() -> void{
        std::lock_guard<std::mutex> lock(latch_);
        for(std::size_t i = 0; i < pool_size_; ++i) 
            if(pages_[i].IsDirty()) FlushPageInternal(pages_[i].GetPageId());
    }


    // == new page for expand the dataset ==
    auto BufferPoolManager::NewPage() ->Page* {
        std::lock_guard<std::mutex> lock(latch_);
        Page* page = nullptr;
        frame_id_t frame_id;

        // has free frame
        if(!free_list_.empty()) {
            frame_id = free_list_.front();
            free_list_.pop_front();
            page = &pages_[frame_id];
        }
        // need vict
        else {
            if(!replacer_->Evict(&frame_id)) return nullptr;
            page = &pages_[frame_id];
            if(page->IsDirty()) FlushPageInternal(page->page_id_);
            page_table_.erase(page->page_id_);
            page->ResetMemory();
        }
        // end to init
        if(page) {
            page->SetId(next_page_id_++);
            PinPage(frame_id);
            page_table_[page->page_id_] = frame_id;
        }
        return page;
    }

    // == delete page ==
    auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool{
        std::lock_guard<std::mutex> lock(latch_);

        // find page
        auto it = page_table_.find(page_id);
        if(it == page_table_.end()) return true;
        frame_id_t frame_id = it->second;
        Page* page = &pages_[frame_id];

        // check status
        if(page->GetPinCount()) return false;

        // return frame, clear replacer's record, erase hash table
        free_list_.push_back((frame_id));
        replacer_->Remove(frame_id);
        page_table_.erase(page->GetPageId());
        page->ResetMemory();
        return true;
    }


    // == fetch page ==
    auto BufferPoolManager::FetchPage(page_id_t page_id) -> Page* {
        std::lock_guard<std::mutex> lock(latch_);

        // find
        auto it = page_table_.find(page_id);
        frame_id_t frame_id;
        Page* page = nullptr;

        // exist in memory
        if(it != page_table_.end()) {
            PinPage(it->second);
            return &pages_[it->second];
        }

        // on disk
        // get frame and new page
        if(!free_list_.empty()) {
            frame_id = free_list_.front();
            free_list_.pop_front();
            page = &pages_[frame_id];
        } else {
            if(replacer_->Evict(&frame_id)) {
                page = &pages_[frame_id];
                if(page->IsDirty()) FlushPageInternal(page->GetPageId());
                page_table_.erase(page->page_id_);
                page->ResetMemory();
            }
            else return nullptr;
        }
        page->SetId(page_id);
        page_table_[page_id] = frame_id;
        // read from disk
        DiskScheduler::DiskSchedulerPromise promise;
        auto future = promise.get_future();
        disk_scheduler_->Schedule({false, page->GetData(), page_id, std::move(promise)});
        if(future.get()) {
            PinPage(frame_id);
            return page;
        }
        return nullptr;
    }

    auto BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) -> void {
        std::lock_guard<std::mutex> lock(latch_);

        auto it = page_table_.find(page_id);
        frame_id_t frame_id;
        Page* page = nullptr;

        // exist in memory
        if(it != page_table_.end()) {
            frame_id = it->second;
            page =  &pages_[frame_id];
            if(page->pin_count_ > 0) page->Unpin();
            else return;
            if(is_dirty) page->SetDirty(true);
            if(!page->pin_count_) {
                replacer_->SetEvictable(frame_id, true);
            } 
        }
    }
}