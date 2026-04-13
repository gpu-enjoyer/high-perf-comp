#include "common.hpp"

using Matrix = std::vector<int>;

int max_row_mins_reduction(const Matrix& m, std::size_t n) {
    int result = std::numeric_limits<int>::min();
    #pragma omp parallel for reduction(max:result)
    for (std::size_t i = 0; i < n; ++i) {
        int row_min = std::numeric_limits<int>::max();
        const std::size_t off = i * n;
        for (std::size_t j = 0; j < n; ++j) row_min = std::min(row_min, m[off + j]);
        result = std::max(result, row_min);
    }
    return result;
}

int max_row_mins_critical(const Matrix& m, std::size_t n) {
    int result = std::numeric_limits<int>::min();
    #pragma omp parallel
    {
        int local_max = std::numeric_limits<int>::min();
        #pragma omp for
        for (std::size_t i = 0; i < n; ++i) {
            int row_min = std::numeric_limits<int>::max();
            const std::size_t off = i * n;
            for (std::size_t j = 0; j < n; ++j) row_min = std::min(row_min, m[off + j]);
            local_max = std::max(local_max, row_min);
        }
        #pragma omp critical
        result = std::max(result, local_max);
    }
    return result;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;

    const std::size_t n = args.size;
    Matrix m(n * n);
    std::mt19937_64 rng(args.seed);
    std::uniform_int_distribution<int> dist(0, 1000000);
    for (auto& v : m) v = dist(rng);

    int result = 0;
    double total = 0.0;
    for (int r = 0; r < args.runs; ++r) {
        total += measure_seconds([&] {
            if (args.mode == "reduction") result = max_row_mins_reduction(m, n);
            else if (args.mode == "critical") result = max_row_mins_critical(m, n);
            else std::exit(1);
        });
    }
    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
