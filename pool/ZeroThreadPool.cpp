#include "ZeroThreadPool.h"



void ZeroThreadPool::thread_routine() {

	while (!isTerminate()) {
		TaskFuncPtr task;
		bool ok = getTask(task);
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


bool ZeroThreadPool::getTask(TaskFuncPtr& task) {

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
		_condition.notify_all();
	}

	for (size_t i = 0; i < _threadQ.size(); i++) {
		if (_threadQ[i]->joinable()) {
			_threadQ[i]->join();
		}
		delete _threadQ[i];
		_threadQ[i] = nullptr;
	}
}



bool ZeroThreadPool::start() {

	std::lock_guard<std::mutex> lock(_mutex);

	// for (size_t i = 0; i < _threadNum; i++) {
	// 	_threadQ.emplace_back(new std::thread())
	// }
}




