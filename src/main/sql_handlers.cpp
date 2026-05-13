#include "main/sql_handlers.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "SQLParserResult.h"
#include "buffer/buffer_pool_manager.h"
#include "catalog/catalog_manager.h"
#include "catalog/schema.h"
#include "execution/delete_executor.h"
#include "execution/execution_context.h"
#include "execution/filter_executor.h"
#include "execution/insert_executor.h"
#include "execution/select_executor.h"
#include "execution/table_scan_executor.h"
#include "execution/update_executor.h"
#include "parser/sql_parser.h"
#include "storage/disk/disk_manager.h"

// Normalize double-quoted string literals to single-quoted so hsql accepts
// SQL like: INSERT INTO t VALUES (1, "alice");
static std::string NormalizeDoubleQuotedStrings(const std::string& s) {
  std::string out = s;
  for (size_t i = 0; i < out.size(); ++i) {
    if (out[i] == '"') {
      out[i] = '\'';
      size_t j = i + 1;
      while (j < out.size() && out[j] != '"') {
        ++j;
      }
      if (j < out.size() && out[j] == '"') {
        out[j] = '\'';
        i = j;
      } else {
        // unmatched quote, leave as-is
      }
    }
  }
  return out;
}

namespace bustub {

void ExecSql(const std::string& sql, bustub::SQLParser& sql_parser,
             bustub::CatalogManager* catalog) {
  std::string normalized_sql = NormalizeDoubleQuotedStrings(sql);
  if (sql_parser.Parse(normalized_sql)) {
    const hsql::SQLParserResult& result = sql_parser.GetResult();
    auto& statements = result.getStatements();
    for (auto statement : statements) {
      // The implementation is identical to the previous one in bustub.cpp.
      // For brevity this file reuses the same logic; it's been moved here
      // to separate SQL handling from the CLI.

      // Handle CREATE TABLE
      if (statement->type() == hsql::kStmtCreate) {
        const hsql::CreateStatement* create_stmt =
            static_cast<const hsql::CreateStatement*>(statement);
        if (create_stmt->type == hsql::kCreateTable) {
          if (create_stmt->tableName == nullptr) {
            std::cout << "CREATE TABLE missing name" << std::endl;
            continue;
          }
          std::string table_name(create_stmt->tableName);
          std::vector<bustub::Column> cols;
          if (create_stmt->columns != nullptr) {
            for (auto col_elem : *create_stmt->columns) {
              auto col_def = dynamic_cast<hsql::ColumnDefinition*>(col_elem);
              if (col_def == nullptr) continue;
              std::string col_name(col_def->name);
              auto dtype = col_def->type.data_type;
              if (dtype == hsql::DataType::INT) {
                cols.emplace_back(col_name, bustub::TypeId::INTEGER);
              } else if (dtype == hsql::DataType::VARCHAR ||
                         dtype == hsql::DataType::TEXT ||
                         dtype == hsql::DataType::CHAR) {
                int64_t len = col_def->type.length;
                if (len <= 0) len = 256;  // default
                cols.emplace_back(col_name, bustub::TypeId::VARCHAR,
                                  static_cast<std::size_t>(len));
              } else {
                std::cout << "Unsupported column type for '" << col_name << "'"
                          << std::endl;
                cols.emplace_back(col_name, bustub::TypeId::INVALID);
              }
            }
          }

          bustub::Schema schema(table_name, cols);
          auto info = catalog->CreateTable(table_name, schema);
          if (info != nullptr) {
            std::cout << "Table '" << table_name
                      << "' created (id=" << info->GetId() << ")" << std::endl;
            if (!cols.empty()) {
              std::cout << "  Columns: ";
              for (size_t i = 0; i < cols.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << cols[i].GetName() << " "
                          << bustub::TypeIdToString(cols[i].GetType());
                if (cols[i].GetType() == bustub::TypeId::VARCHAR) {
                  std::cout << "(" << cols[i].GetStorageSize() << ")";
                }
              }
              std::cout << std::endl;
            }
          } else {
            if (create_stmt->ifNotExists) {
              std::cout << "Notice: Table '" << table_name << "' already exists"
                        << std::endl;
            } else {
              std::cout << "Error: Failed to create table '" << table_name
                        << "'" << std::endl;
            }
          }
          continue;
        }
      }

      // Handle DROP TABLE
      if (statement->type() == hsql::kStmtDrop) {
        const hsql::DropStatement* drop_stmt =
            static_cast<const hsql::DropStatement*>(statement);
        if (drop_stmt->type == hsql::kDropTable) {
          if (drop_stmt->name == nullptr) {
            std::cout << "DROP TABLE missing name" << std::endl;
            continue;
          }
          std::string table_name(drop_stmt->name);
          if (catalog->GetTable(table_name) == nullptr) {
            if (drop_stmt->ifExists) {
              std::cout << "Notice: table '" << table_name << "' does not exist"
                        << std::endl;
            } else {
              std::cout << "Table '" << table_name << "' does not exist"
                        << std::endl;
            }
            continue;
          }

          bool ok = catalog->DropTable(table_name);
          if (ok) {
            std::cout << "Table '" << table_name << "' dropped successfully"
                      << std::endl;
          } else {
            std::cout << "Error: Failed to drop table '" << table_name << "'"
                      << std::endl;
          }
          continue;
        }
      }

      // Handle INSERT
      if (statement->type() == hsql::kStmtInsert) {
        const hsql::InsertStatement* insert_stmt =
            static_cast<const hsql::InsertStatement*>(statement);
        if (insert_stmt->tableName == nullptr) {
          std::cout << "INSERT missing table name" << std::endl;
          continue;
        }
        std::string table_name(insert_stmt->tableName);
        auto table_info = catalog->GetTable(table_name);
        if (table_info == nullptr) {
          std::cout << "Error: Table '" << table_name << "' not found"
                    << std::endl;
          continue;
        }

        // Only support VALUES mode (no INSERT ... SELECT)
        if (insert_stmt->type != hsql::kInsertValues) {
          std::cout << "INSERT ... SELECT not supported yet" << std::endl;
          continue;
        }

        if (insert_stmt->values == nullptr || insert_stmt->values->empty()) {
          std::cout << "INSERT requires VALUES" << std::endl;
          continue;
        }

        const auto& schema = table_info->GetSchema();
        auto values_list = insert_stmt->values;
        if (values_list == nullptr) {
          std::cout << "INSERT VALUES is empty" << std::endl;
          continue;
        }

        if (values_list->size() != schema.GetColumnCount()) {
          std::cout << "Error: Column count mismatch (expected "
                    << schema.GetColumnCount() << ", got "
                    << values_list->size() << ")" << std::endl;
          continue;
        }

        std::vector<bustub::Value> values;
        bool parse_error = false;
        for (size_t i = 0; i < values_list->size(); i++) {
          auto expr = values_list->at(i);
          bustub::Value val = EvaluateExpr(expr);
          if (expr->type != hsql::kExprLiteralInt &&
              expr->type != hsql::kExprLiteralString &&
              expr->type != hsql::kExprLiteralFloat &&
              expr->type != hsql::kExprLiteralNull) {
            std::cout << "Column " << i
                      << " has unsupported expression (only literals supported)"
                      << std::endl;
            parse_error = true;
            break;
          }
          values.push_back(val);
        }

        if (parse_error) {
          continue;
        }

        ExecutionContext exec_ctx(catalog);
        InsertExecutor executor(table_info->GetId(), values);
        executor.Init(&exec_ctx);
        Tuple dummy_tuple;
        executor.Next(&dummy_tuple);
        std::cout << "Inserted 1 row into '" << table_name << "'" << std::endl;
        continue;
      }

      // Handle SELECT
      if (statement->type() == hsql::kStmtSelect) {
        const hsql::SelectStatement* select_stmt =
            static_cast<const hsql::SelectStatement*>(statement);
        if (select_stmt->fromTable == nullptr ||
            select_stmt->fromTable->name == nullptr) {
          std::cout << "SELECT missing FROM clause" << std::endl;
          continue;
        }
        std::string table_name(select_stmt->fromTable->name);
        auto table_info = catalog->GetTable(table_name);
        if (table_info == nullptr) {
          std::cout << "Error: Table '" << table_name << "' not found"
                    << std::endl;
          continue;
        }

        // Simplify: only support SELECT *
        if (select_stmt->selectList == nullptr ||
            select_stmt->selectList->empty()) {
          std::cout << "SELECT requires column list (use *)" << std::endl;
          continue;
        }

        auto first_select = select_stmt->selectList->at(0);
        if (first_select == nullptr || first_select->type != hsql::kExprStar) {
          std::cout << "Only 'SELECT *' is supported" << std::endl;
          continue;
        }

        std::unique_ptr<Executor> exec;
        ExecutionContext exec_ctx(catalog);

        auto table_scan =
            std::make_unique<bustub::TableScanExecutor>(table_info->GetId());

        if (select_stmt->whereClause != nullptr) {
          exec = std::make_unique<bustub::FilterExecutor>(
              std::move(table_scan), select_stmt->whereClause,
              &table_info->GetSchema());
        } else {
          exec = std::move(table_scan);
        }

        bustub::SelectExecutor select_executor(std::move(exec));
        select_executor.Init(&exec_ctx);

        const auto& schema = table_info->GetSchema();
        for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
          if (i > 0) std::cout << " | ";
          std::cout << schema.GetColumn(i).GetName();
        }
        std::cout << std::endl;

        for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
          if (i > 0) std::cout << "-+-";
          std::cout << "--------";
        }
        std::cout << std::endl;

        Tuple result_tuple;
        int row_count = 0;
        while (select_executor.Next(&result_tuple)) {
          for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
            if (i > 0) std::cout << " | ";
            auto val = result_tuple.GetValue(&schema, i);
            std::cout << val.ToString();
          }
          std::cout << std::endl;
          row_count++;
        }
        std::cout << "(" << row_count << " row(s))" << std::endl;
        continue;
      }

