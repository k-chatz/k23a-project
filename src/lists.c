/* file: lists.c */
/* explanation: Functions describing a generic linked list implementation */
/* Written by Theodoros Chatziioannidis (sdi1600197@di.uoa.gr) */

#include <stdlib.h>

#include "../include/lists.h"


#define NEXT(x) (*((void **)x))

/* function: `ll_nth`
 * param: `l` -> a list 
 * param: `n` -> an integer 
 * output: if `n` >= 0, return the nth element of `l`
 * output: if `n` < 0, return the nth element from the end of `l`
 */
void *ll_nth(void *l, int n) {
    if (n >= 0) {
        while (n-- > 0 && l)
            l = NEXT(l);
        return l;
    } else {
        int length = ll_len(l);
        /* n is < 0, we will count n nodes from the last */
        return ll_nth(l, length - n);
    }
}

/* function: `ll_tail`
 *  param: `l` -> a list
 *  output: the last node of `l`
 */
void *ll_tail(void *l) {
    if (!l) /* l is NULL */
        return NULL;
    while (NEXT(l))
        l = NEXT(l);
    return l;
}

/* function: ll_push
 * param: `lp` -> a pointer to a list
 * param: `node` -> a node where NEXT(`node`) is a list
 * makes `node` the new head of `*lp`
 */
void ll_push(void *lp, void *node) {
    if (!node)
        /* no list to push; return */
        return;
    if (!lp)
        /* lp points to NULL; return */
        return;
    NEXT(node) = NEXT(lp);
    NEXT(lp) = node;
}

/* function: ll_pushlist  
 * param: `lp` -> a pointer to a list
 * param: `node` -> a node where NEXT(`node`) is a list
 * like `ll_push` but if NEXT(`node`) is a list, the whole list is pushed to `lp`
 * appends `*lp` to `node` and points `lp` to the new head of the list
 * (.ie the head of `node`)
 */
void ll_pushlist(void *lp, void *node) {
    if (!node)
        /* no list to push; return */
        return;
    if (!lp)
        /* lp points to NULL; return */
        return;
    NEXT(ll_tail(node)) = NEXT(lp);
    NEXT(lp) = node;
}

/* function: ll_pop
 *  param: `lp` -> a pointer to a list
 *  output: the node pointed to by `lp`
 *  `lp` now points to NEXT(`node`)
 */
void *ll_pop(void *l) {
    if (!l)
        /* l is empty; return */
        return NULL;
    void *first = NEXT(l);
    NEXT(l) = NEXT(NEXT(l));
    NEXT(first) = NULL;
    return first;
}

/* function: ll_len
 *  param: `l` -> a list
 *  output: `length` of l
 */
int ll_len(void *l) {
    int len = 0;
    while (l) {
        len++;
        l = NEXT(l);
    }
    return len;
}

/* function: ll_reverse
 * param: `lp` -> input list pointer
 * output: void
 * reverses the list pointed to by `lp`
 * passing a list node to this function reverses the list NEXT(`lp`)
 */
void ll_reverse(void *l) {
    void *r = NULL; /* initialize output to NULL */
    while (NEXT(l)) {
        void *p = ll_pop(l);
        ll_push(&r, p); /* pop and push from l to r */
    }
    NEXT(l) = r; /* return the results */
}

/* function: ll_split
 * param: `in` -> a list 
 * param: `out_a` -> a pointer to a pointer that will point to the head of `A` on exit
 * param: `out_b` -> a pointer to a pointer that will point to the head of `B` on exit
 * param: `n` -> the length of `A`
 * splits `in` to lists `A` and `B`, where `A` is of length `n` and `B`
 * is made of the remaining nodes
 */
void ll_split(void *in, void *out_a, void *out_b, int n) {
    if (n == 0) {
        NEXT(out_b) = NEXT(in);
        NEXT(out_a) = NULL;
        return;
    }
    NEXT(out_a) = NEXT(in);
    void *temp = ll_nth(NEXT(in), n - 1);
    NEXT(out_b) = NEXT(temp);
    NEXT(temp) = NULL;
    return;
}

/* function: ll_sort_merge
 * param: `list_a` -> a list 
 * param: `list_b` -> a list
 * param: `c` -> a comparison function
 * the merging part of the merge sort algorithm 
 * exposed to the interface for adding nodes to a sorted list 
 * and maintaining sortedness 
 */
void *ll_sort_merge(void *list_a, void *list_b, llcmpr c) {
    if (!NEXT(list_b))
        return NEXT(list_a);
    if (!NEXT(list_a))
        return NEXT(list_b);

    int cmpr = c(NEXT(list_a), NEXT(list_b));
    void **rest, **h;
    h = (cmpr < 0 ? ll_pop(list_a) : ll_pop(list_b));
    rest = ll_sort_merge(list_a, list_b, c);
    ll_push(&rest, h);
    return rest;
}

/* function: ll_sort
 * param: `lp` -> a pointer to a list 
 * param: `list_cmpr` -> a comparison function
 * sorts `lp` in place using merge sort
 */
void ll_sort(void *lp, llcmpr c) {
    void *a, *b;
    unsigned int len = ll_len(NEXT(lp));
    if (len <= 1)
        return;
    ll_split(lp, &a, &b, len / 2);
    ll_sort(&a, c);
    ll_sort(&b, c);
    NEXT(lp) = ll_sort_merge(&a, &b, c);
    return;
}

/* identifer: ll_search
 * param: `l` -> a list
 * param: `p` -> a predicate
 * param: `...` -> arguments passed to p 
 * output: the first node of `l` for which p(l, ...) returns true */
void *ll_search(void *l, llpred p, ...) {
    /* returns the nth element of the list */
    va_list vargs, for_func;
    va_start(vargs, p);
    while (l) {
        va_copy(for_func, vargs);
        int success = p(l, for_func);
        va_end(for_func);
        if (success)
            break;
        l = (void *) NEXT((void **) l);
    }
    va_end(vargs);
    return l;
}

/* function: ll_map
 * param: `INs` -> an input list
 * param: `map_func` -> function used to produce the output list
 * output: a list where the nth node is the output of `mapfunc`(nth node of `INs`)
 */
void *ll_map(void *INs, mapfunc_t map_func, ...) {
    void *OUTs = NULL, *OUT_tail;
    va_list args;
    if (INs) {
        /* do first element to initialize OUT_tail */
        va_start(args, map_func);
        void *new = map_func(INs, args);
        ll_push(&OUTs, new);
        OUT_tail = new;
    }
    LL_FOREACH(IN, NEXT(INs)) {
        /* do rest */
        va_start(args, map_func);
        void *new = map_func(IN, args);
        ll_push(OUT_tail, new);    /* push AFTER 'OUT_tail' */
        OUT_tail = new;        /* set OUT_tail to new tail */
    }
    return OUTs;
}

/* function: ll_free
 * param: `l` -> a list 
 * param: free_node -> a function that frees a single node from `l` 
 * destroys `l`, freeing all the nodes with `free_node`
 */
void ll_free(void *l, llfree_f free_node) {
    if (free_node) {
        /* free using free_node */
        while (l) {
            free_node(ll_pop(&l));
        }
    }
}

void ll_iterate(void *l, void (*callback)(void *)) {
    for (void *x = l; x; x = ll_nth(x, 1)) {
        callback(x);
    }
}
