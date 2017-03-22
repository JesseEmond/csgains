#ifndef GRID_H
#define GRID_H

#include <vector>
#include <utility>
#include <map>
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
using CameFrom = std::map<Position, Position>;
using HeapNode = std::pair<int, Position>;

inline int heuristic(Position a, Position b) {
    // manhattan
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

Path reconstruct_path(Position start, Position end, const CameFrom& came_from) {
    Path path;
    while (end != start) {
        path.push_back(end);
        end = came_from.find(end)->second;
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    return path;
}

const std::array<Position, 4> directions = { Position(1,0), Position(-1,0),
                                        Position(0,1), Position(0,-1) };
Path shortest_path(const Grid& grid, Position start, Position end,
                   bool use_heuristic) {
    static const auto frontier_comparator = std::greater_equal<HeapNode>();
    std::vector<HeapNode> frontier;
    frontier.emplace_back(0, start);

    CameFrom came_from;
    std::map<Position, int> cost_so_far;
    came_from[start] = start;
    cost_so_far[start] = 0;

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

            const auto new_cost = cost_so_far[pos] + 1;
            if (!cost_so_far.count(next) || new_cost < cost_so_far[next]) {
                cost_so_far[next] = new_cost;
                const auto priority = new_cost +
                                      (use_heuristic ?
                                       heuristic(next, end) : 0);
                frontier.emplace_back(priority, next);
                std::push_heap(frontier.begin(), frontier.end(), frontier_comparator);
                came_from[next] = pos;
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
