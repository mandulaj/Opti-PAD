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

// int eval(int argc, char **argv) {

//   const int population_size = 100;

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
//     // test_all_dates(&problem, restrictions);
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
// }

typedef uint32_t hash_t;

hash_t murmur64(uint64_t h) {
  h ^= h >> 33;
  h *= 0xff51afd7ed558ccdL;
  h ^= h >> 33;
  h *= 0xc4ceb9fe1a85ec53L;
  return h;
}

float hue2rgb(float p, float q, float t) {

  if (t < 0)
    t += 1;
  if (t > 1)
    t -= 1;
  if (t < 1. / 6)
    return p + (q - p) * 6 * t;
  if (t < 1. / 2)
    return q;
  if (t < 2. / 3)
    return p + (q - p) * (2. / 3 - t) * 6;

  return p;
}

////////////////////////////////////////////////////////////////////////

/*
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns RGB in the set [0, 255].
 */
void hsl2rgb(float h, float s, float l, int *r, int *g, int *b) {

  if (0 == s) {
    *r = *g = *b = l; // achromatic
  } else {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    *r = hue2rgb(p, q, h + 1. / 3) * 255;
    *g = hue2rgb(p, q, h) * 255;
    *b = hue2rgb(p, q, h - 1. / 3) * 255;
  }

  return;
}

/* Test main */
#ifdef TEST_MAIN
#include <stdio.h>
int main(void) {
  printf("e9c6d914c4b8d9ca == %016llx\n",
         (unsigned long long)crc64(0, (unsigned char *)"123456789", 9));
  return 0;
}
#endif

hash_t getSmallestHash(piece_t p) {
  p = piece_origin(p);
  hash_t hash = murmur64((uint64_t)p);
  int i = 4;
  do {
    p = piece_origin(piece_rotate(p));
    hash_t h = murmur64((uint64_t)p);
    // printf("%d Smallest: %x, current: %x piece: %lx\n", i, hash, h, p);
    if (h < hash) {
      hash = h;
    }
  } while (--i);
  return hash;
}

piece_t get_piece_smallest_hash(piece_t p) {
  p = piece_origin(p);
  piece_t smallest = p;
  hash_t hash = murmur64((uint64_t)p);
  int i = 4;

  do {
    p = piece_origin(piece_rotate(p));
    hash_t h = murmur64((uint64_t)p);
    if (h < hash) {
      hash = h;
      smallest = p;
    }
  } while (--i);
  return smallest;
}

