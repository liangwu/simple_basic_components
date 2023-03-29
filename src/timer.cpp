#include <iostream>
#include <functional>
#include <set>
#include <chrono>
#include <memory>
#include <unistd.h>


struct TaskNodeBase {
	uint64_t id;
	time_t timeout;
};

struct TaskNode : public TaskNodeBase {
	using callback = std::function<void(const TaskNode& task_node)>;
	callback func;
};

bool operator < (const TaskNodeBase &lhs, const TaskNodeBase &rhs){
	if(lhs.timeout < rhs.timeout) return true;
	else if(lhs.timeout > rhs.timeout) return false;
	return lhs.id<rhs.id? true: false;
}

class Timer{
private:
	static uint64_t id;
	std::set<TaskNode, std::less<>> task_sets;
private:
	static time_t get_tick();
public:
	TaskNodeBase add_task(time_t, TaskNode::callback);
	bool del_task(const TaskNodeBase& task);
	bool check_task();
	time_t time_to_sleep();
};
uint64_t Timer::id=0;

time_t Timer::get_tick(){
	auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
	auto dt = tp.time_since_epoch();
	return dt.count();
}


TaskNodeBase Timer::add_task(time_t timeout ,TaskNode::callback func){
	TaskNode task;
	task.id = id++;
	task.timeout = timeout+get_tick();
	task.func = func;
	auto ret = task_sets.insert(task);
	if (ret.second) std::cout << "任务插入成功\n";
	else std::cout << "任务插入失败\n";
	return static_cast<TaskNodeBase>(task);
}

bool Timer::del_task(const TaskNodeBase& task){
	auto it = task_sets.find(task);
	if(it != task_sets.end()){
		task_sets.erase(it);
		return true;
	}
	return false;
}

bool Timer::check_task(){
	auto it = task_sets.begin();
	if(it != task_sets.end() && it->timeout <= get_tick()){
		it->func(*it);
		task_sets.erase(it);
		return true;
	}
	return false;
}

time_t Timer::time_to_sleep(){
	auto it = task_sets.begin();
	time_t err = it->timeout - get_tick();
	if(err < 0){return -1;}
	return err;
}


int main(){
	std::unique_ptr<Timer> timer = std::make_unique<Timer>();
	timer->add_task(1000, [&](const TaskNode &task){
		std::cout<<"id:" << task.id << std::endl;
		std::cout << "timeout:" << task.timeout << std::endl;
	});

	timer->add_task(1000, [&](const TaskNode &task){
		std::cout<<"id:" << task.id << std::endl;
		std::cout << "timeout:" << task.timeout << std::endl;
	});

	timer->add_task(10000, [&](const TaskNode &task){
		std::cout<<"id:" << task.id << std::endl;
		std::cout << "timeout:" << task.timeout << std::endl;
	});
	while(1){
		time_t time = timer->time_to_sleep();
		//std::cout << "time_to_sleep:" << time << std::endl;
		sleep(time/1000);
		while(timer->check_task());
	}
	return 0;
}