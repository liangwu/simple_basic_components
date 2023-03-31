#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <queue.h>

#define DEBUG

#define PRINT(str)	\
#if DEBUG			\
	do{				\
		PRINT("%s", str);	\
	} while(0);		\
#endif

typedef struct task_entity{
	struct task_entity *next;
	struct task_entity *priv;
	void (*handler)(void* arg);
	void *arg;			// 传入参数
	void *data;			// 传出参数
}task_entity_t;

typedef struct thread_entity{
	struct thread_entity *next;
	struct thread_entity *priv;
	pthread_t thread_id;
	__uint8_t shutdown;
	struct thread_pool *pool;
}thread_entity_t;

typedef struct thread_pool{
	struct queue *taskq;  	// 任务队列
	struct queue *threadq;	// 工作队列
	pthread_cond_t cond;			// 关于任务量的条件变量
	pthread_mutex_t mutex;			// 用于条件变量的互斥锁
	__uint8_t shutdown;
}thread_pool_t;

static void* thread_task_cycyle(void *arg);


static int is_empty_taskq(thread_pool_t *pool){
	return queue_is_empty(pool->taskq);
}

/*
pool:		线程池
func:		任务函数
*/
int add_task(thread_pool_t *pool, void (*func)(void *arg), void* arg) {
	task_entity_t *pTask = (task_entity_t*)malloc(sizeof(task_entity_t));
	if (pTask==NULL){
		perror("malloc");
		return -1;
	}
	memset(pTask, 0, sizeof(task_entity_t));
	pTask->handler = func;
	pTask->arg = arg;
	push_taskq(pool, pTask);			// 将任务压入线程池所管理的任务队列
	// 发送关于任务加入的信号
	pthread_mutex_lock(&pool->mutex);
	pthread_cond_signal(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);
	PRINT("task signal\n");
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
	queue_push(pool->task, entity);
}

/*
取出任务
*/
static task_entity_t* pop_taskq(thread_pool_t *pool){
	if(is_empty_taskq(pool)){ return NULL; }
	struct task_entity *task = queue_pop(pool->taskq);
	PRINT("pop task\n");
	return task;
}

static int is_empty_threadq(thread_pool_t *pool){
	return queue_is_empty(pool);
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
	PRINT("add thread\n");
	return 0;
}

int del_thread(thread_entity_t **pThread){
	free(*pThread);
	*pThread = NULL;
}

static void push_threadq(thread_pool_t *pool, thread_entity_t *entity){
	PRINT("push_threadq\n");
	queue_push(pool->threadq, entity);
}

static thread_pool_t* pop_thread(thread_pool_t* pool){
	return (thread_pool_t*)queue_pop(pool->threadq);
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
		task = pop_taskq(thread->pool);
		pthread_mutex_unlock(&thread->pool->mutex);

		if(task!=NULL) task->handler(task->arg);	// 执行任务
		del_task(&task);	// 释放任务
		PRINT("free task\n");
	}
	return NULL;
}


/*
*	@func:	线程池初始化
*	@pool:	thread pool
*	@num:	number of threads
*/
int thread_pool_setup(thread_pool_t **pool, size_t num){
	*pool  = (thread_pool_t *)malloc(sizeof(thread_pool_t));
	if(*pool == NULL){
		perror("thread_pool_setup, malloc failed:");
		return -1;
	}
	memset(*pool, 0, sizeof(thread_pool_t));
	queue_create(&pool->taskq);			// 初始化队列
	queue_create(&pool->threadq);
	pthread_create(pool->mutex);
	pthread_cond_create(pool->cond);

	if (num > 0) {		// 添加线程
		int i;
		for (i=0; i < num; i++) {
			add_thread(pool);
		}
	} else {
		add_thread(pool);
	}

	return 0;
}

/*
*	@功能：销毁线程池
*/
void thread_pool_destroy(thread_pool_t *pool) {
	queue_destroy(&pool->taskq);
	queue_destroy(&pool->threadq);
	pthread_mutex_destroy(&pool->mutex);
	pthread_cond_destroy(&pool->cond);
	free(*pool);
}


#ifdef DEBUG

void task_func1(void *arg){
	int n=0;
	while(n++<5){
		sleep(1);
		PRINT("I am task_func1\n");
	}
}

void task_func2(void *arg){
	int n=0;
	while(n++<5){
		sleep(1);
		PRINT("I am task_func2\n");
	}
}

int main(int argc, char **argv){
	int thread_size = 1;
	PRINT("argc: %d\n", argc);
	if(argc==2){
		thread_size = atoi(argv[1]);
	}
	thread_pool_t *pool;
	thread_pool_setup(&pool, thread_size);
	add_task(pool, task_func1, NULL);
	add_task(pool, task_func2, NULL);
	// 线程同步
	sleep(20);
	return 0;
}

#endif