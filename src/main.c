#include "gene.h"
#include "printing.h"
#include <stdint.h>
#include <stdio.h>

int main(int argc, char **argv) {

  //   piece_t piece = 0x80C0000000000000;
  //   print_piece(piece, 0);

  //   for (int i0 = 0; i0 < 4; i0++) {
  //     print_piece(make_gene_piece(i0, 3), 0);

  //     for (uint64_t i1 = 0; i1 < 8; i1++) {
  //       print_piece(make_gene_piece(i1 << 2 | i0, 4), 1);

  //       for (uint64_t i2 = 0; i2 < 16; i2++) {
  //         print_piece(make_gene_piece(i2 << 5 | i1 << 2 | i0, 5), 2);
  //         for (uint64_t i3 = 0; i3 < 16; i3++) {
  //           print_piece(make_gene_piece(i3 << 9 | i2 << 5 | i1 << 2 | i0, 5),
  //           3); printf("================\n"); for (uint64_t i4 = 0; i4 < 16;
  //           i4++) {

  //             piece_t new =
  //                 make_gene_piece(i4 << 13 | i3 << 9 | i2 << 5 | i1 << 2 |
  //                 i0, 6);

  //             print_piece(new, (i4 + 4) % 10);
  //           }
  //         }
  //       }
  //     }
  //   }

  int seed = 0;
  if (argc == 2) {
    sscanf(argv[1], "%d", &seed);

    printf("%d seed\n", seed);
    srand(seed);
    gene_t g = rand() & 0x1FFFF;

    uint32_t length = rand() % 7 + 1;
    printf("%.0lx \n", g);
    printf("%d\n", length);

    piece_t new = make_gene_piece(g, length);

    print_piece(new, 1);
  } else if (argc == 7) {
    int i3, i4, i5, i6, i7, length;
    sscanf(argv[1], "%d", &i3);
    sscanf(argv[2], "%d", &i4);
    sscanf(argv[3], "%d", &i5);
    sscanf(argv[4], "%d", &i6);
    sscanf(argv[5], "%d", &i7);
    sscanf(argv[6], "%d", &length);

    gene_t g = (i3 & 0x3) | ((i4 & 0x7) << 2) | ((i5 & 0xF) << 5) |
               ((i6 & 0xF) << 9) | ((i7 & 0xF) << 13);

    printf("%.0lx \n", g);
    printf("%d\n", length);

    piece_t new = make_gene_piece(g, length);

    print_piece(new, 0);
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