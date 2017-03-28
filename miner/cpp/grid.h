#ifndef GRID_H
#define GRID_H

#include <vector>
#include <utility>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cstdint>

using Coordinate = int;
using Position = std::pair<Coordinate, Coordinate>;
using Cell = char;
using Row = std::vector<Cell>;
using Grid = std::vector<Row>;

// Following Chandler Charruth's advice on compiler optimization
// (https://www.youtube.com/watch?v=eR34r7HOU14), Positions are passed by value.

// see encode/decode comments to know why we encode positions as ints
using EncodedPosition = uint32_t;
using EncodedHeapNode = uint64_t; // node in the A* frontier list (open list)

const Cell Empty = '.';
const Cell Blocker = '#';
const Cell Start = 's';
const Cell End = 'e';

using Path = std::vector<Position>;
using Frontier = std::vector<EncodedHeapNode>;

// Notice that we're using std::vectors of std::pairs instead of maps -- we're
// dealing with a small number of elements and CPU cache locality proves to win
// over algorithmic complexity in our case when profiling.
using CameFrom = std::vector<std::pair<EncodedPosition, EncodedPosition>>;
using CostSoFar = std::vector<std::pair<EncodedPosition, int>>;

inline int heuristic(Position a, Position b) {
    // manhattan
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

inline EncodedPosition encode(Position p) {
    // std::pair's == operator proves to be a bottleneck when profiling.
    // Result: we encode positions as ints. Using a union might be a better idea though...
    return ((p.first & 0xFFFF) << 16) | (p.second & 0xFFFF);
}
inline Position decode(EncodedPosition n) {
    return Position{(n & 0xFFFF0000) >> 16, n & 0xFFFF};
}
inline EncodedHeapNode encode_score(int score, EncodedPosition position) {
    // std::pair's < operator proves to be a bottleneck when profiling.
    // Encoding the fscore-position pairs as ints speeds up the heap operations.
    EncodedHeapNode n = score;
    return (n << 32) | position;
}
inline EncodedPosition decode_score(EncodedHeapNode n) {
    return n & 0xFFFFFFFF;
}

void reconstruct_path(EncodedPosition start, EncodedPosition end,
                      const CameFrom& came_from, Path& path) {
    path.clear();
    while (end != start) {
        path.push_back(decode(end));
        // searching linearly proves to be faster given the small sizes of the containers
        const auto it = std::find_if(came_from.begin(), came_from.end(), [&end] (auto p) {
            return p.first == end;
        });
        end = it->second;
    }
    path.push_back(decode(start));
    reverse(path.begin(), path.end());
}

// same order as defined by the challenge's documentation
const std::array<Position, 4> directions = { Position(1,0), Position(-1,0),
                                             Position(0,1), Position(0,-1) };

inline bool is_on_grid(const Grid& grid, Position pos) {
    // the documentation states that there are blockers on the sides of the grid.
    // we avoid setting those on the grid and implicitely handle that requirement
    // by considering the sides to be outside of the grid.
    return pos.first > 0 &&
           pos.first + 1 < static_cast<Coordinate>(grid.size()) &&
           pos.second > 0 &&
           pos.second + 1 < static_cast<Coordinate>(grid.size());
}

// we reuse the allocated containers as allocation/deallocation proves to be
// a bottleneck when profiling.
void shortest_path(const Grid& grid, Position start, Position end,
                   bool use_heuristic,
                   CameFrom& came_from, CostSoFar& cost_so_far,
                   Path& path) {
    static const auto frontier_comparator = std::greater_equal<EncodedHeapNode>();
    static const int movement_cost = 1;
    Frontier frontier;
    const auto encoded_start = encode(start);
    const auto encoded_end = encode(end);
    frontier.push_back(encode_score(0, encoded_start));

    came_from.clear();
    cost_so_far.clear();

    came_from.emplace_back(encoded_start, encoded_start);
    cost_so_far.emplace_back(encoded_start, 0);

    while (!frontier.empty()) {
        std::pop_heap(frontier.begin(), frontier.end(), frontier_comparator);
        const auto pos = decode_score(frontier.back());
        frontier.pop_back();

        if (pos == encoded_end) {
            reconstruct_path(encoded_start, encoded_end, came_from, path);
            return;
        }

        for (const auto direction : directions) {
            const auto decoded = decode(pos);
            const auto next_pos = Position(decoded.first + direction.first,
                                           decoded.second + direction.second);
            if (!is_on_grid(grid, next_pos) ||
                grid[next_pos.first][next_pos.second] == Blocker) continue;
            const auto next = encode(next_pos);

            // search for both costs at the same time -- the one to the current
            // node and (potentially) the one that we already have to the neighbour.
            const std::array<EncodedPosition, 2> its = { pos, next };
            auto it = std::find_first_of(cost_so_far.begin(), cost_so_far.end(),
                                         its.begin(), its.end(), [] (auto pair, auto pos) {
                return pair.first == pos;
            });
            const auto remaining = it->first == pos ? next : pos;
            auto other = std::find_if(std::next(it), cost_so_far.end(), [remaining] (auto pair) {
                return pair.first == remaining;
            });
            auto current_cost_it = it->first == pos ? it : other;
            auto new_cost_it = current_cost_it == it ? other : it;

            const auto new_cost = current_cost_it->second + movement_cost;
            if (new_cost_it == cost_so_far.end()) {
                cost_so_far.emplace_back(next, std::numeric_limits<int>::max());
                new_cost_it = std::prev(cost_so_far.end());
            }
            if (new_cost < new_cost_it->second) {
                new_cost_it->second = new_cost;
                const auto priority = new_cost +
                                      (use_heuristic ?
                                       heuristic(next_pos, end) : 0);
                frontier.push_back(encode_score(priority, next));
                std::push_heap(frontier.begin(), frontier.end(), frontier_comparator);
                came_from.emplace_back(next, pos);
            }
        }
    }

    path.clear(); // no path
}

// Used for debugging purposes
void display(const Grid &grid) {
    using namespace std;
    cout << string(grid.size(), Blocker) << endl;
    for (Grid::size_type i = 1; i < grid.size() - 1; ++i) {
        const auto row = grid[i];
        cout << Blocker;
        copy(next(begin(row)), prev(end(row)), ostream_iterator<Cell>(cout, ""));
        cout << Blocker << endl;
    }
    cout << string(grid.size(), Blocker) << endl;
}

#endif
