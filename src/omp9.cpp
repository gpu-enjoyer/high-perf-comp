#include "common.hpp"

using Matrix = std::vector<int>;

int max_row_mins_flat(const Matrix& m, std::size_t n) {
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

int max_row_mins_nested(const Matrix& m, std::size_t n) {
    int result = std::numeric_limits<int>::min();
    #pragma omp parallel for reduction(max:result)
    for (std::size_t i = 0; i < n; ++i) {
        int row_min = std::numeric_limits<int>::max();
        const std::size_t off = i * n;
        #pragma omp parallel for reduction(min:row_min)
        for (std::size_t j = 0; j < n; ++j) row_min = std::min(row_min, m[off + j]);
        result = std::max(result, row_min);
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

    if (args.mode == "nested") omp_set_max_active_levels(2);
    else omp_set_max_active_levels(1);

    for (int r = 0; r < args.runs; ++r) {
        total += measure_seconds([&] {
            if (args.mode == "flat") result = max_row_mins_flat(m, n);
            else if (args.mode == "nested") result = max_row_mins_nested(m, n);
            else std::exit(1);
        });
    }

    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
