#pragma once
#include <algorithm>
#include <cmath>
#include <iterator>
#include <functional>
#include <type_traits>
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

// ================= ApexSort 三路 partition 核心 =================
template <typename It, typename Comp>
void apex_sort_core(It first, int n, Comp comp) {
    if (n <= 32) { insertion_sort(first, first + n, comp); return; }

    // 快速判断是否逆序 / 重复元素
    int sample = std::min(n, 32);
    bool reverse_sorted = true, duplicates = false;
    for (int i = 1; i < sample; ++i) {
        if (comp(*(first + i - 1), *(first + i))) reverse_sorted = false;
        if (!comp(*(first + i - 1), *(first + i)) && !comp(*(first + i), *(first + i - 1)))
            duplicates = true;
    }
    if (reverse_sorted) { std::reverse(first, first + n); return; }

    // 三路 partition ApexSort
    if (duplicates || reverse_sorted) {
        // Pivot 选择：中间元素
        auto pivot = *(first + n/2);
        It lt = first, i = first, gt = first + n - 1;
        while (i <= gt) {
            if (comp(*i, pivot)) std::iter_swap(lt++, i++);
            else if (comp(pivot, *i)) std::iter_swap(i, gt--);
            else ++i;
        }
        int left = lt - first;
        int right = gt - lt + 1;
        int rest = n - left - right;
        if (left < rest) { apex_sort_core(first, left, comp); apex_sort_core(gt + 1, rest, comp); }
        else { apex_sort_core(gt + 1, rest, comp); apex_sort_core(first, left, comp); }
    } else {
        // 对随机均匀数据，直接用 pdqsort
        pdqsort(first, first + n, comp);
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