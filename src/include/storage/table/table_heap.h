#pragma once

#include "common/config.h"
#include "common/rid.h"
#include "storage/table/tuple.h"
#include <optional>


/*
    logic table: infact, a delist(table_page)
*/

namespace bustub{

    class TablePage;
    class BufferPoolManager;


    class TableHeap{
    public:
        // ============ Iterator =============
        class TableIterator {
        public:
            // 构造函数
            TableIterator(TableHeap* table_heap, RID rid);

            // 解引用运算符 (*it) -> 获取当前 Tuple
            auto operator*() -> const Tuple;

            // 箭头运算符 (it->) -> 获取当前 Tuple 的指针
            auto operator->() -> const Tuple*;

            // 前置自增 (++it) -> 移动到下一条
            auto operator++() -> TableIterator&;

            // 后置自增 (it++)
            auto operator++(int) -> TableIterator;

            // 比较运算符 (==, !=)
            auto operator==(const TableIterator &itr) const -> bool;
            auto operator!=(const TableIterator &itr) const -> bool;

        private:
            TableHeap *table_heap_;
            RID rid_;
            std::optional<Tuple> tuple_cache_{std::nullopt};
            // 为了性能，迭代器内部可能会缓存当前的 Tuple，避免每次解引用都去 Fetch Page
            // 但简单实现可以先不缓存，每次 *it 都去 Fetch。
        };
        

        auto Begin() -> TableIterator;
        auto End() -> TableIterator;


        // ===== structor & destructor ======
        TableHeap(BufferPoolManager* bpm);
        TableHeap(BufferPoolManager* bpm_, page_id_t first_page_id);
        ~TableHeap() = default;


        // ========= Logic function =========
        auto InsertTuple(const Tuple& tuple) -> RID;
        auto MarkDeleted(const RID rid) -> bool;
        auto UpdateTuple(const Tuple& new_tuple, RID rid) -> bool;
        auto GetTuple(const RID& rid) -> Tuple;


    private:
        BufferPoolManager* bpm_;
        page_id_t first_page_id_;   // head page pointer
        page_id_t last_page_id_;    // tail page pointer

    };

} // namespace bustub