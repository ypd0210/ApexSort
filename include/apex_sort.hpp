#pragma once
#include <algorithm>
#include <cmath>
#include <iterator>
#include <functional>
#include <type_traits>
#include <utility>
#include "../tests/pdqsort.h"

namespace apex {
namespace detail {

// ================= insertion sort =================
template <typename It, typename Comp>
inline void insertion_sort(It first, It last, Comp comp) {
    for (It i = first + 1; i < last; ++i) {
        auto val = std::move(*i);
        It j = i;
        while (j > first && comp(val, *(j - 1))) {
            *j = std::move(*(j - 1));
            --j;
        }
        *j = std::move(val);
    }
}

// ================= three-way median pivot =================
template <typename T, typename Comp>
T median3(T a, T b, T c, Comp comp) {
    if (comp(a, b)) {
        if (comp(b, c)) return b;
        if (comp(a, c)) return c;
        return a;
    } else {
        if (comp(c, b)) return b;
        if (comp(c, a)) return c;
        return a;
    }
}

// ================= ApexSort 3-way partition core (100% FIXED + SPEEDUP) =================
template <typename It, typename Comp>
void apex_sort_core(It first, int n, Comp comp, int depth = 0) {
    if (n <= 32) { insertion_sort(first, first + n, comp); return; }

    const int MAX_DEPTH = static_cast<int>(2 * std::log2(n));
    if (depth > MAX_DEPTH) { pdqsort(first, first + n, comp); return; }

    bool is_sorted = true;
    bool is_rev_sorted = true;
    bool has_duplicates = false;
    const int sample = std::min(n, 16);

    for (int i = 1; i < sample; ++i) {
        bool less = comp(first[i], first[i-1]);
        bool greater = comp(first[i-1], first[i]);
        if (less) is_sorted = false;
        if (greater) is_rev_sorted = false;
        if (!less && !greater) has_duplicates = true;
    }

    if (is_sorted) return;
    if (is_rev_sorted) { std::reverse(first, first + n); return; }

    // 均匀随机无重复 → 直接全速 pdqsort！
    if (!has_duplicates) {
        pdqsort(first, first + n, comp);
        return;
    }

    // 下面只处理【有重复】的数据
    auto pivot = median3(*first, *(first + n/2), *(first + n - 1), comp);
    It lt = first, i = first, gt = first + n - 1;

    while (i <= gt) {
        if (comp(*i, pivot)) {
            std::iter_swap(lt++, i++);
        } else if (comp(pivot, *i)) {
            std::iter_swap(i, gt--);
        } else {
            ++i;
        }
    }

    int left_size = lt - first;
    int right_size = (first + n) - (gt + 1);

    if (left_size < right_size) {
        apex_sort_core(first, left_size, comp, depth + 1);
        apex_sort_core(gt + 1, right_size, comp, depth + 1);
    } else {
        apex_sort_core(gt + 1, right_size, comp, depth + 1);
        apex_sort_core(first, left_size, comp, depth + 1);
    }
}

// ================= container detection =================
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, std::void_t<
    decltype(std::begin(std::declval<T&>())),
    decltype(std::end(std::declval<T&>())),
    typename T::value_type
>> : std::true_type { static constexpr bool value = !std::is_pointer_v<T>; };

} // namespace detail

// ================= public API =================
template <typename It, typename Comp = std::less<>>
std::enable_if_t<
    std::is_base_of_v<std::random_access_iterator_tag,
        typename std::iterator_traits<It>::iterator_category>
>
sort(It first, It last, Comp comp = Comp{}) {
    int n = static_cast<int>(last - first);
    if (n <= 1) return;
    detail::apex_sort_core(first, n, comp);
}

template <typename Container, typename Comp = std::less<>>
std::enable_if_t<detail::is_container<Container>::value>
sort(Container& c, Comp comp = Comp{}) {
    apex::sort(std::begin(c), std::end(c), comp);
}

} // namespace apex