from ctypes import cdll
import ctypes
lib = cdll.LoadLibrary('./libsolvers.so')
lib.solve_list_sort.restype = ctypes.c_uint64
lib.solve_shortest_path.restype = ctypes.c_uint64

lib.unit_test()

def solve_sorted_list(target_prefix, previous_hash, nb_elements):
    return lib.solve_list_sort(target_prefix.encode('ascii'),
                               previous_hash.encode('ascii'),
                               nb_elements,
                               True)

def solve_reverse_sorted_list(target_prefix, previous_hash, nb_elements):
    return lib.solve_list_sort(target_prefix.encode('ascii'),
                               previous_hash.encode('ascii'),
                               nb_elements,
                               False)

def solve_shortest_path(target_prefix, previous_hash, nb_blockers, grid_size):
    return lib.solve_shortest_path(target_prefix.encode('ascii'),
                                   previous_hash.encode('ascii'),
                                   nb_blockers,
                                   grid_size)
