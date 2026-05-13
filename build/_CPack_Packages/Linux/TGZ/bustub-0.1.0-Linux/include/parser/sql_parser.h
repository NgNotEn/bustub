#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SQLParser.h"
#include "SQLParserResult.h"

namespace bustub {

/**
 * SQLParser wrapper.
 *
 * Encapsulates third-party parser usage.
 */
class SQLParser {
 public:
  SQLParser() = default;
  ~SQLParser() = default;

  /**
   * Parse SQL string.
   *
   * @param sql SQL text
   * @return true if parse success
   */
  auto Parse(const std::string &sql) -> bool;

  /**
   * Get parser result.
   */
  auto GetResult() const -> const hsql::SQLParserResult &;

  /**
   * Get error message.
   */
  auto GetErrorMessage() const -> std::string;

 private:
  hsql::SQLParserResult result_;
};

}  // namespace bustub