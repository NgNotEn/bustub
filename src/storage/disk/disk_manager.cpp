#include "storage/disk/disk_manager.h"

#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>

#include "common/config.h"
#include "common/exception.h"

namespace bustub {

DiskManager::DiskManager(const std::filesystem::path& data_dir)
  : data_dir_(data_dir.string()) {
  try {
    if (!std::filesystem::exists(data_dir)) {
      std::filesystem::create_directories(data_dir);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    throw Exception("Failed to create data directory.");
  }
}

DiskManager::~DiskManager() {
  // Close all open table files
  for (auto& [table_id, file] : table_files_) {
    if (file.is_open()) {
      file.close();
    }
  }
}

std::string DiskManager::GetTableFilePath(table_id_t table_id,
                                          const std::string& table_name) const {
  return data_dir_ + "/table_" + std::to_string(table_id) + "_" + table_name +
         ".tbl";
}

bool DiskManager::OpenTableFile(table_id_t table_id,
                                const std::string& table_name) {
  if (table_files_.find(table_id) != table_files_.end()) {
    return true;  // Already open
  }

  std::string file_path = GetTableFilePath(table_id, table_name);

  try {
    // Create file if it doesn't exist
    if (!std::filesystem::exists(file_path)) {
      std::ofstream temp_file(file_path);
      temp_file.close();
    }

    // Open the file
    table_files_[table_id].open(
        file_path, std::ios::binary | std::ios::in | std::ios::out);
    if (!table_files_[table_id].is_open()) {
      return false;
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool DiskManager::CloseTableFile(table_id_t table_id) {
  auto it = table_files_.find(table_id);
  if (it == table_files_.end()) {
    return false;  // Not open
  }

  if (it->second.is_open()) {
    it->second.close();
  }
  table_files_.erase(it);
  return true;
}

bool DiskManager::DeleteTableFile(table_id_t table_id,
                                  const std::string& table_name) {
  // Close the file first
  CloseTableFile(table_id);

  // Delete the file
  try {
    std::string file_path = GetTableFilePath(table_id, table_name);
    if (std::filesystem::exists(file_path)) {
      std::filesystem::remove(file_path);
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

std::size_t DiskManager::GetNumPages(table_id_t table_id) {
  auto it = table_files_.find(table_id);
  if (it == table_files_.end()) {
    return 0;
  }

  std::fstream& file = it->second;
  file.seekg(0, std::ios::end);
  std::size_t size = file.tellg();
  file.clear();
  file.seekg(0, std::ios::beg);
  return size / PAGE_SIZE;
}

void DiskManager::ReadPage(table_id_t table_id, page_id_t page_id,
                           char* page_data) {
  auto it = table_files_.find(table_id);
  if (it == table_files_.end()) {
    throw Exception("Table file not open: " + std::to_string(table_id));
  }

  std::fstream& file = it->second;
  std::size_t offset = static_cast<std::size_t>(page_id) * PAGE_SIZE;

  file.seekg(0, std::ios::end);
  std::streamsize file_size = file.tellg();
  file.clear();

  if (offset >= static_cast<std::size_t>(file_size)) {
    throw std::runtime_error("DiskManager::ReadPage: Page ID out of bound");
  }

  file.seekg(offset);
  file.read(page_data, PAGE_SIZE);
  file.clear();
}

void DiskManager::WritePage(table_id_t table_id, page_id_t page_id,
                            const char* page_data) {
  auto it = table_files_.find(table_id);
  if (it == table_files_.end()) {
    throw Exception("Table file not open: " + std::to_string(table_id));
  }

  std::fstream& file = it->second;
  std::size_t offset = static_cast<std::size_t>(page_id) * PAGE_SIZE;

  file.seekp(offset);
  file.write(page_data, PAGE_SIZE);
  file.flush();
}

}  // namespace bustub