import ctypes
import numpy as np

from numpy.ctypeslib import ndpointer
import pandas as pd

eval = ctypes.CDLL("./eval.so");


print_genes = eval.print_genes
print_genes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]


print_raw_pieces = eval.print_raw_pieces
print_raw_pieces.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]



df = pd.read_csv("out.csv")

for index, row in df.iterrows():
    gene = row[0:8].to_numpy().astype(np.uint16)
    print(f"{index} - Unique: {row['unique']},  No-Sols: {row['no_sol']} Total: {row['total_sol']}")
    print_genes(gene, len(gene))
    print_raw_pieces(gene, len(gene))



# exit()
# a = np.loadtxt("out.csv", delimiter=',')
# genes = a[:,:8].astype(np.uint16)

# print(genes)

# for i, row in enumerate(genes):
#     print(f"{i} - No-Sols: {a[i][-1]}")
#     print(len(row))
#     print_genes(row, len(row))

