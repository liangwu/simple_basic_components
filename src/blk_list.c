#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_ARRAY_SIZE		1024

#define DEBUG				1

static size_t NODE_SIZE;


struct _block {
	struct _block *next;
    unsigned int nums;			// number of valid
	char array[0];
}__attribute__((packed));


typedef struct blk_list {
	struct blk_list *head;
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
	if (size > 0) NODE_SIZE = size;
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


void* blk_list_find(const blk_list* list, const int key) {

}


#if DEBUG

int main(int argc, char **argv) {

	return 0;
}

#endif