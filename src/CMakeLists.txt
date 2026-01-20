# === 设置源文件 ===
# 把所有的 .cpp 实现文件都写在这里
set(BUSTUB_SOURCES
    buffer/lru_k_replacer.cpp
    storage/disk/disk_manager.cpp
    storage/disk/disk_scheduler.cpp
    # primer/orset.cpp
)

# === 打包成库 ===
# 创建 bustub_lib 的动态库 (SHARED) （静态库是 STATIC）
add_library(bustub_lib SHARED ${BUSTUB_SOURCES})

# === 指定库的头文件路径 ===
# 用了bustub_lib，自动找到头文件
target_include_directories(bustub_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)