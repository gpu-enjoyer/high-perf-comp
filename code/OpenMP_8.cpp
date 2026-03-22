#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <omp.h>

#include "common/benchmark.hpp"

struct VectorPair {
    std::vector<double> left;
    std::vector<double> right;
};

void generate_dataset(const std::string& path, std::size_t vectors_count, std::size_t vector_length, unsigned seed)
{
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("failed to create dataset");
    }

    std::uint64_t state = static_cast<std::uint64_t>(seed);
    out << vectors_count << ' ' << vector_length << '\n';
    for (std::size_t i = 0; i < vectors_count; ++i) {
        for (std::size_t j = 0; j < vector_length; ++j) {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            const double value = static_cast<double>(state % 100000ULL) / 100000.0;
            out << value;
            if (j + 1 != vector_length) {
                out << ' ';
            }
        }
        out << '\n';
    }
}

std::string join_args(int argc, char** argv)
{
    std::ostringstream oss;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) {
            oss << ' ';
        }
        oss << argv[i];
    }
    return oss.str();
}

int main(int argc, char** argv)
{
    try {
        if (argc < 5) {
            return 1;
        }

        const std::string task_name = std::filesystem::path(argv[0]).filename().string();
        const std::string arg_string = join_args(argc, argv);

        const std::string dataset_path = argv[1];
        const std::size_t vectors_count = std::stoull(argv[2]);
        const std::size_t vector_length = std::stoull(argv[3]);
        const unsigned seed = std::stoul(argv[4]);

        if (!std::filesystem::exists(dataset_path)) {
            generate_dataset(dataset_path, vectors_count, vector_length, seed);
        }

        std::deque<VectorPair> queue;
        omp_lock_t queue_lock;
        omp_init_lock(&queue_lock);
        bool producer_done = false;
        double total_dot_sum = 0.0;

        const double elapsed = measure_seconds([&]() {
            #pragma omp parallel sections num_threads(2) shared(queue, producer_done, total_dot_sum)
            {
                #pragma omp section
                {
                    std::ifstream in(dataset_path);
                    if (!in) {
                        std::terminate();
                    }

                    std::size_t file_vectors = 0;
                    std::size_t file_len = 0;
                    in >> file_vectors >> file_len;
                    if (file_vectors < 2 || file_len == 0) {
                        omp_set_lock(&queue_lock);
                        producer_done = true;
                        omp_unset_lock(&queue_lock);
                    } else {
                        std::vector<double> prev(file_len);
                        for (std::size_t j = 0; j < file_len; ++j) {
                            in >> prev[j];
                        }

                        for (std::size_t idx = 1; idx < file_vectors; ++idx) {
                            std::vector<double> curr(file_len);
                            for (std::size_t j = 0; j < file_len; ++j) {
                                in >> curr[j];
                            }

                            omp_set_lock(&queue_lock);
                            queue.push_back(VectorPair{prev, curr});
                            omp_unset_lock(&queue_lock);

                            prev = std::move(curr);
                        }

                        omp_set_lock(&queue_lock);
                        producer_done = true;
                        omp_unset_lock(&queue_lock);
                    }
                }

                #pragma omp section
                {
                    while (true) {
                        bool done = false;
                        bool has_pair = false;
                        VectorPair pair;

                        omp_set_lock(&queue_lock);
                        if (!queue.empty()) {
                            pair = std::move(queue.front());
                            queue.pop_front();
                            has_pair = true;
                        }
                        done = producer_done;
                        omp_unset_lock(&queue_lock);

                        if (has_pair) {
                            double dot = 0.0;
                            for (std::size_t j = 0; j < pair.left.size(); ++j) {
                                dot += pair.left[j] * pair.right[j];
                            }
                            total_dot_sum += dot;
                        } else if (done) {
                            break;
                        } else {
                            std::this_thread::yield();
                        }
                    }
                }
            }
        });

        omp_destroy_lock(&queue_lock);

        std::cout << task_name << ','
                  << arg_string << ','
                  << omp_get_max_threads() << ','
                  << elapsed << ','
                  << total_dot_sum << '\n';
        return 0;
    } catch (...) {
        return 1;
    }
}
