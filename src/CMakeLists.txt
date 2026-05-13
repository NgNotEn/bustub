# === 设置源文件 ===
set(BUSTUB_SOURCES
    # primer/orset.cpp

    # buffer 管理
    buffer/lru_k_replacer.cpp
    storage/disk/disk_manager.cpp
    buffer/buffer_pool_manager.cpp

    # type 系统
    type/value.cpp
    type/type.cpp
    type/integer_type.cpp
    type/varchar_type.cpp
    
    # catalog
    catalog/column.cpp
    catalog/schema.cpp
    catalog/catalog_meta.cpp
    catalog/table_info.cpp
    catalog/catalog_manager.cpp

    # 表相关
    storage/table/tuple.cpp
    storage/table/table_page.cpp
    storage/table/table_heap.cpp

    # 执行层
    execution/table_scan_executor.cpp
    execution/insert_executor.cpp
    execution/delete_executor.cpp
    execution/update_executor.cpp
    execution/filter_executor.cpp
    execution/select_executor.cpp
    main/sql_handlers.cpp

    # 解析器
    parser/sql_parser.cpp
)

# === 打包成库 ===
# 创建 bustub_lib 的动态库 (SHARED) (静态库使用 STATIC)
add_library(bustub_lib SHARED ${BUSTUB_SOURCES})


# === 指定库的头文件路径 ===
target_include_directories(bustub_lib PUBLIC 
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third_party/sql-parser
  ${CMAKE_SOURCE_DIR}/third_party/sql-parser/src)

target_link_libraries(bustub_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/sql-parser/libsqlparser.so
)