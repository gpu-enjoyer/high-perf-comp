#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include <string>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <omp.h>

#include "common/benchmark.hpp"

int min_with_reduction(const std::vector<int>& data)
{
    int global_min = std::numeric_limits<int>::max();

    #pragma omp parallel for reduction(min:global_min)
    for (std::size_t i = 0; i < data.size(); ++i) {
        if (data[i] < global_min) {
            global_min = data[i];
        }
    }

    return global_min;
}

int min_without_reduction(const std::vector<int>& data)
{
    int global_min = std::numeric_limits<int>::max();

    #pragma omp parallel
    {
        int local_min = std::numeric_limits<int>::max();

        #pragma omp for
        for (std::size_t i = 0; i < data.size(); ++i) {
            if (data[i] < local_min) {
                local_min = data[i];
            }
        }

        #pragma omp critical
        {
            if (local_min < global_min) {
                global_min = local_min;
            }
        }
    }

    return global_min;
}

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
        if (argc < 3) {
            return 1;
        }

        const std::string task_name = std::filesystem::path(argv[0]).filename().string();
        const std::string arg_string = join_args(argc, argv);

        const std::size_t problem_size = std::stoull(argv[1]);
        const std::string mode = argv[2];
        const unsigned seed = (argc >= 4) ? std::stoul(argv[3]) : 12345u;

        std::vector<int> data(problem_size);

        std::mt19937_64 generator(seed);
        std::uniform_int_distribution<int> distribution(0, 1000000000);

        for (std::size_t i = 0; i < problem_size; ++i) {
            data[i] = distribution(generator);
        }

        int result = 0;
        const double elapsed = measure_seconds([&]() {
            if (mode == "reduction") {
                result = min_with_reduction(data);
            } else if (mode == "no_reduction") {
                result = min_without_reduction(data);
            } else {
                throw std::invalid_argument("unsupported mode");
            }
        });

        std::cout << task_name << ','
                  << arg_string << ','
                  << omp_get_max_threads() << ','
                  << elapsed << ','
                  << result << '\n';

        return 0;
    } catch (...) {
        return 1;
    }
}
