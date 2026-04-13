#include "common.hpp"

long long sum_reduction(const std::vector<int>& data) {
    long long sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (std::size_t i = 0; i < data.size(); ++i) sum += data[i];
    return sum;
}

long long sum_critical(const std::vector<int>& data) {
    long long sum = 0;
    #pragma omp parallel
    {
        long long local = 0;
        #pragma omp for
        for (std::size_t i = 0; i < data.size(); ++i) local += data[i];
        #pragma omp critical
        sum += local;
    }
    return sum;
}

long long sum_atomic(const std::vector<int>& data) {
    long long sum = 0;
    #pragma omp parallel for
    for (std::size_t i = 0; i < data.size(); ++i) {
        #pragma omp atomic
        sum += data[i];
    }
    return sum;
}

long long sum_lock(const std::vector<int>& data) {
    long long sum = 0;
    omp_lock_t lock;
    omp_init_lock(&lock);
    #pragma omp parallel
    {
        long long local = 0;
        #pragma omp for
        for (std::size_t i = 0; i < data.size(); ++i) local += data[i];
        omp_set_lock(&lock);
        sum += local;
        omp_unset_lock(&lock);
    }
    omp_destroy_lock(&lock);
    return sum;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;

    const auto data = gen_random_int_vector(args.size, args.seed);
    long long result = 0;
    double total = 0.0;

    for (int r = 0; r < args.runs; ++r) {
        total += measure_seconds([&] {
            if (args.mode == "reduction") result = sum_reduction(data);
            else if (args.mode == "critical") result = sum_critical(data);
            else if (args.mode == "atomic") result = sum_atomic(data);
            else if (args.mode == "lock") result = sum_lock(data);
            else std::exit(1);
        });
    }

    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
