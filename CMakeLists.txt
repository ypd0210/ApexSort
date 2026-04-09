cmake_minimum_required(VERSION 3.10)
project(ApexSort VERSION 1.0.0 LANGUAGES CXX)

# 1. 强制执行 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 定义一个 INTERFACE 库 (Header-only 库的标准做法)
# 这告诉 CMake：ApexSort 是一个没有源文件的库，只有接口
add_library(ApexSort INTERFACE)

# 3. 指定头文件包含路径
# INTERFACE 表示：任何链接到 ApexSort 的目标，都会自动获得这个包含路径
target_include_directories(ApexSort INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ==============================================================================
# 以下是开发阶段的辅助目标 (仅在本地开发时生效，不影响外部依赖)
# ==============================================================================
option(BUILD_TESTS "Build tests and examples" ON)

if(BUILD_TESTS)
    # 示例程序
    add_executable(ApexSort_Example examples/basic_usage.cpp)
    target_link_libraries(ApexSort_Example PRIVATE ApexSort)

    # 基准测试程序
    add_executable(ApexSort_Benchmark tests/benchmark.cpp)
    target_link_libraries(ApexSort_Benchmark PRIVATE ApexSort)
endif()

# ==============================================================================
# 安装逻辑 (让用户能通过 'make install' 安装到系统)
# ==============================================================================
install(TARGETS ApexSort EXPORT ApexSortTargets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT ApexSortTargets
        FILE ApexSortConfig.cmake
        NAMESPACE apex::
        DESTINATION lib/cmake/ApexSort
)