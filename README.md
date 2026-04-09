# ApexSort 🚀

**ApexSort** is a high-performance, industrial-grade hybrid sorting library for C++. It is designed to be a robust and faster alternative to `std::sort`, specifically optimized for modern CPU architectures and challenging data distributions.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

## 🌟 Key Features

- **Header-Only**: Zero dependencies. Just drop `apex_sort.hpp` into your project.
- **Worst-case $O(n \log n)$**: Guaranteed via Introspective sorting (fallback to HeapSort).
- **3-Way Partitioning**: Handles datasets with massive amounts of duplicate elements in nearly $O(n)$ time.
- **Tukey's Ninther**: Advanced pivot selection to prevent degradation on sorted or reverse-sorted data.
- **Hardware-Aware**:
    - Cache-aligned thresholds for Insertion Sort.
    - Shift-based move semantics to reduce memory writes.
    - Pointer-arithmetic optimization to maximize CPU pipeline efficiency.

## 🚀 Quick Start

### Integration
Since ApexSort is header-only, simply copy `include/apex_sort.hpp` to your project or include the directory in your build system.

### Basic Usage
```cpp
#include "apex_sort.hpp"
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {54, 26, 93, 17, 77, 31, 44, 55, 20};
    
    // Sort the vector using ApexSort
    apex::sort(data);

    for(int x : data) std::cout << x << " "; // Output: 17 20 26 31 44 54 55 77 93
    return 0;
}