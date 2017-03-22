#ifndef GRID_H
#define GRID_H

#include <vector>
#include <utility>
#include <set>
#include <map>

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

int heuristic(Position a, Position b) {
    // manhattan
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

Path shortest_path(const Grid& grid, Position start, Position end) {
    using namespace std;
    vector<vector<int>> min_distance(grid.size(),
                                     vector<int>(grid.size(),
                                                 numeric_limits<int>::max()));
    min_distance[start.first][start.second] = 0;
    set<pair<int, Position>> active;
    active.insert({0, start});

    while (!active.empty()) {
        const auto pos = begin(active)->second;
        if (pos == end) return Path{};
        active.erase(begin(active));
        //TODO
    }
}

#endif
