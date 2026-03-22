#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
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
        const std::size_t steps = std::stoull(argv[1]);

        constexpr double a = 0.0;
        constexpr double b = 3.14159265358979323846;
        const double dx = (b - a) / static_cast<double>(steps);

        double sum = 0.0;
        const double elapsed = measure_seconds([&]() {
            #pragma omp parallel for reduction(+:sum)
            for (std::size_t i = 0; i < steps; ++i) {
                const double x = a + (static_cast<double>(i) + 0.5) * dx;
                sum += std::sin(x);
            }
        });

        const double integral = sum * dx;

        std::cout << task_name << ','
                  << arg_string << ','
                  << omp_get_max_threads() << ','
                  << elapsed << ','
                  << integral << '\n';
        return 0;
    } catch (...) {
        return 1;
    }
}
