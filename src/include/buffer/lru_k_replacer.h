#pragma once

/*
The LRU-K algorithm evicts a frame whose backward k-distance is 
maximum of all frames in the replacer. Backward k-distance is 
computed as the difference in time between current timestamp 
and the timestamp of kth previous access. 
A frame with fewer than k historical accesses is given +inf as
its backward k-distance. When multiple frames have +inf backward k-distance, 
the replacer evicts the frame with the earliest overall timestamp 
(i.e., the frame whose least-recent recorded access is the overall least recent access).
*/

#include <cstddef>
#include <limits>
#include <queue>
#include <mutex>
#include <unordered_map>
#include "common/exception.h"
#include "common/config.h"
#include "common/macros.h"




namespace bustub {
    class LRUKReplacer {
    public:
        explicit LRUKReplacer(size_t, size_t);
        
        DISALLOW_COPY_AND_MOVE(LRUKReplacer);

        auto Evict(frame_id_t*) -> bool;    // try to evict frame
        auto RecordAccess(frame_id_t) -> void;      // recorde a access of the frame
        auto SetEvictable(frame_id_t, bool) -> void;// set the frame's status
        auto Remove(frame_id_t) -> void;            // remove the frame what ever 
        auto Size() -> std::size_t;                      // return the size of is_evictable frames

    private:
        struct LRUKNode_{
            std::queue<size_t> history_;
            std::size_t k_;
            bool is_evictable_{false};
            LRUKNode_(std::size_t k):k_(k){};
            auto getKDistance(size_t timestamp) -> std::size_t {
                if(history_.size() < k_) return std::numeric_limits<size_t>::max();
                return timestamp - history_.front();
            }
            auto update(std::size_t timestamp) -> void {
                history_.push(timestamp);
                if(history_.size() > k_) history_.pop();
            }
            auto getEarliestTimestamp() -> size_t {
                return history_.front();
            }
        };
        std::size_t cur_size_{0};
        std::size_t cur_timestamp_{0};
        std::size_t num_frames_;
        std::size_t k_;
        std::unordered_map<frame_id_t, LRUKNode_> frames_;
        std::mutex latch_;
    };
}