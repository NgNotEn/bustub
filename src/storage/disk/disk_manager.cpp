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

    auto DiskManager::GetNumPages() -> std::size_t {
    // get file size
        db_file_.seekg(0, std::ios::end);
        std::size_t size = db_file_.tellg();
        db_file_.clear();
        db_file_.seekg(0, std::ios::beg); // rollback pointer
    return size / PAGE_SIZE;
}

    auto DiskManager::ReadPage(page_id_t page_id, char* page_data) -> void {
        std::size_t offset = static_cast<std::size_t>(page_id) * PAGE_SIZE;

        db_file_.seekg(0, std::ios::end);
        std::streamsize file_size = db_file_.tellg();

        // 检查是否越界
        if (offset >= static_cast<std::size_t>(file_size)) {
            // 抛出异常（逻辑错误）
            throw std::runtime_error("DiskManager::ReadPage: Page ID out of bound");
            
            // 记录日志并返回（如果是 void 只能 log）
            // LOG_ERROR("ReadPage out of bound: %d", page_id);
            // return;
        }


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