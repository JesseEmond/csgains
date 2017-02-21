from coinslib import MT64, seed_from_hash
from cpp_solvers import solve_sorted_list
import hashlib
import random
import time

class ChallengeSolver:
    def __init__(self, challenge_name):
        self.challenge_name = challenge_name
        self.mt = None
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

class SortedListSolver(ChallengeSolver):
    def __init__(self):
        ChallengeSolver.__init__(self, 'sorted_list')

    def solve(self, parameters, hash_prefix, previous_hash):
        nb_elements = parameters['nb_elements']

        nonce = 0#random.randint(0, 99999999)

        n = solve_sorted_list(hash_prefix, previous_hash, nb_elements)

        while True:
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
    def __init__(self):
        ChallengeSolver.__init__(self, 'reverse_sorted_list')

    def solve(self, parameters, hash_prefix, previous_hash):
        nb_elements = parameters['nb_elements']

        nonce = 0#random.randint(0, 99999999)

        #TODO replace
        n = solve_sorted_list(hash_prefix, previous_hash, nb_elements)

        while True:
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
