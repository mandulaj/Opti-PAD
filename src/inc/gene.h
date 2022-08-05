#pragma once
#include "piece.h"


enum bit_direction {
  DIRECTION_RIGHT,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
  DIRECTION_UP
};


typedef uint64_t gene_t;

piece_t attach_bit(piece_t p, uint32_t pos);


piece_t make_gene_piece(gene_t gene, uint32_t length);