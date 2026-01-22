# 测试GTEST注册
include(GoogleTest)


# ==========================================
#           LRU-K REPLACER 测试
# ==========================================
add_executable(lruk_test buffer/lru_k_replacer_test.cpp)
target_include_directories(lruk_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
)
target_link_libraries(
    # 测试程序
    lruk_test PRIVATE
    # 依赖
    bustub_lib 
    gtest 
    gtest_main 
    pthread
)
add_test(NAME lruk_test COMMAND lruk_test)




# ==========================================
#           DISK SCHEDULER 测试
# ==========================================
add_executable(ds_test storage/disk/disk_scheduler_test.cpp)
target_include_directories(ds_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
)
target_link_libraries(
    # 测试程序
    ds_test PRIVATE
    # 依赖
    bustub_lib 
    gtest 
    gtest_main 
    pthread
)
add_test(NAME ds_test COMMAND ds_test)




# ==========================================
#         Buffer Pool Manager 测试
# ==========================================
add_executable(bpm_test buffer/buffer_pool_manager_test.cpp)
# 告诉编译器去哪里找 gtest 的头文件
target_include_directories(bpm_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
)
target_link_libraries(
    # 测试程序
    bpm_test PRIVATE
    # 依赖
    bustub_lib 
    gtest 
    gtest_main 
    pthread
)
add_test(NAME bpm_test COMMAND bpm_test)



# ==========================================
#             TABLE PAGE 测试
# ==========================================
add_executable(table_page_test storage/table/table_page_test.cpp)
# 告诉编译器去哪里找 gtest 的头文件
target_include_directories(table_page_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
)
target_link_libraries(
    # 测试程序
    table_page_test PRIVATE
    # 依赖
    bustub_lib 
    gtest 
    gtest_main 
    pthread
)
add_test(NAME table_page_test COMMAND table_page_test)



# ==========================================
#             TABLE HEAP 测试
# ==========================================
add_executable(table_heap_test storage/table/table_heap_test.cpp)
target_include_directories(table_heap_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
)
target_link_libraries(
    table_heap_test PRIVATE
    bustub_lib 
    gtest 
    gtest_main 
    pthread
)
add_test(NAME table_heap_test COMMAND table_heap_test)



# ==========================================
#              TUPLE 测试
# ==========================================
add_executable(tuple_test storage/table/tuple_test.cpp)
target_include_directories(tuple_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/third_party/googletest/googlemock/include
)
target_link_libraries(
    tuple_test PRIVATE
    bustub_lib 
    gtest 
    gtest_main 
    pthread
)
add_test(NAME tuple_test COMMAND tuple_test)



# ==========================================
#         自动发现并注册测试
# ==========================================
# gtest_discover_tests(lruk_test)
# gtest_discover_tests(bpm_test)
# gtest_discover_tests(ds_test)