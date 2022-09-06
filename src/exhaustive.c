#include "gene.h"
#include "printing.h"
#include "problem.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define N_PIECES_4 7
#define N_PIECES_5 18
#define N_PIECES_6 60

piece_t pieces_4[N_PIECES_4] = {0x8080808000000000, 0x40c0800000000000,
                                0x8080c00000000000, 0xe020000000000000,
                                0xc0c0000000000000, 0x40e0000000000000,
                                0x80c0400000000000};

piece_t pieces_5[N_PIECES_5] = {
    0x8080808080000000, 0x40c0808000000000, 0xf080000000000000,
    0x20e0800000000000, 0xe060000000000000, 0x8080c08000000000,
    0x2020e00000000000, 0x70c0000000000000, 0x60c0800000000000,
    0x80f0000000000000, 0x20e0400000000000, 0x20e0200000000000,
    0x40f0000000000000, 0x80c0c00000000000, 0x6040c00000000000,
    0x60c0400000000000, 0xa0e0000000000000, 0x40e0400000000000};

piece_t pieces_6[N_PIECES_6] = {
    0x8080808080800000, 0xf018000000000000, 0xf880000000000000,
    0x4040c08080000000, 0x2020e08000000000, 0x4040c0c000000000,
    0xf840000000000000, 0xf080800000000000, 0x78c0000000000000,
    0x202060c000000000, 0xc080808080000000, 0x40c0700000000000,
    0x10f0100000000000, 0x80c0808080000000, 0x10f0200000000000,
    0xf820000000000000, 0xc060300000000000, 0xf0c0000000000000,
    0xc040406000000000, 0x20e0c00000000000, 0xe070000000000000,
    0x40c0e00000000000, 0x40c0c04000000000, 0xc040602000000000,
    0x80e0300000000000, 0x20f0800000000000, 0xe060200000000000,
    0x4060c08000000000, 0x20e0404000000000, 0x20e0a00000000000,
    0xf040400000000000, 0xc080e00000000000, 0x2030e00000000000,
    0x60c0404000000000, 0x4040f00000000000, 0x40f0400000000000,
    0x20f0400000000000, 0xf010100000000000, 0xc060202000000000,
    0x80e0404000000000, 0x30e0400000000000, 0x8080e02000000000,
    0x38e0000000000000, 0xc080c08000000000, 0xc0e0200000000000,
    0x80c0c04000000000, 0x80c0602000000000, 0x604040c000000000,
    0xe0c0400000000000, 0x40f0100000000000, 0x40e0c00000000000,
    0xe0e0000000000000, 0xf0a0000000000000, 0x70d0000000000000,
    0xe040c00000000000, 0xe0a0200000000000, 0x4060c04000000000,
    0xf090000000000000, 0x40c080c000000000, 0x60c0600000000000};

int generate_problem(problem_t *p, piece_t *pieces, size_t n) {
  if (n > 11 || n <= 0) {
    printf("Invalid problem\n");
    return 1;
  }
  p->blank = 0b0000001100000011000000010000000100000001000000010001111111111111;
  p->n_pieces = n;

  size_t total = piece_order(p->blank) + 2;

  for (int i = 0; i < n; i++) {
    p->pieces[i] = pieces[i];
    p->piece_props[i] = get_piece_properties(pieces[i]);
    p->piece_position_num[i] =
        make_positions(pieces[i], p->piece_props[i], p->blank, NULL,
                       (struct solution_restrictions){true, true});

    total += piece_order(pieces[i]);
  }

  if (total != 64) {
    printf("Invlaid problem size");
    return 1;
  } else {
    return 0;
  }
}

struct puzzle_stats {
  uint64_t total_solutions;
  uint64_t max_total_solutions;
  uint64_t min_total_solutions;
  uint64_t total_days_solutions;
  uint64_t total_no_solutions;

  uint64_t faceup_solutions;
  uint64_t max_faceup_solutions;
  uint64_t min_faceup_solutions;
  uint64_t faceup_days_solutions;
  uint64_t faceup_no_solutions;

  uint64_t facedown_solutions;
  uint64_t max_facedown_solutions;
  uint64_t min_facedown_solutions;
  uint64_t facedown_days_solutions;
  uint64_t facedown_no_solutions;
};

