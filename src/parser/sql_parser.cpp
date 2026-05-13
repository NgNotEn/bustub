#include "parser/sql_parser.h"

#include <iostream>

namespace bustub {

auto SQLParser::Parse(const std::string &sql) -> bool {
  result_.reset();

  hsql::SQLParser::parse(sql, &result_);

  return result_.isValid();
}

auto SQLParser::GetResult() const
    -> const hsql::SQLParserResult & {
  return result_;
}

auto SQLParser::GetErrorMessage() const -> std::string {
  if (result_.isValid()) {
    return "Valid sql.";
  }

  return result_.errorMsg();
}

}  // namespace bustub