# CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 项目名称和语言
project(bustub LANGUAGES CXX)
include(GNUInstallDirs)

# 规范编译器
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制使用 C++17 标准，不支持则报错
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用 GCC 特有扩展，保持标准兼容

# 设置输出目录 (编译好的程序统一放到 build/bin 和 build/lib 下)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # 静态库
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # 动态库
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # 可执行文件
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")


# 添加头文件搜索路径
include_directories(third_party/googletest/googletest/include)
include_directories(./include)

# 添加源文件子目录
add_subdirectory(src)   # 添加源代码子目录（包含子CmakeLists）




# 入口程序
add_executable(bustub src/main/bustub.cpp)
target_link_libraries(bustub PRIVATE 
  bustub_lib)

install(TARGETS bustub bustub_lib
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

install(FILES third_party/sql-parser/libsqlparser.so
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(CPACK_PACKAGE_NAME "bustub")
set(CPACK_PACKAGE_VENDOR "bustub")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
include(CPack)

