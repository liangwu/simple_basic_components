#include "skiplist.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


static skpnode_t *skpnode_create(uint64_t ts, _skpnode_cb func, int level) {

	skpnode_t *node = calloc(1, sizeof(skpnode_t) + level* sizeof(struct _skplist_level));
	if (!node) return NULL;

	node->timestamp = ts;
	node->func = func;

	return node;
}


static void default_func(void *arg){}


skplist_t *skplist_create() {

	skplist_t *list = calloc(1, sizeof(skplist_t));
	if (!list) return NULL;

	list->head = skpnode_create(0, default_func, SKIPLIST_LEVEL_MAX);
	list->length = 0;
	list->level = 0;

	return list;
}


static int random_leve(void) {
	int level = 1;
	while ((rand() & 0xFFFF) < SKIPLIST_P * 0xFFFF) level ++;
	return level < SKIPLIST_LEVEL_MAX ? level : SKIPLIST_LEVEL_MAX;
}




int skplist_destroy(skplist_t *list) {
	if (!list) return -1;

	skpnode_t *cur = list->head->level[0].next;
	if (!cur) return 0;
	skpnode_t *next = cur->level[0].next;

	while (next) {
		free(cur);
		cur = next;
		next = next->level[0].next;
	}
	free(cur);
	free(list);

	return 0;
}



int skplist_insert(skplist_t *list, uint64_t ts, _skpnode_cb func) {
	if (!list) return -1;

	int level = random_leve();
	list->level = level > list->level ? level : list->level;
	skpnode_t *node = skpnode_create(ts, func, level);
	if (!node) return -1;

	int i;
	skpnode_t *cur = list->head;
	for (i = list->level - 1; i >= 0; i--) {
		while (cur->level[i].next && cur->level[i].next->timestamp < ts) {
			cur = cur->level[i].next;
		}

		if (i < level) {
			node->level[i].next = cur->level[i].next;
			cur->level[i].next = node;
		}
	}

	list->length += 1;

	return 0;
}


int skplist_remove(skplist_t *list, skpnode_t *node) {
	if (!list || !node) return -1;

	int i, ts = node->timestamp;
	skpnode_t *cur = list->head;
	for (i = list->level - 1; i >= 0; i--) {
		while (cur->level[i].next && cur->level[i].next->timestamp < ts) {
			cur = cur->level[i].next;
		}

		if (cur->level[i].next->timestamp == ts) {
			cur->level[i].next = node->level[i].next;
		}
	}
	free(node);
	list->length -= 1;

	return 0;
}


int skplist_find(skplist_t *list, uint64_t ts, skpnode_t *node) {
	if (!list) return -1;
	if (list->length == 0) return -1;

	int i;
	skpnode_t *cur = list->head;
	for (i = list->level - 1; i >= 0; i--) {
		while (cur->level[i].next && cur->level[i].next->timestamp < ts) {
			cur = cur->level[i].next;
		}

		if (cur->level[i].next->timestamp == ts) {
			node = cur->level[i].next;
			return 0;
		}
	}

	return -1;
}



void skplist_print(skplist_t *list) {
	if (!list) return;

	int i;
	skpnode_t *cur = list->head;
	for (i = list->level-1; i >= 0; i--) {
		cur = list->head->level[i].next;
		while (cur) {
			printf(" %ld ->", cur->timestamp);
			cur = cur->level[i].next;
		}
		printf("\n");
	}
}