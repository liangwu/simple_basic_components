#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>


#define BLK_ARRAY_SIZE_MAX		1024

#if (BLK_ARRAY_SIZE_MAX == 1024)
#define RIGHT_MOVE		10
#endif

#define DEBUG				1

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

static size_t DATA_ELEMENT_SIZE;
// static uint16_t BLK_ARRAY_ITEM_SIZE;

struct _block {
	struct _block *next;
    uint16_t valid_nums;			// number of valid
	char shutdown;
	void *array;					// 用户数据数组
};


typedef struct blk_list {
	struct _block *head;
	struct _block *tail;
	uint16_t blk_nums;					// number of blocks in the list
	uint16_t _blk_array_item_size;		// 数据块的用户数据一个单元的大小
} blk_list;



static inline bool _block_isvalid(const struct _block* blk) {
	return (bool)(!blk->shutdown);
}


/*
*	@brief 在某个节点后，插入block
*/
int _block_insert(struct _block* curNode,struct _block) {
	;
}

/*
*    @brief 在某个节点后，删除block
*/
// void _block_delete(struct _block * prev) {
// 	if (prev == NULL) return;
// 	if (prev->next == NULL) return;

// 	struct _block *blk = prev->next;
// 	prev->next = blk->next;

// 	free(blk);
// 	blk = NULL;
// }


static int _block_free_memory(struct _block* blk) {
	if (blk == NULL) return -1;

	if (!blk->shutdown) { return -1;}
    free(blk->array);
    blk->array = NULL;
	blk->shutdown = 1;

	return 0;
}


static int _block_alloc_memory(struct _block *blk, uint16_t item_size) {
	if (blk == NULL) return -1;

	blk->array = malloc(item_size * BLK_ARRAY_SIZE_MAX);
	if (blk->array == NULL) {
		perror("malloc");
		return -1;
	}
	memset(blk->array, 0, item_size * BLK_ARRAY_SIZE_MAX);
	blk->shutdown = 0;

	return 0;
}


/*
*  	@return 数据首地址
*/
static void* _block_find(const struct _block* blk, const uint16_t item_size, const uint16_t key) {
	if (blk == NULL) return NULL;
	if (!_block_isvalid(blk)) return NULL;

	return (void*)(blk->array + key*item_size);
}



struct _block* _block_create(uint16_t item_size, uint8_t shutdown) {
	struct _block *block = malloc(sizeof(struct _block));
	if (block == NULL) return NULL;
	memset(block, 0, sizeof(struct _block));

    block->valid_nums = 0;
	block->shutdown = 0;
	block->next = NULL;
	if (shutdown) {
		block->shutdown = 1;
	} else {
		block->shutdown = 0;
		if (_block_alloc_memory(block, item_size) == -1) {
			free(block);
			return NULL;
		}
	}

	return block;
}



static int _block_destroy(struct _block* blk) {
	if (blk == NULL) return -1;

	blk->shutdown = 1;
	if (_block_free_memory(blk) == -1) return -1;
	free(blk);
	return 0;
}



//----------------------------------------------------------------
//----------blk_list implementation ------------------------------
//----------------------------------------------------------------

blk_list* blk_list_create(uint16_t item_size) {
	if (item_size == 0) return NULL;

	blk_list* list = (blk_list*)malloc(sizeof(blk_list));
	if (!list) return NULL;
	memset(list, 0, sizeof(blk_list));

	list->head = NULL;
	list->tail = NULL;
	list->blk_nums = 0;
	list->_blk_array_item_size = item_size;

	return list;
}


int blk_list_destroy(struct blk_list *list) {
	if (list == NULL) return -1;

	struct _block *cur = list->head;
	struct _block *next = cur->next;
	while (cur != NULL) {
		_block_destroy(cur);
		cur = next;
		next = next->next;
	}

	free(list);
	return 0;
}


/*
* 	@brief 在链表尾添加 struct _block 数量为 count
*/
int blk_list_add_block(blk_list *list, int count) {
	if (list == NULL || count < 0) return -1;
	if (count == 0) return 0;

	const uint16_t item_size = list->_blk_array_item_size;
	if (list->tail == NULL) {
		list->head = list->tail = _block_create(item_size, 0);
		count--;
	}

	while (count-- == 0) {
		list->tail->next = _block_create(item_size, 0);
		list->tail = list->tail->next;
		if (list->tail == NULL) {
			return -1;
		}
	}

	return count;
}


/*
*	@brief 返回根据key找到的元素首地址
*/
void* blk_list_find(const blk_list* list, const size_t key) {
	if (list == NULL) return NULL;

	uint16_t ui16;
	const uint16_t blk_no = key>>RIGHT_MOVE;
	const uint16_t blk_key = key&(BLK_ARRAY_SIZE_MAX-1);
	struct _block *curblk = NULL;
	void* ptr = NULL;

again:
    static int err_cnt = 0;
	if (list->blk_nums <= blk_no) {
		int count = (blk_no + 1) - list->blk_nums;
		blk_list_add_block(list, count);	// 问题
		// goto again;
	}

	// find block
	curblk = list->head;
	for (ui16=1; ui16 <= blk_no; ui16++) {
		curblk = curblk->next;
	}

	if (!_block_isvalid(curblk)) {
		_block_alloc_memory(curblk, list->_blk_array_item_size);
	}

	return _block_find(curblk, list->_blk_array_item_size, blk_key);
}


void blk_list_clear(blk_list *list) {
	;
}



#if DEBUG

int main(int argc, char **argv) {

	return 0;
}

#endif