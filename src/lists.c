/* file: lists.c */
/* explanation: Functions describing a generic linked list implementation */
/* Written by Theodoros Chatziioannidis (sdi1600197@di.uoa.gr) */

#include <stdlib.h>

#include "lists.h"

#define NEXT(x) (*((void **)x))

void *llnth(void *l, int n) {
    if (n >= 0) {
        while (n-- > 0 && l)
            l = NEXT(l);
        return l;
    } else {
        /* n is < 0, we will count n nodes from the last */
        void *m = llnth(l, -n);
        while (NEXT(m)) {
            l = NEXT(l);
            m = NEXT(m);
        }
        return l;
    }
}

void *lltail(void *l) {
    if (!l) /* l is NULL */
        return NULL;
    while (NEXT(l))
        l = NEXT(l);
    return l;
}

/* pushes `node` to the list pointed to by `lp` */
void llpush(void *lp, void *node) {
    if (!node)
        /* no list to push; return */
        return;
    if (!lp)
        /* lp points to NULL; return */
        return;
    NEXT(node) = NEXT(lp);
    NEXT(lp) = node;
}

/* pushes the list starting from `node` to */
/* the list pointed to by `lp` */
void llpushlist(void *lp, void *node) {
    if (!node)
        /* no list to push; return */
        return;
    if (!lp)
        /* lp points to NULL; return */
        return;
    NEXT(lltail(node)) = NEXT(lp);
    NEXT(lp) = node;
}

void *llpop(void *l) {
    if (!l)
        /* l is empty; return */
        return NULL;
    void *first = NEXT(l);
    NEXT(l) = NEXT(NEXT(l));
    NEXT(first) = NULL;
    return first;
}

int lllen(void *l) {
    int len = 0;
    while (l) {
        len++;
        l = NEXT(l);
    }
    return len;
}

void llreverse(void *l) {
    void *r = NULL; /* initialize output to NULL */
    while (NEXT(l)) {
        void *p = llpop(l);
        llpush(&r, p); /* pop and push from l to r */
    }
    NEXT(l) = r; /* return the results */
}

void llsplit(void *in, void *out_a, void *out_b, int n) {
    if (n == 0) {
        NEXT(out_b) = NEXT(in);
        NEXT(out_a) = NULL;
        return;
    }
    NEXT(out_a) = NEXT(in);
    void *temp = llnth(NEXT(in), n - 1);
    NEXT(out_b) = NEXT(temp);
    NEXT(temp) = NULL;
    return;
}

void *llsort_merge(void *list_a, void *list_b, list_cmpr c) {
    if (!NEXT(list_b))
        return NEXT(list_a);
    if (!NEXT(list_a))
        return NEXT(list_b);

    int cmpr = c(NEXT(list_a), NEXT(list_b));
    void **rest, **h;
    h = (cmpr < 0 ? llpop(list_a) : llpop(list_b));
    rest = llsort_merge(list_a, list_b, c);
    llpush(&rest, h);
    return rest;
}

void llsort(void *l, list_cmpr c) {
    void *a, *b;
    unsigned int len = lllen(NEXT(l));
    if (len <= 1)
        return;
    llsplit(l, &a, &b, len / 2);
    llsort(&a, c);
    llsort(&b, c);
    NEXT(l) = llsort_merge(&a, &b, c);
    return;
}

void *llsearch(void *l, list_predicate p, ...) {
    /* returns the nth element of the list */
    va_list vargs, for_func;
    va_start(vargs, p);
    while (l) {
        va_copy(for_func, vargs);
        int success = p(l, for_func);
        va_end(for_func);
        if (success)
            break;
        l = (void *)NEXT((void **)l);
    }
    va_end(vargs);
    return l;
}

/* from here on we will assume that the lists are stored on the heap */
struct lltemp {
    void *next;
    char data[];
};

void *llmap_reverse2(void *l, size_t new_node_size, mapfunc_t map_func,
                     va_list vargs) {
    struct lltemp *out_l = NULL;
    va_list temp;
    while (l) {
        void **new_node = malloc(new_node_size);
        *new_node = NULL;
        va_copy(temp, vargs);
        map_func(l, new_node, temp);
        va_end(temp);
        llpush(&out_l, new_node);
        l = NEXT(l);
    }
    return out_l;
}

void *llmap_reverse(void *l, size_t new_data_size, mapfunc_t map_func, ...) {
    va_list vargs;
    va_start(vargs, map_func);
    void *out = llmap_reverse2(l, new_data_size, map_func, vargs);
    va_end(vargs);
    return out;
}

void *llmap(void *l, size_t new_data_size, mapfunc_t map_func, ...) {
    va_list vargs;
    va_start(vargs, map_func);
    void *outlist = llmap_reverse2(l, new_data_size, map_func, vargs);
    llreverse(&outlist);
    va_end(vargs);
    return outlist;
}

void llfree(void *l, llfree_f free_node) {
    if (free_node) {
        /* free using free_node */
        while (l) {
            free_node(llpop(&l));
        }
    }
}
