#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "ringbuf.h"


#define DEBUG		0
int RINGBUFF_ERR;

ringbuf_t *ringbuff_create(size_t size) {
	ringbuf_t *r = malloc(sizeof(ringbuf_t));
	if (r == NULL) {
		perror("malloc in func:ringbuff_create\n");
		return NULL;
	}
	memset(r, 0, sizeof(ringbuf_t));

    r->buff = malloc(size);
	if (r->buff == NULL) {
		perror("malloc in func:ringbuff_create\n");
		return NULL;
	}
	memset(r->buff, 0, size);
    r->size = size;
    r->head = 0;
    r->tail = size;
    return r;
}

void ringbuff_destroy(ringbuf_t *r) {
	free(r->buff);
    free(r);
	r->buff = NULL;
}


static inline bool is_empty(ringbuf_t *r) {
	return r->head == r->tail;
}

static inline bool is_full(ringbuf_t *r) {
    return r->head == (r->tail+1)%r->size;
}


static inline size_t get_use_size(ringbuf_t *r) {
    return (r->size + r->tail - r->head) % r->size;
}


static inline size_t get_remain(ringbuf_t *r) {
	return r->size - get_use_size(r);
}

static inline void* get_startptr(ringbuf_t *r) {
	return (void*)(r->buff + r->head);
}

int ringbuff_write(ringbuf_t *r, void *data, size_t size) {
	if (size > get_remain(r)) {
		return -1;
	}
    memcpy(get_startptr(r), data, size);
    r->tail = (r->tail + size) % r->size;
    return 0;
}


int ringbuff_read(ringbuf_t *r, size_t r_sz, void *buf, size_t buf_sz) {
	if (r_sz > buf_sz) return -1;
	if (r_sz > get_use_size(r)) {
		RINGBUFF_ERR = ERR_BUFF;
		return -1;
	}

	memcpy(buf, get_startptr(r), r_sz);
	r->head = (r->head + r_sz) % r->size;

	return r_sz;
}


int ringbuffer_readline(ringbuf_t *r, void *buf, size_t buf_sz) {
	if (is_empty(r) || buf_sz == 0) {return 0;}

	size_t r_size = get_use_size(r);
	size_t pos;
	for (pos=0; pos < r_size && pos < buf_sz; pos++) {
		if (r->buff[r->head + pos] == '\n') break;
	}

	int n = ringbuff_read(r, pos+1, buf, buf_sz);

	return n;
}

void ringbuff_clear(ringbuf_t *r) {
	r->tail = r->head;
}




#if DEBUG

int main(int argc, char** argv) {

	ringbuf_t *ringbuff = ringbuff_create(1024);
	if (ringbuff == NULL) {
		printf("ringbuff_create failed\n");
		exit(1);
	}

	char str[] = "hello world\r\n hello\r\n";
	size_t siz = strlen(str);
	ringbuff_write(ringbuff, str, sizeof(str));
	printf("ringbuff_size: %ld bytes, strlen: %ld\n" , get_use_size(ringbuff), siz );
	char buf[60];
	int readn;
	do {
		memset(buf, 0, sizeof(buf));
		readn = ringbuffer_readline(ringbuff, buf, sizeof(buf));
		printf("ringbuff_readline: %s\n", buf);
	} while(readn > 0);

	char str1[] = "shanghai\n";
	ringbuff_write(ringbuff, str1, sizeof(str1));
	ringbuff_clear(ringbuff);
	readn = ringbuffer_readline(ringbuff, buf, sizeof(buf));
	printf("ringbuff_readline: %s\n", buf);


	ringbuff_write(ringbuff, str, sizeof(str));
	printf("ringbuff_size: %ld bytes, strlen: %ld\n" , get_use_size(ringbuff), siz );
	do {
		memset(buf, 0, sizeof(buf));
		readn = ringbuffer_readline(ringbuff, buf, sizeof(buf));
		printf("ringbuff_readline: %s\n", buf);
	} while(readn > 0);

	ringbuff_destroy(ringbuff);
	ringbuff = NULL;

	return 0;
}



#endif // !DEBUG