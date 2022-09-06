import base64


import ctypes
import numpy as np

from numpy.ctypeslib import ndpointer
import pandas as pd
import colorsys
import sys

import numba

eval = ctypes.CDLL("./eval.so");
N_PCS=8

print_genes = eval.print_genes
print_genes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]


print_raw_pieces = eval.print_raw_pieces
print_raw_pieces.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]


print_gene_pieces_horizontal = eval.print_gene_pieces_horizontal
print_gene_pieces_horizontal.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'),  ctypes.c_size_t]



def genes_to_B64(genes):
    return "".join(([base64.b64encode(bytes(genes[i:i+1]))[:-1].decode() for i in range(len(genes))]))


def hash_to_B64(hash):
    return "".join(([base64.b64encode(bytes(hash[i:i+1]))[:-2].decode() for i in range(len(hash))]))




@numba.njit
def popcount(x):
    b = 0
    while x > 0:
        x &= x - 1
        b += 1
    return b

vec_popcount = np.vectorize(popcount)

@numba.njit
def genetic_distance(ind_a, ind_b):
    dist = ind_a ^ ind_b
    n_genes = len(dist)
    popcont = np.zeros(n_genes)
    
    for i in range(n_genes):
        popcont[i] = popcount(dist[i])

    return np.sqrt(np.sum(popcont))








def get64Pos(c):
    return int.from_bytes(base64.b64decode(f"{c}A=="), "big") >> 2

def print_color_b64(string):
    for s in string:
        pos = get64Pos(s)
        # print(pos)

        (r,g,b) = colorsys.hsv_to_rgb(pos/64.0, 1, 0.9)

        r = int(r*256)
        g = int(g*256)
        b = int(b*256)

        print(f"\x1b[38;2;{r};{g};{b}m{s}\x1b[0m", end="")




if __name__ == "__main__":
    print(sys.argv)
    if len(sys.argv) == 2:
        print("Using Argv")
        df = pd.read_csv(sys.argv[1])
    else:
        df = pd.read_csv("epoch_19.csv")


    i = 0
    for index, row in df.iterrows():
        gene = row[0:N_PCS].to_numpy().astype(np.uint16)
        hashes = row[N_PCS:2*N_PCS].to_numpy().astype(np.uint32)
        b64g = genes_to_B64(gene)
        b64h = hash_to_B64(hashes)

        print_color_b64(b64h)
        print("   ", end="")
        print_color_b64(b64g)

        print(f" {index} -  Unique: {row['unique']},  No-Sols: {row['no_sol']} Total: {row['total_sol']}")
        print_genes(gene, len(gene))
        print_raw_pieces(gene, len(gene))
        i+= 1
        if i > 10:
            break
