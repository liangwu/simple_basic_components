/*
 * @Author: liangwu liangwu.lxy@foxmail.com
 * @Description:
 */
#include "ArrayLockFreeQueue.hpp"
#include <iostream>
#include <pthread.h>
#include <unistd.h>


ArrayLockFreeQueue<int> Queue;

void *thread_routine(void *arg) {
	int i;
	for (i = 0; i < 10000; i++) {
		Queue.push(i);
	}
	int result;
	for (i = 0; i < 10000; i++) {
		Queue.pop(result);
	}
	std::cout << "thread exit \n";
	return NULL;
}

int main(int argc, char *argv[]) {

	for (int i = 0; i < 5; i++) {
		pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, NULL);
	}

	sleep(10);
	std::cout << "Queue size: " << Queue.size() << std::endl;

	return 0;
}