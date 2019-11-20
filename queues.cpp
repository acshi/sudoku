#include "queues.h"

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <forward_list>
#include "binary_heap.h"

using std::list;
using std::forward_list;

// put low costs at the top of the priority queue
class Node_Compare {
 public:
    bool operator() (void *a, void *b) {
        return ((gen_search_node_t*)a)->ordering_cost >
                ((gen_search_node_t*)b)->ordering_cost;
    }
};

typedef binary_heap<void*, Node_Compare> p_queue_t;
// typedef priority_queue<void*, deque<void*>, Node_Compare> p_queue_t;

void *priority_queue_make() {
    return new p_queue_t();
}

void priority_queue_destroy(void *q) {
    delete (p_queue_t*)q;
}

void priority_queue_add(void *q, void *value) {
    ((p_queue_t*)q)->push(value);
}

bool priority_queue_is_empty(void *q) {
    return ((p_queue_t*)q)->empty();
}

void *priority_queue_remove_first(void *q) {
    p_queue_t *queue = (p_queue_t*)q;
    if (queue->empty()) {
        return NULL;
    }
    void *top = queue->top();
    queue->pop();
    return top;
}

void populate_with_priority_queue(general_search_problem_t *p) {
    p->queue_make = priority_queue_make;
    p->queue_destroy = priority_queue_destroy;
    p->queue_add = priority_queue_add;
    p->queue_is_empty = priority_queue_is_empty;
    p->queue_remove_first = priority_queue_remove_first;
}

typedef list<void*> fifo_queue_t;

void *fifo_queue_make() {
    return new fifo_queue_t();
}

void fifo_queue_destroy(void *q) {
    delete (fifo_queue_t*)q;
}

void fifo_queue_add(void *q, void *value) {
    ((fifo_queue_t*)q)->push_back(value);
}

bool fifo_queue_is_empty(void *q) {
    return ((fifo_queue_t*)q)->empty();
}

void *fifo_queue_remove_first(void *q) {
    fifo_queue_t *queue = (fifo_queue_t*)q;
    if (queue->empty()) {
        return NULL;
    }
    void *element = queue->front();
    queue->pop_front();
    return element;
}

void populate_with_fifo(general_search_problem_t *p) {
    p->queue_make = fifo_queue_make;
    p->queue_destroy = fifo_queue_destroy;
    p->queue_add = fifo_queue_add;
    p->queue_is_empty = fifo_queue_is_empty;
    p->queue_remove_first = fifo_queue_remove_first;
}

typedef forward_list<void*> lifo_queue_t;

void *lifo_queue_make() {
    return new lifo_queue_t();
}

void lifo_queue_destroy(void *q) {
    delete (lifo_queue_t*)q;
}

void lifo_queue_add(void *q, void *value) {
    ((lifo_queue_t*)q)->push_front(value);
}

bool lifo_queue_is_empty(void *q) {
    return ((lifo_queue_t*)q)->empty();
}

void *lifo_queue_remove_first(void *q) {
    lifo_queue_t *queue = (lifo_queue_t*)q;
    if (queue->empty()) {
        return NULL;
    }
    void *element = queue->front();
    queue->pop_front();
    return element;
}

void populate_with_lifo(general_search_problem_t *p) {
    p->queue_make = lifo_queue_make;
    p->queue_destroy = lifo_queue_destroy;
    p->queue_add = lifo_queue_add;
    p->queue_is_empty = lifo_queue_is_empty;
    p->queue_remove_first = lifo_queue_remove_first;
}
