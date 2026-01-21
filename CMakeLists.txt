# 定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 定项目名称和语言
project(bustub LANGUAGES CXX)

# 规范编译器
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制使用 C++17 标准，不支持则报错
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用 GCC 特有扩展，保持标准兼容, 纯正的 Standard C++

# 设置输出目录 (编译好的程序统一放到 build/bin 和 build/lib 下)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # 静态库
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # 动态库
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # 可执行文件



# ==========================================
# 测试相关设置
# ==========================================
message(STATUS "Activate GoogleTest ...")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # windows 环境下，强制绑定到动态链接的运行时库

# 启用测试 (必须在 add_subdirectory(test) 之前调用)
# 作用：它会在构建目录下生成一个名为 CTestTestfile.cmake 的文件。
# 只有写了这一行，你在终端里输入 ctest 命令时，CMake 才知道去哪里找测试用例跑
enable_testing()

# ==========================================
# 添加子目录
# ==========================================
# 添加头文件搜索路径
include_directories(third_party/googletest/googletest/include)  # 添加 GoogleTest 头文件搜索路径
include_directories(src/include)  # 添加src头文件搜索路径
# 添加源文件子目录
add_subdirectory(third_party/googletest)  # 添加 GoogleTest 子目录
add_subdirectory(src)   # 添加源代码子目录（包含子CmakeLists）
add_subdirectory(test)  # 添加测试子目录（包含子CmakeLists）
