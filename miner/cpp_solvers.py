from ctypes import cdll
import ctypes
lib = cdll.LoadLibrary('./libsolvers.so')

def solve_sorted_list(target_prefix, previous_hash, nb_elements):
    return lib.solve_sorted_list(target_prefix.encode('ascii'),
                                 previous_hash.encode('ascii'),
                                 nb_elements)

