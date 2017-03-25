from ctypes import cdll
import ctypes
lib = cdll.LoadLibrary('./libsolvers.so')
lib.solve_list_sort.restype = ctypes.c_uint64
lib.solve_shortest_path.restype = ctypes.c_uint64

lib.unit_test()

def solve_sorted_list(target_prefix, previous_hash, nb_elements):
    try:
        return lib.solve_list_sort(target_prefix.encode('ascii'),
                                   previous_hash.encode('ascii'),
                                   nb_elements,
                                   True)
    except:
        return 0

def solve_reverse_sorted_list(target_prefix, previous_hash, nb_elements):
    try:
        return lib.solve_list_sort(target_prefix.encode('ascii'),
                                   previous_hash.encode('ascii'),
                                   nb_elements,
                                   False)
    except:
        return 0

def solve_shortest_path(target_prefix, previous_hash, nb_blockers, grid_size):
    try:
        return lib.solve_shortest_path(target_prefix.encode('ascii'),
                                       previous_hash.encode('ascii'),
                                       nb_blockers,
                                       grid_size)
    except:
        return 0
