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

const Cell Empty = '.';
const Cell Blocker = '#';
const Cell Start = 's';
const Cell End = 'e';

using Path = std::vector<Position>;
using CameFrom = std::vector<std::pair<Position, Position>>;
using CostSoFar = std::vector<std::pair<Position, int>>;
using HeapNode = std::pair<int, Position>;

inline int heuristic(Position a, Position b) {
    // manhattan
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

Path reconstruct_path(Position start, Position end, const CameFrom& came_from) {
    Path path;
    while (end != start) {
        path.push_back(end);
        const auto it = std::find_if(came_from.begin(), came_from.end(), [&end] (const auto& p) {
            return p.first == end;
        });
        end = it->second;
    }
    path.push_back(start);
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
    frontier.emplace_back(0, start);

    came_from.clear();
    cost_so_far.clear();

    came_from.emplace_back(start, start);
    cost_so_far.emplace_back(start, 0);

    while (!frontier.empty()) {
        std::pop_heap(frontier.begin(), frontier.end(), frontier_comparator);
        const auto pos = frontier.back().second;
        frontier.pop_back();

        if (pos == end) {
            return reconstruct_path(start, end, came_from);
        }

        for (const auto direction : directions) {
            const auto next = Position(pos.first + direction.first,
                                       pos.second + direction.second);
            if (next.first <= 0 ||
                static_cast<unsigned int>(next.first+1) >= grid.size() ||
                next.second <= 0 ||
                static_cast<unsigned int>(next.second+1) >= grid.size() ||
                grid[next.first][next.second] == Blocker) continue;

            auto curItCost = std::find_if(cost_so_far.begin(), cost_so_far.end(), [&pos](const auto& cost) {
                return cost.first == pos;
            });
            const auto new_cost = curItCost->second + 1;
            auto newIt = std::find_if(cost_so_far.begin(), cost_so_far.end(), [&next](const auto& cost) {
                return cost.first == next;
            });
            if (newIt == cost_so_far.end()) {
                cost_so_far.emplace_back(next, std::numeric_limits<int>::max());
                newIt = std::prev(cost_so_far.end());
            }
            if (new_cost < newIt->second) {
                newIt->second = new_cost;
                const auto priority = new_cost +
                                      (use_heuristic ?
                                       heuristic(next, end) : 0);
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
