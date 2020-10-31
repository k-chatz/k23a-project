/* file: lists.h */
/* explanation: Functions describing a generic linked list implementation */
/* Written by Theodoros Chatziioannidis (sdi1600197@di.uoa.gr) */

/* A generic linked list interface */
/* Can be applied to any linked list structure as */
/* long as the pointer to the next element is the first */
/* field of the struct */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

/*!
  \defgroup lists
  @{
 */

/*! Macro to easily create list types.
@param[in] TYPE the type of data stored in the list
@hideinitializer
*/
#define LISTOF(TYPE)                                                           \
    struct {                                                                   \
        void *next;                                                            \
        TYPE data;                                                             \
    }

/*!
Implements foreach functionality for lists
@param[out] VAR an identifier that references the current node
@param[in] LIST the list
@hideinitializer
read as "for each VAR in LIST"

###example:
\code{.c}

list *Ls = function_that_produces_list(...);

LLFOREACH(L, Ls){
    action(L);
}
\endcode
 */
#define LLFOREACH(VAR, LIST)                                                   \
    for (typeof(LIST) VAR = LIST; VAR; VAR = llnth(VAR, 1))

/* type: mapfunc_t */
/*!
Function type for llmap. Should map a list node to a new one.
@param[in] node a list node
@param[in] vargs a va_list with the extra arguments passed to llmap
@retval out a new list node
 */
typedef void *(*mapfunc_t)(void *node, va_list vargs);

/* type: llfree_f */
/*!
the logical equivalent of calling free(node)
@param[in] node a list node

 */
typedef void (*llfree_f)(void *node);

/* type: llpred */
/*!
@param[in] nodeptr : a list node
@param[in] vargs : va_list passed to the predicate by llsearch
@returns
true or false, depending on whether the node is a
match for the predicate according to `vargs`
*/
typedef bool (*llpred)(void *nodeptr, va_list vargs);

/* type: llcmpr */
/*!
@param[in] node_a : a list node
@param[in] node_b : a list node
@retval c where\n
c > 0 iff. node_a > node_b,\n
c < 0 iff. node_a < node_b and\n
c = 0 iff. node_a = node_b.
 */
typedef int (*llcmpr)(void *node_a, void *node_b);

/* function: `llnth` */
/*!
@param[in] l : a list
@param[in] n : an integer
@returns the nth element of l
*/
void *llnth(void *l, int n);

/* function: `lltail` */
/*!
@param[in] l : a list
@returns last node of l
 */
void *lltail(void *l);

/* function: llpush */
/*!
makes node the new head of the list pointed to by lp
@param[in] lp : a pointer to a list
@param[in] node : a node where NEXT(`node`) is a list
###example:
@code{.c}
list *make_list(int N) {
    static list some_nodes[100];
    list *out = NULL;
    for (int i = N - 1; i >= 0; i--) {
    // inserting at the start of the list should reverse the elements
    // we expect the list to be (0, 1, 2, 3, .. N)
        some_nodes[i].data = i;
        llpush(&out, &some_nodes[i]);
    }
    return out;
}
@endcode
 */
void llpush(void *lp, void *node);

/* function: llpushlist */
/*!
like llpush but if NEXT(node) is a list, the whole list is pushed to lp
connects lltail(node) to *lp and sets *lp to node

@param[in] lp : a pointer to a list
@param[in] node : a node where NEXT(`node`) is a list

###example
@code{.c}
void pushlist_test(void) {
    int N = 5, M = 5;
    list *As, *Bs;
    list Anodes[N];
    list Bnodes[M];
    As = make_list_r(N, Anodes);
    Bs = make_list_r(M, Bnodes);

    llpushlist(&As, Bs);

    assert(lllen(As) == N + M);

    for (int i = 0; i < N + M; i++) {
        list *temp = llnth(As, i);

        if (temp)
            assert(temp->data == i - (i >= M ? M : 0));
    }
}

@endcode
 */
void llpushlist(void *lp, void *node);

