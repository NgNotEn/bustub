#pragma once

#include <cstdint>

namespace bustub {

// ==========================================
//  basic type
// ==========================================

// 磁盘页 id
using page_id_t = uint32_t;

// 页框 id
using frame_id_t = uint32_t;

// 表id
using table_id_t = uint32_t;

// // (Transaction ID)
// using txn_id_t = uint32_t;

// // (Log Sequence Number)
// using lsn_t = uint32_t;

// ==========================================
//  const value define
// ==========================================

// 页尺寸 4KB
static constexpr int PAGE_SIZE = 4096;  // Bytes
// 非法 id
static constexpr page_id_t INVALID_PAGE_ID = -1;
// // invalid Transaction ID
// static constexpr txn_id_t INVALID_TXN_ID = -1;
// // invalid LSN
// static constexpr lsn_t INVALID_LSN = -1;

// 非法偏移值
static constexpr uint32_t INVALID_OFFSET = -1;
// 起始页 id
static constexpr page_id_t HEADER_PAGE_ID = 0;

}  // namespace bustub