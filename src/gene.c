#include "gene.h"
#include "board.h"
#include "config.h"
#include "piece.h"
#include "printing.h"
#include "problem.h"
#include "solver.h"
#include "utils.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// #include <sys/random.h>

extern problem_t problem_types[];

#define GET_BIT(p, x, y) (!!((p << ((y)*8 + (x))) & 0x8000000000000000))

#define SET_BIT(p, x, y) p |= (0x8000000000000000 >> ((y)*8 + (x)))
#define CLEAR_BIT(p, x, y) p &= ~(0x8000000000000000 >> ((y)*8 + (x)))

#define RANDOM_BLOCK_LENGTH 2048
uint16_t RANDOM_BLOCK[RANDOM_BLOCK_LENGTH];

uint16_t random16() {
  static int position = RANDOM_BLOCK_LENGTH + 1;

  if (position >= RANDOM_BLOCK_LENGTH) {
    // getrandom(RANDOM_BLOCK, sizeof(uint16_t) * RANDOM_BLOCK_LENGTH, 0);
    position = 0;
  }

  return RANDOM_BLOCK[position++];
}

static bool is_valid_bit_position(piece_t p, int x, int y) {
  bool valid_r = false;
  bool valid_l = false;
  bool valid_u = false;
  bool valid_d = false;

  if (x > 0) {
    valid_l = GET_BIT(p, x - 1, y);
  }

  if (x < 8) {
    valid_r = GET_BIT(p, x + 1, y);
  }

  if (y < 8) {
    valid_d = GET_BIT(p, x, y + 1);
  }

  if (y > 0) {
    valid_d = GET_BIT(p, x, y - 1);
  }

  return valid_r || valid_l || valid_u || valid_d;
}

piece_t attach_bit(piece_t p, uint32_t pos) {
  uint32_t c_pos = 0;
  enum bit_direction next_direction = DIRECTION_UP;
  int x = 0;
  int y = 0;

  if (piece_order(p) > 7)
    return 0x00;

  p = piece_sft_right(piece_sft_down(piece_origin(p)));

  while (y < 7 && !GET_BIT(p, x + 1, y)) {
    y++;
  }

  while (1) {
    if (is_valid_bit_position(p, x, y)) {
      c_pos++;
      if (c_pos > pos) {
        break;
      }
    }

    switch (next_direction) {
    case DIRECTION_RIGHT: // >
      if (!GET_BIT(p, x + 1, y + 1)) {
        next_direction = DIRECTION_DOWN;
        x += 1;
        y += 1;
      } else if (!GET_BIT(p, x + 1, y)) {
        x += 1;
      } else if (!GET_BIT(p, x + 1, y - 1)) {
        y -= 1;
        x += 1;
      } else if (!GET_BIT(p, x, y - 1)) {
        next_direction = DIRECTION_UP;
        y -= 1;
      } else if (!GET_BIT(p, x - 1, y - 1)) {
        next_direction = DIRECTION_UP;
        y -= 1;
        x -= 1;
      } else if (!GET_BIT(p, x - 1, y)) {
        next_direction = DIRECTION_RIGHT;
        x -= 1;
      } else {
        printf("Invalid shape\n");
        return 0;
      }
      break;
    case DIRECTION_DOWN: // V
      if (!GET_BIT(p, x - 1, y + 1)) {
        next_direction = DIRECTION_LEFT;
        x -= 1;
        y += 1;
      } else if (!GET_BIT(p, x, y + 1)) {
        y += 1;
      } else if (!GET_BIT(p, x + 1, y + 1)) {
        y += 1;
        x += 1;
      } else if (!GET_BIT(p, x + 1, y)) {
        next_direction = DIRECTION_RIGHT;
        x += 1;
      } else if (!GET_BIT(p, x + 1, y - 1)) {
        next_direction = DIRECTION_RIGHT;
        y -= 1;
        x += 1;
      } else if (!GET_BIT(p, x, y - 1)) {
        next_direction = DIRECTION_UP;
        y -= 1;
      } else {
        printf("Invalid shape\n");
        return 0;
      }
      break;
    case DIRECTION_LEFT: // <
      if (!GET_BIT(p, x - 1, y - 1)) {
        next_direction = DIRECTION_UP;
        x -= 1;
        y -= 1;
      } else if (!GET_BIT(p, x - 1, y)) {
        x -= 1;
      } else if (!GET_BIT(p, x - 1, y + 1)) {
        y += 1;
        x -= 1;
      } else if (!GET_BIT(p, x, y + 1)) {
        next_direction = DIRECTION_DOWN;
        y += 1;
      } else if (!GET_BIT(p, x + 1, y + 1)) {
        next_direction = DIRECTION_DOWN;
        y += 1;
        x += 1;
      } else if (!GET_BIT(p, x + 1, y)) {
        next_direction = DIRECTION_RIGHT;
        x += 1;
      } else {
        printf("Invalid shape\n");
        return 0;
      }
      break;
    case DIRECTION_UP: // ^
    default:
      if (!GET_BIT(p, x + 1, y - 1)) {
        next_direction = DIRECTION_RIGHT;
        x += 1;
        y -= 1;
      } else if (!GET_BIT(p, x, y - 1)) {
        y -= 1;
      } else if (!GET_BIT(p, x - 1, y - 1)) {
        y -= 1;
        x -= 1;
      } else if (!GET_BIT(p, x - 1, y)) {
        next_direction = DIRECTION_LEFT;
        x -= 1;
      } else if (!GET_BIT(p, x - 1, y + 1)) {
        next_direction = DIRECTION_LEFT;
        y += 1;
        x -= 1;
      } else if (!GET_BIT(p, x, y + 1)) {
        next_direction = DIRECTION_DOWN;
        y += 1;
      } else {
        printf("Invalid shape\n");
        return 0;
      }
      break;
    }
  }

  SET_BIT(p, x, y);

  return piece_origin(p);
}

