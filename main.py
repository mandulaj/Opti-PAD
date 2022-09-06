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

import numba
import vis

N_POPULATION = 50
N_PIECES = 8
PIECE_SIZES = [5, 5, 5, 5, 5, 5, 5, 6]
# PIECE_SIZES = [4, 5, 5, 5, 5, 5, 6, 6]
N_SCORES = 3
N_GENERATIONS = 40

MUT_PROB= 0.01
CROSS_SPLIT=1
N_BEST=3 # N best that survive








if len(sys.argv) == 2:
    MUT_PROB = float(sys.argv[1])
else:
    MUT_PROB = 0.01

OUT=f"./results/run_{MUT_PROB:.2f}_"

for i in range(1000):
    path = OUT + str(i)
    if not os.path.exists(path):
        os.mkdir(path)
        OUT = path
        break

print(f"Output in {OUT}")





@numba.njit
def make_distance_matrix(genes):
    dist_matrix = np.zeros((len(genes), len(genes)))

    for i in range(len(genes)):
        for j in range(len(genes)):
            dist_matrix[i][j] = vis.genetic_distance(genes[i], genes[j])
    return dist_matrix






def fitness_function(no_sol, unqe_pieces, total_solutions):
    no_sol_fitness = 1/((no_sol)**2+1) # Minimize number of no solutions
    unique_pieces_fitness = unqe_pieces/N_PIECES # Maximize Unique Pieces
    # total_solutions_fitness = np.sqrt((total_solutions+1)/(np.max(total_solutions)+1))
    # total_solutions_fitness = 1/(total_solutions/1000+1)
    # total_solutions_fitness = 1-1/(total_solutions+1)

    # # zero no sols
    # total_solutions[(no_sol != 0)] = 1000
    # minimal = np.min(total_solutions)



    total_solutions_fitness = np.minimum(3200/(0.2 * total_solutions + 3000),1)
    # total_solutions_fitness = 2/((total_solutions+1)/minimal)
    # print(total_solutions, total_solutions_fitness)
    # total_solutions_fitness = total_solutions/np.max(total_solutions)
    # print(no_sol_fitness, unique_pieces_fitness, total_solutions_fitness)

    return no_sol_fitness * unique_pieces_fitness ** 3  * total_solutions_fitness


def _process(elems):
    scores_a = np.zeros( N_SCORES, np.uint32)
    eval_numpy_individual(elems[0], scores_a,len(elems[0]), len(scores_a), elems[1])
    # eval_numpy_individual(gene_pool_b[i], gene_pool_scores_b[i], gene_pool_b.shape[1], gene_pool_scores_b.shape[1], restrictions)
    return scores_a





def main():

    gene_pool = np.zeros((N_POPULATION, N_PIECES), np.uint16)

    # init random gene pools 
    for indiv_id in range(N_POPULATION):
        for piece_id in range(N_PIECES):
            gene_pool[indiv_id][piece_id] = makeRandomGene(PIECE_SIZES[piece_id])

    restrictions = Sol_Restrictions(True, False)
    
    for gen in range(N_GENERATIONS):
        

        print(f"Generation {gen}")
        
        # Evaluate Gene pool
        with Pool(None) as p:
            r = tqdm(p.imap(_process, zip( gene_pool, itertools.repeat(restrictions))), total= gene_pool.shape[0])
            gene_pool_scores = np.vstack(list(r))

        gene_pool_scores = gene_pool_scores.astype(np.int64)    
        no_solution_cases = gene_pool_scores[:,1]                 # Number of No-solution cases 
        total_solutions = gene_pool_scores[:,0]                   # Total number of solutions

        unq_pieces = np.zeros(len(gene_pool), dtype=np.uint32)    # Unique Pieces used
        for i, gene in enumerate(gene_pool):
            unq_pieces[i] = unique_pieces(gene, len(gene))


        fitness = fitness_function(no_solution_cases, unq_pieces, total_solutions)
        best_fit = np.argsort(-fitness)
        probability = fitness/np.sum(fitness)
     
     
     
        gene_hash = np.zeros_like(gene_pool, dtype=np.uint32)
        genome_to_hashes(gene_pool, gene_hash, len(gene_pool), N_PIECES)
        df_hash = pd.DataFrame(gene_hash)
    

        dist_matrix = make_distance_matrix(gene_pool)
        # dist_matrix = np.zeros((len(genes),len(genes)))
        dm_avg = np.average(dist_matrix)
        dm_sum = np.sum(dist_matrix)
        dm_median = np.median(dist_matrix)


        ADDITIONAL_MUTATION = max(7-dm_median, 0) * 0.05


        # Record Data
        df = pd.DataFrame(gene_pool)
        df = pd.concat([df, df_hash], axis=1)
        df['probability'] = probability
        df['diveristy'] = dm_avg
        df['extra_mutation'] = ADDITIONAL_MUTATION
        df['fitness'] = fitness
        df['unique'] = unq_pieces
        df['total_sol'] = total_solutions
        df['no_sol'] = no_solution_cases


        df.sort_values("fitness", inplace=True, ascending=False)
        df.reset_index(drop=True, inplace=True)
        df.to_csv(f"{OUT}/epoch_{gen}.csv", index=False)
        # print_genome_color(gene_pool, len(gene_pool), N_PIECES)




        # print_gene_pieces_horizontal(gene_pool[0], gene_hash[0],N_PIECES)



        # Make new pool by cross mixing fit individuals
        new_gene_pool = makeNewGenePool(gene_pool, probability, cross=CROSS_SPLIT)

        # Introduce mutations
        for i in range(len(new_gene_pool)):
            for j in range(N_PIECES):
                if j == 7:
                    prob = MUT_PROB + ADDITIONAL_MUTATION
                else:
                    prob = MUT_PROB * 2 + ADDITIONAL_MUTATION
                new_gene_pool[i][j] = mutate(new_gene_pool[i][j], prob)

        # Add N best to gene pool without change
        gene_pool = np.vstack((new_gene_pool, gene_pool[best_fit[0:N_BEST]]))

        # # Print Summary of best
        # for i in range(3):
        #     j = best_fit[i]
        #     print(f"{i} - NoSol: {no_solution_cases[j]} fitness:{fitness[j]} Prob: {probability[j]}")
        #     print_raw_pieces(gene_pool[j], N_PIECES)
        
       


