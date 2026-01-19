#pragma once

#include <cassert>
#include <stdexcept>

namespace bustub {

// ==========================================
//  Macro：Ban copy and move
// ==========================================
#define DISALLOW_COPY_AND_MOVE(TypeName) \
  TypeName(const TypeName &) = delete;   \
  TypeName &operator=(const TypeName &) = delete; \
  TypeName(TypeName &&) = delete;        \
  TypeName &operator=(TypeName &&) = delete

// ==========================================
//  (Assert)
//  BUSTUB_ASSERT(x > 0, "x must be positive");
// ==========================================
#define BUSTUB_ASSERT(condition, message) \
  assert((condition) && (message))

// ==========================================
//  Macro: unreachable code
//  ：use in switch default branch，mean program shoud't run to here
// ==========================================
#define UNREACHABLE(message) \
  throw std::logic_error(message)

} // namespace bustub