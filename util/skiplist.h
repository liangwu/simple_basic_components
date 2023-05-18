#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

#include <stdint.h>

#define SKIPLIST_P				0.25
#define SKIPLIST_LEVEL_MAX		32



typedef void (*_skpnode_cb)(void *arg);

typedef struct _skp_node {
	uint64_t timestamp;
	_skpnode_cb func;
	struct _skplist_level {
		struct _skp_node *next;
	} level[0];
} skpnode_t;


typedef struct skplist {
	struct _skp_node *head;
	int length;
	int level;		// x
} skplist_t;



extern skplist_t *skplist_create();
extern int skplist_destroy(skplist_t *list);
extern int skplist_insert(skplist_t *list, uint64_t ts, _skpnode_cb func);
extern int skplist_remove(skplist_t *list, skpnode_t *node);
extern int skplist_find(skplist_t *list, uint64_t ts, skpnode_t *node);
extern skpnode_t *skplist_get_min(skplist_t *list);

#ifdef __SKIPLIST_PRINT
extern void skplist_print(skplist_t *list);
#endif // !__SKIPLIST_PRINT


#endif // !__SKIPLIST_H