/* Encoding of Pieces into gene

  bits[1:0] - 2bits Code for Base 3-block piece
            X       X
            X X   X X   X X X

  bits[4:2] - Next 3bits code for location of next block along the piece

            X 0
            X X

  bits[8:5] - 4bits coding for block 5 position
  bits[12:9]  - 4bits codig for block 6 position
  bits[16:13]   - 4bits coding for block 7 position

*/

piece_t gene_to_piece(gene_t gene) {
  piece_t p = 0x00;
  uint32_t length = ((gene >> 14) & 0x3) + 3;
  bool flip = !!((gene >> 13) & 0x1);

  if (length >= 7) {
    return 0xFFFFFFFFFFFFFFFF;
  }
  // First 2 bits (3 pieces)
  if ((gene & 0x03) == 2) {
    p = 0x80C0000000000000;
  } else if ((gene & 0x03) == 3) {
    p = 0x40C0000000000000;
  } else {
    p = 0xE000000000000000;
  }

  // Next 3 bits (4 pieces)
  if (length >= 4) {
    p = attach_bit(p, (gene >> (2)) & 0x07);
  }

  // Next 4 bit sets
  if (length >= 5) {
    for (int i = 0; i < length - 4; i++) {
      p = attach_bit(p, (gene >> (5 + i * 4)) & 0x0F);
    }
  }

  if (flip)
    return piece_origin(piece_flip(p));
  else
    return p;
}

void print_gene(gene_t gene) {
  for (int i = 15; i >= 0; i--) {
    // printf("%d  ", i);
  }
  for (int i = 15; i >= 0; i--) {
    printf("%c", (gene >> i) & 0x01 ? '1' : '0');
  }
  printf("\n");
}

gene_t gene_make(uint32_t *positions, uint32_t len, bool flip) {
  gene_t g = 0;
  uint8_t gene_length = 0;

  if (len >= 3 && len < 7) {
    gene_length = len - 3;
  } else {
    gene_length = 0;
  }

  switch (gene_length) {
  case 3:
    g |= (positions[3] & 0xF) << 9;
  case 2:
    g |= (positions[2] & 0xF) << 5;
  case 1:
    g |= (positions[1] & 0x7) << 2;
  case 0:
    g |= (positions[0] & 0x3);
  default:
    break;
  }

  g |= gene_length << 14;

  if (flip)
    g |= 0x1 << 13;

  return g;
}

gene_t gene_random(uint32_t len) {
  gene_t g = random16();

  return gene_set_length(g, len);
}

gene_t gene_mutate(gene_t gene, uint32_t iter) {
  gene_t mask = 0x3FFF;

  for (int i = 0; i < iter; i++) {
    mask &= random16();
  }
  print_gene(mask);
  return gene ^ mask;
}

gene_t gene_cross(gene_t gene1, gene_t gene2) {
  gene_t mask = random16();

  return (gene1 & mask) | (gene2 & ~mask);
}

gene_t gene_set_length(gene_t gene, uint32_t len) {

  uint32_t gene_length = 0;

  if (len >= 3 && len < 7) {
    gene_length = len - 3;
  } else {
    gene_length = 0;
  }
  gene &= 0x3FFF;
  gene |= gene_length << 14;
  return gene;
}