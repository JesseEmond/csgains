from ctypes import cdll
import ctypes
lib = cdll.LoadLibrary('./libsolvers.so')

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
