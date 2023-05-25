/*
 * @Author: 凉屋 liangwu.lxy@foxmail.com
 * @Description:
 */


#include "ArrayLockFreeQueue.hpp"

#ifdef __GNUC__
	#define CAS(ptr, oldval, newval) __sync_bool_compare_and_swap(ptr, oldval, newval)
	#define AtomicSub(ptr, cnt)	__sync_fetch_and_sub(ptr, cnt)
	#define AtomicAdd(ptr, cnt) __sync_fetch_and_add(ptr, cnt)
#endif



template<typename T, QUEUE_INT Q_SIZE>
inline QUEUE_INT ArrayLockFreeQueue<T, Q_SIZE>::calIndex(QUEUE_INT index) {
	return (index + Q_SIZE) % Q_SIZE;
}

template<typename T, QUEUE_INT Q_SIZE>
ArrayLockFreeQueue<T, Q_SIZE>::ArrayLockFreeQueue() :
	m_count(0), m_readIndex(0), m_writeIndex(0) {}


template<typename T, QUEUE_INT Q_SIZE>
ArrayLockFreeQueue<T, Q_SIZE>::~ArrayLockFreeQueue() {}


template<typename T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueue<T, Q_SIZE>::push(const T &item) {

	QUEUE_INT curReadIndex, curWriteIndex;
	do {
		curReadIndex = m_readIndex;
		curWriteIndex = m_writeIndex;
		if (calIndex(curReadIndex) == calIndex(curWriteIndex+1)) {
			return false; // 满
		}
	} while (!CAS(&m_writeIndex, curWriteIndex, curWriteIndex+1));

	dataList[calIndex(curWriteIndex)] = item;
	AtomicAdd(&m_count, 1);

	return true;
}


template<typename T, QUEUE_INT Q_SIZE>
bool ArrayLockFreeQueue<T, Q_SIZE>::pop(T &item) {

	QUEUE_INT curReadIndex, curWriteIndex;
	do {
		curReadIndex = m_readIndex;
		curWriteIndex = m_writeIndex;
		if (calIndex(curReadIndex) == calIndex(curWriteIndex)) {
			return false; // empty
		}
	} while (!CAS(&m_readIndex, curReadIndex, (curReadIndex + 1)));

	item = dataList(calIndex(curReadIndex));
	AtomicSub(&m_count, 1);

	return true;
}


template<typename T, QUEUE_INT Q_SIZE>
QUEUE_INT ArrayLockFreeQueue<T, Q_SIZE>::size() const {
	return m_count;
}
