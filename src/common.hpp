#pragma once

#include <omp.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Args {
    std::size_t size = 0;
    std::string mode;
    unsigned seed = 42;
    int runs = 1;
    int threads = omp_get_max_threads();
};

inline int parse_int(const std::string& v, const char* name) {
    try {
        return std::stoi(v);
    } catch (...) {
        std::cerr << "Invalid integer for " << name << ": " << v << "\n";
        std::exit(2);
    }
}

inline unsigned parse_uint(const std::string& v, const char* name) {
    try {
        return static_cast<unsigned>(std::stoul(v));
    } catch (...) {
        std::cerr << "Invalid unsigned integer for " << name << ": " << v << "\n";
        std::exit(2);
    }
}

inline std::size_t parse_size(const std::string& v, const char* name) {
    try {
        return static_cast<std::size_t>(std::stoull(v));
    } catch (...) {
        std::cerr << "Invalid size for " << name << ": " << v << "\n";
        std::exit(2);
    }
}

inline Args parse_cli(int argc, char** argv) {
    Args args;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg.rfind("--runs=", 0) == 0) {
            args.runs = parse_int(arg.substr(7), "runs");
        } else if (arg == "--runs" && i + 1 < argc) {
            args.runs = parse_int(argv[++i], "runs");
        } else if (arg.rfind("--threads=", 0) == 0) {
            args.threads = parse_int(arg.substr(10), "threads");
        } else if (arg == "--threads" && i + 1 < argc) {
            args.threads = parse_int(argv[++i], "threads");
        } else if (arg.rfind("--seed=", 0) == 0) {
            args.seed = parse_uint(arg.substr(7), "seed");
        } else if (arg == "--seed" && i + 1 < argc) {
            args.seed = parse_uint(argv[++i], "seed");
        } else if (!arg.empty() && arg[0] != '-') {
            positional.push_back(arg);
        }
    }

    if (!positional.empty()) {
        if (positional[0] == "small" || positional[0] == "s") args.size = 100000;
        else if (positional[0] == "medium" || positional[0] == "m") args.size = 1000000;
        else if (positional[0] == "large" || positional[0] == "l") args.size = 10000000;
        else args.size = parse_size(positional[0], "size");
    }
    if (positional.size() >= 2) {
        args.mode = positional[1];
    }

    if (args.runs <= 0) {
        std::cerr << "runs must be > 0\n";
        std::exit(2);
    }
    if (args.threads <= 0) {
        std::cerr << "threads must be > 0\n";
        std::exit(2);
    }

    omp_set_num_threads(args.threads);
    return args;
}

template <typename Func>
inline double measure_seconds(Func&& fn) {
    const auto t0 = std::chrono::high_resolution_clock::now();
    fn();
    const auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(t1 - t0).count();
}

inline std::vector<int> gen_random_int_vector(std::size_t n, unsigned seed, int lo = 0, int hi = 1000000) {
    std::vector<int> out(n);
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(lo, hi);
    for (std::size_t i = 0; i < n; ++i) out[i] = dist(rng);
    return out;
}

inline std::vector<double> gen_random_double_vector(std::size_t n, unsigned seed, double lo = 0.0, double hi = 1.0) {
    std::vector<double> out(n);
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> dist(lo, hi);
    for (std::size_t i = 0; i < n; ++i) out[i] = dist(rng);
    return out;
}

struct RunResult {
    std::string program;
    std::string mode;
    int threads;
    double time;
    std::string extra;
};

inline void output_csv_row(const RunResult& r) {
    std::cout << fs::path(r.program).filename().string() << ","
              << r.mode << ","
              << r.threads << ","
              << r.time << ","
              << r.extra << "\n";
}
