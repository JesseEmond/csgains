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
using EncodedPosition = uint32_t;
using EncodedHeapNode = uint64_t;

const Cell Empty = '.';
const Cell Blocker = '#';
const Cell Start = 's';
const Cell End = 'e';

using Path = std::vector<Position>;
using CameFrom = std::vector<std::pair<EncodedPosition, EncodedPosition>>;
using CostSoFar = std::vector<std::pair<EncodedPosition, int>>;
using Frontier = std::vector<EncodedHeapNode>;

inline int heuristic(Position a, Position b) {
    // manhattan
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

inline EncodedPosition encode(Position p) {
    return ((p.first & 0xFFFF) << 16) | (p.second & 0xFFFF);
}
inline Position decode(EncodedPosition n) {
    return Position{(n & 0xFFFF0000) >> 16, n & 0xFFFF};
}
inline EncodedHeapNode encode_score(int score, EncodedPosition position) {
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
        const auto it = std::find_if(came_from.begin(), came_from.end(), [&end] (auto p) {
            return p.first == end;
        });
        end = it->second;
    }
    path.push_back(decode(start));
    reverse(path.begin(), path.end());
}

const std::array<Position, 4> directions = { Position(1,0), Position(-1,0),
                                             Position(0,1), Position(0,-1) };
void shortest_path(const Grid& grid, Position start, Position end,
                   bool use_heuristic,
                   CameFrom& came_from, CostSoFar& cost_so_far,
                   Path& path) {
    static const auto frontier_comparator = std::greater_equal<EncodedHeapNode>();
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
            if (next_pos.first <= 0 ||
                static_cast<unsigned int>(next_pos.first+1) >= grid.size() ||
                next_pos.second <= 0 ||
                static_cast<unsigned int>(next_pos.second+1) >= grid.size() ||
                grid[next_pos.first][next_pos.second] == Blocker) continue;
            const auto next = encode(next_pos);

            const std::array<EncodedPosition, 2> its = { pos, next };
            auto it = std::find_first_of(cost_so_far.begin(), cost_so_far.end(),
                                         its.begin(), its.end(), [] (auto pair, auto pos) {
                return pair.first == pos;
            });
            const auto remaining = it->first == pos ? next : pos;
            auto other = std::find_if(std::next(it), cost_so_far.end(), [remaining] (auto pair) {
                return pair.first == remaining;
            });
            auto curItCost = it->first == pos ? it : other;
            auto newIt = curItCost == it ? other : it;

            const auto new_cost = curItCost->second + 1;
            if (newIt == cost_so_far.end()) {
                cost_so_far.emplace_back(next, std::numeric_limits<int>::max());
                newIt = std::prev(cost_so_far.end());
            }
            if (new_cost < newIt->second) {
                newIt->second = new_cost;
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
