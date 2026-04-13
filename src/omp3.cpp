#include "common.hpp"

static inline double f(double x) { return std::sin(x); }

double integrate_reduction(std::size_t n, double a, double b) {
    const double h = (b - a) / static_cast<double>(n);
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (std::size_t i = 0; i < n; ++i) sum += f(a + (i + 0.5) * h);
    return sum * h;
}

double integrate_critical(std::size_t n, double a, double b) {
    const double h = (b - a) / static_cast<double>(n);
    double sum = 0.0;
    #pragma omp parallel
    {
        double local = 0.0;
        #pragma omp for
        for (std::size_t i = 0; i < n; ++i) local += f(a + (i + 0.5) * h);
        #pragma omp critical
        sum += local;
    }
    return sum * h;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;
    constexpr double a = 0.0;
    constexpr double b = 1.0;

    double result = 0.0;
    double total = 0.0;
    for (int r = 0; r < args.runs; ++r) {
        total += measure_seconds([&] {
            if (args.mode == "reduction") result = integrate_reduction(args.size, a, b);
            else if (args.mode == "critical") result = integrate_critical(args.size, a, b);
            else std::exit(1);
        });
    }
    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
