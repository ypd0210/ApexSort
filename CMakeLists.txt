cmake_minimum_required(VERSION 3.10)
project(ApexSort VERSION 1.0.0 LANGUAGES CXX)

# 1. Enforce C++17 standard (cross-platform compatibility)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2. Default to Release mode (avoid Debug default)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Release/Debug)" FORCE)
endif()

# 3. Check if compiler supports LTO (key: adapt to CLion built-in MinGW)
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_ERROR)

# 4. Compile option configuration (remove LTO, fix optimization option conflicts)
if(MSVC)
    # Windows MSVC compiler
    set(COMMON_COMPILE_OPTIONS
            /permissive-
            /Zc:__cplusplus
            /wd4996
    )
    set(RELEASE_COMPILE_OPTIONS
            /O2                   # MSVC highest optimization
            /DNDEBUG              # Disable debugging
            /D_HAS_ITERATOR_DEBUGGING=0
            /D_SECURE_SCL=0
    )
    set(DEBUG_COMPILE_OPTIONS
            /Od                   # Only keep debugging, disable optimization
            /Zi
            /D_HAS_ITERATOR_DEBUGGING=0
            /D_SECURE_SCL=0
    )
    set(RELEASE_LINK_OPTIONS "")
else()
    # GCC/Clang (adapt to CLion built-in MinGW)
    set(COMMON_COMPILE_OPTIONS
            -Wall
            -Wextra
            -Wpedantic
    )
    set(RELEASE_COMPILE_OPTIONS
            -O3                   # Highest optimization (Release only)
            -march=native         # Adapt to CPU architecture
            -ffast-math           # Floating point optimization
            -funroll-loops        # Loop unrolling
            -fomit-frame-pointer  # Omit frame pointer
            -DNDEBUG              # Disable debugging
    )
    set(DEBUG_COMPILE_OPTIONS
            -g                    # Debug information
            -O0                   # Completely disable optimization (avoid -O1/-O3 conflicts)
            -DNDEBUG              # Disable STL debugging
    )
    # Enable LTO only if compiler supports it
    if(LTO_SUPPORTED)
        list(APPEND RELEASE_COMPILE_OPTIONS -flto)
        set(RELEASE_LINK_OPTIONS -flto)
    else()
        set(RELEASE_LINK_OPTIONS "")
        message(STATUS "LTO not supported by compiler, disabled")
    endif()
endif()

# 5. Apply compile options (by build type, avoid conflicts)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(${COMMON_COMPILE_OPTIONS} ${RELEASE_COMPILE_OPTIONS})
    if(RELEASE_LINK_OPTIONS)
        add_link_options(${RELEASE_LINK_OPTIONS})
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(${COMMON_COMPILE_OPTIONS} ${DEBUG_COMPILE_OPTIONS})
endif()

# 6. Define INTERFACE library (keep original logic)
add_library(ApexSort INTERFACE
        tests/pdqsort.h)
target_include_directories(ApexSort INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ==============================================================================
# Test targets (fix Benchmark optimization options)
# ==============================================================================
option(BUILD_TESTS "Build tests and examples" ON)
if(BUILD_TESTS)
    enable_testing()

    # Example program
    add_executable(ApexSort_Example examples/basic_usage.cpp)
    target_link_libraries(ApexSort_Example PRIVATE ApexSort)

    # Benchmark program (force Release optimization, remove LTO)
    add_executable(ApexSort_Benchmark tests/benchmark.cpp)
    target_link_libraries(ApexSort_Benchmark PRIVATE ApexSort)
    # Override Benchmark compile options (adapt to MinGW)
    if(MSVC)
        target_compile_options(ApexSort_Benchmark PRIVATE
                /O2 /DNDEBUG /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
        )
    else()
        target_compile_options(ApexSort_Benchmark PRIVATE
                -O3 -march=native -ffast-math -funroll-loops -fomit-frame-pointer -DNDEBUG
        )
        # Enable LTO only if supported
        if(LTO_SUPPORTED)
            target_compile_options(ApexSort_Benchmark PRIVATE -flto)
            target_link_options(ApexSort_Benchmark PRIVATE -flto)
        endif()
    endif()

    # UnitTest program (pure Debug, no optimization)
    add_executable(ApexSort_UnitTests tests/unit_tests.cpp)
    target_link_libraries(ApexSort_UnitTests PRIVATE ApexSort)
    target_compile_options(ApexSort_UnitTests PRIVATE -g -O0)
    add_test(NAME ApexSort.UnitTests COMMAND ApexSort_UnitTests)
endif()

# ==============================================================================
# Installation logic (keep original)
# ==============================================================================
install(TARGETS ApexSort EXPORT ApexSortTargets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT ApexSortTargets
        FILE ApexSortConfig.cmake
        NAMESPACE apex::
        DESTINATION lib/cmake/ApexSort
)

# Output configuration information (verification)
message(STATUS "ApexSort build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler supports LTO: ${LTO_SUPPORTED}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release compile options: ${RELEASE_COMPILE_OPTIONS}")
endif()