      // Handle DELETE
      if (statement->type() == hsql::kStmtDelete) {
        const hsql::DeleteStatement* delete_stmt =
            static_cast<const hsql::DeleteStatement*>(statement);
        if (delete_stmt->tableName == nullptr) {
          std::cout << "DELETE missing table name" << std::endl;
          continue;
        }
        std::string table_name(delete_stmt->tableName);
        auto table_info = catalog->GetTable(table_name);
        if (table_info == nullptr) {
          std::cout << "Error: Table '" << table_name << "' not found"
                    << std::endl;
          continue;
        }

        if (delete_stmt->expr != nullptr) {
          ExecutionContext exec_ctx(catalog);
          auto table_scan =
              std::make_unique<bustub::TableScanExecutor>(table_info->GetId());
          bustub::FilterExecutor filter(std::move(table_scan),
                                        delete_stmt->expr,
                                        &table_info->GetSchema());
          filter.Init(&exec_ctx);

          Tuple filtered_tuple;
          int delete_count = 0;
          while (filter.Next(&filtered_tuple)) {
            auto table_heap = bustub::TableHeap(exec_ctx.catalog_->GetBPM(),
                                                table_info->GetId(),
                                                table_info->GetFirstPageId());
            table_heap.MarkDeleted(filtered_tuple.GetRid());
            delete_count++;
          }
          std::cout << "Deleted " << delete_count << " row(s) from '"
                    << table_name << "'" << std::endl;
          continue;
        }

        ExecutionContext exec_ctx(catalog);
        DeleteExecutor executor(table_info->GetId());
        executor.Init(&exec_ctx);
        Tuple dummy_tuple;
        executor.Next(&dummy_tuple);
        std::cout << "Deleted all rows from '" << table_name << "'"
                  << std::endl;
        continue;
      }