/* function: llpop */
/*!
extracts and returns the first node of the list pointed to by lp
@param[in] lp : a pointer to a list
@returns the node pointed to by `lp`

###example
@code{.c}
void pop_test(void) {
    int N = 5;
    list *Is = make_list(N);

    int counter = 0;
    while (Is) {
        for (int i = 0; llnth(Is, i); i++) {
            list *temp = llnth(Is, i);
            assert(temp->data == i + counter);
        }
        llpop(&Is);
        counter++;
    }
    assert(counter == N);
}

@endcode
 */
void *llpop(void *lp);

/* function: lllen */
/*!
@param[in] l : a list
@returns the length of l
 */
int lllen(void *l);

/* function: llreverse */
/*!
reverses the list pointed to by lp in place
@param[in] lp : input list pointer

###example
@code{.c}
// L = (1, 2, 3)
llreverse(&L);
// L = (3, 2, 1)
@endcode
 */
void llreverse(void *lp);

/* function: llmap */
/*!
@param[in] INs : an input list
@param[in] map_func : function used to produce the output list
@retval OUTs a list where the nth node is the output of mapfunc(nth node of INs)
 */
void *llmap(void *INs, mapfunc_t map_func, ...);

/* function: llfree */
/*!
destroys the input list, freeing all the nodes with free_node

@param[in] l : a list
@param[in] free_node : a function that frees a single node from `l`
 */
void llfree(void *l, llfree_f free_data);

/* function: llsearch */
/*!
@param[in] l : a list
@param[in] p : a predicate
@param[in] ... : arguments passed to p
@returns
 the first node of l for which p(l, ...) returns true */
void *llsearch(void *l, llpred p, ...);

/* function: llsplit */
/*!
Splits in to out_a and out_b, giving the first n nodes to out_a and the rest to
b the reverse of llpushlist(&out_b, out_a)

@param[in] in : input list
@param[out] out_a : first output list
@param[out] out_b : second output list
@param[in] n : the length of the first output list

###example
\code{.c}
typedef LISTOF(int) intList;

int main(void){
  intList *Is = NULL;
  for(int i = 10; i > 0; i--){
    intList *new = newNode(i); // creates a node containing i
    llpush(&Is, new);
  }
  // Is = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
  intList *oneToFive, *sixToTen;
  llsplit(&Is, &oneToFive, &sixToTen, 5);
  printf("oneToFive = ");
  printIntList(oneToFive);
  // oneToFive = (1, 2, 3, 4, 5)
  printf("sixToTen = ");
  printIntList(sixToTen);
  // sixToTen = (6, 7, 8, 9, 10)
  return 0;
}
\endcode
 */
void llsplit(void *in, void *out_a, void *out_b, int n);

/* function: llsort_merge */
/*!
the merging part of the merge sort algorithm
exposed to the interface for adding nodes to a sorted list
and maintaining sortedness

@param[in] list_a : a list
@param[in] list_b : a list
@param[in] c : a comparison function

###example
\code{.c}
// file: lists.c
void llsort(void *lp, llcmpr c) {
    void *a, *b;
    unsigned int len = lllen(NEXT(l));
    if (len <= 1)
        return;
    llsplit(lp, &a, &b, len / 2);
    llsort(&a, c);
    llsort(&b, c);
    NEXT(lp) = llsort_merge(&a, &b, c);
    return;
}
\endcode
 */
void *llsort_merge(void *list_a, void *list_b, llcmpr c);

/* function: llsort */
/*!
sorts lp in place using merge sort

@param[in] lp : a pointer to a list
@param[in] c : a comparison function

\code{.c}
void sort_test(void) {
    list *As = make_list2((int[]){1, 9, 2, 4, 3, 7, 6, 5, 8, 0}, 10);
    // As = (1, 9, 2, 4, 3, 7, 6, 5, 8, 0)
    llsort(&As, (llcmpr)&int_sort);
    for (list *A = As; A; A = llnth(A, 1)) {
        list *nextA = llnth(A, 1);
        if (nextA) {
            assert(A->data <= nextA->data);
        }
    }
}

\endcode
 */
void llsort(void *l, llcmpr c);

/*! @} */
