#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct node{
	void* value;
	struct node* next;
} node;

struct queue {
	/* TODO Phase 1 */
	node* head;
	node* tail;
    size_t size;
};

queue_t queue_create(void)
{
	/* TODO Phase 1 */
	// allocate memory to new queue
	queue_t new_queue = malloc(sizeof(queue_t));
	// failed to allocate memory to new queue
	if (new_queue == NULL) 
		return NULL;
	new_queue -> head = NULL;
	new_queue -> tail = NULL;
    new_queue -> size = 0;
	return new_queue;
}

int queue_destroy(queue_t queue)
{
	/* TODO Phase 1 */
	// queue don't exist or queue is not empty
	if (queue == NULL || queue->head != NULL) {
        return -1;
    }
	queue -> size = 0;
	free(queue);
	// success
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{	/* TODO Phase 1 */

	// check queue and data
	if (queue == NULL || data == NULL)
		return -1;

	
	// create a new node
	node* new_node = malloc(sizeof(node));
	// if memeory allocation for new_node failed
	if (new_node == NULL)
		return -1;


	// set data for this node
	new_node -> value = data;
	new_node -> next = NULL;
	// set tail for node
	if (queue -> tail != NULL)
		queue -> tail -> next = new_node;
	queue -> tail = new_node;
	// initial queue head
	if (queue -> head == NULL)
		queue -> head = new_node;


	// successfully insert the node:
    queue -> size += 1;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	/* TODO Phase 1 */
	// queue is empty or queue is NULL
	if (queue -> head == NULL || queue == NULL)
		return -1;


	// get the head value
	node* head_node = queue->head;
	void* head_value = head_node -> value;
	*data = head_value;
	// data == NULL
	if (data == NULL)
		return -1;

	// let head point to the next node
	queue -> head = queue -> head -> next;


	// handle the case where queue is now empty
	if (queue -> head == NULL) {
		//queue now empty, don't have tail
		queue -> tail = NULL;
	}

	// success
    free(head_node);
    queue -> size -= 1;
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	/* TODO Phase 1 */
    // convert void ptr to a compareable ptr
    int* data_to_find = data;

    // get the head node
    node* current_node = queue -> head;
    // get the value in head node
    int* node_value = current_node -> value;
    // compare two value;
    if (*data_to_find == *node_value) {
        //delete the data in queue;
        queue -> head = queue -> head -> next;
        free(current_node);
        queue -> size -= 1;
        return 0;
    }
    // record a prev node
    node* prev = current_node;
    while(1) {
        // get the next node
        current_node = current_node -> next;

        if (current_node == NULL) {
            //queue reached end;
            break;
        }
        // get the node value
        node_value = current_node -> value;
        // compare two value
        if (*data_to_find == *node_value) {
            //delete the data in queue;
            prev -> next = current_node -> next;
            current_node -> next = NULL;
            free(current_node);
            queue -> size -= 1;
            return 0;
        }
        
        prev = current_node;
    }
	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	/* TODO Phase 1 */
    node* current = queue -> head;
    while(1) {
        void* current_value = current -> value;
        current = current -> next;
        if (current == NULL)
            break;
        func(queue, current_value);
    }
	return 0;
}

int queue_length(queue_t queue)
{
	/* TODO Phase 1 */
    if (queue == NULL) {
        return -1;
    }
    
	return queue -> size;
}
