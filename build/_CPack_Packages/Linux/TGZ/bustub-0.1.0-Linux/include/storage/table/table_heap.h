#pragma once

#include <optional>

#include "common/config.h"
#include "common/rid.h"
#include "storage/table/tuple.h"

/*
    逻辑表：一个双向链表，把一连串 TablePage 串起来，构成一个完整的表
    一张表不是单个页，而是一个双向链表。
    每个页记录 prev_page_id、next_page_id，形成链。
    TableHeap 持有 first_page_id 和 last_page_id，找到表的起点和终点。
    所有页操作都通过 BufferPoolManager 进行。
*/

namespace bustub {

class TablePage;
class BufferPoolManager;

class TableHeap {
 public:
  // ============ Iterator =============
  class TableIterator {
   public:
    // 构造函数
    TableIterator(TableHeap* table_heap, RID rid);

    // 解引用运算符 (*it) -> 获取当前 Tuple
    const Tuple operator*();

    // 箭头运算符 (it->) -> 获取当前 Tuple 的指针
    const Tuple* operator->();

    // 前置自增 (++it) -> 移动到下一条
    TableIterator& operator++();

    // 后置自增 (it++)
    TableIterator operator++(int);

    // 比较运算符 (==, !=)
    bool operator==(const TableIterator& itr) const;
    bool operator!=(const TableIterator& itr) const;

   private:
    TableHeap* table_heap_;
    RID rid_;
    std::optional<Tuple> tuple_cache_{std::nullopt};
    // 为了性能，迭代器内部可能会缓存当前的 Tuple，避免每次解引用都去 Fetch Page
    // 但简单实现可以先不缓存，每次 *it 都去 Fetch。
  };

  TableIterator Begin();
  TableIterator End();

  // ===== structor & destructor ======
  TableHeap(BufferPoolManager* bpm, table_id_t table_id);  // 新建表
  TableHeap(BufferPoolManager* bpm, table_id_t table_id,
            page_id_t first_page_id);  // 从disk读出的表
  ~TableHeap() = default;

  // ========= Logic function =========
  RID InsertTuple(const Tuple& tuple);                // 插入记录
  bool MarkDeleted(const RID rid);                    // 标记删除记录
  bool UpdateTuple(const Tuple& new_tuple, RID rid);  // 更新记录
  Tuple GetTuple(const RID& rid);                     // 获取记录

 private:
  BufferPoolManager* bpm_;
  table_id_t table_id_;
  page_id_t first_page_id_;  // head page pointer
  page_id_t last_page_id_;   // tail page pointer
};

}  // namespace bustub