int compare_hashes(const void *a, const void *b) {
  hash_t int_a = *((hash_t *)a);
  hash_t int_b = *((hash_t *)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

void genome_to_hashes(gene_t *genes, hash_t *hashes, size_t n_population,
                      size_t n_pieces) {
  for (int i = 0; i < n_population; i++) {
    for (int j = 0; j < n_pieces; j++) {
      hashes[i * n_pieces + j] =
          getSmallestHash(gene_to_piece(genes[i * n_pieces + j]));
    }
    // qsort(&hashes[i * n_pieces], n_pieces, sizeof(hash_t), compare_hashes);
  }
}

void print_genome_color(gene_t *genes, size_t n_population, size_t n_pieces) {
  hash_t *hashes = malloc(sizeof(hash_t) * n_population * n_pieces);
  genome_to_hashes(genes, hashes, n_population, n_pieces);

  for (int i = 0; i < n_population; i++) {

    hash_t *hs = &hashes[i * n_pieces];
    int r = 0, g = 0, b = 0;
    int r_total = 0, g_total = 0, b_total = 0;
    for (int j = 0; j < n_pieces; j++) {
      hsl2rgb((hs[j]) / (float)0xffffffff, 1, 0.6, &r, &g, &b);
      r_total += r;
      b_total += b;
      g_total += g;
    }
    r = r_total / n_pieces;
    g = g_total / n_pieces;
    b = b_total / n_pieces;

    // for (int j = 0; j < n_genes; j++) {
    //   r += hs[j] & 0xF0;
    //   g += (hs[j] >> 8) & 0xF0;
    //   b += (hs[j] >> 16) & 0xF0;
    // }

    for (int j = 0; j < n_pieces; j++) {

      char buffer[64];

      // hsl2rgb((hs[j]) / (float)0xffffffff, 1, 0.6, &r, &g, &b);

      hash_t h = hashes[i * n_pieces + j];
      snprintf(buffer, 64, "%08x ", h);

      print_rgb(buffer, r, g, b);
      // print_color(buffer, h % 16);
    }
    printf("\n");
  }
  free(hashes);
}

void print_gene_pieces_horizontal(gene_t *genes, uint32_t *colors,
                                  size_t n_pieces) {
  piece_t *pieces = malloc(sizeof(piece_t) * n_pieces);
  for (int i = 0; i < n_pieces; i++) {
    pieces[i] = gene_to_piece(genes[i]);
  }

  char *buffer = malloc(sizeof(char) * n_pieces * 8 * 20 * n_pieces);
  for (int i = 0; i < 8; i++) {
    char *b = buffer;
    for (int p = 0; p < n_pieces; p++) {
      b += get_piece_line_rgb(pieces[p], colors[p], i, b);
      b[0] = ' ';
      b++;
    }
    printf("%s\n", buffer);
  }

  free(buffer);
  free(pieces);
}

void print_bin(uint32_t b, size_t size) {
  uint32_t mask = 0x01 << (size - 1);
  for (int i = 0; i < size; i++) {
    if (mask & b)
      printf("1");
    else
      printf("0");

    mask >>= 1;
  }
}

piece_t pieces[1000];
hash_t hashes[1000];
size_t num = 0;

int main(int argc, char **argv) {

  gene_t start = 0b0100000000000000;
  piece_t p = gene_to_piece(start);
  hash_t h = getSmallestHash(p);
  p = get_piece_smallest_hash(p);
  hashes[0] = h;
  pieces[0] = p;
  num++;

  for (gene_t g = start + 1; g < 0xFFFF; g++) {
    p = gene_to_piece(g);
    h = getSmallestHash(p);
    p = get_piece_smallest_hash(p);

    bool found = false;
    for (int i = 0; i < num; i++) {
      if (hashes[i] == h) {
        found = true;
        break;
      }
    }

    if (!found) {
      hashes[num] = h;
      pieces[num] = p;
      num++;
    }
  }

  printf("Num: %ld\n", num);

  // FILE *fp4 = fopen("4-piece.txt", "w");
  // FILE *fp5 = fopen("5-piece.txt", "w");
  // FILE *fp6 = fopen("6-piece.txt", "w");

  printf("4 Piece\n");
  for (int i = 0; i < 7; i++) {
    // print_piece(pieces[i], i % 10);
    printf("0x%lx,", pieces[i]);
  }

  printf("\n5 Piece\n");
  for (int i = 7; i < 25; i++) {
    // print_piece(pieces[i], i % 10);
    printf("0x%lx,", pieces[i]);
  }

  printf("\n6 Piece\n");
  for (int i = 25; i < 85; i++) {
    // print_piece(pieces[i], i % 10);
    printf("0x%lx,", pieces[i]);
  }

  return 0;

  problem_t prob = {
      .blank =
          0b0000001100000011000000010000000100000001000000010001111111111111,
      .reverse_lookup = reverse_lookup_standard_local,
      .n_pieces = 8,
      .piece_position_num = {198, 154, 196, 96, 151, 80, 48, 150},
      .pieces = {0xe0e0000000000000, 0xe020200000000000, 0xa0e0000000000000,
                 0xc0f0000000000000, 0x10f0000000000000, 0x40f0000000000000,
                 0xc0c0800000000000, 0x80e0000000000000},
      .piece_props = {{4, true},
                      {4, true},
                      {4, true},
                      {4, false},
                      {4, true},
                      {4, false},
                      {2, false},
                      {4, true}}};

  for (int i = 0; i < 8; i++) {
    prob.piece_props[i] = get_piece_properties(prob.pieces[i]);

    prob.piece_position_num[i] =
        make_positions(prob.pieces[i], prob.piece_props[i], prob.blank, NULL,
                       (struct solution_restrictions){true, true});
    printf("%ld, ", prob.piece_position_num[i]);
  }

  printf("\n");

  for (int i = 0; i < 8; i++) {
    print_piece(prob.pieces[i], i);
  }

  printf("\n");
  for (int i = 0; i < 8; i++) {
    printf("{%ld, %s}, ", prob.piece_props[i].rotations,
           prob.piece_props[i].asymetric ? "true" : "false");
  }

  return 0;

  uint32_t scores[7];
  struct solution_restrictions r = {true, true};

  test_all_dates(&prob, scores, r);

  printf("Total: %d No Solution: %d/%d\n", scores[0], scores[1], scores[2]);

  solutions_t sol;

  prob.problem = prob.blank | ((board_t)0x01 << month_location(8)) |
                 ((board_t)0x01 << day_location(26));

  init_solutions(&sol, &prob, r);

  for (int i = 0; i < 8; i++) {
    printf("Sol id: %ld\n", sol.sorted_sol_indexes[i]);
  }

  return 0;
  solve(&sol);

  printf("Found %ld solutions:\n", sol.num_solutions);

  for (int i = 0; i < sol.num_solutions; i++) {
    print_solution(&sol.solutions[i], &prob);
  }

  destroy_solutions(&sol);

  int N = 0x1 << (2 + 3);
  int n_shapes = 0;
  piece_t shapes[N];
  int shape_counts[N];
  for (int i = 0; i < N; i++) {
    shape_counts[i] = 0;
  }

  // gene_t g = 0x81f7;
  // print_piece(gene_to_piece(g), 0);
  // printf("%x\n", getSmallestHash(gene_to_piece(g)));
  // print_piece(get_piece_smallest_hash(gene_to_piece(g)), 0);

  // g = 0x81fe;
  // print_piece(gene_to_piece(g), 1);
  // printf("%x\n", getSmallestHash(gene_to_piece(g)));
  // print_piece(get_piece_smallest_hash(gene_to_piece(g)), 1);

  // return;
  // // for (gene_t g = 0; g < 32; g++) {
  //   gene_t gg = g | 0x4000;
  //   printf("%04x\n", gg);
  //   print_bin(gg >> 5, 4);
  //   printf(" ");
  //   print_bin(gg >> 2, 3);
  //   printf(" ");
  //   print_bin(gg, 2);
  //   printf("\n");
  //   print_piece(get_piece_smallest_hash(gene_to_piece(gg)), g % 10);
  // }

  for (gene_t g = 0; g < N; g++) {
    gene_t gg = g | 0x4000;
    // gene_t gg = g | 0xC000;
    // gene_t gg = g | 0x8000;
    // printf("%04x\n", gg);
    // print_bin(gg >> 5, 4);
    // printf(" ");
    // print_bin(gg >> 2, 3);
    // printf(" ");
    // print_bin(gg, 2);
    // printf("\n");
    // print_piece(get_piece_smallest_hash(gene_to_piece(gg)), g % 10);
    piece_t smallest = get_piece_smallest_hash(gene_to_piece(gg));
    int index = 0;
    for (int i = 0; i < n_shapes; i++) {
      if (smallest == shapes[i])
        break;
      index++;
    }
    if (index >= n_shapes) {
      shapes[index] = smallest;
      n_shapes++;
    }

    shape_counts[index]++;
  }

  int total = 0;
  for (int i = 0; i < n_shapes; i++) {
    printf("%d\n", shape_counts[i]);
    print_piece(shapes[i], i % 10);
    total += shape_counts[i];
  }
  printf("N-shapes: %d, Total: %d\n", n_shapes, total);

  return 0;
  const int pop = 50;
  const int n_genes = 8;
  gene_t genes[pop * n_genes];

  for (int i = 0; i < pop * n_genes; i++) {
    genes[i] = gene_random(i % 3 + 4);
    piece_t p = gene_to_piece(genes[i]);
    hash_t h = getSmallestHash(p);
    printf("%x\n", h);
    print_piece(p, h);
  }

  hash_t hashes[pop * n_genes];
  genome_to_hashes(genes, hashes, pop, n_genes);

  for (int i = 0; i < pop; i++) {

    hash_t *hs = &hashes[i * n_genes];
    int r = 0, g = 0, b = 0;
    // for (int j = 0; j < n_genes; j++) {
    //   r += hs[j] & 0xF0;
    //   g += (hs[j] >> 8) & 0xF0;
    //   b += (hs[j] >> 16) & 0xF0;
    // }

    for (int j = 0; j < n_genes; j++) {

      char buffer[64];

      hsl2rgb((hs[j]) / (float)0xffffffff, 1, 0.6, &r, &g, &b);

      hash_t h = hashes[i * n_genes + j];
      snprintf(buffer, 64, "%08x ", h);

      print_rgb(buffer, r, g, b);
      // print_color(buffer, h % 16);
    }
    printf("\n");
  }

  // for (int i = 0; i < sizeof(pieces) / sizeof(pieces[0]); i++) {
  //   piece_t p = pieces[i];
  //   for (int j = 0; j < 4; j++) {
  //     p = piece_sft_down(piece_rotate(p));
  //     print_piece(p, i % 10);

  //     printf("HASH: %lx\n", getSmallestHash(p));
  //   }
  // }
  // for (int i = 0; i < sizeof(pieces) / sizeof(pieces[0]); i++) {
  //   piece_t p = pieces[i];
  // }
}

void getPieceHashList() {}
