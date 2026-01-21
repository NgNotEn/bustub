#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>

#include "buffer/buffer_pool_manager.h"
#include "storage/table/table_page.h"
#include "storage/table/tuple.h"
#include "catalog/schema.h"
#include "catalog/column.h"
#include "type/value.h"

using namespace bustub;

// === 辅助工具 ===
std::string RandomString(int len) {
    std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string newstr;
    for (int i = 0; i < len; ++i) {
        newstr += str[rand() % str.length()];
    }
    return newstr;
}

int main() {
    // 0. 环境准备
    const std::string db_file = "test_table_page.db";
    remove(db_file.c_str()); 

    // 初始化 BPM
    auto *bpm = new BufferPoolManager(10, 2, db_file);

    // 定义 Schema
    std::vector<Column> cols;
    cols.emplace_back("id", TypeId::INTEGER);
    // VARCHAR(32) 足够容纳后面的测试字符串
    cols.emplace_back("name", TypeId::VARCHAR, 32); 
    
    Schema schema("test_table", cols);

    std::cout << "=== [Start] TablePage Comprehensive Benchmark ===" << std::endl;

    // ========================================================
    // Test 1: 初始化与插入 (Insert)
    // ========================================================
    std::cout << "\n[Test 1] Insert until near full..." << std::endl;
    
    Page *page = bpm->NewPage();
    assert(page != nullptr);
    page_id_t page_id = page->GetPageId();

    auto *table_page = reinterpret_cast<TablePage *>(page);
    
    table_page->Init(page_id, INVALID_PAGE_ID, INVALID_PAGE_ID);

    std::vector<RID> rids;
    int tuple_count = 0;

    // 插入 40 条数据
    for (int i = 0; i < 40; ++i) {
        Value v_id(i);
        Value v_name("val_" + std::to_string(i));
        Tuple tuple({v_id, v_name}, &schema);

        RID rid = table_page->InsertTuple(tuple);
        if (rid.GetPageId() == INVALID_PAGE_ID) {
            std::cout << "Page full unexpectedly at " << i << std::endl;
            break;
        }
        rids.push_back(rid);
        tuple_count++;
    }
    std::cout << "-> Inserted " << tuple_count << " tuples. Free space: " << table_page->GetFreeSpaceRemaining() << std::endl;

    // ========================================================
    // Test 2: 读取验证 (Read)
    // ========================================================
    std::cout << "\n[Test 2] Verify inserted data..." << std::endl;
    for (int i = 0; i < tuple_count; ++i) {
        Tuple t = table_page->GetTuple(rids[i]);
        assert(t.GetValue(&schema, 0).GetAsInteger() == i);
        
        std::string expected_name = "val_" + std::to_string(i);
        assert(t.GetValue(&schema, 1).ToString() == expected_name);
    }
    std::cout << "-> All tuples verified." << std::endl;

    // ========================================================
    // Test 3: 原地更新 (Update In-Place)
    // ========================================================
    std::cout << "\n[Test 3] Update Tuple (Same Size)..." << std::endl;
    RID update_rid = rids[0];
    Tuple old_tuple = table_page->GetTuple(update_rid);
    
    Value v_id_new(9999);
    Value v_name_new("new_0"); 
    Tuple new_tuple({v_id_new, v_name_new}, &schema);

    bool res = table_page->UpdateTuple(new_tuple, old_tuple, update_rid);
    assert(res == true);

    Tuple updated_t = table_page->GetTuple(update_rid);
    assert(updated_t.GetValue(&schema, 0).GetAsInteger() == 9999);
    assert(updated_t.GetValue(&schema, 1).ToString() == "new_0");
    std::cout << "-> Update In-Place success." << std::endl;

    // ========================================================
    // Test 4: 更新较长字符串 (Update Larger String)
    // ========================================================
    std::cout << "\n[Test 4] Update Tuple (Larger String Content)..." << std::endl;
    RID move_rid = rids[1];
    Tuple old_tuple_2 = table_page->GetTuple(move_rid);

    Value v_name_large("large_string_update_test"); 
    Tuple larger_tuple({old_tuple_2.GetValue(&schema, 0), v_name_large}, &schema);

    // 对于定长 Tuple 实现，StorageSize 不会变，所以空间可能不会减少，这里不强制检查 space_after < space_before
    res = table_page->UpdateTuple(larger_tuple, old_tuple_2, move_rid);
    
    if (res) {
        Tuple t = table_page->GetTuple(move_rid);
        assert(t.GetValue(&schema, 1).ToString() == "large_string_update_test");
        std::cout << "-> Update success (Value Updated Correctly)." << std::endl;
    } else {
        std::cout << "-> Update failed (Page full)." << std::endl;
    }

    // ========================================================
    // Test 5: 删除 (Mark Delete)
    // ========================================================
    std::cout << "\n[Test 5] Delete Tuple..." << std::endl;
    RID delete_rid = rids[2];
    table_page->MarkDelete(delete_rid);

    Tuple deleted_t = table_page->GetTuple(delete_rid);
    assert(deleted_t.GetStorageSize() == 0); 
    std::cout << "-> Delete success." << std::endl;

    // ========================================================
    // Test 6: 持久化验证 (Persistence)
    // ========================================================
    std::cout << "\n[Test 6] Persistence Check..." << std::endl;
    bpm->UnpinPage(page_id, true);
    bpm->FlushPage(page_id);
    std::cout << "-> Flushed page to disk." << std::endl;

    Page *page_again = bpm->FetchPage(page_id);
    assert(page_again != nullptr);
    auto *table_page_again = reinterpret_cast<TablePage *>(page_again);
    
    Tuple t_0 = table_page_again->GetTuple(rids[0]);
    assert(t_0.GetValue(&schema, 1).ToString() == "new_0");

    Tuple t_2 = table_page_again->GetTuple(rids[2]);
    assert(t_2.GetStorageSize() == 0);

    std::cout << "-> Persistence verified." << std::endl;

    bpm->UnpinPage(page_id, false);
    delete bpm;
    remove(db_file.c_str());

    std::cout << "\n=== [Success] All Benchmark Tests Passed! ===" << std::endl;
    return 0;
}