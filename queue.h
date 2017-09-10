/*
 * Simple queue implementation.
 *
 * Henric Lindén, rt-labs AB
 */

 #ifndef __QUEUE_H
 #define __QUEUE_H

 typedef struct _node_t {
     void* item;
     struct _node_t* next;
 } node_t;

 typedef struct _queue_t {
     node_t* rear;
     node_t* front;
 } queue_t;

 queue_t* queue_alloc(void);
 queue_t* queue_init(queue_t* queue);
 void queue_free(queue_t* queue);
 void queue_enqueue(queue_t* queue, void* item);
 void* queue_dequeue(queue_t* queue);
 void* queue_peek(queue_t* queue);
 int queue_is_empty(queue_t* queue);

 #endif
