# 测试注册

# ==========================================
#         Buffer Pool Manager 测试
# ==========================================
add_executable(bpm_test buffer/buffer_pool_manager_test.cpp)
target_link_libraries(
    # 测试程序
    bpm_test 
    # 依赖
    PRIVATE

    bustub_lib 
    gtest 
    gtest_main 
    pthread
)
add_test(NAME bpm_test COMMAND bpm_test)