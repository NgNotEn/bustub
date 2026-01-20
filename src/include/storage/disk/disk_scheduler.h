#pragma once

#include <future>   // for std::promise, std::future
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
        std::promise<bool> callback_;

        DiskRequest(page_id_t page_id, bool is_write, char* page_data)
        : page_id_(page_id), is_write_(is_write), page_data_(page_data) {}

        DiskRequest(DiskRequest &&other) = default;     // move-only
    };

    class DiskScheduler{
    public:
        explicit DiskScheduler(DiskManager *disk_manager);
        ~DiskScheduler();

        auto Schedule(DiskRequest dr) -> void;   // move-only request into queue, but wont block
        auto StartWorkerThread() -> void;       // main loop to process request

        using DiskSchedulerPromise = std::promise<bool>;
    
    private:
        DiskManager* disk_manager_;
        Channel<DiskRequest> channel_;
        std::optional<std::thread> background_thread_;
    };

}