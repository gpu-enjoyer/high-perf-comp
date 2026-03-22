#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <omp.h>

#include "common/benchmark.hpp"

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

        const std::size_t iterations = std::stoull(argv[1]);
        const std::string schedule_name = argv[2];
        const int chunk = std::stoi(argv[3]);
        const unsigned seed = std::stoul(argv[4]);

        omp_set_schedule(parse_schedule(schedule_name), chunk);

        double accumulator = 0.0;
        const double elapsed = measure_seconds([&]() {
            #pragma omp parallel for schedule(runtime) reduction(+:accumulator)
            for (std::size_t i = 0; i < iterations; ++i) {
                std::uint64_t hash = static_cast<std::uint64_t>(i + 1) * 11400714819323198485ULL
                    ^ static_cast<std::uint64_t>(seed) * 6364136223846793005ULL;

                int inner_work = 100 + static_cast<int>(hash % 400);
                if ((i % 7U) == 0U) {
                    inner_work += 6000;
                }
                if ((i % 23U) == 0U) {
                    inner_work += 12000;
                }

                double local = 0.0;
                for (int k = 1; k <= inner_work; ++k) {
                    local += std::sqrt(static_cast<double>((k * 13) % 1000 + 1));
                }
                accumulator += local;
            }
        });

        std::cout << task_name << ','
                  << arg_string << ','
                  << omp_get_max_threads() << ','
                  << elapsed << ','
                  << accumulator << '\n';
        return 0;
    } catch (...) {
        return 1;
    }
}
