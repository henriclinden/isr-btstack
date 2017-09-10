/*
 * Simple queue implementation.
 *
 * Henric Lindén, rt-labs AB
 */
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

queue_t* queue_alloc()
{
    return queue_init(malloc(sizeof(queue_t)));
}

queue_t* queue_init(queue_t* queue)
{
    queue->rear = NULL;
    queue->front = NULL;

    return queue;
}

void queue_free(queue_t* queue)
{
    while (!queue_is_empty(queue)) {
        queue_dequeue(queue);
    }

    free(queue);
}

void queue_enqueue(queue_t* queue, void* item)
{
    if (item) {
        node_t* node = malloc(sizeof(node_t));
        node->item = item;
        node->next = NULL;
        if (queue->front == NULL) {
            queue->front = node;
            queue->rear = node;
        }
        else {
            queue->rear->next = node;
            queue->rear = node;
        }
    }
}

void* queue_dequeue(queue_t* queue)
{
    node_t* node = queue->front;

    if (node) {
        queue->front = queue->front->next;
        if (queue->front == NULL) {
            queue->rear = NULL;
        }
        void* item = node->item;
        free(node);
        return item;
    }

    return NULL;
}

void* queue_peek(queue_t* queue)
{
    return queue->front != NULL ? queue->front->item : NULL;
}

int queue_is_empty(queue_t* queue)
{
	return queue->rear == NULL && queue->front == NULL;
}
