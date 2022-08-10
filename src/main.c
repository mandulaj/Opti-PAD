#include "gene.h"
#include "printing.h"
#include "problem.h"
#include <stdint.h>
#include <stdio.h>

const char *reverse_lookup_standard_local[] = {
    "",    "",   "",   "",    "",    "",    "",    "",    "",    "",    "",
    "",    "",   "31", "30",  "29",  "",    "28",  "27",  "26",  "25",  "24",
    "23",  "22", "",   "21",  "20",  "19",  "18",  "17",  "16",  "15",  "",
    "14",  "13", "12", "11",  "10",  "9",   "8",   "",    "7",   "6",   "5",
    "4",   "3",  "2",  "1",   "",    "",    "Dec", "Nov", "Oct", "Sep", "Aug",
    "Jul", "",   "",   "Jun", "May", "Apr", "Mar", "Feb", "Jan"};

struct game_individual {
  uint64_t n_pieces;
  // uint64_t *gene_sizes;
  gene_t *genes;
};

void make_game_individual(struct game_individual *gi, uint64_t *sizes,
                          uint64_t n_pieces) {
  gi->n_pieces = n_pieces;
  // gi->gene_sizes = malloc(sizeof(*gi->gene_sizes) * n_pieces);
  gi->genes = malloc(sizeof(*gi->genes) * n_pieces);

  for (int i = 0; i < n_pieces; i++) {
    // gi->gene_sizes[i]
    gi->genes[i] = gene_random(sizes[i]);
  }
}

void destroy_game_individual(struct game_individual *gi) { free(gi->genes); }

void game_individual_to_problem(problem_t *p, struct game_individual *gi,
                                board_t blank,
                                struct solution_restrictions restrictions) {
  p->blank = blank;
  p->problem = blank;
  p->n_pieces = gi->n_pieces;
  p->reverse_lookup = reverse_lookup_standard_local;

  for (int i = 0; i < p->n_pieces; i++) {
    p->pieces[i] = gene_to_piece(gi->genes[i]);
    p->piece_props[i] = get_piece_properties(p->pieces[i]);
    p->piece_position_num[i] = make_positions(p->pieces[i], p->piece_props[i],
                                              blank, NULL, restrictions);
  }
}

int test_all_dates(problem_t *prob, struct solution_restrictions restrictions) {
  solutions_t sol;
  uint64_t total_solutions = 0;
  uint64_t no_solutions = 0;

  for (int day = 1; day <= 31; day++) {
    for (int month = 1; month <= 12; month++) {

      // make_from_date(prob, day, month);

      prob->problem = prob->blank | ((board_t)0x01 << month_location(month)) |
                      ((board_t)0x01 << day_location(day));

      init_solutions(&sol, prob, restrictions);

      solve(&sol);

      total_solutions += sol.num_solutions;
      if (sol.num_solutions == 0) {
        no_solutions++;
      }

      destroy_solutions(&sol);
    }
  }
  printf("Total Solutions: %ld, No Solutions: %ld/%d\n", total_solutions,
         no_solutions, 31 * 12);
  return no_solutions;
}

int main(int argc, char **argv) {

  //   const int population_size = 100;

  //   board_t STANDARD_BLANK =
  //       0b0000001100000011000000010000000100000001000000010001111111111111;

  //   uint64_t game_size = 8;
  //   uint64_t game_piece_sizes[] = {5, 5, 5, 5, 5, 5, 5, 6};

  //   struct solution_restrictions restrictions = {.use_faceup = true,
  //                                                .use_facedown = false};

  //   struct game_individual *population =
  //       malloc(sizeof(struct game_individual) * population_size);
  //   printf("Making Gene Pool\n");
  //   for (int i = 0; i < population_size; i++) {
  //     make_game_individual(population + i, game_piece_sizes, game_size);
  //   }

  //   printf("Making & testing Problems\n");
  // #pragma omp parallel for schedule(dynamic)
  //   for (int i = 0; i < population_size; i++) {
  //     problem_t problem;
  //     game_individual_to_problem(&problem, &population[i], STANDARD_BLANK,
  //                                restrictions);
  //     // print_problem(&problem);
  //     test_all_dates(&problem, restrictions);
  //   }

  //   printf("Done\n");

  //   // Clean up
  //   for (int i = 0; i < population_size; i++) {
  //     destroy_game_individual(&population[i]);
  //   }
  //   free(population);
  //   // for (int i = 0; i < population[population_size - 1].n_pieces; i++) {
  //   //   print_gene(population[population_size - 1].genes[i]);
  //   //   printf("\n");
  //   //   print_piece(gene_to_piece(population[population_size - 1].genes[i]),
  //   i);
  //   // }

  //   return 0;

  // gene_t genes[8];

  int seed = 0;
  if (argc == 2) {
    sscanf(argv[1], "%d", &seed);

    printf("%d seed\n", seed);
    srand(seed);

    int lengths[] = {5, 5, 5, 5, 5, 5, 5, 6};
    gene_t genes[8];
    for (int i = 0; i < 8; i++) {
      gene_t g = gene_random(lengths[i]);
      genes[i] = g;
      // print_gene(g);
      // printf("\n");
      // piece_t new = gene_to_piece(g);
      // print_piece(new, i);
    }

    printf("Original:\n");
    print_gene(genes[0]);
    printf("\n");
    print_piece(gene_to_piece(genes[0]), 0);
    print_gene(genes[1]);
    printf("\n");
    print_piece(gene_to_piece(genes[1]), 1);

    gene_t mix = gene_cross(genes[0], genes[1]);
    print_gene(mix);
    printf("\n");
    print_piece(gene_to_piece(mix), 2);

    // for (int i = 10; i >= 0; i--) {
    //   gene_t g = gene_mutate(genes[0], i);
    //   print_gene(g);
    //   printf("\n");
    //   print_piece(gene_to_piece(g), i);
    // }
  } else if (argc == 7) {
    int length;
    uint32_t positions[4];
    uint32_t flip;

    sscanf(argv[1], "%d", &length);
    sscanf(argv[2], "%d", &positions[0]);
    sscanf(argv[3], "%d", &positions[1]);
    sscanf(argv[4], "%d", &positions[2]);
    sscanf(argv[5], "%d", &positions[3]);
    sscanf(argv[6], "%d", &flip);

    gene_t g = gene_make(positions, length, !!flip);

    printf("Gene:");
    print_gene(g);
    printf("Length: %d\n", length);

    piece_t new = gene_to_piece(g);

    print_piece(new, 0);
  } else {
  }
  //   for (int i = 0; i < 4; i++) {
  //     printf("Problem %d\n", i);

  //     for (int j = 0; j < problem_types[i].n_pieces; j++) {
  //       piece_t piece = problem_types[i].pieces[j];

  //       piece_properties_t computed_ppts = get_piece_properties(piece);
  //       piece_properties_t true_ppts = problem_types[i].piece_props[j];
  //       print_piece(piece, j);
  //       printf("True rotations: %ld, true asymetry: %d || Comp rot: %ld,
  //       comp
  //       "
  //              "asym: %d\n",
  //              true_ppts.rotations, true_ppts.asymetric,
  //              computed_ppts.rotations, computed_ppts.asymetric);
  //       if ((true_ppts.rotations != computed_ppts.rotations) ||
  //           (true_ppts.asymetric != computed_ppts.asymetric)) {
  //         printf("NO MATCH!!!!!!!\n");
  //       }
  //     }
  //   }
}