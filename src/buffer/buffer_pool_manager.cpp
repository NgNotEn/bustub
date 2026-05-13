#include "buffer/buffer_pool_manager.h"

#include <cstddef>
#include <mutex>

#include "buffer/lru_k_replacer.h"
#include "common/config.h"
#include "storage/disk/disk_manager.h"

namespace bustub {

BufferPoolManager::BufferPoolManager(std::size_t num_pages, std::size_t lru_k,
                                     DiskManager* disk_manager)
  : pool_size_(num_pages), disk_manager_(disk_manager) {
  pages_ = new Page[num_pages];
  replacer_ = std::make_unique<LRUKReplacer>(pool_size_, lru_k);
  for (size_t i = 0; i < pool_size_; ++i) {
    pages_[i].ResetMemory();
    free_list_.emplace_back(static_cast<frame_id_t>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  FlushAllPages();
  delete[] pages_;
}

inline auto BufferPoolManager::PinPage(frame_id_t frame_id) -> void {
  pages_[frame_id].Pin();
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);
}

auto BufferPoolManager::FetchPage(table_id_t table_id,
                                  page_id_t page_id) -> Page* {
  std::lock_guard<std::mutex> lock(latch_);

  PageKey key{table_id, page_id};

  // Check if already in memory
  auto it = page_table_.find(key);
  frame_id_t frame_id;
  Page* page = nullptr;

  if (it != page_table_.end()) {
    // Already in buffer
    PinPage(it->second);
    return &pages_[it->second];
  }

  // Need to load from disk
  if (!free_list_.empty()) {
    frame_id = free_list_.front();
    free_list_.pop_front();
    page = &pages_[frame_id];
  } else {
    if (!replacer_->Evict(&frame_id)) return nullptr;
    page = &pages_[frame_id];
    if (page->IsDirty()) {
      // Find old key and flush
      for (auto& [old_key, old_frame_id] : page_table_) {
        if (old_frame_id == frame_id) {
          FlushPageInternal(old_key);
          page_table_.erase(old_key);
          break;
        }
      }
    }
    page->ResetMemory();
  }

  // Load from disk
  page->SetId(page_id);
  page_table_[key] = frame_id;
  try {
    disk_manager_->ReadPage(table_id, page_id, page->GetData());
    PinPage(frame_id);
    return page;
  } catch (...) {
    return nullptr;
  }
}

auto BufferPoolManager::FlushPage(table_id_t table_id,
                                  page_id_t page_id) -> void {
  std::lock_guard<std::mutex> lock(latch_);

  PageKey key{table_id, page_id};
  auto it = page_table_.find(key);
  if (it == page_table_.end()) {
    return;
  }

  frame_id_t frame_id = it->second;
  Page* page = &pages_[frame_id];

  try {
    disk_manager_->WritePage(table_id, page_id, page->GetData());
    page->is_dirty_ = false;
  } catch (...) {
    // Ignore errors for now
  }
}

auto BufferPoolManager::FlushPageInternal(const PageKey& page_key) -> void {
  auto it = page_table_.find(page_key);
  if (it == page_table_.end()) {
    return;
  }

  frame_id_t frame_id = it->second;
  Page* page = &pages_[frame_id];

  try {
    disk_manager_->WritePage(page_key.table_id, page_key.page_id,
                             page->GetData());
    page->is_dirty_ = false;
  } catch (...) {
    // Ignore errors
  }
}

auto BufferPoolManager::UnpinPage(table_id_t table_id, page_id_t page_id,
                                  bool is_dirty) -> void {
  std::lock_guard<std::mutex> lock(latch_);

  PageKey key{table_id, page_id};
  auto it = page_table_.find(key);
  if (it == page_table_.end()) {
    return;
  }

  frame_id_t frame_id = it->second;
  Page* page = &pages_[frame_id];

  if (page->GetPinCount() > 0) {
    page->Unpin();
  }

  if (is_dirty) {
    page->SetDirty(true);
  }

  if (page->GetPinCount() == 0) {
    replacer_->SetEvictable(frame_id, true);
  }
}

auto BufferPoolManager::FlushAllPages() -> void {
  std::lock_guard<std::mutex> lock(latch_);
  for (auto& [key, frame_id] : page_table_) {
    Page* page = &pages_[frame_id];
    if (page->IsDirty()) {
      FlushPageInternal(key);
    }
  }
}

auto BufferPoolManager::NewPage(table_id_t table_id,
                                page_id_t* page_id) -> Page* {
  std::lock_guard<std::mutex> lock(latch_);

  // Get next page id for this table
  if (table_next_page_id_.find(table_id) == table_next_page_id_.end()) {
    table_next_page_id_[table_id] = 0;
  }

  page_id_t new_page_id = table_next_page_id_[table_id]++;
  PageKey key{table_id, new_page_id};

  Page* page = nullptr;
  frame_id_t frame_id;

  // Get free frame
  if (!free_list_.empty()) {
    frame_id = free_list_.front();
    free_list_.pop_front();
    page = &pages_[frame_id];
  } else {
    if (!replacer_->Evict(&frame_id)) return nullptr;
    page = &pages_[frame_id];
    if (page->IsDirty()) {
      for (auto& [old_key, old_frame_id] : page_table_) {
        if (old_frame_id == frame_id) {
          FlushPageInternal(old_key);
          page_table_.erase(old_key);
          break;
        }
      }
    }
    page->ResetMemory();
  }

  // Initialize new page
  page->SetId(new_page_id);
  page_table_[key] = frame_id;
  PinPage(frame_id);

  *page_id = new_page_id;
  return page;
}

auto BufferPoolManager::DeletePage(table_id_t table_id,
                                   page_id_t page_id) -> bool {
  std::lock_guard<std::mutex> lock(latch_);

  PageKey key{table_id, page_id};
  auto it = page_table_.find(key);
  if (it == page_table_.end()) {
    return true;
  }

  frame_id_t frame_id = it->second;
  Page* page = &pages_[frame_id];

  if (page->GetPinCount() > 0) {
    return false;
  }

  free_list_.push_back(frame_id);
  replacer_->Remove(frame_id);
  page_table_.erase(key);
  page->ResetMemory();
  return true;
}

}  // namespace bustub