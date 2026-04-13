#include "common.hpp"

int min_reduction(const std::vector<int>& data) {
    int mn = std::numeric_limits<int>::max();
    #pragma omp parallel for reduction(min:mn)
    for (std::size_t i = 0; i < data.size(); ++i) mn = std::min(mn, data[i]);
    return mn;
}

int min_no_reduction(const std::vector<int>& data) {
    int mn = std::numeric_limits<int>::max();
    #pragma omp parallel
    {
        int local = std::numeric_limits<int>::max();
        #pragma omp for
        for (std::size_t i = 0; i < data.size(); ++i) local = std::min(local, data[i]);
        #pragma omp critical
        mn = std::min(mn, local);
    }
    return mn;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;

    const auto data = gen_random_int_vector(args.size, args.seed);
    int result = 0;
    double total = 0.0;

    for (int r = 0; r < args.runs; ++r) {
        total += measure_seconds([&] {
            if (args.mode == "reduction") result = min_reduction(data);
            else if (args.mode == "no_reduction") result = min_no_reduction(data);
            else std::exit(1);
        });
    }

    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
