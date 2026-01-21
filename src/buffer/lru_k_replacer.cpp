#include "buffer/lru_k_replacer.h"
#include "common/exception.h"
#include <cstddef>
#include <cstring>
#include <mutex>

namespace bustub {
    // default constructor
    LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k)
    :num_frames_(num_frames), k_(k){};


    // record a access
    auto LRUKReplacer::RecordAccess(frame_id_t frame_id) -> void {
        if(frame_id >= num_frames_) throw Exception(ExceptionType::OUT_OF_RANGE, "Frame ID out of range.");

        std::lock_guard<std::mutex> lock(latch_);

        // create node case
        auto it = frames_.find(frame_id);
        if(it == frames_.end()) {
            it = frames_.emplace(frame_id, LRUKReplacer::LRUKNode_{k_}).first;
        }
        // update newest timestamp
        it->second.update(cur_timestamp_++);
    }

    auto LRUKReplacer::Evict(frame_id_t* frame_id) -> bool{
        std::lock_guard<std::mutex> lock(latch_);

        // skip empty
        if(cur_size_ == 0) return false;

        frame_id_t victim_fid = -1;
        std::size_t max_k_dist = 0;
        std::size_t min_timestamp = std::numeric_limits<size_t>::max();
        bool victim_found = false;

        for (auto&[fid, lrunode]: frames_) {
            // skip
            if(!lrunode.is_evictable_) continue;
            victim_found = true;
            auto cur_k_dist = lrunode.getKDistance(cur_timestamp_);
            auto cur_min_timestamp = lrunode.getEarliestTimestamp();
            
            // update max_k_distance
            if(cur_k_dist > max_k_dist) {
                victim_fid = fid;
                max_k_dist = cur_k_dist;
                min_timestamp = cur_min_timestamp;
                
            }
            // equal k-distance, choose min_ealier_timestamp
            else if(cur_k_dist == max_k_dist && cur_min_timestamp < min_timestamp) {
                victim_fid = fid;
                max_k_dist = cur_k_dist;
                min_timestamp = cur_min_timestamp;
            }
        }
        *frame_id = victim_fid;
        if(victim_found) {
            frames_.erase(victim_fid);
            cur_size_--;
        }
        return victim_found;
    }


    auto LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) ->void {
        if(frame_id >= num_frames_) throw Exception(ExceptionType::OUT_OF_RANGE, "Frame ID out of range.");

        std::lock_guard<std::mutex> lock(latch_);


        auto it = frames_.find(frame_id);
        if(it == frames_.end()) throw Exception(ExceptionType::OUT_OF_RANGE, "Frame ID not in store.");
        // only diffent status, change cur_size
        if(it->second.is_evictable_ ^ set_evictable) {
            it->second.is_evictable_ = set_evictable;
            if(set_evictable) cur_size_++;
            else cur_size_--;
        }

    }


    auto LRUKReplacer::Remove(frame_id_t frame_id) -> void {
        if(frame_id >= num_frames_) throw Exception(ExceptionType::OUT_OF_RANGE, "Frame ID out of range.");
        std::lock_guard<std::mutex> lock(latch_);
        auto it = frames_.find(frame_id);
        if(it == frames_.end()) return;
        if(it->second.is_evictable_) cur_size_--;
        frames_.erase(it);
    }

    std::size_t LRUKReplacer::Size(){
        std::lock_guard<std::mutex> lock(latch_);
        return cur_size_;
    }

}