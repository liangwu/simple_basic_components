#ifndef _QUEUE_H_
#define _QUEUE_H_

struct Node
{
	struct Node *prev;
	struct Node *next;
	void *data;
};

struct Queue{
	struct Node *head;
};

#ifdef __cplusplus
extern "C" {
#endif
int queue_create(struct Queue **queue);
int queue_push(struct Queue *queue, void *data);
void* queue_pop(struct Queue *queue);
void queue_destroy(struct Queue **queue);
#ifdef __cplusplus
}
#endif

#endif // !_QUEUE_H_
