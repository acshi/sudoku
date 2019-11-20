#pragma once

#include "general_search.h"
#include "queues.h"

// from sudoku_search.cpp
void sudoku_search(char *board, uint16_t *board_domain, bool debug);
bool is_board_solveable(char *board, uint16_t *board_domain);

// from sudoku.cpp
bool run_constraint_update(char *board, uint16_t *board_domain,
                           uint64_t *needs_reprocessing, int *all_sections);
int count_on_bits(uint16_t val);
int which_on_bit(uint16_t val);
int *get_all_sections();
void check_add(char *board, int i, int number);
void print_board(char *board);

// From http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
static const unsigned char BitsSetTable256[256] = {
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

// number of 1 bits set
inline int count_on_bits(uint16_t val) {
    return BitsSetTable256[val & 0xff] + BitsSetTable256[(val >> 8) & 0xff];
}

// 1 if bit-1 is on, 2 if it is bit-2, etc...
// returns the lowest if there are more than one
inline int which_on_bit(uint16_t val) {
    return __builtin_ctz(val);
}

// 1 if bit-1 is on, 2 if it is bit-2, etc...
// returns the lowest if there are more than one
inline int which_on_bit64(uint64_t val) {
    return __builtin_ctzl(val);
}

inline void reprocessing_insert(uint64_t *needs_reprocessing, int i) {
    if (i >= 64) {
        // note the constant must be marked long
        // or the operation only yields a 32-bit int
        needs_reprocessing[1] |= 1L << (i - 64);
    } else {
        needs_reprocessing[0] |= 1L << i;
    }
}

inline int reprocessing_pop(uint64_t *needs_reprocessing) {
    int i;
    if (needs_reprocessing[0]) {
        i = which_on_bit64(needs_reprocessing[0]);
        needs_reprocessing[0] &= ~(1L << i);
    } else {
        i = which_on_bit64(needs_reprocessing[1]) + 64;
        needs_reprocessing[1] &= ~(1L << (i - 64));
    }
    // printf("popping %d\n", i);
    return i;
}

inline bool reprocessing_contains(uint64_t *needs_reprocessing, int i) {
    if (i >= 64) {
        return needs_reprocessing[1] & (1L << (i - 64));
    } else {
        return needs_reprocessing[0] & (1L << i);
    }
}
