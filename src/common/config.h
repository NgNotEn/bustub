#pragma once

#include <cstdint>
#include <atomic>
#include <chrono>

namespace bustub {

// ==========================================
//  basic type
// ==========================================

// (Page ID) -  ID on disk
using page_id_t = int32_t;

// (Frame ID) - ID in memery
using frame_id_t = int32_t;

// (Transaction ID)
using txn_id_t = int32_t;

// (Log Sequence Number)
using lsn_t = int32_t;

// ==========================================
//  const value define
// ==========================================

// invalid Page ID
static constexpr page_id_t INVALID_PAGE_ID = -1;
// invalid Transaction ID
static constexpr txn_id_t INVALID_TXN_ID = -1;
// invalid LSN
static constexpr lsn_t INVALID_LSN = -1;
// Start of Page ID
static constexpr page_id_t HEADER_PAGE_ID = 0;
// Page Size (4KB)
static constexpr int PAGE_SIZE = 4096;  // Bytes

} 