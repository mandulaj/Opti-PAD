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

board_t STANDARD_BLANK =
    0b0000001100000011000000010000000100000001000000010001111111111111;

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

int test_all_dates(problem_t *prob, uint32_t *scores,
                   struct solution_restrictions restrictions) {
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

  scores[0] = total_solutions;
  scores[1] = no_solutions;
  scores[2] = 31 * 12;
  return total_solutions;
}

int test_pass_indiv(struct game_individual *gi) {
  printf("n_genes: %ld\n", gi->n_pieces);
  uint64_t game_piece_sizes[] = {5, 5, 5, 5, 5, 5, 5, 6};

  for (int i = 0; i < gi->n_pieces; i++) {
    print_gene(gi->genes[i]);
    printf("\n");
    gi->genes[i] = gene_random(game_piece_sizes[i]);
  }
  return 0;
}

// int eval_individual(struct game_individual *gi,
//                     struct solution_restrictions restrictions) {
//   problem_t problem;
//   game_individual_to_problem(&problem, gi, STANDARD_BLANK, restrictions);
//   printf("Made problem\n");
//   return test_all_dates(&problem, restrictions);
// }

int eval_numpy_genes(gene_t *genes, uint32_t *scores, size_t n_population,
                     size_t n_pieces, size_t n_scores,
                     struct solution_restrictions restrictions) {
  // struct game_individual *gi =
  //     malloc(sizeof(struct game_individual) * n_population);
  // if (gi == NULL) {
  //   return 1;
  // }

#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < n_population; i++) {
    problem_t problem;
    struct game_individual gi;
    gi.n_pieces = n_pieces;
    gi.genes = &genes[i * n_pieces];
    game_individual_to_problem(&problem, &gi, STANDARD_BLANK, restrictions);

    test_all_dates(&problem, &scores[i * n_scores], restrictions);

    // printf("%d, Total Solutions: %d, No Solutions: %d/%d\n", i,
    //        scores[i * n_scores + 0], scores[i * n_scores + 1],
    //        scores[i * n_scores + 2]);
  }

  return 0;
}

void eval_numpy_individual(gene_t *genes, uint32_t *scores, size_t n_pieces,
                           size_t n_scores,
                           struct solution_restrictions restrictions) {
  problem_t problem;
  struct game_individual gi;
  gi.n_pieces = n_pieces;
  gi.genes = genes;
  game_individual_to_problem(&problem, &gi, STANDARD_BLANK, restrictions);

  test_all_dates(&problem, scores, restrictions);
}

uint32_t unique_pieces(gene_t *genes, size_t n_pieces) {
  uint32_t n_unique = 0;
  piece_t *pieces = malloc(sizeof(piece_t) * n_pieces);

  for (int i = 0; i < n_pieces; i++) {
    bool unique = true;
    piece_t current_piece = gene_to_piece(genes[i]);
    for (int j = 0; j < n_unique; j++) {
      if (same_piece(pieces[j], current_piece)) {
        unique = false;
        break;
      }
    }

    if (unique) {
      pieces[n_unique++] = current_piece;
    }
  }

  free(pieces);
  return n_unique;
}

void print_raw_pieces(gene_t *genes, size_t n_pieces) {
  for (int j = 0; j < n_pieces; j++) {
    printf("%lx\n", gene_to_piece(genes[j]));
  }
}

void print_genes(gene_t *genes, size_t n_pieces) {

  for (int j = 0; j < n_pieces; j++) {
    print_piece(gene_to_piece(genes[j]), j);
  }
}

int eval(int argc, char **argv) {

  const int population_size = 100;

  uint64_t game_size = 8;
  uint64_t game_piece_sizes[] = {5, 5, 5, 5, 5, 5, 5, 6};

  struct solution_restrictions restrictions = {.use_faceup = true,
                                               .use_facedown = false};

  struct game_individual *population =
      malloc(sizeof(struct game_individual) * population_size);
  printf("Making Gene Pool\n");
  for (int i = 0; i < population_size; i++) {
    make_game_individual(population + i, game_piece_sizes, game_size);
  }

  printf("Making & testing Problems\n");
#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < population_size; i++) {
    problem_t problem;
    game_individual_to_problem(&problem, &population[i], STANDARD_BLANK,
                               restrictions);
    // print_problem(&problem);
    // test_all_dates(&problem, restrictions);
  }

  printf("Done\n");

  // Clean up
  for (int i = 0; i < population_size; i++) {
    destroy_game_individual(&population[i]);
  }
  free(population);
  // for (int i = 0; i < population[population_size - 1].n_pieces; i++) {
  //   print_gene(population[population_size - 1].genes[i]);
  //   printf("\n");
  //   print_piece(gene_to_piece(population[population_size - 1].genes[i]), i);
  // }

  return 0;
}