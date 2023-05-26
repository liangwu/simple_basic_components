/*
 * @Author: liangwu liangwu.lxy@foxmail.com
 * @Description:
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include <unistd.h>

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *mutex);
typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *mutex);

pthread_mutex_lock_t pthread_mutex_lock_f;
pthread_mutex_unlock_t pthread_mutex_unlock_f;

pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;

#define MAX	(1 << 10)


struct vertex {

};

struct source_type {

};

struct task_graph {
	struct vertex list[MAX]; 	// <selfid, thid>
	struct source_type locklist[MAX]; // thid = get_threadid_from_mutex
};


int pthread_mutex_lock(pthread_mutex_t *mutex) {
	printf("phtread_mutex_lock selfid: %ld, mutex: %p\n", pthread_self(), mutex);
	pthread_mutex_lock_f(mutex);
	return 0;
}


int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	printf("phtread_mutex_unlock\n");
	pthread_mutex_unlock_f(mutex);
	return 0;
}

static int init_hook() {
	pthread_mutex_lock_f = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	pthread_mutex_unlock_f = dlsym(RTLD_NEXT, "pthread_mutex_unlock");

	return 0;
}


static void* thread_routine_a(void *arg) {
	printf("thread_routin_a enter\n");
	pthread_mutex_lock(&mutex_1);
	sleep(1);
	pthread_mutex_lock(&mutex_2);
	pthread_mutex_lock(&mutex_3);
	pthread_mutex_unlock(&mutex_3);
	pthread_mutex_unlock(&mutex_2);
	pthread_mutex_unlock(&mutex_1);
	return NULL;
}


static void* thread_routine_b(void *arg) {
	printf("thread_routin_b enter\n");
	pthread_mutex_lock(&mutex_2);
	sleep(1);
	pthread_mutex_lock(&mutex_1);
	pthread_mutex_lock(&mutex_3);
	pthread_mutex_unlock(&mutex_3);
	pthread_mutex_unlock(&mutex_1);
	pthread_mutex_unlock(&mutex_2);
	return NULL;
}


static void* thread_routine_c(void *arg) {
	printf("thread_routin_c enter\n");
	pthread_mutex_lock(&mutex_3);
	sleep(1);
	pthread_mutex_lock(&mutex_2);
	pthread_mutex_lock(&mutex_1);
	pthread_mutex_unlock(&mutex_1);
	pthread_mutex_unlock(&mutex_2);
	pthread_mutex_unlock(&mutex_3);
	return NULL;
}


int main(void) {
	init_hook();
	pthread_t pidlist[3];
	pthread_create(&pidlist[0], NULL, thread_routine_a, NULL);
	pthread_create(&pidlist[1], NULL, thread_routine_b, NULL);
	pthread_create(&pidlist[2], NULL, thread_routine_c, NULL);

	sleep(10);
	int i;
	for (i=0; i<3; i++) {
		pthread_join(pidlist[i], NULL);
	}

	return 0;
}