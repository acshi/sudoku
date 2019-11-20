#include "sudoku.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct sudoku_state {
    char board[81];
    uint16_t board_domain[81];
} sudoku_state_t;

// looks for unsolved cells that have a domain of 0
// this indicates an impossible board state
bool is_board_solveable(char *board, uint16_t *board_domain) {
    for (int i = 0; i < 81; i++) {
        if (board[i] != 0) {
            continue;
        }
        if (board_domain[i] == 0) {
            return false;
        }
    }
    return true;
}

bool is_state_solveable(sudoku_state_t *state) {
    return is_board_solveable(state->board, state->board_domain);
}

bool is_goal(gen_search_node_t *node) {
    sudoku_state_t *state = (sudoku_state_t*)node->state;
    for (int i = 0; i < 81; i++) {
        if (state->board[i] == 0) {
            return false;
        }
    }
    return true;
}

void find_decision_cell(char *board, uint16_t *board_domain,
                        int *i, int *number) {
    // use the first with only two items in domain
    // if or the smallest number of items in domain

    int best_i = -1;
    int best_count = -1;
    for (int j = 0; j < 81; j++) {
        if (board[j] != 0) {
            continue;
        }
        int count = count_on_bits(board_domain[j]);
        if (count == 2) {
            *i = j;
            *number = which_on_bit(board_domain[j]);
            return;
        } else if (count > 2 && (best_i == -1 || count < best_count)) {
            best_i = j;
            best_count = count;
        }
    }

    *i = best_i;
    *number = which_on_bit(board_domain[best_i]);
}

void *expand_state(void *state) {
    sudoku_state_t *s = (sudoku_state_t*)state;
    if (!is_state_solveable(s)) {
        print_board(s->board);
        // printf("nah... didn't work...\n");
        return NULL;
    }

    // printf("Expanding...\n");
    // print_board(s->board);
    // printf("\r");

    sudoku_state_t *expansion =
                        (sudoku_state_t*)calloc(1, sizeof(sudoku_state_t));
    memcpy(expansion, s, sizeof(sudoku_state_t));
    return expansion;
}

bool next_new_state(void *expansion, void **new_state, int32_t *new_action) {
    if (!expansion) {
        return false;
    }

    sudoku_state_t *expand = (sudoku_state_t*)expansion;

    sudoku_state_t *state = (sudoku_state_t*)calloc(1, sizeof(sudoku_state_t));
    // might have to attempt several times to find a valid child board
    while (is_state_solveable(expand)) {
        memcpy(state, expand, sizeof(sudoku_state_t));

        int i;
        int number;
        find_decision_cell(state->board, state->board_domain, &i, &number);
        if (i == -1) {
            // completely solved the board in this process!
            free(state);
            free(expand);
            return false;
        }

        check_add(state->board, i, number);
        state->board[i] = number;
        state->board_domain[i] = 1 << number;

        uint64_t needs_reprocessing[2] = { 0, 0 };
        reprocessing_insert(needs_reprocessing, i);

        bool normal = run_constraint_update(
                        state->board, state->board_domain,
                        needs_reprocessing, get_all_sections());

        // for later calls to this function, remove the option just tried
        expand->board_domain[i] &= ~(1 << number);

        if (normal) {
            *new_state = state;
            *new_action = -1;  // unused
            return true;
        }
    }
    free(state);
    free(expand);
    return false;
}

void destroy_state(void *state) {
    free(state);
}

void sudoku_search(char *board, uint16_t *board_domain, bool debug) {
    sudoku_state_t state = { 0 };
    memcpy(state.board, board, sizeof(state.board));
    memcpy(state.board_domain, board_domain, sizeof(state.board_domain));

    general_search_problem_t p = { 0 };
    populate_with_lifo(&p);
    p.allow_cycles = true;
    p.initial_state = &state;
    p.is_goal = is_goal;
    p.expand_state = expand_state;
    p.next_new_state = next_new_state;
    p.destroy_state = destroy_state;
    // p.step_cost = step_cost;

    gen_search_node_t *goal_node = tree_search(&p);
    if (!goal_node) {
        printf("No solution found after expanding %d nodes\n",
               p.expansion_count);
        return;
    }

    if (debug) {
        printf("Found sol. at depth %d after expanding %d nodes\n\n",
                goal_node->depth, p.expansion_count);
    }

    // copy solution
    sudoku_state_t *result_state = (sudoku_state_t*)goal_node->state;
    memcpy(board, result_state->board, sizeof(result_state->board));
    memcpy(board_domain, result_state->board_domain,
                         sizeof(result_state->board_domain));

    general_search_result_destroy(&p, goal_node);
}
