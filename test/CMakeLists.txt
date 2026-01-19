# ============================================
# Test 1: LRU-K Replacer
# ============================================

# 1. 造一个可执行程序 (Executable)
add_executable(lru_k_replacer_benchmark buffer/lru_k_replacer_benchmark.cpp)

# 2. 链接 (Link)
#    (1) lru_k_replacer_benchmark: 刚造出来的测试空壳
#    (2) bustub_lib:          在 src 里造好的核心库 (如果不链接这个，就找不到 LRUKReplacer 类)
#    (3) gtest_main:          GoogleTest 提供的现成 main 函数 (省得你自己写 int main)
#    (4) pthread:             Linux 系统线程库 (必须加，否则报错)
target_link_libraries(
    lru_k_replacer_benchmark 
    PRIVATE 
    bustub_lib 
    gtest_main 
    gtest       # 某些环境可能需要显式加 gtest
    pthread
)

# 3. 注册测试
#    "以后 ctest 命令的时候，自动运行 lru_k_replacer_benchmark"
add_test(NAME lru_k_replacer_benchmark COMMAND lru_k_replacer_benchmark)