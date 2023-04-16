#ifndef __RINGBUF_H__
#define __RINGBUF_H__


#define ERR_BUFF	0	// 缓冲区空间不足

extern int RINGBUFF_ERR;		// 错误号

typedef struct _ringbuff {
	char *buff;
	size_t size;
	size_t head;
	size_t tail;
} ringbuf_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern ringbuf_t *ringbuff_create(size_t size);
extern void ringbuff_destroy(ringbuf_t *r);
extern int ringbuff_write(ringbuf_t *r, void *data, size_t size);
extern int ringbuff_read(ringbuf_t *r, size_t r_sz, void *buf, size_t buf_sz);
extern int ringbuffer_readline(ringbuf_t *r, void *buf, size_t buf_sz);
extern void ringbuff_clear(ringbuf_t *r);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !__RINGBUF_H__
