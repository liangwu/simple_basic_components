#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "queue.h"

#define DEBUG		1



// 初始化循环双端链表
int queue_create(struct Queue **queue){
	struct Node *head = (struct Node*) malloc(sizeof(struct Node));
	*queue = (struct Queue*) malloc(sizeof(struct Queue));
	if(head==NULL){
		perror("init_queue:");
		return -1;
	}
	memset(head, 0, sizeof(struct Node));
	(*queue)->head = head;
	head->prev = head;
	head->next = head;
	return 0;
}

static int queue_is_empty(struct Queue *queue){
	struct Node *head = queue->head;
	if(head->prev == head->next && head->next == head){
		return true;
	}
	return false;
}

// 入队操作
int queue_push(struct Queue *queue, void *data){
	struct Node *_node = (struct Node*)malloc(sizeof(struct Node));
	if(_node == NULL){
		perror("enqueue malloc failed");
		return -1;
	}
	memset(_node, 0, sizeof(struct Node));
	_node->data = data;
	struct Node *head = queue->head;
	if(queue_is_empty(queue)){
		head->prev = _node;
		head->next = _node;
		_node->prev = head;
		_node->next = head;
	}
	_node->prev = head;
	_node->next = head->next;
	(head->next)->prev = _node;
	head->next = _node;
	return 0;
}

// 出队操作
void* queue_pop(struct Queue *queue){
	if(queue_is_empty(queue)){
		return NULL;
	}
	struct Node *head = queue->head;
	struct Node *_node = head->prev;
	head->prev = _node->prev;
	if(head->prev == head->next && head->next != head){
		head->next = head;
	}
	free(_node);
	return _node->data;
}

// 查询队列头部元素
void* queue_first(struct Queue *queue){
	if(queue_is_empty(queue)){return NULL;}
	return queue->head->prev->data;
}


void queue_destroy(struct Queue **queue) {
	while (!queue_is_empty(*queue)){
		queue_pop(*queue);
	}
	free(*queue);
	*queue = NULL;
}



#if DEBUG

int main(int argc, char** argv){
	struct Queue *queue;
	char str1[] = "hello world";
	char str2[] = "Linux";
	queue_create(&queue);
	queue_push(queue,str1);
	queue_push(queue,str2);
	int i;
	for(i=0;i<3;i++){
		char* str = (char*)queue_pop(queue);
		printf("out: %s\n",str);
	}
	// char *pStr1 = (char*)dequeue(queue);
	// char *pStr2 = (char*)dequeue(queue);
	// char *pStr3 = (char*)dequeue(queue);
	queue_destroy(&queue);
	return 0;
}

#endif