cmake_minimum_required(VERSION 3.10)
project(ApexSort VERSION 1.0.0 LANGUAGES CXX)

# 1. Enforce the C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Define an INTERFACE library (standard for header-only libraries)
# This tells CMake: ApexSort has no source files, only headers (interface)
add_library(ApexSort INTERFACE)

# 3. Specify include directories
# INTERFACE means: any target that links ApexSort will inherit these include paths
target_include_directories(ApexSort INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ==============================================================================
# Development-only helper targets (local use; no impact on external consumers)
# ==============================================================================
option(BUILD_TESTS "Build tests and examples" ON)

if(BUILD_TESTS)
    enable_testing()

    # Example program
    add_executable(ApexSort_Example examples/basic_usage.cpp)
    target_link_libraries(ApexSort_Example PRIVATE ApexSort)

    # Benchmark program
    add_executable(ApexSort_Benchmark tests/benchmark.cpp)
    target_link_libraries(ApexSort_Benchmark PRIVATE ApexSort)

    # Unit tests
    add_executable(ApexSort_UnitTests tests/unit_tests.cpp)
    target_link_libraries(ApexSort_UnitTests PRIVATE ApexSort)
    add_test(NAME ApexSort.UnitTests COMMAND ApexSort_UnitTests)
endif()

# ==============================================================================
# Install logic (so users can 'make install')
# ==============================================================================
install(TARGETS ApexSort EXPORT ApexSortTargets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT ApexSortTargets
        FILE ApexSortConfig.cmake
        NAMESPACE apex::
        DESTINATION lib/cmake/ApexSort
)