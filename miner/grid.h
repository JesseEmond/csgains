#ifndef GRID_H
#define GRID_H

#include <vector>
#include <utility>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <iostream>
#include <iterator>

using Coordinate = int;
using Position = std::pair<Coordinate, Coordinate>;
using Cell = char;
using Row = std::vector<Cell>;
using Grid = std::vector<Row>;
using EncodedPosition = int;

const Cell Empty = '.';
const Cell Blocker = '#';
const Cell Start = 's';
const Cell End = 'e';

using Path = std::vector<Position>;
using CameFrom = std::vector<std::pair<EncodedPosition, EncodedPosition>>;
using CostSoFar = std::vector<std::pair<EncodedPosition, int>>;
using HeapNode = std::pair<int, EncodedPosition>;

inline int heuristic(Position a, Position b) {
    // manhattan
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

inline EncodedPosition encode(Position p) {
    return ((p.first & 0xFFFF) << 16) + (p.second & 0xFFFF);
}
inline Position decode(EncodedPosition n) {
    return Position{(n & 0xFFFF0000) >> 16, n & 0xFFFF};
}

Path reconstruct_path(EncodedPosition start, EncodedPosition end, const CameFrom& came_from) {
    Path path;
    while (end != start) {
        path.push_back(decode(end));
        const auto it = std::find_if(came_from.begin(), came_from.end(), [&end] (auto p) {
            return p.first == end;
        });
        end = it->second;
    }
    path.push_back(decode(start));
    reverse(path.begin(), path.end());
    return path;
}

const std::array<Position, 4> directions = { Position(1,0), Position(-1,0),
                                             Position(0,1), Position(0,-1) };
Path shortest_path(const Grid& grid, Position start, Position end,
                   bool use_heuristic,
                   CameFrom& came_from, CostSoFar& cost_so_far) {
    static const auto frontier_comparator = std::greater_equal<HeapNode>();
    std::vector<HeapNode> frontier;
    const auto encoded_start = encode(start);
    const auto encoded_end = encode(end);
    frontier.emplace_back(0, encoded_start);

    came_from.clear();
    cost_so_far.clear();

    came_from.emplace_back(encoded_start, encoded_start);
    cost_so_far.emplace_back(encoded_start, 0);

    while (!frontier.empty()) {
        std::pop_heap(frontier.begin(), frontier.end(), frontier_comparator);
        const auto pos = frontier.back().second;
        frontier.pop_back();

        if (pos == encoded_end) {
            return reconstruct_path(encoded_start, encoded_end, came_from);
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

            auto curItCost = cost_so_far.end();
            auto newIt = cost_so_far.end();
            for (auto it = cost_so_far.begin(); it != cost_so_far.end(); ++it) {
                if (it->first == pos) {
                    curItCost = it;
                    if (newIt != cost_so_far.end()) break;
                } else if (it->first == next) {
                    newIt = it;
                    if (curItCost != cost_so_far.end()) break;
                }
            }
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
                frontier.emplace_back(priority, next);
                std::push_heap(frontier.begin(), frontier.end(), frontier_comparator);
                came_from.emplace_back(next, pos);
            }
        }
    }

    return {};
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
