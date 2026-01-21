#pragma once
#include "buffer/lru_k_replacer.h"
#include "common/config.h"
#include "storage/page/page.h"
#include "storage/disk/disk_scheduler.h"
#include <cstddef>
#include <unordered_map>
#include <list>

namespace bustub {
    class BufferPoolManager {
    public:
        explicit BufferPoolManager(std::size_t num_pages, std::size_t lru_k, const std::filesystem::path& db_file);
        ~BufferPoolManager();

        auto FetchPage(page_id_t page_id) -> Page*;
        auto FlushPage(page_id_t page_id) -> void;
        auto UnpinPage(page_id_t page_id, bool is_dirty) -> void;
        auto FlushAllPages() -> void;
        auto NewPage() -> Page*;
        auto DeletePage(page_id_t page_id) -> bool;

    private:
        // function
        inline auto PinPage(frame_id_t frame_id) -> void;
        auto FlushPageInternal(page_id_t page_id) -> void;

        // stack
        std::unordered_map<page_id_t, frame_id_t> page_table_;
        std::list<frame_id_t> free_list_;
        std::atomic<page_id_t> next_page_id_{0};   
        std::mutex latch_; 
        std::size_t pool_size_;
        // heap
        Page *pages_;
        std::unique_ptr<LRUKReplacer> replacer_;
        std::unique_ptr<DiskScheduler> disk_scheduler_;
    };
}