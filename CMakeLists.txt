cmake_minimum_required(VERSION 3.10)
project(ApexSort VERSION 1.0.0 LANGUAGES CXX)

# 1. 强制C++17标准（跨平台兼容）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2. 默认Release模式（避免Debug默认）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Release/Debug)" FORCE)
endif()

# 3. 检测编译器是否支持LTO（关键：适配CLion内置MinGW）
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_ERROR)

# 4. 编译选项配置（移除LTO，修复优化选项冲突）
if(MSVC)
    # Windows MSVC编译器
    set(COMMON_COMPILE_OPTIONS
            /permissive-
            /Zc:__cplusplus
            /wd4996
    )
    set(RELEASE_COMPILE_OPTIONS
            /O2                   # MSVC最高优化
            /DNDEBUG              # 禁用调试
            /D_HAS_ITERATOR_DEBUGGING=0
            /D_SECURE_SCL=0
    )
    set(DEBUG_COMPILE_OPTIONS
            /Od                   # 仅保留调试，禁用优化
            /Zi
            /D_HAS_ITERATOR_DEBUGGING=0
            /D_SECURE_SCL=0
    )
    set(RELEASE_LINK_OPTIONS "")
else()
    # GCC/Clang（适配CLion内置MinGW）
    set(COMMON_COMPILE_OPTIONS
            -Wall
            -Wextra
            -Wpedantic
    )
    set(RELEASE_COMPILE_OPTIONS
            -O3                   # 最高优化（仅Release）
            -march=native         # 适配CPU架构
            -ffast-math           # 浮点优化
            -funroll-loops        # 循环展开
            -fomit-frame-pointer  # 省略帧指针
            -DNDEBUG              # 禁用调试
    )
    set(DEBUG_COMPILE_OPTIONS
            -g                    # 调试信息
            -O0                   # 完全禁用优化（避免-O1/-O3冲突）
            -DNDEBUG              # 禁用STL调试
    )
    # 仅当编译器支持时才启用LTO
    if(LTO_SUPPORTED)
        list(APPEND RELEASE_COMPILE_OPTIONS -flto)
        set(RELEASE_LINK_OPTIONS -flto)
    else()
        set(RELEASE_LINK_OPTIONS "")
        message(STATUS "LTO not supported by compiler, disabled")
    endif()
endif()

# 5. 应用编译选项（按构建类型，避免冲突）
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(${COMMON_COMPILE_OPTIONS} ${RELEASE_COMPILE_OPTIONS})
    if(RELEASE_LINK_OPTIONS)
        add_link_options(${RELEASE_LINK_OPTIONS})
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(${COMMON_COMPILE_OPTIONS} ${DEBUG_COMPILE_OPTIONS})
endif()

# 6. 定义INTERFACE库（原有逻辑保留）
add_library(ApexSort INTERFACE
        tests/pdqsort.h)
target_include_directories(ApexSort INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ==============================================================================
# 测试目标（修复Benchmark优化选项）
# ==============================================================================
option(BUILD_TESTS "Build tests and examples" ON)
if(BUILD_TESTS)
    enable_testing()

    # Example程序
    add_executable(ApexSort_Example examples/basic_usage.cpp)
    target_link_libraries(ApexSort_Example PRIVATE ApexSort)

    # Benchmark程序（强制Release优化，移除LTO）
    add_executable(ApexSort_Benchmark tests/benchmark.cpp)
    target_link_libraries(ApexSort_Benchmark PRIVATE ApexSort)
    # 覆盖Benchmark编译选项（适配MinGW）
    if(MSVC)
        target_compile_options(ApexSort_Benchmark PRIVATE
                /O2 /DNDEBUG /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
        )
    else()
        target_compile_options(ApexSort_Benchmark PRIVATE
                -O3 -march=native -ffast-math -funroll-loops -fomit-frame-pointer -DNDEBUG
        )
        # 仅支持时启用LTO
        if(LTO_SUPPORTED)
            target_compile_options(ApexSort_Benchmark PRIVATE -flto)
            target_link_options(ApexSort_Benchmark PRIVATE -flto)
        endif()
    endif()

    # UnitTest程序（纯Debug，无优化）
    add_executable(ApexSort_UnitTests tests/unit_tests.cpp)
    target_link_libraries(ApexSort_UnitTests PRIVATE ApexSort)
    target_compile_options(ApexSort_UnitTests PRIVATE -g -O0)
    add_test(NAME ApexSort.UnitTests COMMAND ApexSort_UnitTests)
endif()

# ==============================================================================
# 安装逻辑（原有保留）
# ==============================================================================
install(TARGETS ApexSort EXPORT ApexSortTargets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT ApexSortTargets
        FILE ApexSortConfig.cmake
        NAMESPACE apex::
        DESTINATION lib/cmake/ApexSort
)

# 输出配置信息（验证）
message(STATUS "ApexSort build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler supports LTO: ${LTO_SUPPORTED}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release compile options: ${RELEASE_COMPILE_OPTIONS}")
endif()