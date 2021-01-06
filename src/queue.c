#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

enum {
    HT_ENTRY_FLAGS_CLEAR = 0,
    HT_ENTRY_FLAGS_OCCUPIED = 0b1,
    HT_ENTRY_FLAGS_DELETED = 0b10
};


typedef struct queue_s {
	int front;
	int rear;
	int counter;
    int buf_sz;
    int type_sz;
	char array[];
} queue_t;

typedef struct entry_s {
    u_int8_t flags;
    char data[];
} entry_t;

void QCreate(Queue * q, int buf_sz, int type_sz) {
	assert(*q == NULL);
    *q = (queue_t *)malloc(sizeof(queue_t) + buf_sz * (sizeof(entry_t) + type_sz));
    assert(*q != NULL);    
    (*q)->buf_sz = buf_sz;
    (*q)->type_sz = type_sz;
    (*q)->counter = 0;
	(*q)->front = 0;
	(*q)->rear = 0;
    memset((*q)->array, 0, (*q)->buf_sz * (*q)->type_sz);	
}

void QDestroy(Queue *q) {
	free(*q);
    *q = NULL;
}

int QEmpty(Queue q) {
	return (q->counter == 0);
}

int QSize(Queue q) {
	return q->counter;
}

int QFull(Queue q) {
	return (q->counter == q->buf_sz);
}

bool Qenqueue(Queue q, void *item) {
	if (QFull(q))
		return false;
	else
	{
		q->counter++;
        entry_t entry = q->array[q->rear];
        entry.flags = HT_ENTRY_FLAGS_OCCUPIED;
		memcpy(&(entry.data) ,item, q->type_sz);
	}
	return true;
}

bool Qdequeue(Queue q, void *item) {
	if (QEmpty(q))
		return false;
	else
	{
		q->counter--;
		entry_t entry = q->array[q->front];
        entry.flags = HT_ENTRY_FLAGS_CLEAR;
        memcpy(item, &entry.data, q->type_sz);
		q->front = ( q->front + 1 )% q->buf_sz;
	}
	return true;
}

void inspectQbyOrder(Queue q) {
	int current = q->front;
	if (!QEmpty(q))
	{
		do
		{
			CustWriteValue(stdout, &(q->array[current]) );
			printf("-");
			current = (current + 1) % q->buf_sz;
		} while (current != q->rear);
		printf("\n");
	}
}

void inspectQbyArray(Queue q) {
	int i;
	for (i = 0; i < q->buf_sz; i++){
		CustWriteValue(stdout, &(q->array[i]) );
		printf("-");
	}
	printf("\n");
}