      // Handle UPDATE
      if (statement->type() == hsql::kStmtUpdate) {
        const hsql::UpdateStatement* update_stmt =
            static_cast<const hsql::UpdateStatement*>(statement);
        if (update_stmt->table == nullptr ||
            update_stmt->table->name == nullptr) {
          std::cout << "UPDATE missing table name" << std::endl;
          continue;
        }
        std::string table_name(update_stmt->table->name);
        auto table_info = catalog->GetTable(table_name);
        if (table_info == nullptr) {
          std::cout << "Error: Table '" << table_name << "' not found"
                    << std::endl;
          continue;
        }

        if (update_stmt->where != nullptr) {
          ExecutionContext exec_ctx(catalog);
          auto table_scan =
              std::make_unique<bustub::TableScanExecutor>(table_info->GetId());
          bustub::FilterExecutor filter(std::move(table_scan),
                                        update_stmt->where,
                                        &table_info->GetSchema());
          filter.Init(&exec_ctx);

          if (update_stmt->updates == nullptr ||
              update_stmt->updates->empty()) {
            std::cout << "UPDATE requires SET clause" << std::endl;
            continue;
          }

          const auto& schema = table_info->GetSchema();
          std::unordered_map<std::string, uint32_t> col_index;
          for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
            col_index[schema.GetColumn(i).GetName()] = i;
          }
          std::unordered_map<uint32_t, bustub::Value> updates_map;
          bool parse_error = false;
          for (const auto* update_clause : *update_stmt->updates) {
            if (update_clause == nullptr || update_clause->column == nullptr) {
              std::cout << "Invalid UPDATE clause" << std::endl;
              parse_error = true;
              break;
            }
            auto it = col_index.find(update_clause->column);
            if (it == col_index.end()) {
              std::cout << "Unknown column '" << update_clause->column << "'"
                        << std::endl;
              parse_error = true;
              break;
            }
            uint32_t col_idx = it->second;
            if (update_clause->value == nullptr) {
              std::cout << "Invalid UPDATE value for column '"
                        << update_clause->column << "'" << std::endl;
              parse_error = true;
              break;
            }
            if (update_clause->value->type != hsql::kExprLiteralInt &&
                update_clause->value->type != hsql::kExprLiteralString &&
                update_clause->value->type != hsql::kExprLiteralFloat &&
                update_clause->value->type != hsql::kExprLiteralNull) {
              std::cout
                  << "Column '" << update_clause->column
                  << "' has unsupported expression (only literals supported)"
                  << std::endl;
              parse_error = true;
              break;
            }
            updates_map.emplace(col_idx, EvaluateExpr(update_clause->value));
          }
          if (parse_error) {
            continue;
          }

          Tuple filtered_tuple;
          int update_count = 0;
          while (filter.Next(&filtered_tuple)) {
            // (no debug) build new_values from existing tuple and updates_map

            std::vector<bustub::Value> new_values;
            new_values.reserve(schema.GetColumnCount());
            for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
              auto it2 = updates_map.find(i);
              if (it2 != updates_map.end()) {
                new_values.push_back(it2->second);
              } else {
                new_values.push_back(filtered_tuple.GetValue(&schema, i));
              }
            }

            auto table_heap = bustub::TableHeap(exec_ctx.catalog_->GetBPM(),
                                                table_info->GetId(),
                                                table_info->GetFirstPageId());
            // (no debug) create tuple
            bustub::Tuple new_tuple(new_values,
                                    const_cast<bustub::Schema*>(&schema));
            new_tuple.SetRid(filtered_tuple.GetRid());
            table_heap.UpdateTuple(new_tuple, filtered_tuple.GetRid());
            update_count++;
          }
          std::cout << "Updated " << update_count << " row(s) in '"
                    << table_name << "'" << std::endl;
          continue;
        }

        if (update_stmt->updates == nullptr || update_stmt->updates->empty()) {
          std::cout << "UPDATE requires SET clause" << std::endl;
          continue;
        }

        // No WHERE: apply SET to all rows by scanning the table and updating
        // per-row
        const auto& schema = table_info->GetSchema();

        // Build col name -> index
        std::unordered_map<std::string, uint32_t> col_index;
        for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
          col_index[schema.GetColumn(i).GetName()] = i;
        }
        std::unordered_map<uint32_t, bustub::Value> updates_map;
        bool parse_error = false;
        for (const auto* update_clause : *update_stmt->updates) {
          if (update_clause == nullptr || update_clause->column == nullptr) {
            std::cout << "Invalid UPDATE clause" << std::endl;
            parse_error = true;
            break;
          }
          auto it = col_index.find(update_clause->column);
          if (it == col_index.end()) {
            std::cout << "Unknown column '" << update_clause->column << "'"
                      << std::endl;
            parse_error = true;
            break;
          }
          uint32_t col_idx = it->second;
          if (update_clause->value == nullptr) {
            std::cout << "Invalid UPDATE value for column '"
                      << update_clause->column << "'" << std::endl;
            parse_error = true;
            break;
          }
          if (update_clause->value->type != hsql::kExprLiteralInt &&
              update_clause->value->type != hsql::kExprLiteralString &&
              update_clause->value->type != hsql::kExprLiteralFloat &&
              update_clause->value->type != hsql::kExprLiteralNull) {
            std::cout
                << "Column '" << update_clause->column
                << "' has unsupported expression (only literals supported)"
                << std::endl;
            parse_error = true;
            break;
          }
          updates_map.emplace(col_idx, EvaluateExpr(update_clause->value));
        }
        if (parse_error) {
          continue;
        }

        // Iterate all rows and apply updates
        ExecutionContext exec_ctx(catalog);
        TableHeap table_heap(exec_ctx.catalog_->GetBPM(), table_info->GetId(),
                             table_info->GetFirstPageId());
        int update_count = 0;
        for (auto iter = table_heap.Begin(); iter != table_heap.End(); ++iter) {
          Tuple old_tuple = *iter;
          std::vector<bustub::Value> new_values;
          new_values.reserve(schema.GetColumnCount());
          for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
            auto it2 = updates_map.find(i);
            if (it2 != updates_map.end()) {
              new_values.push_back(it2->second);
            } else {
              new_values.push_back(old_tuple.GetValue(&schema, i));
            }
          }
          bustub::Tuple new_tuple(new_values,
                                  const_cast<bustub::Schema*>(&schema));
          new_tuple.SetRid(old_tuple.GetRid());
          table_heap.UpdateTuple(new_tuple, old_tuple.GetRid());
          update_count++;
        }
        std::cout << "Updated " << update_count << " row(s) in '" << table_name
                  << "'" << std::endl;
        continue;
      }

      // Fallback: print statement type
      std::cout << "Statement type: " << statement->type() << std::endl;
    }
  } else {
    std::cout << sql_parser.GetErrorMessage() << std::endl;
  }
}

}  // namespace bustub
