#include <algorithm>
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

enum class MatrixType {
    triangular,
    banded,
};

omp_sched_t parse_schedule(const std::string& schedule)
{
    if (schedule == "static") {
        return omp_sched_static;
    }
    if (schedule == "dynamic") {
        return omp_sched_dynamic;
    }
    if (schedule == "guided") {
        return omp_sched_guided;
    }
    throw std::invalid_argument("unsupported schedule type");
}

MatrixType parse_matrix_type(const std::string& matrix_type)
{
    if (matrix_type == "triangular") {
        return MatrixType::triangular;
    }
    if (matrix_type == "banded") {
        return MatrixType::banded;
    }
    throw std::invalid_argument("unsupported matrix type");
}

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
        if (argc < 6) {
            return 1;
        }

        const std::string task_name = std::filesystem::path(argv[0]).filename().string();
        const std::string arg_string = join_args(argc, argv);

        const std::size_t rows = std::stoull(argv[1]);
        const std::size_t cols = std::stoull(argv[2]);
        const MatrixType matrix_type = parse_matrix_type(argv[3]);
        const omp_sched_t schedule = parse_schedule(argv[4]);
        const int chunk = std::stoi(argv[5]);
        const std::size_t band_width = (argc >= 7) ? std::stoull(argv[6]) : 8;
        const unsigned seed = (argc >= 8) ? std::stoul(argv[7]) : 12345u;

        omp_set_schedule(schedule, chunk);

        int max_of_row_mins = std::numeric_limits<int>::min();
        const double elapsed = measure_seconds([&]() {
            #pragma omp parallel
            {
                int local_max = std::numeric_limits<int>::min();

                #pragma omp for schedule(runtime) nowait
                for (std::size_t r = 0; r < rows; ++r) {
                    int row_min = std::numeric_limits<int>::max();
                    bool has_any = false;

                    if (matrix_type == MatrixType::triangular) {
                        const std::size_t end_col = std::min(cols, r + 1);
                        for (std::size_t c = 0; c < end_col; ++c) {
                            const int value = cell_value(r, c, seed);
                            row_min = std::min(row_min, value);
                            has_any = true;
                        }
                    } else {
                        const std::size_t left = (r > band_width) ? (r - band_width) : 0;
                        const std::size_t right = std::min(cols, r + band_width + 1);
                        for (std::size_t c = left; c < right; ++c) {
                            const int value = cell_value(r, c, seed);
                            row_min = std::min(row_min, value);
                            has_any = true;
                        }
                    }

                    if (has_any && row_min > local_max) {
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
