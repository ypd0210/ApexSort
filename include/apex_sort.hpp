#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <functional>
#include <type_traits>

namespace apex {
namespace detail {

    // Insertion sort (generic iterator version; works with random access iterators) - optimized for cache
    template <typename RandomIt, typename Compare>
    inline void insertion_sort(RandomIt first, RandomIt last, Compare comp) {
        if (first == last) return;
        using Value = typename std::iterator_traits<RandomIt>::value_type;

        for (RandomIt i = std::next(first); i != last; ++i) {
            Value key = std::move(*i);
            RandomIt j = i;
            // Use pre-decrement to avoid std::prev calls
            while (j > first) {
                RandomIt prev_j = j - 1;
                if (!comp(key, *prev_j)) break;
                *j = std::move(*prev_j);
                --j;
            }
            *j = std::move(key);
        }
    }

    // Heap sort sift-down operation (optimized with loop unrolling)
    template <typename RandomIt, typename Compare>
    inline void siftDown(RandomIt first, int start, int end, Compare comp) {
        auto root_val = std::move(*(first + start));
        int current = start;

        // Unroll the loop for better performance
        while (current * 2 + 1 <= end) {
            int child = current * 2 + 1;
            // Find the largest child
            if (child + 1 <= end && comp(*(first + child), *(first + child + 1))) {
                child++;
            }
            // If root is smaller than largest child, swap and continue sifting
            if (comp(root_val, *(first + child))) {
                *(first + current) = std::move(*(first + child));
                current = child;
            } else {
                break;
            }
        }
        *(first + current) = std::move(root_val);
    }

    // Heap sort implementation (optimized)
    template <typename RandomIt, typename Compare>
    void heap_sort(RandomIt first, int n, Compare comp) {
        // Build max heap
        for (int i = (n - 2) / 2; i >= 0; --i) {
            siftDown(first, i, n - 1, comp);
        }
        // Extract elements one by one
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

    // Efficient pivot selection strategy (similar to std::sort)
    template <typename RandomIt, typename Compare>
    inline auto get_pivot(RandomIt first, int n, Compare comp) {
        using Value = typename std::iterator_traits<RandomIt>::value_type;
        if (n < 7) {
            // For small arrays, use middle element
            return *(first + (n / 2));
        } else {
            // For all other sizes, use median-of-three
            // This is similar to std::sort's approach and more efficient than Tukey's ninther
            int mid = n / 2;
            return med3(*first, *(first + mid), *(first + n - 1), comp);
        }
    }

    // Check if there are duplicate elements in the range
    template <typename RandomIt, typename Compare>
    inline bool has_duplicates(RandomIt first, int n, Compare comp) {
        if (n <= 1) return false;

        // Sample 32 elements to check for duplicates
        int sample_size = std::min(n, 32);
        int step = n / sample_size;

        for (int i = 0; i < sample_size; ++i) {
            auto& current = *(first + i * step);
            for (int j = i + 1; j < sample_size; ++j) {
                auto& other = *(first + j * step);
                if (!comp(current, other) && !comp(other, current)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Two-way partition for arrays with few duplicates
    template <typename RandomIt, typename Compare>
    inline std::pair<RandomIt, RandomIt> two_way_partition(RandomIt first, RandomIt last, const typename std::iterator_traits<RandomIt>::value_type& pivot, Compare comp) {
        RandomIt i = first;
        RandomIt j = last - 1;

        while (true) {
            while (i <= j && comp(*i, pivot)) ++i;
            while (i <= j && comp(pivot, *j)) --j;
            if (i > j) break;
            std::iter_swap(i++, j--);
        }
        return {i, j};
    }

    // Core sorting engine (introsort with adaptive partitioning) - recursive version
    template <typename RandomIt, typename Compare>
    void sort_engine(RandomIt first, int n, int depth_limit, Compare comp) {
        while (n > 48) {
            if (depth_limit == 0) {
                heap_sort(first, n, comp);
                return;
            }
            --depth_limit;

            auto pivot = get_pivot(first, n, comp);

            // Check for duplicates and choose appropriate partition method
            bool use_three_way = has_duplicates(first, n, comp);

            RandomIt lt, gt;
            int left, right;

            if (use_three_way) {
                // 3-way partition: < pivot | == pivot | > pivot - optimized
                lt = first;
                gt = first + n - 1;
                RandomIt i = first;

                while (i <= gt) {
                    auto& current_val = *i;
                    if (comp(current_val, pivot)) {
                        if (lt != i) std::iter_swap(lt, i);
                        ++lt;
                        ++i;
                    } else if (comp(pivot, current_val)) {
                        while (gt > i && comp(pivot, *gt)) {
                            --gt;
                        }
                        if (i != gt) std::iter_swap(i, gt);
                        --gt;
                    } else {
                        ++i;
                    }
                }

                // Calculate partition sizes
                left = static_cast<int>(std::distance(first, lt));
                right = n - static_cast<int>(std::distance(first, gt + 1));
            } else {
                // Two-way partition for arrays with few duplicates
                auto [partition_point, _] = two_way_partition(first, first + n, pivot, comp);

                // Calculate partition sizes
                left = static_cast<int>(std::distance(first, partition_point));
                right = n - left;
                gt = partition_point - 1;
            }

            // Recurse on the smaller partition (optimize stack depth)
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
        if (n > 1) {
            insertion_sort(first, first + n, comp);
        }
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
        // Exclude raw pointers; iterator types won't satisfy begin/end + value_type.
        static constexpr bool value = !std::is_pointer_v<T>;
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

        // Fast path: already sorted / all equal / reverse sorted
        bool sorted = true;
        bool reverse_sorted = true;
        bool all_same = true;
        const auto& first_val = *first;
        auto i = std::next(first);
        // Only check first 32 elements for sorted/all_same/reverse to avoid O(n) overhead
        int check_count = 0;
        const int max_check = 32;
        while (i != last && check_count < max_check) {
            if (comp(*i, *std::prev(i))) sorted = false;
            if (comp(*std::prev(i), *i)) reverse_sorted = false;
            if (comp(*i, first_val) || comp(first_val, *i)) all_same = false;
            if (!sorted && !reverse_sorted && !all_same) break;
            ++i;
            ++check_count;
        }
        if (sorted || all_same) return;
        if (reverse_sorted) {
            // If reverse sorted, simply reverse the array
            std::reverse(first, last);
            return;
        }

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