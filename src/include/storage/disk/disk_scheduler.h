#pragma once

#include <future>   // for std::promise, std::future
#include <memory>
#include <optional> // for std::optional
#include <thread>   // for std::thread
#include "common/config.h"
#include "storage/disk/disk_manager.h"
#include "common/channel.h"

namespace bustub {
    /**
    * @brief Represents a Write or Read request for the DiskManager to execute.
    */
    struct DiskRequest {
        using DiskSchedulerPromise = std::promise<bool>;

        /** Flag indicating whether the request is a write or a read. */
        bool is_write_;

        /**
        *  Pointer to the start of the memory location where a page is either:
        *   1. being read into from disk (on a read).
        *   2. being written out to disk (on a write).
        */
        char* page_data_;

        /** ID of the page being read from / written to disk. */
        page_id_t page_id_;

        /** Callback used to signal to the request issuer when the request has been completed. */
        DiskSchedulerPromise callback_;

        DiskRequest(bool is_write, char* page_data, page_id_t page_id, DiskSchedulerPromise promise)
        : page_id_(page_id), is_write_(is_write), page_data_(page_data), callback_(std::move(promise)){}

        DiskRequest(DiskRequest &&other) = default;     // move-only
    };

    class DiskScheduler{
    public:
        using DiskSchedulerPromise = std::promise<bool>;

        explicit DiskScheduler(std::unique_ptr<DiskManager> disk_manager);
        ~DiskScheduler();

        auto Schedule(DiskRequest dr) -> void;   // move-only request into queue, but wont block
        auto StartWorkerThread() -> void;       // main loop to process request

    
    private:
        // heap
        std::unique_ptr<DiskManager> disk_manager_;
        // stack
        Channel<DiskRequest> channel_;
        std::optional<std::thread> background_thread_;
    };

}