int test_all_dates(problem_t *prob, struct puzzle_stats *ps) {

  const struct solution_restrictions restrictions = {true, true};
  const struct solution_restrictions restrictions_faceup = {true, false};
  const struct solution_restrictions restrictions_facedown = {false, true};

  solutions_t sol_all;
  solutions_t sol_faceup;
  solutions_t sol_facedown;

  // Reset statistics
  memset(ps, 0x00, sizeof(struct puzzle_stats));

  ps->min_total_solutions = UINT64_MAX;
  ps->min_facedown_solutions = UINT64_MAX;
  ps->min_faceup_solutions = UINT64_MAX;

  for (int day = 1; day <= 31; day++) {
    for (int month = 1; month <= 12; month++) {

      // make_from_date(prob, day, month);

      prob->problem = prob->blank | ((board_t)0x01 << month_location(month)) |
                      ((board_t)0x01 << day_location(day));

      init_solutions(&sol_all, prob, restrictions);
      init_solutions(&sol_faceup, prob, restrictions_faceup);
      init_solutions(&sol_facedown, prob, restrictions_facedown);

      solve(&sol_all);
      solve(&sol_faceup);
      solve(&sol_facedown);

      // All solutions
      ps->total_solutions += sol_all.num_solutions;
      if (sol_all.num_solutions == 0) {
        ps->total_no_solutions++;
      } else {
        ps->total_days_solutions++;
      }

      if (sol_all.num_solutions > ps->max_total_solutions) {
        ps->max_total_solutions = sol_all.num_solutions;
      }

      if (sol_all.num_solutions < ps->min_total_solutions) {
        ps->min_total_solutions = sol_all.num_solutions;
      }

      // Faceup solutions
      ps->faceup_solutions += sol_faceup.num_solutions;
      if (sol_faceup.num_solutions == 0) {
        ps->faceup_no_solutions++;
      } else {
        ps->faceup_days_solutions++;
      }

      if (sol_faceup.num_solutions > ps->max_faceup_solutions) {
        ps->max_faceup_solutions = sol_faceup.num_solutions;
      }

      if (sol_faceup.num_solutions < ps->min_faceup_solutions) {
        ps->min_faceup_solutions = sol_faceup.num_solutions;
      }

      // Facedown solutions
      ps->facedown_solutions += sol_facedown.num_solutions;
      if (sol_facedown.num_solutions == 0) {
        ps->facedown_no_solutions++;
      } else {
        ps->facedown_days_solutions++;
      }

      if (sol_facedown.num_solutions > ps->max_facedown_solutions) {
        ps->max_facedown_solutions = sol_facedown.num_solutions;
      }

      if (sol_facedown.num_solutions < ps->min_facedown_solutions) {
        ps->min_facedown_solutions = sol_facedown.num_solutions;
      }

      destroy_solutions(&sol_all);
      destroy_solutions(&sol_faceup);
      destroy_solutions(&sol_facedown);
    }
  }

  return 0;
}

void check_pieces(size_t idx[], size_t n_pieces, struct puzzle_stats *ps) {
  piece_t selected[] = {pieces_5[idx[0]], pieces_5[idx[1]], pieces_5[idx[2]],
                        pieces_5[idx[3]], pieces_5[idx[4]], pieces_5[idx[5]],
                        pieces_5[idx[6]], pieces_6[idx[7]]};

  problem_t problem;
  size_t res = 0;
  res = generate_problem(&problem, selected, n_pieces);
  if (!res) {
    test_all_dates(&problem, ps);

    // printf("%d,%d,%d,%d,%d,%d,%d,%d, %ld, %ld, %ld, %ld, %ld, "
    //        "%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld\n",
    //        idx[0], idx[1], idx[2], idx[3], idx[4], idx[5], idx[6], idx[7],
    //        ps.total_days_solutions, ps.total_no_solutions,
    //        ps.max_total_solutions, ps.min_total_solutions,
    //        ps.total_solutions,

    //        ps.faceup_days_solutions, ps.faceup_no_solutions,
    //        ps.max_faceup_solutions, ps.min_faceup_solutions,
    //        ps.faceup_solutions,

    //        ps.facedown_days_solutions, ps.facedown_no_solutions,
    //        ps.max_facedown_solutions, ps.min_facedown_solutions,
    //        ps.facedown_solutions);
  } else {
    printf("ERROR");
    exit(1);
    // printf("%d,%d,%d,%d,%d,%d,%d,%d - %ld\n", p1, p2, p3, p4,
    //  p5, p6, p7, p8, problem.piece_position_num[0]);
  }
}

