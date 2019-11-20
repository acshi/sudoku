#include "unordered_set.h"
#include <unordered_set>

using std::unordered_set;

typedef unordered_set<uint64_t> set_t;

void *unordered_set_make() {
    return new set_t();
}

void unordered_set_destroy(void *s) {
    delete (set_t*)s;
}

void unordered_set_add(void *s, uint64_t value) {
    set_t *set = (set_t*)s;
    set->insert(value);
}

bool unordered_set_contains(void *s, uint64_t value) {
    set_t *set = (set_t*)s;
    return set->count(value) != 0;
}

void populate_with_unordered_set(general_search_problem_t *p) {
    p->unordered_set_make = unordered_set_make;
    p->unordered_set_destroy = unordered_set_destroy;
    p->unordered_set_add = unordered_set_add;
    p->unordered_set_contains = unordered_set_contains;
}
