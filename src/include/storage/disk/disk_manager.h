#pragma once

#include <fstream>
#include <filesystem>
#include <string>

#include "common/config.h"
#include "common/macros.h"
#include "common/exception.h"

namespace bustub {

    class DiskManager{
    public:
        // construct with open a db_file
        explicit DiskManager(const std::filesystem::path &);
        // destroey with close the db_file
        ~DiskManager();
        // ban copy and move
        DISALLOW_COPY_AND_MOVE(DiskManager);


        auto ReadPage(page_id_t, char*) -> void;
        auto WritePage(page_id_t, const char*) -> void;

    private:
        std::fstream db_file_;
        std::string file_name_;
    };

}