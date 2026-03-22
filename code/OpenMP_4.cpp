#include <filesystem>
#include <iostream>
#include <limits>
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
        if (argc < 3) {
            return 1;
        }

        const std::string task_name = std::filesystem::path(argv[0]).filename().string();
        const std::string arg_string = join_args(argc, argv);

        const std::size_t rows = std::stoull(argv[1]);
        const std::size_t cols = std::stoull(argv[2]);
        const unsigned seed = (argc >= 4) ? std::stoul(argv[3]) : 12345u;

        std::vector<int> matrix(rows * cols);
        std::mt19937 generator(seed);
        std::uniform_int_distribution<int> distribution(0, 1000000);

        for (std::size_t i = 0; i < rows * cols; ++i) {
            matrix[i] = distribution(generator);
        }

        int max_of_row_mins = std::numeric_limits<int>::min();
        const double elapsed = measure_seconds([&]() {
            #pragma omp parallel
            {
                int local_max = std::numeric_limits<int>::min();

                #pragma omp for nowait
                for (std::size_t r = 0; r < rows; ++r) {
                    int row_min = std::numeric_limits<int>::max();
                    for (std::size_t c = 0; c < cols; ++c) {
                        const int value = matrix[r * cols + c];
                        if (value < row_min) {
                            row_min = value;
                        }
                    }
                    if (row_min > local_max) {
                        local_max = row_min;
                    }
                }

                #pragma omp critical
                {
                    if (local_max > max_of_row_mins) {
                        max_of_row_mins = local_max;
                    }
                }
            }
        });

        std::cout << task_name << ','
                  << arg_string << ','
                  << omp_get_max_threads() << ','
                  << elapsed << ','
                  << max_of_row_mins << '\n';
        return 0;
    } catch (...) {
        return 1;
    }
}