def randomGene():
    return np.random.randint(0x3FFF, dtype=np.uint16)


def makeRandomGene(length):
    if length > 6 or length < 3:
        return 0

    gene = randomGene()

    return gene | ((length - 3) << 14)






def makeNewGenePool(gene_pool, probability, cross=1):

    parent_idx1 = np.random.choice(len(gene_pool), size=len(gene_pool), replace=True, p=probability)
    parent_idx2 = np.random.choice(len(gene_pool), size=len(gene_pool), replace=True, p=probability)
    
    parent1 = gene_pool[parent_idx1]
    parent2 = gene_pool[parent_idx2]

    new_gene_pool = np.zeros_like(gene_pool)
    for i, (p1, p2) in enumerate(zip(parent1, parent2)):
        new_gene_pool[i] = cross_indivs(p1, p2, cross)

  
    
    return new_gene_pool


def mutate(gene, prob=0):
    mask = np.random.choice([0,1], size=14, replace=True, p=[1-prob,prob]) @ (1 << np.arange(14))
    mask = mask.astype(np.uint16)

    return gene ^ mask

def mutate_old(gene, iterations, flip=0.3):
    mask = randomGene()

    while iterations:
        mask &= randomGene()
        iterations -= 1

    # Chance of plipping
    if(np.random.rand() < flip):
        mask |= 0x1 << 13

    return gene ^ mask

def cross_indivs(indiv1, indiv2, prob=0.5):
    select = np.random.choice([True, False], size=len(indiv1), p=[prob, 1-prob])

    res = np.zeros_like(indiv1)
    res[select] = indiv1[select]
    res[~select] = indiv2[~select]
    return res

class GameIndividual(ctypes.Structure):
    _fields_ = [
        ("n_pieces", ctypes.c_uint64),
        ("genes", ctypes.POINTER(ctypes.c_uint16))
    ]


class Sol_Restrictions(ctypes.Structure):
    _fields_ = [
        ("use_faceup", ctypes.c_bool),
        ("use_facedown", ctypes.c_bool)
    ]

eval = ctypes.CDLL("./eval.so");
test_pass_indiv = eval.test_pass_indiv
game_individual_to_problem = eval.game_individual_to_problem

test_pass_indiv.argtypes = [ctypes.POINTER(GameIndividual)]
test_pass_indiv.restype = ctypes.c_int

# eval_individual = eval.eval_individual
# eval_individual.argtypes = [ctypes.POINTER(GameIndividual), Sol_Restrictions]

print_genes = eval.print_genes
print_genes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]


print_raw_pieces = eval.print_raw_pieces
print_raw_pieces.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]



eval_numpy_genes = eval.eval_numpy_genes
eval_numpy_genes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t, Sol_Restrictions]

eval_numpy_individual = eval.eval_numpy_individual
eval_numpy_individual.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t, Sol_Restrictions]


unique_pieces = eval.unique_pieces
unique_pieces.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t]
unique_pieces.restype = ctypes.c_uint32



print_genome_color = eval.print_genome_color
print_genome_color.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t]


genome_to_hashes = eval.genome_to_hashes
genome_to_hashes.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t, ctypes.c_size_t]


print_gene_pieces_horizontal = eval.print_gene_pieces_horizontal
print_gene_pieces_horizontal.argtypes = [ndpointer(ctypes.c_uint16, flags='C_CONTIGUOUS'), ndpointer(ctypes.c_uint32, flags='C_CONTIGUOUS'), ctypes.c_size_t]




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
