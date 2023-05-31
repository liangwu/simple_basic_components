/**
 * @ Author: LiangWu
 * @ Create Time: 2023-05-29 14:00:00
 * @ Modified by: LiangWu
 * @ Modified time: 2023-05-29 15:58:26
 * @ Description:
 */
#ifndef __ZERO_THREAD_POOL_H__
#define __ZERO_THREAD_POOL_H__

#include <iostream>
#include <functional>
#include <future>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <stdint.h>

#define NOW		1

using namespace std;

constexpr size_t THREAD_NUM_MIN = 5;

class ZeroThreadPool
{
protected:
	struct TaskFunc
	{
		TaskFunc(time_t expire) : _expire(expire) {}
		std::function<void()> _func;
		time_t _expire = 0;
	};

	typedef std::shared_ptr<TaskFunc> TaskFuncPtr;
	std::queue<TaskFuncPtr> 	_taskQ {};
	std::vector<std::thread*> 	_threads;
	std::mutex					_mutex;
	std::condition_variable		_condition;
	size_t 						_threadNum;
	bool						_bTerminate;
	std::atomic<int>			_atomic_int {0};
public:
	ZeroThreadPool(size_t num = 5);
	virtual ~ZeroThreadPool();
	inline size_t get_thread_num() const;
	inline size_t get_task_num() const;
	void stop();
	bool wait_for_all_done(int millsecond = -1);
protected:
	void thread_routine();
	bool get_task(TaskFuncPtr& task);
	bool isTerminate() {return _bTerminate;}
public:
	template<class F, class... Args>
	auto exec(time_t timeout, F&& f, Args&& ... args)  -> std::future<decltype(f(args...))>
	{
		time_t expire = timeout == 0 ? 0 : NOW + timeout;
		//定义返回值类型
		using RetType = decltype(f(args...));	// 推导返回值
		// 封装任务
		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expire);
		fPtr->_func = [task] { (*task)(); };	// 具体执行的函数

		std::unique_lock<std::mutex> lock(_mutex);
		_taskQ.push(fPtr);
		if (_taskQ.empty()) _condition.notify_one();	// 只有任务队列为空的情况再去notify
		lock.unlock();

		return task->get_future();
	}

	template <class F, class... Args>
    auto exec(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        return exec(0, std::forward<F>(f), std::forward<Args>(args)...);
    }
};






#endif // !__ZERO_THREAD_POOL_H__