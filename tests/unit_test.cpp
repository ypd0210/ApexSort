#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <functional>
#include <type_traits>

namespace apex {
namespace detail {

    // Insertion sort (generic iterator version; works with random access iterators)
    template <typename RandomIt, typename Compare>
    inline void insertion_sort(RandomIt first, RandomIt last, Compare comp) {
        if (first == last) return;
        for (RandomIt i = std::next(first); i != last; ++i) {
            auto key = std::move(*i);
            RandomIt j = std::prev(i);
            while (j >= first && comp(key, *j)) {
                *std::next(j) = std::move(*j);
                --j;
            }
            *std::next(j) = std::move(key);
        }
    }

    // Heap sort sift-down operation
    template <typename RandomIt, typename Compare>
    inline void siftDown(RandomIt first, int start, int end, Compare comp) {
        auto root_val = std::move(*(first + start));
        while (start * 2 + 1 <= end) {
            int child = start * 2 + 1;
            if (child + 1 <= end && comp(*(first + child), *(first + child + 1))) {
                child++;
            }
            if (comp(root_val, *(first + child))) {
                *(first + start) = std::move(*(first + child));
                start = child;
            } else {
                break;
            }
        }
        *(first + start) = std::move(root_val);
    }

    // Heap sort implementation
    template <typename RandomIt, typename Compare>
    void heap_sort(RandomIt first, int n, Compare comp) {
        for (int i = (n - 2) / 2; i >= 0; --i) {
            siftDown(first, i, n - 1, comp);
        }
        for (int i = n - 1; i > 0; --i) {
            std::iter_swap(first, first + i);
            siftDown(first, 0, i - 1, comp);
        }
    }

    // Median-of-three (standalone function to avoid lambda capture issues)
    template <typename T, typename Compare>
    inline T med3(const T& a, const T& b, const T& c, Compare comp) {
        if (comp(a, b)) {
            return comp(b, c) ? b : (comp(a, c) ? c : a);
        }
        return comp(a, c) ? a : (comp(b, c) ? c : b);
    }

    // Tukey's ninther pivot selection (robust against degeneration)
    template <typename RandomIt, typename Compare>
    inline auto get_pivot(RandomIt first, int n, Compare comp) {
        using Value = typename std::iterator_traits<RandomIt>::value_type;
        if (n < 7) return *(first + (n / 2));
        int m = n / 8;
        return med3(
            med3(*first, *(first+m), *(first+2*m), comp),
            med3(*(first+3*m), *(first+4*m), *(first+5*m), comp),
            med3(*(first+6*m), *(first+7*m), *(first+n-1), comp),
            comp
        );
    }

    // Core sorting engine (introsort + 3-way partitioning)
    template <typename RandomIt, typename Compare>
    void sort_engine(RandomIt first, int n, int depth_limit, Compare comp) {
        while (n > 48) {
            if (depth_limit == 0) {
                heap_sort(first, n, comp);
                return;
            }
            --depth_limit;

            auto pivot = get_pivot(first, n, comp);
            RandomIt lt = first, gt = first + n - 1, i = first;

            // 3-way partition: < pivot | == pivot | > pivot
            while (i <= gt) {
                if (comp(*i, pivot)) {
                    std::iter_swap(lt++, i++);
                } else if (comp(pivot, *i)) {
                    std::iter_swap(i, gt--);
                } else {
                    ++i;
                }
            }

            // Recurse on the smaller partition (optimize stack depth)
            int left = static_cast<int>(std::distance(first, lt));
            int right = n - static_cast<int>(std::distance(first, gt + 1));

            if (left < right) {
                sort_engine(first, left, depth_limit, comp);
                first = gt + 1;
                n = right;
            } else {
                sort_engine(gt + 1, right, depth_limit, comp);
                n = left;
            }
        }
        // For small ranges use insertion sort
        insertion_sort(first, first + n, comp);
    }

    // ========== Core fix: precise container type detection ==========
    // Basic check: has begin/end/value_type (exclude iterators/pointers)
    template <typename T, typename = void>
    struct is_container : std::false_type {};

    template <typename T>
    struct is_container<T, std::void_t<
        decltype(std::begin(std::declval<T&>())),
        decltype(std::end(std::declval<T&>())),
        typename T::value_type
    >> : std::true_type {
        // Also exclude iterator types (key fix: prevent container overload matching iterators)
        static constexpr bool value = !std::is_same_v<
            typename std::iterator_traits<T>::iterator_category,
            std::random_access_iterator_tag
        > && !std::is_pointer_v<T>;
    };

} // namespace detail

    // ========== Public API (fully resolves overload ambiguity) ==========
    // API 1: iterator overload (only matches random access iterators)
    template <typename RandomIt, typename Compare = std::less<>>
    std::enable_if_t<
        std::is_base_of_v<std::random_access_iterator_tag,
                          typename std::iterator_traits<RandomIt>::iterator_category>
    > sort(RandomIt first, RandomIt last, Compare comp = Compare{}) {
        const auto n = std::distance(first, last);
        if (n <= 1) return;

        // Fast path: already sorted / all equal
        bool sorted = true;
        bool all_same = true;
        const auto& first_val = *first;
        for (auto i = std::next(first); i != last; ++i) {
            if (comp(*i, *std::prev(i))) sorted = false;
            if (comp(*i, first_val) || comp(first_val, *i)) all_same = false;
            if (!sorted && !all_same) break;
        }
        if (sorted || all_same) return;

        // Recursion depth limit (prevent quicksort degeneration)
        int depth = 2 * static_cast<int>(std::log2(n));
        detail::sort_engine(first, static_cast<int>(n), depth, comp);
    }

    // API 2: container overload (only matches standard containers; excludes iterators/pointers)
    template <typename Container, typename Compare = std::less<>>
    std::enable_if_t<detail::is_container<Container>::value>
    sort(Container& cont, Compare comp = Compare{}) {
        apex::sort(std::begin(cont), std::end(cont), comp); // Explicitly call the iterator overload to remove ambiguity
    }

} // namespace apex