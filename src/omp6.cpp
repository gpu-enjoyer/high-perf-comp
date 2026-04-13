#include "common.hpp"

static inline double heavy_work(double x, int i) {
    const int loops = (i % 10 == 0) ? 20000 : 500;
    double sum = 0.0;
    for (int k = 0; k < loops; ++k) sum += std::sin(x + k * 1e-3);
    return sum;
}

double run_sched(const std::vector<double>& data, const std::string& mode) {
    double sum = 0.0;
    if (mode == "static") {
        #pragma omp parallel for reduction(+:sum) schedule(static)
        for (std::size_t i = 0; i < data.size(); ++i) sum += heavy_work(data[i], static_cast<int>(i));
    } else if (mode == "dynamic") {
        #pragma omp parallel for reduction(+:sum) schedule(dynamic, 32)
        for (std::size_t i = 0; i < data.size(); ++i) sum += heavy_work(data[i], static_cast<int>(i));
    } else if (mode == "guided") {
        #pragma omp parallel for reduction(+:sum) schedule(guided, 32)
        for (std::size_t i = 0; i < data.size(); ++i) sum += heavy_work(data[i], static_cast<int>(i));
    } else {
        std::exit(1);
    }
    return sum;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;

    const auto data = gen_random_double_vector(args.size, args.seed, 0.0, 10.0);
    double result = 0.0;
    double total = 0.0;
    for (int r = 0; r < args.runs; ++r) total += measure_seconds([&] { result = run_sched(data, args.mode); });

    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
