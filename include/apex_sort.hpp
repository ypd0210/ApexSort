#ifndef APEX_SORT_HPP
#define APEX_SORT_HPP

#include <vector>
#include <algorithm>
#include <cmath>
#include <iterator>

/**
 * ApexSort: An Industrial-Grade, High-Performance Hybrid Sorting Library.
 * 
 * Features:
 * - Worst-case O(n log n) guaranteed via Introspection (HeapSort fallback).
 * - 3-Way Partitioning to handle duplicate elements in O(n).
 * - Tukey's Ninther pivot selection to avoid skewed distribution degradation.
 * - Cache-aligned threshold (48) for Insertion Sort.
 * - Header-only, zero-dependency, template-based.
 */

namespace apex {
    namespace detail {

        // --- 1. Shift-based Insertion Sort (Optimized for small ranges) ---
        template <typename T>
        inline void insertion_sort(T* arr, T* end) {
            for (T* i = arr + 1; i < end; ++i) {
                T key = std::move(*i);
                T* j = i - 1;
                while (j >= arr && *j > key) {
                    *(j + 1) = std::move(*j);
                    --j;
                }
                *(j + 1) = std::move(key);
            }
        }

        // --- 2. Custom HeapSort to avoid std::make_heap overhead ---
        template <typename T>
        inline void siftDown(T* arr, int start, int end) {
            T root_val = std::move(arr[start]);
            while (start * 2 + 1 <= end) {
                int child = start * 2 + 1;
                if (child + 1 <= end && arr[child] < arr[child + 1]) child++;
                if (root_val < arr[child]) {
                    arr[start] = std::move(arr[child]);
                    start = child;
                } else break;
            }
            arr[start] = std::move(root_val);
        }

        template <typename T>
        void heap_sort(T* arr, int n) {
            for (int i = (n - 2) / 2; i >= 0; --i) siftDown(arr, i, n - 1);
            for (int i = n - 1; i > 0; --i) {
                std::swap(arr[0], arr[i]);
                siftDown(arr, 0, i - 1);
            }
        }

        // --- 3. Tukey's Ninther Pivot Selection ---
        template <typename T>
        inline T get_pivot(T* arr, int n) {
            auto med3 = [](const T& a, const T& b, const T& c) {
                return (a > b) ? ((b > c) ? b : (a > c ? c : a)) 
                               : ((a > c) ? a : (b > c ? c : b));
            };
            if (n < 7) return arr[n >> 1];
            int m = n >> 3;
            return med3(med3(arr[0], arr[m], arr[2 * m]),
                        med3(arr[3 * m], arr[4 * m], arr[5 * m]),
                        med3(arr[6 * m], arr[7 * m], arr[n - 1]));
        }

        // --- 4. Core Engine: 3-Way Partitioning with Depth Control ---
        template <typename T>
        void sort_engine(T* arr, int n, int depth_limit) {
            while (n > 48) { // Threshold for switching to Insertion Sort
                if (depth_limit == 0) {
                    heap_sort(arr, n);
                    return;
                }
                --depth_limit;

                T pivot = get_pivot(arr, n);

                T *lt = arr, *gt = arr + n - 1, *i = arr;
                while (i <= gt) {
                    if (*i < pivot) {
                        std::swap(*lt++, *i++);
                    } else if (*i > pivot) {
                        std::swap(*i, *gt--);
                    } else {
                        ++i;
                    }
                }

                int left_size = (int)(lt - arr);
                int right_size = n - (int)(gt - arr + 1);

                if (left_size < right_size) {
                    sort_engine(arr, left_size, depth_limit);
                    arr = gt + 1;
                    n = right_size;
                } else {
                    sort_engine(gt + 1, right_size, depth_limit);
                    n = left_size;
                }
            }
            insertion_sort(arr, arr + n);
        }
    } // namespace detail

    /**
     * Sorts a vector using the ApexSort hybrid algorithm.
     * Complexity: O(n log n) worst-case, O(n) for many repeated elements.
     */
    template <typename T>
    void sort(std::vector<T>& v) {
        if (v.size() < 2) return;
        
        // Top-level short-circuit: O(n) for already sorted or identical data
        bool sorted = true;
        bool all_same = true;
        for (size_t i = 1; i < v.size(); ++i) {
            if (v[i] < v[i-1]) sorted = false;
            if (v[i] != v[0]) all_same = false;
            if (!sorted && !all_same) break;
        }
        if (sorted || all_same) return;

        int depth_limit = 2 * static_cast<int>(std::log2(v.size()));
        detail::sort_engine(v.data(), static_cast<int>(v.size()), depth_limit);
    }
}

#endif // APEX_SORT_HPP