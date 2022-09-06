import ctypes
import numpy as np
import os

from numpy.ctypeslib import ndpointer
from tqdm import tqdm

from scipy.special import softmax
from multiprocessing import Pool
from tqdm.contrib.concurrent import process_map

import itertools
import sys
import pandas as pd



N_PIECES = 8


SKIP=-1


def _process(elems):
    ps = Puzzle_Stats(0)

    check_pieces(elems, len(elems), ctypes.byref(ps))

    res = np.array([ps.total_days_solutions, ps.total_no_solutions, ps.max_total_solutions, ps.min_total_solutions, ps.total_solutions,
             ps.faceup_days_solutions, ps.faceup_no_solutions, ps.max_faceup_solutions, ps.min_faceup_solutions,  ps.faceup_solutions,
             ps.facedown_days_solutions, ps.facedown_no_solutions, ps.max_facedown_solutions, ps.min_facedown_solutions, ps.facedown_solutions], dtype=np.uint64)
    
    res = np.hstack((elems, res))
    
    return res




N_PIECES_4 = 7
N_PIECES_5 = 18
N_PIECES_6 = 60

N_CHUNKS=100000


DESTINATION="exhaustive_results"
DESTINATION="er"

def main():

    puzzle_pieces_idxs = []
    for p1 in range(0, N_PIECES_5-6):
        for p2 in range(p1+1, N_PIECES_5-5):
            for p3 in range(p2+1, N_PIECES_5-4):
                for p4 in range(p3+1, N_PIECES_5-3):
                    for p5 in range(p4+1, N_PIECES_5-2):
                        for p6 in range(p5+1, N_PIECES_5-1):
                            for p7 in range(p6+1, N_PIECES_5):
                                for p8 in range(0, N_PIECES_6):
                                    puzzle_pieces_idxs.append(np.array([p1, p2,p3,p4,p5,p6,p7,p8], dtype=np.uint64))
       

    # puzzle_pieces_idxs = [np.array([0,1,2,3,4,5,6,0], dtype=np.uint64)]
    # puzzle_pieces_idxs = puzzle_pieces_idxs[:200000]


    chunks = np.array_split(puzzle_pieces_idxs, N_CHUNKS)

 
    for i, c in enumerate(chunks):
        if len(c) == 0:
            continue

        if i < SKIP:
            continue

        print(f"Processing Chunk {i+1}/{N_CHUNKS}")
        with Pool(None) as p:
            r = tqdm(p.imap(_process, c), total= len(c))
            results = np.vstack(list(r))
        np.savetxt(f"{DESTINATION}/chunk_{i:03d}.csv", results, delimiter=',', fmt="%u")
   

class Puzzle_Stats(ctypes.Structure):
    _fields_ = [
        ("total_solutions", ctypes.c_uint64),
        ("max_total_solutions", ctypes.c_uint64),
        ("min_total_solutions", ctypes.c_uint64),
        ("total_days_solutions", ctypes.c_uint64),
        ("total_no_solutions", ctypes.c_uint64),

        ("faceup_solutions", ctypes.c_uint64),
        ("max_faceup_solutions", ctypes.c_uint64),
        ("min_faceup_solutions", ctypes.c_uint64),
        ("faceup_days_solutions", ctypes.c_uint64),
        ("faceup_no_solutions", ctypes.c_uint64),

        ("facedown_solutions", ctypes.c_uint64),
        ("max_facedown_solutions", ctypes.c_uint64),
        ("min_facedown_solutions", ctypes.c_uint64),
        ("facedown_days_solutions", ctypes.c_uint64),
        ("facedown_no_solutions", ctypes.c_uint64),
    ]


class Sol_Restrictions(ctypes.Structure):
    _fields_ = [
        ("use_faceup", ctypes.c_bool),
        ("use_facedown", ctypes.c_bool)
    ]

exhaustive = ctypes.CDLL("./exhaustive.so");


check_pieces = exhaustive.check_pieces
check_pieces.argtypes = [ndpointer(ctypes.c_size_t, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.POINTER(Puzzle_Stats)]



# test_pass_indiv = eval.test_pass_indiv
# game_individual_to_problem = eval.game_individual_to_problem

# test_pass_indiv.argtypes = [ctypes.POINTER(GameIndividual)]
# test_pass_indiv.restype = ctypes.c_int

# # eval_individual = eval.eval_individual
# # eval_individual.argtypes = [ctypes.POINTER(GameIndividual), Sol_Restrictions]

# print_genes = eval.print_genes
# print_genes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]


# print_raw_pieces = eval.print_raw_pieces
# print_raw_pieces.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]



# eval_numpy_genes = eval.eval_numpy_genes
# eval_numpy_genes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t, Sol_Restrictions]

# eval_numpy_individual = eval.eval_numpy_individual
# eval_numpy_individual.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t, Sol_Restrictions]


# unique_pieces = eval.unique_pieces
# unique_pieces.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]
# unique_pieces.restype = ctypes.c_uint32



# print_genome_color = eval.print_genome_color
# print_genome_color.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t]


# genome_to_hashes = eval.genome_to_hashes
# genome_to_hashes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t]


# print_gene_pieces_horizontal = eval.print_gene_pieces_horizontal
# print_gene_pieces_horizontal.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t]







if __name__ == "__main__":
    main()



# n_pieces = 8
# p = (ctypes.c_uint16 * n_pieces)()
# test = GameIndividual(len(p), p )




# array[0] = GameIndividual(len(p), p )


# print(array[0].n_pieces)



# p[0] = 0b1100001001001010
# p[1] = 0b1000000000100101
# p[2] = 0b1000000011100011
# p[3] = 0b1000000001000110
# p[4] = 0b1010000000000011
# p[5] = 0b1000000010000011
# p[6] = 0b1000000001100011
# p[7] = 0b1000000000111010

# # res = test_pass_indiv(test)

# # print(f"Res: {res}")

# for i in range(test.n_pieces):
#     print(f"{test.genes[i]:b}")

# res = eval_individual(test, restrictions)





# def create_individuals(n):
#     elems = (GameIndividual * n)()
#     array = ctypes.cast(elems, ctypes.POINTER(GameIndividual))
    
#     for i in range(n):
#         array[i].n_pieces = 8










# print(f"Res: {res}")
