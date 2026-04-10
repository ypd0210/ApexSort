#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "apex_sort.hpp"
#include "pdqsort.h"

using namespace std;
using namespace std::chrono;

inline void run_test(string name, vector<int> data) {
    vector<int> d1 = data;
    vector<int> d2 = data;
    vector<int> d3 = data;

    // std::sort
    auto s1 = high_resolution_clock::now();
    std::sort(d1.begin(), d1.end());
    auto e1 = high_resolution_clock::now();

    // ApexSort
    auto s2 = high_resolution_clock::now();
    apex::sort(d2);
    auto e2 = high_resolution_clock::now();

    // pdqsort
    auto s3 = high_resolution_clock::now();
    pdqsort(d3.begin(), d3.end());
    auto e3 = high_resolution_clock::now();

    bool correct1 = (d1 == d2);
    bool correct2 = (d1 == d3);

    auto t1 = duration_cast<microseconds>(e1 - s1).count();
    auto t2 = duration_cast<microseconds>(e2 - s2).count();
    auto t3 = duration_cast<microseconds>(e3 - s3).count();

    cout << name
            << " | std::sort: " << t1 << "us"
            << " | ApexSort: " << t2 << "us"
            << " | PDQSort: " << t3 << "us"
            << " | Ratio(Apex/std::sort): " << (double) t2 / t1 * 100 << "%"
            << " | Ratio(Apex/pdq): " << (double) t2 / t3 * 100 << "%"
            << " | Correct: " << ((correct1 && correct2) ? "YES" : "NO") << endl;
}

int main() {
    const int N = 1000000;
    mt19937 rng(42);

    vector<int> uniform(N), repeated(N), reverse(N);
    for (int i = 0; i < N; ++i) {
        uniform[i] = rng();
        repeated[i] = rng() % 10;
        reverse[i] = N - i;
    }

    cout << "Benchmark (N=" << N << ")\n";
    run_test("Uniform  ", uniform);
    run_test("Repeated ", repeated);
    run_test("Reverse  ", reverse);

    return 0;
}
