#ifndef __MMPOOL_H__
#define __MMPOOL_H__

#include <unistd.h>

struct mp_large_s {
	struct mp_large_s *next;
	void *alloc;
};

struct mp_node_s {

	unsigned char *last;	// 本节点的结束地址 | 下个可分配空间的起始地址
	unsigned char *end;		// 内存块的结束地址

	struct mp_node_s *next;
	size_t failed;
};

struct mp_pool_s {

	size_t max;		// 分配空间的大小

	struct mp_node_s *current;
	struct mp_large_s *large;

	struct mp_node_s head[0];		// 空闲空间首地址

};


extern struct mp_pool_s *mp_create_pool(size_t size);
extern void mp_destory_pool(struct mp_pool_s *pool);
extern void *mp_alloc(struct mp_pool_s *pool, size_t size);
extern void *mp_nalloc(struct mp_pool_s *pool, size_t size);
extern void *mp_calloc(struct mp_pool_s *pool, size_t size);
extern void mp_free(struct mp_pool_s *pool, void *p);

#endif // !__MMPOOL_H