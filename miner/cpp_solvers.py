from ctypes import cdll
lib = cdll.LoadLibrary('./libsolvers.so')

def solve_sorted_list():
    lib.solve_sorted_list()
