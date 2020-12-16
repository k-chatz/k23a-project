#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/acutest.h"

#ifndef ACUTEST_H
#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

#include "../include/lists.h"

typedef struct list_s list;

struct list_s {
    list *next;
    int data;
};

char *list_to_str(list *Ls) {
    int outlen = 2, outsize = 100;
    char *out = malloc(20 * sizeof(char));
    strcpy(out, "(");
    static char current[50] = {0};

    for (list *L = Ls; L; L = ll_nth(L, 1)) {
        if (ll_nth(L, 1))
            sprintf(current, "%d, ", L->data);
        else
            sprintf(current, "%d", L->data);
        int curr_len = strlen(current);
        if (curr_len + outlen >= outsize) {
            outsize *= 2;
            out = realloc(out, outsize);
        }
        strcat(out, current);
        outlen += curr_len;
    }
    strcat(out, ")");
    return out;
}

void print_list(list *l) {
    char *str = list_to_str(l);
    printf("%s\n", str);
    free(str);
}

int int_sort(list *a, list *b, va_list vargs) {
    /* printf("%d - %d = %d\n", a->data, b->data, a->data-b->data); */
    return a->data - b->data;
}

bool eq_pred(void *node, va_list vargs) {
    int x = va_arg(vargs, int);
    list *n = (list *) node;
    return n->data == x;
}

int eq_pred_node(void *node, va_list vargs) {
    list *x = va_arg(vargs, list *);
    list *n = (list *) node;
    return n->data == x->data;
}

/* helper function to make lists */
list *make_list2_r(int Data[], int N, list Nodes[]) {
    list *out = NULL;
    for (int i = N - 1; i >= 0; i--) {
        /* inserting at the start of the list should reverse the elements */
        /* we expect the list to be (0, 1, 2, 3, .. N) */
        Nodes[i].data = Data[i];
        ll_push(&out, &Nodes[i]);
    }
    return out;
}

list *make_list2(int Data[], int N) {
    static list Nodes[100];
    list *out = NULL;
    for (int i = N - 1; i >= 0; i--) {
        /* inserting at the start of the list should reverse the elements */
        /* we expect the list to be (0, 1, 2, 3, .. N) */
        Nodes[i].data = Data[i];
        ll_push(&out, &Nodes[i]);
    }
    return out;
}

list *make_list(int N) {
    static list some_nodes[100];
    list *out = NULL;
    for (int i = N - 1; i >= 0; i--) {
        /* inserting at the start of the list should reverse the elements */
        /* we expect the list to be (0, 1, 2, 3, .. N) */
        some_nodes[i].data = i;
        ll_push(&out, &some_nodes[i]);
    }
    return out;
}

list *make_list_r(int N, list nodes[]) {
    list *out = NULL;
    for (int i = N - 1; i >= 0; i--) {
        /* inserting at the start of the list should reverse the elements */
        /* we expect the list to be (0, 1, 2, 3, .. N) */
        nodes[i].data = i;
        ll_push(&out, &nodes[i]);
    }
    return out;
}

int ints[10] = {10, 5, 2, -8, 1, 48, 43, 5, 0, 10};

void push_test(void) {
    int N = 5;
    list *Is = NULL;
    list some_nodes[N];
    for (int i = N - 1; i >= 0; i--) {
        /* inserting at the start of the list should reverse the elements */
        /* we expect the list to be (0, 1, 2, 3, .. N) */
        some_nodes[i].data = i;
        ll_push(&Is, &some_nodes[i]);
    }

    /* i-th element of the list should contain i */
    int i = 0;
    for (list *t = Is; t; t = t->next) {
        assert(t->data == i);
        i++;
    }
}

void nth_test(void) {
    list *Is = make_list(5);

    int i = 0;
    for (list *t = Is; t; t = t->next) {
        assert(t == ll_nth(Is, i));
        i++;
    }

    /* ll_nth should be able to handle NULL (empty) lists*/
    assert(ll_nth(NULL, 100) == NULL);
}