piece_t get_nth_piece(size_t piece_id, size_t piece_size) {
  piece_t p = 0;
  switch (piece_size) {
  case 4:
    p = pieces_4[piece_id];
    break;
  case 5:
    p = pieces_5[piece_id];
    break;
  case 6:
  default:
    p = pieces_6[piece_id];
    break;
  }
  return p;
}

void print_nth_piece(size_t piece_id, size_t piece_size, size_t color) {
  piece_t p = get_nth_piece(piece_id, piece_size);
  print_piece(p, color);
}

int main(int argc, char **argv) {

  printf("p1,p2,p3,p4,p5,p6,p7,p8,total_days,total_no_sol,total_max,total_min,"
         "total,faceup_days,faceup_no_sol,faceup_max,faceup_min,faceup,"
         "facedown_days,facedown_no_sol,facedown_max,facedown_min,facedown\n");
  // #pragma omp parallel for collapse(7)
  for (int p1 = 0; p1 < N_PIECES_5 - 6; p1++) {
    for (int p2 = p1 + 1; p2 < N_PIECES_5 - 5; p2++) {
      for (int p3 = p2 + 1; p3 < N_PIECES_5 - 4; p3++) {
        for (int p4 = p3 + 1; p4 < N_PIECES_5 - 3; p4++) {
          for (int p5 = p4 + 1; p5 < N_PIECES_5 - 2; p5++) {
            for (int p6 = p5 + 1; p6 < N_PIECES_5 - 1; p6++) {
              for (int p7 = p6 + 1; p7 < N_PIECES_5; p7++) {
                for (int p8 = 0; p8 < N_PIECES_6; p8++) {
                  piece_t selected[] = {
                      pieces_5[p1], pieces_5[p2], pieces_5[p3], pieces_5[p4],
                      pieces_5[p5], pieces_5[p6], pieces_5[p7], pieces_6[p8]};
                  const size_t n_pieces = 8;

                  struct puzzle_stats ps;
                  problem_t problem;
                  size_t res = 0;
                  res = generate_problem(&problem, selected, n_pieces);
                  if (!res) {
                    test_all_dates(&problem, &ps);

                    printf("%d,%d,%d,%d,%d,%d,%d,%d, %ld, %ld, %ld, %ld, %ld, "
                           "%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld\n",
                           p1, p2, p3, p4, p5, p6, p7, p8,
                           ps.total_days_solutions, ps.total_no_solutions,
                           ps.max_total_solutions, ps.min_total_solutions,
                           ps.total_solutions,

                           ps.faceup_days_solutions, ps.faceup_no_solutions,
                           ps.max_faceup_solutions, ps.min_faceup_solutions,
                           ps.faceup_solutions,

                           ps.facedown_days_solutions, ps.facedown_no_solutions,
                           ps.max_facedown_solutions, ps.min_facedown_solutions,
                           ps.facedown_solutions);
                  } else {
                    printf("ERROR");
                    exit(1);
                    // printf("%d,%d,%d,%d,%d,%d,%d,%d - %ld\n", p1, p2, p3, p4,
                    //  p5, p6, p7, p8, problem.piece_position_num[0]);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // problem_t prob;

  // piece_t pieces[8]

  //     for (int i = 0; i < 8; i++) {
  //   prob.piece_props[i] = get_piece_properties(prob.pieces[i]);

  //   prob.piece_position_num[i] =
  //       make_positions(prob.pieces[i], prob.piece_props[i], prob.blank, NULL,
  //                      (struct solution_restrictions){true, true});
  //   printf("%d, ", prob.piece_position_num[i]);
  // }

  // printf("\n");

  // for (int i = 0; i < 8; i++) {
  //   print_piece(prob.pieces[i], i);
  // }

  // printf("\n");
  // for (int i = 0; i < 8; i++) {
  //   printf("{%d, %s}, ", prob.piece_props[i].rotations,
  //          prob.piece_props[i].asymetric ? "true" : "false");
  // }

  // return;

  // uint32_t scores[7];
  // struct solution_restrictions r = {true, true};

  // test_all_dates(&prob, scores, r);

  // printf("Total: %d No Solution: %d/%d\n", scores[0], scores[1], scores[2]);

  // solutions_t sol;

  // prob.problem = prob.blank | ((board_t)0x01 << month_location(8)) |
  //                ((board_t)0x01 << day_location(26));

  // init_solutions(&sol, &prob, r);

  // for (int i = 0; i < 8; i++) {
  //   printf("Sol id: %d\n", sol.sorted_sol_indexes[i]);
  // }

  // return 0;
  // solve(&sol);

  // printf("Found %ld solutions:\n", sol.num_solutions);

  // for (int i = 0; i < sol.num_solutions; i++) {
  //   print_solution(&sol.solutions[i], &prob);
  // }

  // destroy_solutions(&sol);

  // int N = 0x1 << (2 + 3);
  // int n_shapes = 0;
  // piece_t shapes[N];
  // int shape_counts[N];
  // for (int i = 0; i < N; i++) {
  //   shape_counts[i] = 0;
  // }

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

  // for (gene_t g = 0; g < N; g++) {
  //   gene_t gg = g | 0x4000;
  //   // gene_t gg = g | 0xC000;
  //   // gene_t gg = g | 0x8000;
  //   // printf("%04x\n", gg);
  //   // print_bin(gg >> 5, 4);
  //   // printf(" ");
  //   // print_bin(gg >> 2, 3);
  //   // printf(" ");
  //   // print_bin(gg, 2);
  //   // printf("\n");
  //   // print_piece(get_piece_smallest_hash(gene_to_piece(gg)), g % 10);
  //   piece_t smallest = get_piece_smallest_hash(gene_to_piece(gg));
  //   int index = 0;
  //   for (int i = 0; i < n_shapes; i++) {
  //     if (smallest == shapes[i])
  //       break;
  //     index++;
  //   }
  //   if (index >= n_shapes) {
  //     shapes[index] = smallest;
  //     n_shapes++;
  //   }

  //   shape_counts[index]++;
  // }

  // int total = 0;
  // for (int i = 0; i < n_shapes; i++) {
  //   printf("%d\n", shape_counts[i]);
  //   print_piece(shapes[i], i % 10);
  //   total += shape_counts[i];
  // }
  // printf("N-shapes: %d, Total: %d\n", n_shapes, total);

  // return 0;
  // const int pop = 50;
  // const int n_genes = 8;
  // gene_t genes[pop * n_genes];

  // for (int i = 0; i < pop * n_genes; i++) {
  //   genes[i] = gene_random(i % 3 + 4);
  //   piece_t p = gene_to_piece(genes[i]);
  //   hash_t h = getSmallestHash(p);
  //   printf("%x\n", h);
  //   print_piece(p, h);
  // }

  // hash_t hashes[pop * n_genes];
  // genome_to_hashes(genes, hashes, pop, n_genes);

  // for (int i = 0; i < pop; i++) {

  //   hash_t *hs = &hashes[i * n_genes];
  //   int r = 0, g = 0, b = 0;
  //   // for (int j = 0; j < n_genes; j++) {
  //   //   r += hs[j] & 0xF0;
  //   //   g += (hs[j] >> 8) & 0xF0;
  //   //   b += (hs[j] >> 16) & 0xF0;
  //   // }

  //   for (int j = 0; j < n_genes; j++) {

  //     char buffer[64];

  //     hsl2rgb((hs[j]) / (float)0xffffffff, 1, 0.6, &r, &g, &b);

  //     hash_t h = hashes[i * n_genes + j];
  //     snprintf(buffer, 64, "%08x ", h);

  //     print_rgb(buffer, r, g, b);
  //     // print_color(buffer, h % 16);
  //   }
  //   printf("\n");
  // }

  // // for (int i = 0; i < sizeof(pieces) / sizeof(pieces[0]); i++) {
  // //   piece_t p = pieces[i];
  // //   for (int j = 0; j < 4; j++) {
  // //     p = piece_sft_down(piece_rotate(p));
  // //     print_piece(p, i % 10);

  // //     printf("HASH: %lx\n", getSmallestHash(p));
  // //   }
  // // }
  // // for (int i = 0; i < sizeof(pieces) / sizeof(pieces[0]); i++) {
  // //   piece_t p = pieces[i];
  // // }
}
