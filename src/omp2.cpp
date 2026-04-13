#include "common.hpp"

long long dot_reduction(const std::vector<int>& a, const std::vector<int>& b) {
    long long sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (std::size_t i = 0; i < a.size(); ++i) sum += static_cast<long long>(a[i]) * b[i];
    return sum;
}

long long dot_atomic(const std::vector<int>& a, const std::vector<int>& b) {
    long long sum = 0;
    #pragma omp parallel
    {
        long long local = 0;
        #pragma omp for
        for (std::size_t i = 0; i < a.size(); ++i) local += static_cast<long long>(a[i]) * b[i];
        #pragma omp atomic
        sum += local;
    }
    return sum;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;

    const auto a = gen_random_int_vector(args.size, args.seed);
    const auto b = gen_random_int_vector(args.size, args.seed + 1);
    long long result = 0;
    double total = 0.0;

    for (int r = 0; r < args.runs; ++r) {
        total += measure_seconds([&] {
            if (args.mode == "reduction") result = dot_reduction(a, b);
            else if (args.mode == "atomic") result = dot_atomic(a, b);
            else std::exit(1);
        });
    }

    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
