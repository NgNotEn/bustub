#pragma once
#include "common/config.h"
#include <cassert>
#include <shared_mutex>
#include <cstring>
namespace bustub {
    // class ReaderWriterLatch{
    // public:
    //     // read-shared-lock
    //     auto RLatch() -> void {
    //         std::unique_lock<std::mutex> lock(mutex_);
    //         cv_.wait(lock, [this]() { return !is_writing_; });
    //         reader_cnt_++;
    //     }
    //     // read-shared-unlock
    //     auto RUnlatch() -> void {
    //         std::unique_lock<std::mutex> lock(mutex_);
    //         reader_cnt_--;
    //         if(reader_cnt_ == 0) {
    //             cv_.notify_all();
    //         }
    //     }

    //     // read-unique-lock
    //     auto WLatch() -> void {
    //         std::unique_lock<std::mutex> lock(mutex_);
    //         cv_.wait(lock, [&](){return !reader_cnt_;});
    //         is_writing_++;
            
    //     }

    //     // read-unique-unlock
    //     auto WUnlatch() -> void {
    //         std::unique_lock<std::mutex> lock(mutex_);
    //         is_writing_--;
    //         cv_.notify_all();
    //     }

    // private:
    //     std::shared_mutex mutex_;
    // };

    class ReaderWriterLatch{
    public:
        // read-shared-lock
        auto RLatch() -> void { mutex_.lock_shared(); }
        // read-shared-unlock
        auto RUnlatch() -> void { mutex_.unlock_shared(); }

        // read-unique-lock
        auto WLatch() -> void { mutex_.lock(); }

        // read-unique-unlock
        auto WUnlatch() -> void { mutex_.unlock(); };

    private:
        std::shared_mutex mutex_;
    };

    class Page {
        friend class BufferPoolManager;
    protected:
        /** real data (4KB) */
        char data_[PAGE_SIZE];
        /** page_id */
        page_id_t page_id_{INVALID_PAGE_ID};
        /** pined counter */
        int pin_count_{0};
        /** dirty page need write back to disk */
        bool is_dirty_{false};
        /** read / write latch */
        ReaderWriterLatch rwlatch_;

        auto ResetMemory() -> void {
            std::memset(data_, 0, PAGE_SIZE);
            page_id_ = INVALID_PAGE_ID;
            is_dirty_ = false;
        }
        auto SetId(page_id_t page_id) -> void {
            page_id_ = page_id;
        }
    public:
        Page() = default;
        ~Page() = default;

        // get data
        auto GetData() -> char*  {
            return data_;
        }
        auto GetPageId () const -> page_id_t  {
            return page_id_;
        }
        auto GetPinCount() const -> int  {
            return pin_count_;
        }

        // check status
        auto IsDirty() const -> bool  { return is_dirty_; }
        auto SetDirty(bool is_dirty) -> void { 
            is_dirty_ |= is_dirty; 
        };

        // conccurence control
        auto RLatch() -> void { rwlatch_.RLatch(); }
        auto RUnlatch() -> void { rwlatch_.RUnlatch(); }
        auto WLatch() -> void { rwlatch_.WLatch(); }
        auto WUnlatch() -> void { rwlatch_.WUnlatch(); }

    private:
        auto Pin() -> void { pin_count_++; }
        auto Unpin() -> void { 
            assert(pin_count_ > 0);
            pin_count_--;
        }
    };
}
