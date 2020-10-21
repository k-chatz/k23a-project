#include <stdio.h>
#include <stdlib.h>
#include "lists.h"

typedef
struct {
    void *next;
    int data;
} intList;

intList *new_node(int data) {
    intList *out = malloc(sizeof(intList));
    out->data = data;
    return out;
}

int main(int argc, char *argv[]) {
    int list_elements[] = {1, 2, 3, 4, 5, 6};
    int i;
    intList *L = NULL;
    for (i = 0; i < 6; i++) {
        llpush(&L, new_node(list_elements[i]));
    }

    intList *temp = L;
    printf("(");
    while (temp) {
        printf("%d ", temp->data);
        temp = llnth(temp, 1);
    }
    printf(")\n");

    llfree(temp, free);
    return 0;
}
