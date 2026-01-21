#include "storage/disk/disk_scheduler.h"
#include "storage/disk/disk_manager.h"
#include <memory>
#include <utility>

namespace bustub {

    DiskScheduler::DiskScheduler(std::unique_ptr<DiskManager> disk_manager){
        disk_manager_ = std::move(disk_manager);
        // start thread after construct
        background_thread_.emplace(&DiskScheduler::StartWorkerThread, this);    
    }

    DiskScheduler::~DiskScheduler() {
        DiskSchedulerPromise promise;
        DiskRequest poison_pill(false, nullptr, -1, std::move(promise));
        channel_.Put(std::move(poison_pill));

        if (background_thread_.has_value()) {
            // wait
            if (background_thread_->joinable()) {
                background_thread_->join(); 
            }
            // clear thread box
            background_thread_.reset(); 
        }
    }

    auto DiskScheduler::Schedule(DiskRequest dr) -> void {
        channel_.Put(std::move(dr));
    }

    auto DiskScheduler::StartWorkerThread() -> void {
        while (true) {
            auto dr = channel_.Get();

            if (dr.page_id_ == -1 && dr.page_data_ == nullptr) {
                break; 
            }

            if(dr.is_write_) {
                disk_manager_->WritePage(dr.page_id_, dr.page_data_);
            }
            else {
                disk_manager_->ReadPage(dr.page_id_, dr.page_data_);
            }
            dr.callback_.set_value(true);
        }
    }

}