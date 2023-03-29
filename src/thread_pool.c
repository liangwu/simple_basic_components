#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define DEBUG

typedef struct task_entity{
	struct task_entity *next;
	struct task_entity *priv;
	void (*handler)(void* arg);
	void *userdata;			// 传出参数
}task_entity_t;

typedef struct thread_entity{
	struct thread_entity *next;
	struct thread_entity *priv;
	pthread_t thread_id;
	__uint8_t shutdown;
	struct thread_pool *pool;
}thread_entity_t;

typedef struct thread_pool{
	struct task_entity *taskq;		// 任务队列
	struct thread_entity *threadq;	// 工作队列
	pthread_cond_t cond;			// 关于任务量的条件变量
	pthread_mutex_t mutex;			// 用于条件变量的互斥锁
	__uint8_t shutdown;
}thread_pool_t;

static void* thread_task_cycyle(void *arg);


static int is_empty_taskq(thread_pool_t *pool){
	task_entity_t *taskq = pool->taskq;
	if(taskq->next==NULL && taskq->priv==NULL) return true;
	return false;
}

/*
pool:		线程池
func:		任务函数
*/
int add_task(thread_pool_t *pool, void (*func)(void *arg)){
	task_entity_t *pTask = (task_entity_t*)malloc(sizeof(task_entity_t));
	if (pTask==NULL){
		perror("malloc");
		return -1;
	}
	memset(pTask, 0, sizeof(task_entity_t));
	pTask->handler = func;
	push_taskq(pool, pTask);		// 将任务压入线程池所管理的任务队列

	// 发送关于任务加入的信号
	pthread_mutex_lock(&pool->mutex);
	pthread_cond_signal(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);
	printf("task signal\n");
	return 0;
}

/*
释放任务空间
*/
int del_task(task_entity_t **task){
	free(*task);
	*task = NULL;
}

static void push_taskq(thread_pool_t *pool, task_entity_t *entity){
	task_entity_t *taskq = pool->taskq;
	if(is_empty_taskq(pool)){
		taskq->priv = entity;
		taskq->next = entity;
		entity->priv = taskq;
		entity->next = taskq;
		return;
	}
	entity->next = taskq;
	entity->priv = taskq->priv;
	taskq->priv = entity;
}

/*
取出任务
*/
static void* pop_taskq(thread_pool_t *pool){
	// task_entity_t *pTaskq = pool->taskq;
	if(is_empty_taskq(pool)){ return NULL; }
	struct task_entity *task = pool->taskq->priv;
	pool->taskq->priv = task->priv;

	printf("pop task\n");
	return task;
}

static int is_empty_threadq(thread_pool_t *pool){
	thread_entity_t *threadq = pool->threadq;
	if(threadq->next== threadq && threadq->priv == threadq) return true;
	return false;
}

/*
向线程池的线程队列，加入线程
*/
int add_thread(thread_pool_t *pool){
	thread_entity_t *pTread = (thread_entity_t*)malloc(sizeof(thread_entity_t));
	if(pTread==NULL){
		perror("malloc");
		return -1;
	}
	memset(pTread, 0, sizeof(thread_entity_t));
	pTread->pool = pool;
	pthread_create(&pTread->thread_id, NULL,thread_task_cycyle, pTread);
	push_threadq(pool, pTread);
	printf("add thread\n");
	return 0;
}

int del_thread(thread_entity_t **pThread){
	free(*pThread);
	*pThread = NULL;
}

static void push_threadq(thread_pool_t *pool, thread_entity_t *entity){
	printf("push_threadq\n");
	if(is_empty_threadq(pool)){
		pool->threadq->priv = entity;
		pool->threadq->next = entity;
		entity->priv = pool->threadq;
		entity->next = NULL;
		return;
	}
	entity->next = NULL;
	entity->priv = pool->threadq->priv;
	pool->threadq->priv = entity;
}

static void* pop_thread(thread_pool_t* pool){
	thread_entity_t *threadq = pool->threadq;
	if(is_empty_threadq(pool)) return NULL;
	thread_entity_t *ret = threadq->priv;
	threadq->priv = ret->priv;
	return ret;
}

// 线程任务循环
static void* thread_task_cycle(void *arg){
	struct thread_entity *thread = (struct thread_entity*)arg;
	struct task_entity *task = NULL;
	while(!thread->shutdown){
		pthread_mutex_lock(&thread->pool->mutex);
		while(is_empty_taskq(thread->pool)){
			pthread_cond_wait(&thread->pool->cond, &thread->pool->mutex);
		}
		// get a task from the taskq in the thread pool
		task = (struct task_entity*)push_taskq(thread->pool);
		pthread_mutex_unlock(&thread->pool->mutex);
		if(task!=NULL) task->handler(task->userdata);	// 执行任务
		del_task(&task);	// 释放任务
		printf("free task\n");
	}
	return NULL;
}


/*
func:	线程池初始化
pool:	thread pool
num:	number of threads
*/
int thread_pool_setup(thread_pool_t **pool, size_t num){
	*pool  = (thread_pool_t *)malloc(sizeof(thread_pool_t));
	if(*pool == NULL){
		perror("thread_pool_setup, malloc failed:");
		return -1;
	}
	memset(*pool, 0, sizeof(thread_pool_t));


	return 0;
}

#ifdef DEBUG

void task_func1(void *arg){
	int n=0;
	while(n++<5){
		sleep(1);
		printf("I am task_func1\n");
	}
}

void task_func2(void *arg){
	int n=0;
	while(n++<5){
		sleep(1);
		printf("I am task_func2\n");
	}
}

int main(int argc, char **argv){
	int thread_size = 1;
	printf("argc: %d\n", argc);
	if(argc==2){
		thread_size = atoi(argv[1]);
	}
	task_entity_t task1, task2;
	thread_pool_t pool;
	memset(&pool, 0, sizeof(pool));
	thread_pool_setup(&pool, thread_size);
	memset(&task1, 0, sizeof(task1));
	memset(&task2, 0, sizeof(task2));
	task_init(&task1, task_func1);
	task_init(&task2, task_func2);
	push_taskq(&pool, &task1);
	push_taskq(&pool, &task2);
	// 线程同步
	sleep(20);
	return 0;
}

#endif