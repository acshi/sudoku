#pragma once

#include <vector>
#include <functional>

using std::vector;
using std::greater;

template <class T, class Compare = greater<T> >
class binary_heap {
    vector<T> v;
    void percolate_up(int i);
    void percolate_down(int i);
    int min_child_i(int i);
 public:
    binary_heap();
    bool empty();
    void push(const T &el);
    void pop();
    const T &top();
};

template <class T, class Compare>
binary_heap<T, Compare>::binary_heap() {
    this->v.push_back(T());
}

template <class T, class Compare>
bool binary_heap<T, Compare>::empty() {
    return this->v.size() <= 1;
}

template <class T, class Compare>
void binary_heap<T, Compare>::percolate_up(int start_i) {
    int i = start_i;
    while (i > 1) {
        int parent_i = i / 2;
        // printf("Comparing at %d %d\n", i, parent_i);
        if (Compare()(this->v[parent_i], this->v[i])) {
            // printf("less: %d %d\n", this->v[i], this->v[parent_i]);
            const T el = this->v[i];
            this->v[i] = this->v[parent_i];
            this->v[parent_i] = el;
            // printf("now: %d %d\n", this->v[i], this->v[parent_i]);
        }
        i = parent_i;
    }
}

template <class T, class Compare>
int binary_heap<T, Compare>::min_child_i(int i) {
    if (i * 2 + 1 >= this->v.size()) {
        return i * 2;
    }
    if (Compare()(this->v[i * 2 + 1], this->v[i * 2])) {
        return i * 2;
    } else {
        return i * 2 + 1;
    }
}

template <class T, class Compare>
void binary_heap<T, Compare>::percolate_down(int start_i) {
    int i = start_i;
    while (i * 2 < this->v.size()) {
        int child_i = min_child_i(i);
        if (Compare()(this->v[i], this->v[child_i])) {
            const T el = this->v[i];
            this->v[i] = this->v[child_i];
            this->v[child_i] = el;
        }
        i = child_i;
    }
}

template <class T, class Compare>
void binary_heap<T, Compare>::push(const T &el) {
    this->v.push_back(el);
    this->percolate_up(this->v.size() - 1);
}

template <class T, class Compare>
void binary_heap<T, Compare>::pop() {
    this->v[1] = this->v.back();
    this->v.pop_back();
    this->percolate_down(1);
}

template <class T, class Compare>
const T &binary_heap<T, Compare>::top() {
    if (this->empty()) {
        fprintf(stderr, "Fatal Error: Called `top` on empty binary_heap\n");
        exit(1);
    }
    return v[1];
}
