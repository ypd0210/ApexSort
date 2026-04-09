#include <iostream>
#include <vector>
#include "apex_sort.hpp"

int main() {
    // 1. 随机数据
    std::vector<int> data = {54, 26, 93, 17, 77, 31, 44, 55, 20};
    
    std::cout << "Original: ";
    for(int x : data) std::cout << x << " ";
    std::cout << "\n";

    // 使用 ApexSort
    apex::sort(data);

    std::cout << "Sorted:   ";
    for(int x : data) std::cout << x << " ";
    std::cout << "\n";

    return 0;
}