#pragma once
#include "piece.h"


enum bit_direction {
  DIRECTION_RIGHT,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
  DIRECTION_UP
};


typedef uint16_t gene_t;

piece_t attach_bit(piece_t p, uint32_t pos);


piece_t gene_to_piece(gene_t gene);

gene_t gene_make(uint32_t *positions, uint32_t len, bool flip);


void print_gene(gene_t gene);

gene_t gene_random(uint32_t len);

gene_t gene_mutate(gene_t gene, uint32_t iter);
gene_t gene_set_length(gene_t gene, uint32_t len);
gene_t gene_cross(gene_t gene1, gene_t gene2);