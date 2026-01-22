# === 设置源文件 ===
set(BUSTUB_SOURCES
    # primer/orset.cpp

    # buffer 管理子系统
    buffer/lru_k_replacer.cpp
    storage/disk/disk_manager.cpp
    storage/disk/disk_scheduler.cpp
    buffer/buffer_pool_manager.cpp

    # type 系统
    type/value.cpp
    type/type.cpp
    type/integer_type.cpp
    type/varlen_type.cpp
    
    # catalog
    catalog/column.cpp
    catalog/schema.cpp

    # storage table
    storage/table/tuple.cpp
    storage/table/table_page.cpp
    storage/table/table_heap.cpp
)

# === 打包成库 ===
# 创建 bustub_lib 的动态库 (SHARED) (静态 库使用 STATIC)
add_library(bustub_lib SHARED ${BUSTUB_SOURCES})

# === 指定库的头文件路径 ===
target_include_directories(bustub_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)