#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <chrono>

template <typename Func>
double measure_seconds(Func&& fn)
{
    const auto start = std::chrono::high_resolution_clock::now();
    fn();
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

#endif
