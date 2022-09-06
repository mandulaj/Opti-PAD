import base64


import ctypes
import numpy as np

from numpy.ctypeslib import ndpointer
import pandas as pd
import colorsys
import sys

import numba


import vis

import os

import matplotlib.pyplot as plt
import tqdm
import sys

import re

if len(sys.argv) == 2:
    PATH = sys.argv[1]
else:
    PATH = "results/run_good4"


def natural_sort(l): 
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)]
    return sorted(l, key=alphanum_key)

files  = [os.path.join(PATH, f) for f in os.listdir(PATH) if os.path.isfile(os.path.join(PATH, f)) and os.path.splitext(f)[1] == ".csv"]
files = natural_sort(files)

@numba.njit
def make_distance_matrix(genes):
    dist_matrix = np.zeros((len(genes), len(genes)))

    for i in range(len(genes)):
        for j in range(len(genes)):
            dist_matrix[i][j] = vis.genetic_distance(genes[i], genes[j])
    return dist_matrix


N_BEST_FIT = 5

best_fitnesses  = np.zeros((len(files), N_BEST_FIT))
best_nosol  = np.zeros((len(files), N_BEST_FIT))
diversity_epochs  = np.zeros((len(files), 1))

for i, f in tqdm.tqdm(enumerate(files), total=len(files)):
    df = pd.read_csv(f)
    genes = df.to_numpy()[:, 0:8].astype(np.uint16)

    indexes = np.floor(np.linspace(0, len(genes)-1, 4)).astype(np.int32)
    fitness = df["fitness"].to_numpy()
    no_sols = df['no_sol'].to_numpy()
    best_fitnesses[i] = fitness[:N_BEST_FIT]
    best_nosol[i] = no_sols[:N_BEST_FIT]


    fitness = fitness[indexes]
    fitness = [f"{f:.3e}" for f in fitness]
    
    dist_matrix = make_distance_matrix(genes)
    # dist_matrix = np.zeros((len(genes),len(genes)))
    dm_avg = np.average(dist_matrix)
    dm_sum = np.sum(dist_matrix)
    dm_median = np.median(dist_matrix)

    diversity_epochs[i] = dm_avg
    print(f"Sum:{dm_sum}, Avg:{dm_avg} Median: {dm_median}")    

    fig, ax = plt.subplots()
    ax.set_title(os.path.basename(f))
    im1 = ax.imshow(dist_matrix)
    fig.colorbar(im1)
    im1.set_clim(0,8)


    ax.xaxis.tick_top()

    ax.set_xticks(indexes, fitness)
    ax.set_yticks(indexes, fitness)
    ax.set_xlabel("Fitness")
    ax.set_ylabel("Fitness")

    fig.savefig(os.path.join(PATH, f"{i:02}.png"))
    plt.close()


fig,ax = plt.subplots()
ax.set_title(f"{N_BEST_FIT} best fitness values over evolutions")
ax.plot(np.arange(len(best_fitnesses)), best_fitnesses)
ax.set_xlabel("Epochs")
ax.set_ylabel("Fitness")

fig.savefig(os.path.join(PATH, f"5best_fit.png"))


fig,ax = plt.subplots()
ax.set_title(f"{N_BEST_FIT} best no_sol values over evolutions")
ax.plot(np.arange(len(best_nosol)), best_nosol)
ax.set_xlabel("Epochs")
ax.set_ylabel("No-Solution Days")
fig.savefig(os.path.join(PATH, f"5best_no_sol.png"))

fig,ax = plt.subplots()
ax.set_title(f"Diversity over evolutions")
ax.plot(np.arange(len(best_nosol)), diversity_epochs)
ax.set_ylabel("Mean Euclidian Diversity")
ax.set_xlabel("Epochs")
fig.savefig(os.path.join(PATH, f"diversity.png"))
