/* file: lists.h */
/* explanation: Functions describing a generic linked list implementation */
/* Written by Theodoros Chatziioannidis (sdi1600197@di.uoa.gr) */

#include <stddef.h>
#include <stdarg.h>

typedef void (*mapfunc_t)(void *node, void *newnode, va_list vargs);

typedef void (*llfree_f)(void *node);

void *llnth(void *l, int n);

void *lltail(void *l);

void llpush(void *lp, void *node);

void llpushlist(void *lp, void *node);

void *llpop(void *l);

int lllen(void *l);

void llreverse(void *l);

void *llmap_reverse2(void *l, size_t new_data_size, mapfunc_t map_func, va_list vargs);

void *llmap_reverse(void *l, size_t new_data_size, mapfunc_t map_func, ...);

void *llmap(void *l, size_t new_data_size, mapfunc_t map_func, ...);

void llfree(void *l, llfree_f free_data);

/* A generic linked list interface */
/* Can be applied to any linked list structure as */
/* long as the pointer to the next element is the first */
/* field of the struct */

#include <stdarg.h>

/* predicate type for search */
/* nodeptr is a pointer to a node */
/* vargs are the variadic arguments passed to llsearch() */
/* it is now the predicate's responsibility to va_end the arguments */
/* should return non-zero on succes */
/* zero otherwise  */
typedef int (*list_predicate)(void *nodeptr, va_list vargs);
/* comparator type for sort */
/* node_a and node_b are nodes of the list */
/* vargs are the same as for predicates */
/* it is not the responsiblity of the comparator to va_end the list */
/* returns: */
/* zero => node_a = node_b */
/* positive => node_a > node_b */
/* negative => node_a < node_b */
typedef int (*list_cmpr)(void *node_a, void *node_b);

void *llsearch(void *l, list_predicate p, ...);

void llsplit(void *in, void *out_a, void *out_b, int n);

void *llsort_merge(void *list_a, void *list_b, list_cmpr c);

void llsort(void *l, list_cmpr c);

#define LISTOF(TYPE) struct {void *next; TYPE data;}
#define LLFOREACH(VAR, LIST)            \
    for(typeof(LIST) VAR = LIST; VAR; VAR = llnth(VAR, 1))
