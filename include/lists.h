/* file: lists.h */
/* explanation: Functions describing a generic linked list implementation */
/* Written by Theodoros Chatziioannidis (sdi1600197@di.uoa.gr) */

/* A generic linked list interface */
/* Can be applied to any linked list structure as */
/* long as the pointer to the next element is the first */
/* field of the struct */

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>


#define LISTOF(TYPE) struct {void *next; TYPE data;}
#define LLFOREACH(VAR, LIST)            \
    for(typeof(LIST) VAR = LIST; VAR; VAR = llnth(VAR, 1))


/* type: mapfunc_t 
 * a function type
 * param: `node` -> a list node
 * param: `vargs` -> a va_list with the extra arguments passed to llmap 
 * output: a new list node
 */
typedef void *(*mapfunc_t)(void *node, va_list vargs);

/* type: llfree_f
 * a function type
 * param: `node`
 * frees the resources used by node, destroying it
 */
typedef void (*llfree_f)(void *node);

/* type: llpred
 * a function type
 * param: `nodeptr` -> a list node
 * param: `vargs` -> va_list passed to the predicate by `llsearch`
 * output: true or false, depending on whether the node is a 
 * match for the predicate according to `vargs`
 */
typedef bool (*llpred)(void *nodeptr, va_list vargs);

/* type: llcmpr
 * a function type
 * param: `node_a` -> a list node
 * param: `node_b` -> a list node
 * output: negative if `node_a` < `node_b`, 
 *         positive if `node_b` > `node_a`, 
 *         zero if `node_a` = `node_b`
 */
typedef int (*llcmpr)(void *node_a, void *node_b);

/* function: `llnth`
 * param: `l` -> a list 
 * param: `n` -> an integer 
 * output: if `n` >= 0, return the nth element of `l`
 * output: if `n` < 0, return the nth element from the end of `l`
 */
void *llnth(void *l, int n);

/* function: `lltail` 
 *  param: `l` -> a list
 *  output: the last node of `l`
 */
void *lltail(void *l);

/* function: llpush
 * param: `lp` -> a pointer to a list
 * param: `node` -> a node where NEXT(`node`) is a list
 * makes `node` the new head of `*lp`
 */
void llpush(void *lp, void *node);

/* function: llpushlist 
 * param: `lp` -> a pointer to a list
 * param: `node` -> a node where NEXT(`node`) is a list
 * like `llpush` but if NEXT(`node`) is a list, the whole list is pushed to `lp`
 * appends `*lp` to `node` and points `lp` to the new head of the list
 * (.ie the head of `node`)
 */
void llpushlist(void *lp, void *node);


/* function: llpop 
 *  param: `lp` -> a pointer to a list
 *  output: the node pointed to by `lp`
 *  `lp` now points to NEXT(`node`)
 */
void *llpop(void *lp);

/* function: lllen 
 *  param: `l` -> a list
 *  output: `length` of l
 */
int lllen(void *l);

/* function: llreverse
 * param: `lp` -> input list pointer
 * output: void
 * reverses the list pointed to by `lp`
 * passing a list node to this function reverses the list NEXT(`lp`)
 */
void llreverse(void *lp);

/* function: llmap
 * param: `INs` -> an input list
 * param: `map_func` -> function used to produce the output list
 * output: a list where the nth node is the output of `mapfunc`(nth node of `INs`)
 */
void *llmap(void *INs, mapfunc_t map_func, ...);

/* function: llfree
 * param: `l` -> a list 
 * param: free_node -> a function that frees a single node from `l` 
 * destroys `l`, freeing all the nodes with `free_node`
 */
void llfree(void *l, llfree_f free_data);

/* identifer: llsearch
 * param: `l` -> a list
 * param: `p` -> a predicate
 * param: `...` -> arguments passed to p 
 * output: the first node of `l` for which p(l, ...) returns true */
void *llsearch(void *l, llpred p, ...);

/* function: llsplit
 * param: `in` -> a list 
 * param: `out_a` -> a pointer to a pointer that will point to the head of `A` on exit
 * param: `out_b` -> a pointer to a pointer that will point to the head of `B` on exit
 * param: `n` -> the length of `A`
 * splits `in` to lists `A` and `B`, where `A` is of length `n` and `B`
 * is made of the remaining nodes
 */
void llsplit(void *in, void *out_a, void *out_b, int n);

/* function: llsort_merge
 * param: `list_a` -> a list 
 * param: `list_b` -> a list
 * param: `c` -> a comparison function
 * the merging part of the merge sort algorithm 
 * exposed to the interface for adding nodes to a sorted list 
 * and maintaining sortedness 
 */ 
void *llsort_merge(void *list_a, void *list_b, llcmpr c);

/* function: llsort
 * param: `lp` -> a pointer to a list 
 * param: `c` -> a comparison function
 * sorts `lp` in place using merge sort
 */
void llsort(void *l, llcmpr c);
