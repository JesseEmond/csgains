from coinslib import MT64, seed_from_hash
from cpp_solvers import (solve_sorted_list, solve_reverse_sorted_list,
                         solve_shortest_path)
import hashlib
import random
import time
import threading
import Grid


class ChallengeSolver(threading.Thread):
    def __init__(self, challenge_name, challenge):
        threading.Thread.__init__(self)
        self.challenge_name = challenge_name
        self.mt = None
        self.alive = True
        self.solution_found = False
        self.solution = ()
        self.challenge = challenge
        self.attempts = 0
        self.last_display_time = time.clock()

    def feed_prng(self, previous_hash, nonce):
        hasher = hashlib.sha256()
        hasher.update("{0}{1}".format(previous_hash, nonce).encode("ascii"))
        seed_hash = hasher.hexdigest()

        seed = seed_from_hash(seed_hash)
        self.mt = MT64(seed)

    def solve(self, parameters, hash_prefix, previous_hash):
        pass

    def run(self):
        self.solution = self.solve(self.challenge.parameters, self.challenge.hash_prefix, self.challenge.last_solution_hash)
        self.solution_found = True


class SortedListSolver(ChallengeSolver):
    def __init__(self, challenge):
        ChallengeSolver.__init__(self, 'sorted_list', challenge)

    def solve(self, parameters, hash_prefix, previous_hash):
        nb_elements = parameters['nb_elements']

        #nonce = random.randint(0, 99999999)
        nonce = solve_sorted_list(hash_prefix, previous_hash, nb_elements)
        print("Verifying nonce %d..." % nonce)

        while self.alive:
            self.feed_prng(previous_hash, nonce)

            elements = []

            for i in range(nb_elements):
                elements.append(self.mt.extract_number())

            elements.sort()

            solution_string = ""
            for i in elements:
                solution_string += "{0}".format(i)

            sha256 = hashlib.sha256()
            sha256.update(solution_string.encode('ascii'))
            solution_hash = sha256.hexdigest()

            if solution_hash.startswith(hash_prefix):
                print("Solution found ! nonce:{0} hash:{1}".format(nonce, solution_hash))
                return solution_hash, nonce

            nonce = random.randint(0, 99999999)

            self.attempts += 1
            if time.clock() - self.last_display_time >= 1:
                print("speed: %d/s" % self.attempts)
                self.attempts = 0
                self.last_display_time = time.clock()


class ReverseSortedListSolver(ChallengeSolver):
    def __init__(self, challenge):
        ChallengeSolver.__init__(self, 'reverse_sorted_list', challenge)

    def solve(self, parameters, hash_prefix, previous_hash):
        nb_elements = parameters['nb_elements']
        print("List: %d elems" % nb_elements)

        #nonce = random.randint(0, 99999999)
        nonce = solve_reverse_sorted_list(hash_prefix, previous_hash, nb_elements)
        print("Verifying nonce %d..." % nonce)

        while self.alive:
            self.feed_prng(previous_hash, nonce)

            elements = []

            for i in range(nb_elements):
                elements.append(self.mt.extract_number())

            elements.sort(reverse=True)

            solution_string = ""
            for i in elements:
                solution_string += "{0}".format(i)

            sha256 = hashlib.sha256()
            sha256.update(solution_string.encode('ascii'))
            solution_hash = sha256.hexdigest()

            if solution_hash.startswith(hash_prefix):
                print("Solution found ! nonce:{0} hash:{1}".format(nonce, solution_hash))
                return solution_hash, nonce

            nonce = random.randint(0, 99999999)

            self.attempts += 1
            if time.clock() - self.last_display_time >= 1:
                print("speed: %d/s" % self.attempts)
                self.attempts = 0
                self.last_display_time = time.clock()


class ShortestPathSolver(ChallengeSolver):
    def __init__(self, challenge):
        ChallengeSolver.__init__(self, 'shortest_path', challenge)

    def solve(self, parameters, hash_prefix, previous_hash):
        nb_blockers = parameters['nb_blockers']
        grid_size = parameters['grid_size']

        print("Grid: %dx%d, blocks: %d" % (grid_size, grid_size, nb_blockers))

        print("Hash prefix: %s" % hash_prefix)
        print("Previous hash: %s" % previous_hash)

        # nonce = random.randint(0, 9999999999)
        nonce = solve_shortest_path(hash_prefix, previous_hash,
                                   nb_blockers, grid_size)
        print("Verifying nonce %d..." % nonce)

        while self.alive:
            self.feed_prng(previous_hash, nonce)

            grid = Grid.Grid(grid_size)

            # placing initial walls
            for i in range(grid_size):
                grid.walls.append((i, 0))
                grid.walls.append((i, grid_size - 1))

                if i > 0 and i < (grid_size - 1):
                    grid.walls.append((0, i))
                    grid.walls.append((grid_size - 1, i))

            start_pos = (self.mt.extract_number() % grid_size, self.mt.extract_number() % grid_size)
            while start_pos in grid.walls:
                start_pos = (self.mt.extract_number() % grid_size, self.mt.extract_number() % grid_size)

            end_pos = (self.mt.extract_number() % grid_size, self.mt.extract_number() % grid_size)
            while end_pos in grid.walls or start_pos == end_pos:
                end_pos = (self.mt.extract_number() % grid_size, self.mt.extract_number() % grid_size)

            # placing walls
            for i in range(nb_blockers):
                # wall pos (row, col)
                block_pos = (self.mt.extract_number() % grid_size, self.mt.extract_number() % grid_size)
                if block_pos != start_pos and block_pos != end_pos and block_pos not in grid.walls:
                    grid.walls.append(block_pos)

            #trying to resolve the grid
            path = []
            solution_string = ""
            try:
                came_from, cost_so_far = Grid.dijkstra_search(grid, start_pos, end_pos)
                path = Grid.reconstruct_path(came_from, start_pos, end_pos)
                # print("PATH!!! ")
                # print(path)
                # grid.display(start_pos, end_pos)

                for coord in path:
                    solution_string += "{0}{1}".format(coord[0], coord[1])

                sha256 = hashlib.sha256()
                sha256.update(solution_string.encode("ascii"))

                solution_hash = sha256.hexdigest()

                if solution_hash.startswith(hash_prefix):
                    print("Solution found ! nonce:{0} hash:{1}".format(nonce, solution_hash))
                    return solution_hash, nonce

            except Exception as e:
                # No solution exists
                pass

            # print("No solution found! Bug? ... :(")
            # 1/0

            nonce = random.randint(0, 99999999)

            self.attempts += 1
            if time.clock() - self.last_display_time >= 1:
                print("speed: %d/s" % self.attempts)
                self.attempts = 0
                self.last_display_time = time.clock()