void length_test(void) {
    int N = 5;
    list *Is = make_list(N);

    /* length of the list should be N */
    assert(ll_len(Is) == N);
    assert(ll_len(ll_nth(Is, 2)) == N - 2);
    assert(ll_len(NULL) == 0);
}

void pop_test(void) {
    int N = 5;
    list *Is = make_list(N);

    int counter = 0;
    while (Is) {
        for (int i = 0; ll_nth(Is, i); i++) {
            list *temp = ll_nth(Is, i);
            assert(temp->data == i + counter);
        }
        ll_pop(&Is);
        counter++;
    }
    assert(counter == N);
}

void search_test(void) {
    int N = 5;
    list *Is = make_list(N);

    for (int i = 0; i <= N; i++) {
        list *result = ll_search(Is, &eq_pred, i);
        assert((result != NULL) == (i < N));
        if (i < N) {
            assert(result == ll_nth(Is, i));
        }
    }
}

void split_test(void) {
    int N = 9;
    list *Is = make_list(N);
    list *As, *Bs;
    ll_split(&Is, &As, &Bs, N / 2);
    assert(ll_len(As) == N / 2);
    assert(ll_len(Bs) == N - N / 2);

    for (int i = 0; i < N / 2; i++) {
        list *t = ll_nth(As, i);
        assert(t->data == i);
    }

    for (int i = 0; i < N - N / 2; i++) {
        list *t = ll_nth(Bs, i);
        assert(t->data == i + N / 2);
    }
}

void tail_test(void) {
    int N = 5;
    list *As = make_list(N);
    As = ll_tail(As);
    assert(As->data == N - 1);
}

void pushlist_test(void) {
    int N = 5, M = 5;
    list *As, *Bs;
    list Anodes[N];
    list Bnodes[M];
    As = make_list_r(N, Anodes);
    Bs = make_list_r(M, Bnodes);

    ll_pushlist(&As, Bs);

    assert(ll_len(As) == N + M);

    for (int i = 0; i < N + M; i++) {
        list *temp = ll_nth(As, i);

        if (temp)
            assert (temp->data == i - (i >= M ? M : 0));
    }
}

void merge_test(void) {
    list *As, *Bs;
    list Anodes[4], Bnodes[7];
    As = make_list2_r((int[]) {1, 3, 5, 7}, 4, Anodes);
    Bs = make_list2_r((int[]) {0, 2, 4, 6, 8, 9, 10}, 7, Bnodes);

    list *Ms = ll_sort_merge(&As, &Bs, (llcmpr) &int_sort);

    int i = 0;
    for (list *x = Ms; x; x = ll_nth(x, 1)) {
        assert(x->data == i++);
    }
}

void sort_test(void) {
    list *As = make_list2((int[]) {1, 9, 2, 4, 3, 7, 6, 5, 8, 0}, 10);
    ll_sort(&As, (llcmpr) &int_sort);
    for (list *A = As; A; A = ll_nth(A, 1)) {
        list *nextA = ll_nth(A, 1);
        if (nextA) {
            assert(A->data <= nextA->data);
        }
    }
}

/* llfunc_t */
void *inc1(void *node, va_list args) {
    static list nodes[100];
    static int node_i = 0;
    nodes[node_i].data = ((list *) node)->data + 1;
    return &nodes[node_i++];
}

void map_test(void) {
    list *As = make_list(10);
    list *Bs = ll_map(As, inc1);
    list *B = Bs;
    LL_FOREACH(A, As) {
        TEST_CHECK(A->data + 1 == B->data);
        B = ll_nth(B, 1);
    }
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        {"push",     push_test},
        {"nth",      nth_test},
        {"length",   length_test},
        {"pop",      pop_test},
        {"search",   search_test},
        {"split",    split_test},
        {"tail",     tail_test},
        {"pushlist", pushlist_test},
        {"merge",    merge_test},
        {"sort",     sort_test},
        {"map",      map_test},
        {NULL, NULL}};

#ifndef ACUTEST_H

int main(int argc, char *argv[]) {
    int i;
    for (i = 0; test_list_[i].name != NULL; i++) {
        test_list_[i].func();
    }
    return 0;
}

#endif
