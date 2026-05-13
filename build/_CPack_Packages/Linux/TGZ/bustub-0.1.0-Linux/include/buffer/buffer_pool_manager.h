#pragma once
#include <cstddef>
#include <list>
#include <unordered_map>

#include "buffer/lru_k_replacer.h"
#include "common/config.h"
#include "storage/disk/disk_manager.h"
#include "storage/page/page.h"

namespace bustub {

class BufferPoolManager {
 public:
  explicit BufferPoolManager(std::size_t num_pages, std::size_t lru_k,
                             DiskManager* disk_manager);
  ~BufferPoolManager();

  // Fetch a page (per-table)
  auto FetchPage(table_id_t table_id, page_id_t page_id) -> Page*;

  // Flush a page
  auto FlushPage(table_id_t table_id, page_id_t page_id) -> void;

  // Unpin a page
  auto UnpinPage(table_id_t table_id, page_id_t page_id, bool is_dirty) -> void;

  // Flush all pages
  auto FlushAllPages() -> void;

  // Create new page (per-table)
  auto NewPage(table_id_t table_id, page_id_t* page_id) -> Page*;

  // Delete a page
  auto DeletePage(table_id_t table_id, page_id_t page_id) -> bool;

 private:
  // Composite key: (table_id, page_id) -> frame_id
  struct PageKey {
    table_id_t table_id;
    page_id_t page_id;

    bool operator==(const PageKey& other) const {
      return table_id == other.table_id && page_id == other.page_id;
    }
  };

  struct PageKeyHash {
    std::size_t operator()(const PageKey& key) const {
      return std::hash<uint64_t>()((static_cast<uint64_t>(key.table_id) << 32) |
                                   static_cast<uint64_t>(key.page_id));
    }
  };

  // function
  inline auto PinPage(frame_id_t frame_id) -> void;
  auto FlushPageInternal(const PageKey& page_key) -> void;

  // Per-table next page id
  std::unordered_map<table_id_t, page_id_t> table_next_page_id_;

  // Composite key page table: (table_id, page_id) -> frame_id
  std::unordered_map<PageKey, frame_id_t, PageKeyHash> page_table_;

  std::list<frame_id_t> free_list_;
  std::mutex latch_;
  std::size_t pool_size_;

  // heap
  Page* pages_;
  std::unique_ptr<LRUKReplacer> replacer_;

  // Disk manager (shared across tables)
  DiskManager* disk_manager_;
};

}  // namespace bustub