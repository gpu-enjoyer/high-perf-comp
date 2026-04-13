#include "common.hpp"

long long dot(const std::vector<int>& a, const std::vector<int>& b) {
    long long sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (std::size_t i = 0; i < a.size(); ++i) sum += static_cast<long long>(a[i]) * b[i];
    return sum;
}

long long dot_seq(const std::vector<int>& a, const std::vector<int>& b) {
    long long sum = 0;
    for (std::size_t i = 0; i < a.size(); ++i) sum += static_cast<long long>(a[i]) * b[i];
    return sum;
}

long long sequential(std::size_t vec_size, unsigned seed, std::size_t num_vecs = 10) {
    std::vector<int> prev = gen_random_int_vector(vec_size, seed);
    long long total = 0;
    for (std::size_t i = 1; i < num_vecs; ++i) {
        auto cur = gen_random_int_vector(vec_size, seed + static_cast<unsigned>(i));
        total += dot_seq(prev, cur);
        prev = std::move(cur);
    }
    return total;
}

long long parallel_sections(std::size_t vec_size, unsigned seed, std::size_t num_vecs = 10) {
    std::vector<int> a;
    std::vector<int> b;
    #pragma omp parallel sections
    {
        #pragma omp section
        { a = gen_random_int_vector(vec_size, seed); }
        #pragma omp section
        { b = gen_random_int_vector(vec_size, seed + 1); }
    }

    long long total = dot(a, b);
    for (std::size_t i = 2; i < num_vecs; ++i) {
        a = std::move(b);
        b = gen_random_int_vector(vec_size, seed + static_cast<unsigned>(i));
        total += dot(a, b);
    }
    return total;
}

int main(int argc, char** argv) {
    const auto args = parse_cli(argc, argv);
    if (args.size == 0 || args.mode.empty()) return 1;

    long long result = 0;
    double total = 0.0;
    for (int r = 0; r < args.runs; ++r) {
        total += measure_seconds([&] {
            if (args.mode == "sequential") result = sequential(args.size, args.seed);
            else if (args.mode == "parallel_sections") result = parallel_sections(args.size, args.seed);
            else std::exit(1);
        });
    }

    output_csv_row({argv[0], args.mode, args.threads, total / args.runs, std::to_string(result)});
    return 0;
}
