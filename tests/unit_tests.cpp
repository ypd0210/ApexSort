#include <algorithm>
#include <cassert>
#include <cstddef>
#include <random>
#include <vector>

#include "apex_sort.hpp"

namespace {

template <typename T>
void expect_sorted_equals_std_sort(std::vector<T> data) {
    auto expected = data;
    std::sort(expected.begin(), expected.end());

    apex::sort(data);
    assert(data == expected);
}

} // namespace

int main() {
    // Small / edge cases
    expect_sorted_equals_std_sort<int>({});
    expect_sorted_equals_std_sort<int>({1});
    expect_sorted_equals_std_sort<int>({2, 1});
    expect_sorted_equals_std_sort<int>({1, 1, 1, 1});
    expect_sorted_equals_std_sort<int>({1, 2, 3, 4, 5});
    expect_sorted_equals_std_sort<int>({5, 4, 3, 2, 1});
    expect_sorted_equals_std_sort<int>({3, 1, 2, 3, 2, 1, 3});

    // Randomized tests (deterministic seed)
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(-100000, 100000);
    for (int t = 0; t < 200; ++t) {
        std::vector<int> v;
        v.reserve(1000);
        for (std::size_t i = 0; i < 1000; ++i) v.push_back(dist(rng));
        expect_sorted_equals_std_sort(v);
    }

    // Container overload smoke test
    std::vector<int> v = {9, 7, 5, 3, 1, 2, 4, 6, 8, 0};
    apex::sort(v);
    assert(std::is_sorted(v.begin(), v.end()));

    return 0;
}
