#include "sudoku.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#define BITS_1_9_SET ((1 << 10) - 1) & (~(uint16_t)1)

double seconds() {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now)) {
        fprintf(stderr, "Retrieving system time failed.\n");
        exit(1);
    }
    return now.tv_sec + (double)now.tv_nsec * 1e-9;
}

void read_board(char *board) {
    FILE *f = fopen("suinput.csv", "r");

    int val_on = 0;
    int line_on = 0;
    while (val_on < 81) {
        int c = fgetc(f);
        if (c == EOF) {
            fprintf(stderr, "Unexpected EOF in suinput.csv "
                            "after reading %d values\n", val_on);
            exit(1);
        }
        if (c == ',' || c == '\r') {
            continue;
        }
        if (c == '\n') {
            line_on++;
            continue;
        }
        if (c < '0' || c > '9') {
            fprintf(stderr, "Unexpected character '%c' on line %d\n",
                            c, line_on);
            exit(1);
        }
        board[val_on++] = c - '0';
    }

    fclose(f);
}

void print_board(char *board) {
    FILE *f = fopen("suoutput.csv", "w");

    for (int i = 0; i < 9; i++) {
        char *row = &board[i * 9];
        for (int j = 0; j < 8; j++) {
            printf("%d,", row[j]);
            fprintf(f, "%d,", row[j]);
        }
        printf("%d\n", row[8]);
        fprintf(f, "%d\n", row[8]);
    }
    printf("\n");
    fprintf(f, "\n");

    fclose(f);
}

bool check_add_fails(char *board, int i, int number) {
    // check row, col, and square to see if this add makes sense
    // consider neighbors in same row, column, and 3x3
    int row = i / 9;
    int col = i % 9;

    int j = row * 9;
    for (int col_j = 0; col_j < 9; col_j++, j++) {
        if (j != i && board[j] == number) {
            return true;
        }
    }

    j = col;
    for (int row_j = 0; row_j < 9; row_j++, j += 9) {
        if (j != i && board[j] == number) {
            return true;
        }
    }

    int row_three = (row / 3) * 3;
    int col_three = (col / 3) * 3;
    j = row_three * 9 + col_three;
    for (int row_j = 0; row_j < 3; row_j++, j += 9) {
        for (int col_j = 0; col_j < 3; col_j++) {
            if (j != i && board[j] == number) {
                return true;
            }
        }
    }

    return false;
}

void check_add(char *board, int i, int number) {
    if (check_add_fails(board, i, number)) {
        printf("Error! Tried to make invalid board by "
               "adding %d at row %d col %d\n", number, i / 9, i % 9);
        print_board(board);
        exit(1);
    }
    // printf("Added %d at row %d col %d\n", number, i / 9, i % 9);
}

// For any cells that becomes known, reduce the domain of all cells in the same
// column, row, and 3x3 square so that they cannot be this known value.
// returns true if ran normally, false if incountered an inconsistent board
bool run_ac3(char *board, uint16_t *board_domain,
             uint64_t *needs_reprocessing) {
    while (needs_reprocessing[0] || needs_reprocessing[1]) {
        int i = reprocessing_pop(needs_reprocessing);

        uint16_t mask_i = board_domain[i];
        int row = i / 9;
        int col = i % 9;

        int neighbors[27] = { 0 };
        int neighbor_i = 0;
        // consider neighbors in same row, column, and 3x3
        int j = row * 9;
        for (int col_j = 0; col_j < 9; col_j++, j++) {
            neighbors[neighbor_i++] = j;
        }

        j = col;
        for (int row_j = 0; row_j < 9; row_j++, j += 9) {
            neighbors[neighbor_i++] = j;
        }

        int row_three = (row / 3) * 3;
        int col_three = (col / 3) * 3;
        j = row_three * 9 + col_three;
        for (int row_j = 0; row_j < 3; row_j++, j += 9) {
            for (int col_j = 0; col_j < 3; col_j++) {
                neighbors[neighbor_i++] = j + col_j;
            }
        }

        // process all neighbors
        for (int n_i = 0; n_i < 27; n_i++) {
            j = neighbors[n_i];
            if (j == i || board[j] != 0) {
                continue;
            }
            board_domain[j] &= ~mask_i;
            if (count_on_bits(board_domain[j]) == 1) {
                int number = which_on_bit(board_domain[j]);
                if (check_add_fails(board, j, number)) {
                    // inconsistent -- we're done
                    // printf("Ending ac3 early with inconsistent board\n");
                    return false;
                }
                // printf("Added %d at row %d col %d\n", number, i / 9, i % 9);
                board[j] = number;
                reprocessing_insert(needs_reprocessing, j);
            }
        }
    }
    return true;
}

