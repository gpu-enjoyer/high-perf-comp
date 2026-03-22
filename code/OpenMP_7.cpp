#include <filesystem>
#include <iostream>
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

        const std::size_t n = std::stoull(argv[1]);
        const std::string mode = argv[2];
        const unsigned seed = (argc >= 4) ? std::stoul(argv[3]) : 12345u;

        std::vector<double> values(n);
        std::mt19937 generator(seed);
        std::uniform_real_distribution<double> distribution(0.0, 1.0);
        for (std::size_t i = 0; i < n; ++i) {
            values[i] = distribution(generator);
        }

        double sum = 0.0;
        const double elapsed = measure_seconds([&]() {
            if (mode == "reduction") {
                #pragma omp parallel for reduction(+:sum)
                for (std::size_t i = 0; i < n; ++i) {
                    sum += values[i];
                }
            } else if (mode == "atomic") {
                #pragma omp parallel for
                for (std::size_t i = 0; i < n; ++i) {
                    #pragma omp atomic update
                    sum += values[i];
                }
            } else if (mode == "critical") {
                #pragma omp parallel for
                for (std::size_t i = 0; i < n; ++i) {
                    #pragma omp critical
                    {
                        sum += values[i];
                    }
                }
            } else if (mode == "lock") {
                omp_lock_t lock;
                omp_init_lock(&lock);

                #pragma omp parallel for
                for (std::size_t i = 0; i < n; ++i) {
                    omp_set_lock(&lock);
                    sum += values[i];
                    omp_unset_lock(&lock);
                }

                omp_destroy_lock(&lock);
            } else {
                throw std::invalid_argument("unsupported mode");
            }
        });

        std::cout << task_name << ','
                  << arg_string << ','
                  << omp_get_max_threads() << ','
                  << elapsed << ','
                  << sum << '\n';
        return 0;
    } catch (...) {
        return 1;
    }
}
