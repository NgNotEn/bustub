#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "common/config.h"
#include "common/macros.h"

namespace bustub {

class DiskManager {
 public:
  // construct with data directory
  explicit DiskManager(const std::filesystem::path& data_dir);
  // destroy and close all table files
  ~DiskManager();
  // ban copy and move
  DISALLOW_COPY_AND_MOVE(DiskManager);

  // Open or create a table file
  auto OpenTableFile(table_id_t table_id,
                     const std::string& table_name) -> bool;

  // Close a table file
  auto CloseTableFile(table_id_t table_id) -> bool;

  // Delete a table file
  auto DeleteTableFile(table_id_t table_id,
                       const std::string& table_name) -> bool;

  // Get number of pages in a table
  auto GetNumPages(table_id_t table_id) -> std::size_t;

  // Read a page from a table
  auto ReadPage(table_id_t table_id, page_id_t page_id,
                char* page_data) -> void;

  // Write a page to a table
  auto WritePage(table_id_t table_id, page_id_t page_id,
                 const char* page_data) -> void;

 private:
  auto GetTableFilePath(table_id_t table_id,
                        const std::string& table_name) const -> std::string;

  std::string data_dir_;
  std::unordered_map<table_id_t, std::fstream> table_files_;
};

}  // namespace bustub