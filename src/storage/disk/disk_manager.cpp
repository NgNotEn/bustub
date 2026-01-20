#include "storage/disk/disk_manager.h"
#include "common/config.h"
#include "common/exception.h"
#include <cstddef>
#include <iostream>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>

namespace bustub {
    DiskManager::DiskManager(const std::filesystem::path& db_file) {
        try {
            if(!std::filesystem::exists(db_file)){
                std::ofstream o2db{db_file};
                o2db.close();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
        db_file_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
        if(!db_file_.is_open()) throw Exception("Can not open database file.");
    }

    DiskManager::~DiskManager(){
        if(db_file_.is_open()) db_file_.close();
    }

    auto DiskManager::ReadPage(page_id_t page_id, char* page_data) -> void {
        std::size_t offset = static_cast<std::size_t>(page_id) * PAGE_SIZE;
        db_file_.seekg(offset);
        db_file_.read(page_data, PAGE_SIZE);    // output page data to page_data
    }

    auto DiskManager::WritePage(page_id_t page_id, const char* page_data) -> void {
        std::size_t offset = static_cast<std::size_t>(page_id) * PAGE_SIZE;
        db_file_.seekp(offset);
        db_file_.write(page_data, PAGE_SIZE);   // write page data in page
        db_file_.flush();
    }
}