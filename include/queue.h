#ifndef _QUEUE_H_
#define _QUEUE_H_

struct Node
{
	struct Node *prev;
	struct Node *next;
	void *data;
};

struct queue{
	struct Node *head;
};

#ifdef __cplusplus
extern "C" {
#endif

int queue_create(struct queue **queue);
int queue_is_empty(struct queue *queue);
int queue_push(struct queue *queue, void *data);
void* queue_pop(struct queue *queue);

/*
*	功能：销毁队列，并清空里面的所有节点
*/
void queue_destroy(struct queue **queue);

#ifdef __cplusplus
};
#endif

#endif // !_QUEUE_H_
