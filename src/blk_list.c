#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_ARRAY_SIZE		1024

#define DEBUG				1

static size_t DATA_ELEMENT_SIZE;


struct _block {
	struct _block *next;
    unsigned int nums;			// number of valid
	char data[0];
}__attribute__((packed));


typedef struct blk_list {
	struct blk_list *head;
	struct blk_list *tail;
	unsigned int nums;			// number of blocks in the list
	int shutdown;
} blk_list;

/*
*	@brief 在某个节点后，插入block
*/
struct _block* _block_insert(struct _block* prev, size_t size) {
	if (size == 0) return NULL;

	struct _block *block = malloc(sizeof(struct _block) + size);
	if (block == NULL) {
		perror("malloc");
		return NULL;
	}
	memset(block, 0, sizeof(struct _block));

	if (prev != NULL) {
		block->next = prev->next;
		prev->next = block;
	} else {
		block->next = NULL;
	}
	block->nums = 0;

	return block;
}

/*
*    @brief 在某个节点后，删除block
*/
void _block_delete(struct _block * prev) {
	if (prev == NULL) return;
	if (prev->next == NULL) return;

	struct _block *blk = prev->next;
	prev->next = blk->next;

	free(blk);
	blk = NULL;
}

//----------------------------------------------------------------

int blk_list_init(struct blk_list *list, size_t size) {
	if (list == NULL) return -1;
	if (size > 0) DATA_ELEMENT_SIZE = size;
	else return -1;

	list->head = _block_insert(NULL, size);
	list->nums = 1;
	list->shutdown = 0;

	return 0;
}

int blk_list_destroy(struct blk_list *list) {
	if (list == NULL) return -1;

	int i16;
	for (i16 = 0; i16 < list->nums; i16++) {
        _block_delete(list->head);
    }

	free(list);
	return 0;
}


/*
*	@brief 返回根据key找到的元素首地址
*/
void* blk_list_find(blk_list* list, const int key) {
	if (list == NULL) return NULL;
	if (key < 0) return NULL;

	size_t page = key/MAX_ARRAY_SIZE;
	size_t idx = key&(MAX_ARRAY_SIZE-1);
	struct _block *curblk = NULL;
	void* ptr = NULL;

again:
    static int err_cnt = 0;
	if (page >= list->nums) {
		while (page >= list->nums) {
			curblk = _block_insert(curblk, DATA_ELEMENT_SIZE);
			if (curblk == NULL) {
				printf("insert failed in %s\n", __func__);
				err_cnt++;
				if (err_cnt <= 10) goto again;
				else {err_cnt=0; return NULL;}
			}
			list->nums++;
		}
	} else {
		int i16;
		curblk = list->head;
		for (i16 = 0; i16 < page; i16++) {
			curblk = curblk->next;
		}
	}

	err_cnt = 0;
	curblk->nums++;
	return &(curblk->data[idx]);
}


void blk_list_clear(blk_list *list) {
	;
}



#if DEBUG

int main(int argc, char **argv) {

	return 0;
}

#endif