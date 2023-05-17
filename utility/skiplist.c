#include "skiplist.h"
#include <stdlib.h>
#include <time.h>


static skpnode_t *skpnode_create(uint64_t ts, _skpnode_cb func, int level) {

	skpnode_t *node = calloc(1, sizeof(skpnode_t) + level* sizeof(struct _skplist_level));
	if (!node) return NULL;

	node->timestamp = ts;
	node->func = func;

	return node;
}



static void skpnode_free(skpnode_t *node) {

}


skplist_t *skplist_create() {

	skplist_t *list = calloc(1, sizeof(skplist_t));
	if (!list) return NULL;

	list->head = skpnode_create(0, NULL, SKIPLIST_LEVEL_MAX);
	list->length = 0;
	list->level = 0;

	return list;
}


static int random_leve(void) {
	int level = 1;
	while (rand() & 0xFFFF < SKIPLIST_P * 0xFFFF) level ++;
	return level < SKIPLIST_LEVEL_MAX ? level : SKIPLIST_LEVEL_MAX;
}




int skplist_destroy(skplist_t *list) {
	if (!list) return -1;

	skpnode_free(list->head);
	free(list);
}



int skplist_insert(skplist_t *list, uint64_t ts, _skpnode_cb func) {
	if (!list) return -1;

	int level = random_leve();
	skpnode_t *node = skpnode_create(ts, func, level);
	if (!node) return -1;

	int i;
	skpnode_t *cur = list->head, *next = NULL;
	for (i = node->level - 1; i >= 0; i--) {

		next = cur->level[i].next;	// 下一层
		while (next && next->timestamp < ts) {	// 下一个
			cur = next;
			next = next->level[i].next;
		}

		node->level[i].next = next;
		cur->level[i].next = cur;
	}

	return 0;
}


int skplist_remove(skplist_t *list, skpnode_t *node) {

}