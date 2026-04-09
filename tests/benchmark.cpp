#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "apex_sort.hpp"

using namespace std;
using namespace std::chrono;

inline void run_test(string name, vector<int> data) {
    vector<int> d1 = data;
    vector<int> d2 = data;

    auto s1 = high_resolution_clock::now();
    std::sort(d1.begin(), d1.end());
    auto e1 = high_resolution_clock::now();

    auto s2 = high_resolution_clock::now();
    apex::sort(d2);
    auto e2 = high_resolution_clock::now();

    bool correct = (d1 == d2);
    auto t1 = duration_cast<microseconds>(e1-s1).count();
    auto t2 = duration_cast<microseconds>(e2-s2).count();

    cout << name << " | std::sort: " << t1 << "us | ApexSort: " << t2 
         << "us | Ratio: " << (double)t2/t1*100 << "% | Correct: " << (correct?"YES":"NO") << endl;
}
int main() {
    const int N = 1000000;
    mt19937 rng(42);
    
    vector<int> uniform(N), repeated(N), reverse(N);
    for(int i=0; i<N; ++i) {
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