// check for spots that are the only one that can take a given value
// So if a cell has domain [2, 3, 4, 5, 6], but no other cell has 6
// in the domain, this cell must be 6!
// returns true if ran normally, false if incountered an inconsistent board
bool run_only_viable_for_section(char *board, uint16_t *board_domain,
                                 uint64_t *needs_reprocessing, int *section) {
    uint16_t has_any_set_bits = 0;
    uint16_t has_multiple_set_bits = 0;
    for (int section_i = 0; section_i < 9; section_i++) {
        int i = section[section_i];
        if (board[i] == 0) {
            has_multiple_set_bits |= has_any_set_bits & board_domain[i];
            has_any_set_bits |= board_domain[i];
        }
    }
    uint16_t has_single_set_bit = has_any_set_bits & ~has_multiple_set_bits;
    while (has_single_set_bit) {
        int num = which_on_bit(has_single_set_bit);
        has_single_set_bit &= ~(1 << num);  // clear bit

        // find the appropriate cell to place value into
        int num_mask = 1 << num;
        for (int section_i = 0; section_i < 9; section_i++) {
            int i = section[section_i];
            if (board[i] == 0 && (num_mask & board_domain[i])) {
                check_add(board, i, num);
                board[i] = num;
                board_domain[i] = 1 << board[i];
                if (!reprocessing_contains(needs_reprocessing, i)) {
                    reprocessing_insert(needs_reprocessing, i);
                    bool normal = run_ac3(board, board_domain,
                                          needs_reprocessing);
                    if (!normal) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool run_only_viable(char *board, uint16_t *board_domain,
                     uint64_t *needs_reprocessing, int *all_sections) {
    int *section = all_sections;
    for (int section_i = 0; section_i < 27; section_i++) {
        bool normal = run_only_viable_for_section(board, board_domain,
                                                  needs_reprocessing, section);
        if (!normal) {
            return false;
        }
        section += 9;
    }
    return true;
}

// look for a set of n cells with the same n numbers in the domain
// if found, the other cells in the section can not contain these numbers
// for example, three cells have domains [2, 3, 4], [2, 3], [2, 3]
// we find exactly two cells with [2, 3], meaning that the others should
// have this removed from their domains. So the first cell's domain is [4].
// returns true if it made any change to the board_domain
bool run_n_domain_reduction_for_section(
        char *board, uint16_t *board_domain,
        uint64_t *needs_reprocessing, int *section) {
    bool made_change = false;

    // which elements of the section still to look at, bitmask
    uint16_t needs_examination = (1 << 9) - 1;
    for (int section_i = 0; section_i < 9; section_i++) {
        int i = section[section_i];
        if (board[i] != 0) {
            needs_examination &= ~(1 << section_i);  // clear bit
        }
    }

    while (needs_examination) {
        int section_i = which_on_bit(needs_examination);
        int domain = board_domain[section[section_i]];
        int domains_found = 1 << section_i;
        for (int section_j = section_i + 1; section_j < 9; section_j++) {
            if (!(needs_examination & (1 << section_j))) {
                continue;
            }
            if (board_domain[section[section_j]] == domain) {
                domains_found |= 1 << section_j;
            }
        }
        // all these won't need to be examined again
        needs_examination &= ~domains_found;

        int domain_count = count_on_bits(domains_found);
        if (count_on_bits(domain) != domain_count ||
            domain_count == 9 || domain_count == 1) {
            continue;
        }

        // printf("Found %d sections (%d) with domain %d\n",
        //         domain_count, domains_found, domain);

        // reduce domains of other cells
        for (int section_i = 0; section_i < 9; section_i++) {
            int i = section[section_i];
            if (board[i] != 0 || board_domain[i] == domain) {
                continue;
            }
            int new_board_domain = board_domain[i] & ~domain;
            if (new_board_domain == board_domain[i]) {
                continue;
            }
            board_domain[i] = new_board_domain;

            made_change = true;
        }
    }

    return made_change;
}

// returns true if any changes are made to board_domain
bool run_n_domain_reduction(char *board, uint16_t *board_domain,
                     uint64_t *needs_reprocessing, int *all_sections) {
    bool made_change = false;

    int *section = all_sections;
    for (int section_i = 0; section_i < 27; section_i++) {
        bool change = run_n_domain_reduction_for_section(
                        board, board_domain, needs_reprocessing, section);
        made_change = made_change || change;
        section += 9;
    }

    return made_change;
}

int *get_all_sections() {
    static bool needs_init = true;
    static int all_sections[27*9];

    if (!needs_init) {
        return all_sections;
    }
    needs_init = false;

    int *section = all_sections;

    // check for spots that are the only one that can take a given value
    // by row
    for (int row = 0; row < 9; row++) {
        int i = row * 9;
        for (int col = 0; col < 9; col++, i++) {
            section[col] = i;
        }
        section += 9;
    }

    for (int col = 0; col < 9; col++) {
        int i = col;
        for (int row = 0; row < 9; row++, i += 9) {
            section[row] = i;
        }
        section += 9;
    }

    for (int row_three = 0; row_three < 9; row_three += 3) {
        for (int col_three = 0; col_three < 9; col_three += 3) {
            int i = row_three * 9 + col_three;
            int section_i = 0;
            for (int row = 0; row < 3; row++, i += 9) {
                for (int col = 0; col < 3; col++) {
                    section[section_i++] = i + col;
                }
            }
            section += 9;
        }
    }

    return all_sections;
}

// returns true if board is valid, false if inconsistent
bool run_constraint_update(char *board, uint16_t *board_domain,
                           uint64_t *needs_reprocessing, int *all_sections) {
    bool normal = run_ac3(board, board_domain, needs_reprocessing);
    if (!normal) {
        return false;
    }
    do {
        normal = run_only_viable(board, board_domain,
                                 needs_reprocessing, all_sections);
        if (!normal) {
            return false;
        }
    } while (run_n_domain_reduction(
                    board, board_domain, needs_reprocessing, all_sections));
    return is_board_solveable(board, board_domain);
}

void solve(char *board, bool debug) {
    // we will use a bitmask to represent the domain of each cell
    // bit-1 is "1", bit-2 is "2", and so forth... bit-0 is unused.
    uint16_t board_domain[81];
    for (int i = 0; i < 81; i++) {
        if (board[i] != 0) {
            board_domain[i] = 1 << board[i];
        } else {
            // bits-1 through 9 set.
            board_domain[i] = BITS_1_9_SET;
        }
    }

    // there are 27 different sections which must all have 1-9 once each.
    int *all_sections = get_all_sections();

    // Bitmask. again, bit-0 indicates cell 0, and so forth
    // Split over two 64-bit ints to get at least 81 bits.
    uint64_t needs_reprocessing[2] = { 0, 0 };
    for (int i = 0; i < 81; i++) {
        if (board[i] == 0) {
            continue;
        }
        reprocessing_insert(needs_reprocessing, i);
    }

    bool normal = run_constraint_update(board, board_domain,
                                        needs_reprocessing, all_sections);
    if (!normal) {
        return;
    }
    sudoku_search(board, board_domain, debug);
}

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        exit(1);
    }

    char board[81];
    read_board(board);
    double start_time = seconds();
    int n = 0;
    while (seconds() - start_time < 0.5) {
        char board_copy[81];
        memcpy(board_copy, board, sizeof(board));
        solve(board_copy, false);
        n++;
    }
    double elapsed_time = seconds() - start_time;

    solve(board, true);
    print_board(board);

    printf("solved board %d times in avg of %.6f seconds\n", n, elapsed_time / n);
}
