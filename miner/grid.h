#ifndef GRID_H
#define GRID_H

#include <vector>
#include <utility>
#include <set>
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

int heuristic(Position a, Position b) {
    // manhattan
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

Path shortest_path(const Grid& grid, Position start, Position end) {
    using namespace std;
    const array<Position, 4> directions = { Position(1,0), Position(-1,0),
                                            Position(0,1), Position(0,-1) };

    vector<vector<int>> min_distance(grid.size(),
                                     vector<int>(grid.size(),
                                                 numeric_limits<int>::max()));
    min_distance[start.first][start.second] = 0;
    set<pair<int, Position>> active;
    vector<vector<Position>> came_from(grid.size(),
                                       vector<Position>(grid.size(),
                                                        Position(-1,-1)));
    active.insert({0, start});

    while (!active.empty()) {
        const auto pos = begin(active)->second;
        if (pos == end) {
            Path path;
            auto current = pos;
            while (current != start) {
                path.push_back(current);
                current = came_from[current.first][current.second];
            }
            path.push_back(start);
            reverse(path.begin(), path.end());
            return path;
        }
        active.erase(begin(active));
        for (const auto direction : directions) {
            const auto neighbour = Position(pos.first + direction.first,
                                            pos.second + direction.second);
            if (neighbour.first <= 0 || neighbour.first >= grid.size()-1 ||
                neighbour.second <= 0 || neighbour.second >= grid.size()-1 ||
                grid[neighbour.first][neighbour.second] == Blocker) continue;

            const auto new_g = min_distance[pos.first][pos.second] + 1;
            auto& g = min_distance[neighbour.first][neighbour.second];
            if (new_g < g) {
                active.erase({g, neighbour});
                g = new_g;
                active.insert({new_g, neighbour});
            }
        }
    }
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
