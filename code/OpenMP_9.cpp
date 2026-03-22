#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <omp.h>

#include "common/benchmark.hpp"

int cell_value(std::size_t row, std::size_t col, unsigned seed)
{
    const std::uint64_t x = static_cast<std::uint64_t>(row + 1) * 6364136223846793005ULL
        ^ static_cast<std::uint64_t>(col + 1) * 1442695040888963407ULL
        ^ static_cast<std::uint64_t>(seed);
    return static_cast<int>((x % 1000000ULL) + 1ULL);
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
        if (argc < 5) {
            return 1;
        }

        const std::string task_name = std::filesystem::path(argv[0]).filename().string();
        const std::string arg_string = join_args(argc, argv);

        const std::size_t rows = std::stoull(argv[1]);
        const std::size_t cols = std::stoull(argv[2]);
        const std::string mode = argv[3];
        const int inner_threads = std::stoi(argv[4]);
        const unsigned seed = (argc >= 6) ? std::stoul(argv[5]) : 12345u;

        int max_of_row_mins = std::numeric_limits<int>::min();
        const double elapsed = measure_seconds([&]() {
            if (mode == "flat") {
                #pragma omp parallel
                {
                    int local_max = std::numeric_limits<int>::min();

                    #pragma omp for nowait
                    for (std::size_t r = 0; r < rows; ++r) {
                        int row_min = std::numeric_limits<int>::max();
                        for (std::size_t c = 0; c < cols; ++c) {
                            row_min = std::min(row_min, cell_value(r, c, seed));
                        }
                        local_max = std::max(local_max, row_min);
                    }

                    #pragma omp critical
                    {
                        max_of_row_mins = std::max(max_of_row_mins, local_max);
                    }
                }
            } else if (mode == "nested") {
                omp_set_max_active_levels(2);
                omp_set_nested(1);

                #pragma omp parallel
                {
                    int local_max = std::numeric_limits<int>::min();

                    #pragma omp for nowait
                    for (std::size_t r = 0; r < rows; ++r) {
                        int row_min = std::numeric_limits<int>::max();

                        #pragma omp parallel for reduction(min:row_min) num_threads(inner_threads)
                        for (std::size_t c = 0; c < cols; ++c) {
                            row_min = std::min(row_min, cell_value(r, c, seed));
                        }

                        local_max = std::max(local_max, row_min);
                    }

                    #pragma omp critical
                    {
                        max_of_row_mins = std::max(max_of_row_mins, local_max);
                    }
                }
            } else {
                throw std::invalid_argument("unsupported mode");
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
