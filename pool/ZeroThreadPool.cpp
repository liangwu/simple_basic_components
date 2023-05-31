#include "ZeroThreadPool.h"

#define DEBUG

ZeroThreadPool::ZeroThreadPool(size_t num) {
	num = num > THREAD_NUM_MIN ? num : THREAD_NUM_MIN;
	_threads.resize(num);
	for (int i=0; i < num; i++) {
		_threads.emplace_back(new std::thread(&ZeroThreadPool::thread_routine, this));
	}
}

ZeroThreadPool::~ZeroThreadPool() {

}

void ZeroThreadPool::thread_routine() {

	while (!isTerminate()) {
		TaskFuncPtr task;
		bool ok = get_task(task);
		if (ok) {
			_atomic_int++;
			if (task->_expire != 0 && task->_expire <= NOW) {
				//超时任务，是否需要处理?
			} else {
				task->_func();
			}
		}
	}

}


bool ZeroThreadPool::get_task(TaskFuncPtr& task) {

	std::unique_lock<std::mutex> lock(_mutex);
	if (_taskQ.empty()) {
		_condition.wait(lock, [this] {return _bTerminate || !_taskQ.empty();});
	}

	if (_bTerminate) return false;

	if (!_taskQ.empty()) {
		task = std::move(_taskQ.front());
		_taskQ.pop();
		return true;
	}

	return false;
}


void ZeroThreadPool::stop() {

	{
		std::lock_guard<std::mutex> lock(_mutex);
		_bTerminate = true;
		_condition.notify_all();	// 促使所有任务结束
	}

	for (size_t i = 0; i < _threads.size(); i++) {
		if (_threads[i]->joinable()) {
			_threads[i]->join();
		}
		delete _threads[i];
		_threads[i] = nullptr;
	}
}


bool ZeroThreadPool::wait_for_all_done(int millsecond) {

	std::unique_lock<std::mutex> lock(_mutex);
	if (_taskQ.empty()) {
		return true;
	}

	if (millsecond < 0) {
		_condition.wait(lock, [this]{ return _taskQ.empty(); });
	} else {
		_condition.wait_for(lock, std::chrono::seconds(millsecond), [this]{ return _taskQ.empty(); });
	}

	return true;
}


inline size_t ZeroThreadPool::get_task_num() const {
	return _taskQ.size();
}

inline size_t ZeroThreadPool::get_thread_num() const {
	return _threads.size();
}


#ifdef DEBUG


int task_a(int a, int b) {
	int c = a + b;
	std::cout << "sum: c " << c << std::endl;
	return c;
}

void task_b(void) {
	std::cout <<"hello world" << std::endl;
}

void task_c(double a) {
	std::cout <<"hello world" << a << std::endl;
}

int main(int argc, char** argv) {

	ZeroThreadPool pool = ZeroThreadPool();
	pool.exec(task_a, 1, 2);
	pool.exec(task_a, 7, 2);
	pool.exec(task_b);
	pool.exec(task_c, 1.5f);
	std::cout << "thread size : " << pool.get_thread_num() << "task size : " << pool.get_task_num() << std::endl;
	pool.wait_for_all_done();

	return 0;
}



#endif // !DEBUG


