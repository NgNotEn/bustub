#pragma once

#include <string>

#include "type/type_id.h"

namespace hsql {
struct Expr;
}

namespace bustub {

class SQLParser;
class CatalogManager;
class Value;

// Forward declarations of helpers defined in bustub.cpp
std::string TypeIdToString(bustub::TypeId type);
std::string Trim(const std::string& str);
std::string ParseExecCommand(const std::string& command);
bustub::Value EvaluateExpr(const hsql::Expr* expr);
void PrintHelp();

// Dispatch SQL statements (moved out of bustub.cpp for modularity)
void ExecSql(const std::string& sql, bustub::SQLParser& sql_parser,
             bustub::CatalogManager* catalog);

}  // namespace bustub
