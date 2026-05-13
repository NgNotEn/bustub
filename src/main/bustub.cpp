#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "buffer/buffer_pool_manager.h"
#include "catalog/catalog_manager.h"
#include "catalog/schema.h"
#include "main/sql_handlers.h"
#include "parser/sql_parser.h"
#include "storage/disk/disk_manager.h"
#include "type/type_id.h"
#include "type/value.h"

namespace bustub {

std::string TypeIdToString(TypeId type) {
  switch (type) {
    case TypeId::INTEGER:
      return "INT";
    case TypeId::VARCHAR:
      return "VARCHAR";
    default:
      return "UNKNOWN";
  }
}

std::string Trim(const std::string& str) {
  const auto start = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  });
  const auto end = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                     return !std::isspace(ch);
                   }).base();

  if (start >= end) {
    return "";
  }
  return {start, end};
}

std::string ParseExecCommand(const std::string& command) {
  const std::size_t first_quote = command.find('"');
  const std::size_t last_quote = command.rfind('"');

  if (first_quote == std::string::npos || last_quote == std::string::npos ||
      first_quote == last_quote) {
    return "";
  }

  return command.substr(first_quote + 1, last_quote - first_quote - 1);
}

// 评估表达式为 Value（仅支持字面量常数）
// 返回是否成功解析取决于表达式类型
bustub::Value EvaluateExpr(const hsql::Expr* expr) {
  if (expr == nullptr) {
    return bustub::Value(0);  // 默认值
  }

  switch (expr->type) {
    case hsql::kExprLiteralInt:
      return bustub::Value(static_cast<int32_t>(expr->ival));
    case hsql::kExprLiteralFloat:
      return bustub::Value(static_cast<int32_t>(expr->fval));
    case hsql::kExprLiteralString:
      return bustub::Value(
          std::string(expr->name != nullptr ? expr->name : ""));
    case hsql::kExprLiteralNull:
      return bustub::Value(std::string(""));  // NULL 用空字符串表示
    default:
      // 复杂表达式（操作符、函数等）不支持
      return bustub::Value(0);
  }
}

void PrintHelp() {
  std::cout << "Commands:" << std::endl;
  std::cout << "  help                    Show this help" << std::endl;
  std::cout << "  tables                  List all tables" << std::endl;
  std::cout << "  desc <table>            Show table schema" << std::endl;
  std::cout
      << "  [SQL statement]         Execute SQL (with or without semicolon)"
      << std::endl;
  std::cout << "  exit                    Exit" << std::endl;
}

}  // namespace bustub

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: bustub <dbname>" << std::endl;
    return 1;
  }

  const std::string db_name = argv[1];
  const std::filesystem::path db_path = std::filesystem::path("data") / db_name;

  try {
    std::cout << "Opening database: " << db_path.string() << std::endl;

    // Initialize disk manager
    auto disk_manager = std::make_unique<bustub::DiskManager>(db_path);

    // Initialize buffer pool manager (50 pages, LRU-K with k=2)
    auto bpm =
        std::make_unique<bustub::BufferPoolManager>(50, 2, disk_manager.get());

    // Initialize catalog manager
    std::string meta_path = (db_path / "catalog.meta").string();
    auto catalog = std::make_unique<bustub::CatalogManager>(
        bpm.get(), disk_manager.get(), meta_path);

    std::cout << "Database initialized successfully." << std::endl;
    std::cout << "Type 'help' for available commands." << std::endl;

    std::string command;
    bustub::SQLParser sql_parser;

    while (true) {
      std::cout << "bustub> ";

      if (!std::getline(std::cin, command)) {
        break;
      }

      command = bustub::Trim(command);
      // Remove trailing semicolon
      if (!command.empty() && command.back() == ';') {
        command.pop_back();
        command = bustub::Trim(command);
      }
      if (command.empty()) {
        continue;
      }

      if (command == "help") {
        bustub::PrintHelp();
      } else if (command == "exit") {
        std::cout << "Goodbye!" << std::endl;
        break;
      } else if (command == "tables") {
        // List all tables
        auto tables = catalog->GetTableNames();
        if (tables.empty()) {
          std::cout << "No tables found." << std::endl;
        } else {
          std::cout << "Tables:" << std::endl;
          for (const auto& table_name : tables) {
            std::cout << "  - " << table_name << std::endl;
          }
        }
      } else if (command.rfind("desc", 0) == 0) {
        // desc <table_name>
        std::string rest = bustub::Trim(command.substr(4));
        if (rest.empty()) {
          std::cout << "Error: desc requires table name. Use: desc <table_name>"
                    << std::endl;
        } else {
          auto table_info = catalog->GetTable(rest);
          if (table_info == nullptr) {
            std::cout << "Table '" << rest << "' not found" << std::endl;
          } else {
            const auto& schema = table_info->GetSchema();
            std::cout << "Table '" << rest << "' (id=" << table_info->GetId()
                      << ", " << schema.GetColumnCount() << " columns)"
                      << std::endl;
            for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
              const auto& col = schema.GetColumn(i);
              std::string type_str = bustub::TypeIdToString(col.GetType());
              std::cout << "  [" << i << "] " << col.GetName() << " "
                        << type_str;
              if (col.GetType() == bustub::TypeId::VARCHAR) {
                std::cout << "(" << col.GetStorageSize() << ")";
              }
              std::cout << std::endl;
            }
          }
        }
      } else if (command.rfind("exec", 0) == 0) {
        // exec may be used with quotes or without. Allow: exec "sql" OR exec
        // sql
        std::string sql;
        // trim off the 'exec' keyword
        std::string rest = bustub::Trim(command.substr(4));
        if (rest.empty()) {
          std::cout << "Error: exec requires SQL. Use: exec \"your sql\" or "
                       "exec your sql"
                    << std::endl;
        } else if (rest.front() == '"') {
          sql = bustub::ParseExecCommand(command);
          if (sql.empty()) {
            std::cout << "Error: Invalid syntax. Use: exec \"your sql here\" "
                         "or exec your sql"
                      << std::endl;
            continue;
          }
          bustub::ExecSql(sql, sql_parser, catalog.get());
        } else {
          // rest is raw SQL
          bustub::ExecSql(rest, sql_parser, catalog.get());
        }
      } else {
        // Treat unknown input as direct SQL and attempt to execute
        bustub::ExecSql(command, sql_parser, catalog.get());
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
