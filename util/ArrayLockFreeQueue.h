/*
 * @Author: 凉屋 liangwu.lxy@foxmail.com
 * @Description:
 */
#ifndef __ARRAY_LOCK_FREE_QUEUE_H__
#define __ARRAY_LOCK_FREE_QUEUE_H__

#define ARRAY_LOCK_FREE_Q_DEFAULT_SIZE	(1 << 16)

typedef unsigned long QUEUE_INT;

template<typename T, QUEUE_INT Q_SIZE = ARRAY_LOCK_FREE_Q_DEFAULT_SIZE>
class ArrayLockFreeQueue
{
private:
	T dataList[Q_SIZE] {};
	QUEUE_INT m_count;
	QUEUE_INT m_readIndex;
	QUEUE_INT m_writeIndex;

public:
	ArrayLockFreeQueue();
	virtual ~ArrayLockFreeQueue();
	QUEUE_INT size() const;
	bool push(const T &item);
	bool pop(T &item);
private:
	inline QUEUE_INT calIndex(QUEUE_INT index);
};




#endif // !__ARRAY_LOCK_FREE_QUEUE_H__