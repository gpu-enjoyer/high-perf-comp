#include "common.hpp"

using Matrix = std::vector<int>;

Matrix gen_upper(std::size_t n, unsigned seed) {
    Matrix m(n * n, std::numeric_limits<int>::max());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(0, 1000000);
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i; j < n; ++j)
            m[i * n + j] = dist(rng);
    return m;
}

Matrix gen_banded(std::size_t n, std::size_t bw, unsigned seed) {
    Matrix m(n * n, std::numeric_limits<int>::max());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(0, 1000000);
    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t lo = (i > bw ? i - bw : 0);
        const std::size_t hi = std::min(n - 1, i + bw);
        for (std::size_t j = lo; j <= hi; ++j) m[i * n + j] = dist(rng);
    }
    return m;
}

int solve(const Matrix& m, std::size_t n, bool dynamic_sched) {
    int result = std::numeric_limits<int>::min();
    if (dynamic_sched) {
        #pragma omp parallel for schedule(dynamic, 8) reduction(max:result)
        for (std::size_t i = 0; i < n; ++i) {
            int row_min = std::numeric_limits<int>::max();
            const std::size_t off = i * n;
            for (std::size_t j = 0; j < n; ++j) row_min = std::min(row_min, m[off + j]);
            result = std::max(result, row_min);
        }
    } else {
        #pragma omp parallel for schedule(static) reduction(max:result)
        for (std::size_t i = 0; i < n; ++i) {
            int row_min = std::numeric_limits<int>::max();
            const std::size_t off = i * n;
            for (std::size_t j = 0; j < n; ++j) row_min = std::min(row_min, m[off + j]);
            result = std::max(result, row_min);
        }
    }
    return result;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;

    const bool is_upper = args.mode.find("upper") != std::string::npos;
    const bool is_dynamic = args.mode.find("dynamic") != std::string::npos;
    const std::size_t n = args.size;

    Matrix m = is_upper ? gen_upper(n, args.seed) : gen_banded(n, std::max<std::size_t>(1, n / 20), args.seed);

    int result = 0;
    double total = 0.0;
    for (int r = 0; r < args.runs; ++r) total += measure_seconds([&] { result = solve(m, n, is_dynamic); });

    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
