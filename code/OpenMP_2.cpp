#include <filesystem>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <omp.h>

#include "common/benchmark.hpp"

std::string join_args(int argc, char** argv)
{
    std::ostringstream oss;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) {
            oss << ' ';
        }
        oss << argv[i];
    }
    return oss.str();
}

int main(int argc, char** argv)
{
    try {
        if (argc < 2) {
            return 1;
        }

        const std::string task_name = std::filesystem::path(argv[0]).filename().string();
        const std::string arg_string = join_args(argc, argv);

        const std::size_t n = std::stoull(argv[1]);
        const unsigned seed = (argc >= 3) ? std::stoul(argv[2]) : 12345u;

        std::vector<double> a(n);
        std::vector<double> b(n);

        std::mt19937 generator(seed);
        std::uniform_real_distribution<double> distribution(-1.0, 1.0);

        for (std::size_t i = 0; i < n; ++i) {
            a[i] = distribution(generator);
            b[i] = distribution(generator);
        }

        double dot = 0.0;
        const double elapsed = measure_seconds([&]() {
            #pragma omp parallel for reduction(+:dot)
            for (std::size_t i = 0; i < n; ++i) {
                dot += a[i] * b[i];
            }
        });

        std::cout << task_name << ','
                  << arg_string << ','
                  << omp_get_max_threads() << ','
                  << elapsed << ','
                  << dot << '\n';
        return 0;
    } catch (...) {
        return 1;
    }
}
