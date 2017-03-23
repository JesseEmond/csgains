#include <iostream>
#include <sstream>
#include <string>
#include <climits>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <future>
#include <thread>
#include <atomic>
#include <random>
#include <cstdint>

#include "sha256.h"
#include "grid.h"

using namespace std;

using nonce_t = int64_t;
using MT64 = std::mt19937_64;
using value_t = MT64::result_type;

nonce_t random_nonce() {
    static MT64 r;
    nonce_t n = r();
    return n >= 0 ? n : -n;
}

template <typename T>
T swap_endian(T u) {
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

string sha256(const string& src) {
    SHA256 h;
    return h(src);
}

value_t seed_from_hash(const string& hash) {
    string prefix(begin(hash), next(begin(hash), 16)); // 8 bytes
    stringstream converter(prefix);
    value_t seed;
    converter >> std::hex >> seed;
    return swap_endian(seed);
}

void feed_prng(MT64& mt, const char* previous, nonce_t nonce) {
    stringstream ss;
    ss << previous << nonce;
    string hash = sha256(ss.str());
    auto seed = seed_from_hash(hash);
    mt.seed(seed);
}

bool test_list_sort_nonce(MT64& mt, const string& target,
                          const char* previous_hash, int nb_elements,
                          bool asc, nonce_t nonce, vector<value_t>& values,
                          stringstream& ss) {
    feed_prng(mt, previous_hash, nonce);
    values.resize(nb_elements);
    generate(begin(values), end(values), [&] { return mt(); });

    if (asc) sort(begin(values), end(values));
    else sort(begin(values), end(values), std::greater<value_t>());

    ss.str(string());
    copy(begin(values), end(values), ostream_iterator<value_t>(ss));
    auto hash = sha256(ss.str());
    return hash.compare(0, target.size(), target) == 0;
}

Position random_position(MT64& mt, int grid_size) {
    while (true) {
        const int row = mt() % grid_size;
        const int column = mt() % grid_size;
        if (row == 0 || row+1 == grid_size ||
            column == 0 || column+1 == grid_size) continue;
        return std::move(Position(row, column));
    }
}

Position random_end_position(MT64& mt, int grid_size, Position start) {
    while (true) {
        const int row = mt() % grid_size;
        const int column = mt() % grid_size;
        if (row == 0 || row+1 == grid_size ||
            column == 0 || column+1 == grid_size) continue;
        if (row == start.first && column == start.second) continue;
        return std::move(Position(row, column));
    }
}

Position random_unchecked_position(MT64& mt, int grid_size) {
    const auto row = mt() % grid_size;
    const auto column = mt() % grid_size;
    return std::move(Position(row, column));
}


void random_grid(MT64& mt, int nb_blockers, int grid_size,
                 Position start, Position end, Grid& grid, Row& empty_row) {
    grid.clear();
    empty_row.resize(grid_size, Empty);
    for (int i = 0; i < grid_size; ++i) grid.push_back(empty_row);
    grid[start.first][start.second] = Start;
    grid[end.first][end.second] = End;

    for (int i = 0; i < nb_blockers; ++i) {
        const auto blocker = random_unchecked_position(mt, grid_size);
        if (blocker != start && blocker != end) {
            grid[blocker.first][blocker.second] = Blocker;
        }
    }
}

bool test_shortest_path_nonce(MT64& mt, const string& target,
                              const char* previous_hash, int nb_blockers,
                              int grid_size, nonce_t nonce,
                              CameFrom& came_from, CostSoFar& cost_so_far,
                              Grid& grid, Row& empty_row, Path& path,
                              stringstream& ss) {
    feed_prng(mt, previous_hash, nonce);

    const auto start = random_position(mt, grid_size);
    const auto end = random_end_position(mt, grid_size, start);
    random_grid(mt, nb_blockers, grid_size, start, end, grid, empty_row);
    shortest_path(grid, start, end, true, came_from, cost_so_far, path);
    if (path.empty()) return false;
    ss.str(string());
    for_each(path.begin(), path.end(), [&ss](Position pos) {
        ss << pos.first << pos.second;
    });
    auto hash = sha256(ss.str());
    if (hash.compare(0, target.size(), target) != 0) return false;

    // we might be unlucky and have A* path != dijkstra path (e.g. dijkstra
    // is allowed to go in directions that increase the heuristic but give
    // the same path) => verify that dijkstra (no heuristic) gives the same
    // path.
    cout << "Potential path found. Checking Dijkstra." << endl;
    const auto original_path = path;
    shortest_path(grid, start, end, false, came_from, cost_so_far, path);

    return original_path == path;
}

const auto n_threads = max(thread::hardware_concurrency(), 1u);
template <class TaskCreator>
nonce_t multithreaded_task(nonce_t start, int max_tries_per_thread, TaskCreator task_creator) {
    cout << "Launching " << n_threads << " threads." << endl;
    vector<future<nonce_t>> threads;
    threads.reserve(n_threads);
    const auto before = chrono::steady_clock::now();
    const int workload = max_tries_per_thread;
    atomic_bool done(false);

    for (unsigned int i = 0; i < n_threads; ++i) {
        const nonce_t my_start = start + workload * i;
        const nonce_t my_end = my_start + workload;
        threads.push_back(async(launch::async, [i, my_start, my_end, workload, &task_creator, &done] () -> nonce_t {
            auto task = task_creator();
            for (nonce_t nonce = my_start; nonce < my_end; ++nonce) {
                // try a couple of times before checking the atomic flag
                for (nonce_t j = 0; j < 1000; ++j, ++nonce) {
                    if (task(nonce)) {
                        done.store(true);
                        cout << "Thread " << i << " found nonce " << nonce << endl;
                        return nonce;
                    }
                }
                if (done) {
                    // return how many nonces were checked
                    return -(nonce - my_start);
                }
            }
            // return how many nonces were checked
            return -workload;
        }));
    }
    vector<nonce_t> results;
    transform(begin(threads), end(threads), back_inserter(results), [] (auto &thread) { return thread.get(); });
    const auto found_nonce = *max_element(begin(results), end(results));

    const auto after = chrono::steady_clock::now();
    const auto diff = chrono::duration_cast<chrono::duration<float>>(after - before);
    nonce_t explored = 0;
    for (unsigned int i = 0; i < n_threads; ++i) {
        explored += results[i] >= 0 ? results[i] - workload * i - start : -results[i];
    }
    cout << "speed: " << static_cast<double>(explored) / diff.count() << "/s" << endl;

    return found_nonce;
}


extern "C" {
    void unit_test() {
        MT64 mt;
        vector<value_t> values;
        stringstream ss;
        if (!test_list_sort_nonce(mt, "433e", "9cc5a925757e626b1febbdf62c1643d5bab6473c0a960ad823ab742e18560977", 100, false, 15236, values, ss)) {
            class unittest_failed_list_sort{};
            throw unittest_failed_list_sort{};
        }

        class unittest_failed_shortest_path{};
        CameFrom came_from;
        CostSoFar cost_so_far;
        Grid grid;
        Row empty_row;
        Path path;
        if (!test_shortest_path_nonce(mt, "8fe4", "9551d9f2b91df3381938ddc8ee97dcf0663113ceacd8f766912aa6bcf35bb18b", 80, 25, 21723, came_from, cost_so_far, grid, empty_row, path, ss)) {
            throw unittest_failed_shortest_path{};
        }
        if (!test_shortest_path_nonce(mt, "12f7", "8fe4ed64fc0397a07dfe3a270d7e148aeb9fbac7c54d1eb870d0f379c0f4c211", 80, 25, 95148, came_from, cost_so_far, grid, empty_row, path, ss)) {
            throw unittest_failed_shortest_path{};
        }
        if (test_shortest_path_nonce(mt, "7134", "72c59bc893cc40dd9101500b558bdd35e612e935339bd017eb69802391d0d038", 80, 25, 114393, came_from, cost_so_far, grid, empty_row, path, ss)) {
            throw unittest_failed_shortest_path{};
        }
        if (test_shortest_path_nonce(mt, "3b6b", "228178a3b76322dd6e7c6329921a83c7d6a5b20dbf00002eca98db00054acf44", 80, 25, 124097, came_from, cost_so_far, grid, empty_row, path, ss)) {
            throw unittest_failed_shortest_path{};
        }

        cout << "Unit tests passed." << endl;
    }

    nonce_t solve_list_sort(const char* target_prefix,
                            const char* previous_hash,
                            int nb_elements,
                            bool asc) {
        string target(target_prefix);
        struct sort_task {
            string target;
            const char* previous_hash;
            int nb_elements;
            bool asc;
            MT64 mt;
            vector<value_t> values;
            stringstream ss;

            sort_task(const string& target, const char* previous_hash, int nb_elements, bool asc)
                : target{target}, previous_hash{previous_hash}, nb_elements{nb_elements}, asc{asc}, mt{}, values{}, ss{} {}

            bool operator()(nonce_t nonce) {
                return test_list_sort_nonce(mt, target, previous_hash, nb_elements, asc, nonce, values, ss);
            }
        };

        const int step_size_per_thread = 5000;
        const int step_size = step_size_per_thread * n_threads;
        const int max_steps = 7;
        const nonce_t start_nonce = random_nonce();
        const nonce_t end_nonce = start_nonce + max_steps * step_size;
        for (nonce_t nonce = start_nonce; nonce < end_nonce; nonce += step_size) {
            cout << "Start nonce: " << nonce << endl;
            nonce_t found_nonce = multithreaded_task(nonce, step_size_per_thread,
                                                     [&] { return sort_task{target, previous_hash, nb_elements, asc}; });
            if (found_nonce >= 0) return found_nonce;
        }
        return 0;
    }

    nonce_t solve_shortest_path(const char* target_prefix,
                                const char* previous_hash,
                                int nb_blockers,
                                int grid_size) {
        string target(target_prefix);
        struct pathfinding_task {
            string target;
            const char* previous_hash;
            int nb_blockers;
            int grid_size;
            MT64 mt;
            CameFrom came_from;
            CostSoFar cost_so_far;
            Grid grid;
            Row empty_row;
            Path path;
            stringstream ss;

            pathfinding_task(const string& target,
                             const char* previous_hash,
                             int nb_blockers, int grid_size)
                : target{target}, previous_hash{previous_hash},
                nb_blockers{nb_blockers},
                grid_size{grid_size}, mt{}, came_from{}, cost_so_far{},
                grid{}, empty_row{}, path{}, ss{} {}

            bool operator()(nonce_t nonce) {
                return test_shortest_path_nonce(mt, target, previous_hash, nb_blockers, grid_size, nonce,
                                                came_from, cost_so_far, grid, empty_row, path, ss);
            }
        };

        const int step_size_per_thread = 50000;
        const int step_size = step_size_per_thread * n_threads;
        const int max_steps = 7;
        const nonce_t start_nonce = random_nonce();
        const nonce_t end_nonce = start_nonce + max_steps * step_size;
        for (nonce_t nonce = start_nonce; nonce < end_nonce; nonce += step_size) {
            cout << "Start nonce: " << nonce << endl;
            nonce_t found_nonce = multithreaded_task(nonce,
                                                 step_size_per_thread,
                                                 [&] {
                                                 return pathfinding_task{target, previous_hash, nb_blockers, grid_size};
                                                 });
            if (found_nonce >= 0) return found_nonce;
        }
        return 0;
    }
}
